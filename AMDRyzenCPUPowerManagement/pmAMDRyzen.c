//
//  pmAMDRyzen.c
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 3/27/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#include "pmAMDRyzen.h"



x86_lcpu_t *pmRyzen_cpunum_to_lcpu[XNU_MAX_CPU];

void *pmRyzen_io_service_handle;

pmProcessor_t pmRyzen_cpus[XNU_MAX_CPU];

uint32_t pmRyzen_num_phys;
uint32_t pmRyzen_num_logi;
uint64_t pmRyzen_timeout_TSC = 0;

uint64_t pmRyzen_exit_idle_c;
uint64_t pmRyzen_exit_idle_ipi_c;
uint64_t pmRyzen_exit_idle_false_c;

void(*pmRyzen_pmUnRegister)(pmDispatch_t*) = 0;
void(*pmRyzen_cpu_NMI)(int) = 0;
void(*pmRyzen_NMI_enabled)(boolean_t) = 0;
void(*pmRyzen_cpu_IPI)(int) = 0;

//kern_return_t       (*pmCPUStateInit)(void);
//void                (*cstateInit)(void);
//uint64_t            (*MachineIdle)(uint64_t maxIdleDuration);
//uint64_t            (*GetDeadline)(x86_lcpu_t *lcpu);
//uint64_t            (*SetDeadline)(x86_lcpu_t *lcpu, uint64_t);
//void                (*Deadline)(x86_lcpu_t *lcpu);
//boolean_t           (*exitIdle)(x86_lcpu_t *lcpu);
//void                (*markCPURunning)(x86_lcpu_t *lcpu);
//int                 (*pmCPUControl)(uint32_t cmd, void *datap);
//void                (*pmCPUHalt)(void);
//uint64_t            (*getMaxSnoop)(void);
//void                (*setMaxBusDelay)(uint64_t time);
//uint64_t            (*getMaxBusDelay)(void);
//void                (*setMaxIntDelay)(uint64_t time);
//uint64_t            (*getMaxIntDelay)(void);
//void                (*pmCPUSafeMode)(x86_lcpu_t *lcpu, uint32_t flags);
//void                (*pmTimerStateSave)(void);
//void                (*pmTimerStateRestore)(void);
//kern_return_t       (*exitHalt)(x86_lcpu_t *lcpu);
//kern_return_t       (*exitHaltToOff)(x86_lcpu_t *lcpu);
//void                (*markAllCPUsOff)(void);
//void                (*pmSetRunCount)(uint32_t count);
//boolean_t           (*pmIsCPUUnAvailable)(x86_lcpu_t *lcpu);
//int                 (*pmChooseCPU)(int startCPU, int endCPU, int preferredCPU);
//int                 (*pmIPIHandler)(void *state);
//void                (*pmThreadTellUrgency)(int urgency, uint64_t rt_period, uint64_t rt_deadline);
//void                (*pmActiveRTThreads)(boolean_t active);
//boolean_t           (*pmInterruptPrewakeApplicable)(void);
//void                (*pmThreadGoingOffCore)(thread_t old_thread, boolean_t transfer_load,
//uint64_t last_dispatch, boolean_t thread_runnable);

pmDispatch_t pmRyzen_cpuFuncs = {
    .pmCPUStateInit = 0,
    .cstateInit = 0,
    .MachineIdle = &pmRyzen_machine_idle,
    .GetDeadline = 0,
    .SetDeadline = 0,
    .Deadline = 0,
    .exitIdle = &pmRyzen_exit_idle,
    .markCPURunning = 0,
    .pmCPUControl = 0,
    .pmCPUHalt = 0,
    .getMaxSnoop = 0,
    .setMaxBusDelay = 0,
    .getMaxBusDelay = 0,
    .setMaxIntDelay = 0,
    .getMaxIntDelay = 0,
    .pmCPUSafeMode = 0,
    .pmTimerStateSave = 0,
    .pmTimerStateRestore = 0,
    .exitHalt = 0,
    .exitHaltToOff = 0,
    .markAllCPUsOff = 0,
    .pmSetRunCount = 0,
    .pmIsCPUUnAvailable = 0,
    .pmChooseCPU = &pmRyzen_choose_cpu,
    .pmIPIHandler = 0,
    .pmThreadTellUrgency = 0,
    .pmActiveRTThreads = 0,
    .pmInterruptPrewakeApplicable = 0,
    .pmThreadGoingOffCore = 0,
};

void pmRyzen_init(void *handle){
    
    pmRyzen_io_service_handle = handle;
    
    
    void **kernelDisp = lookup_symbol("_pmDispatch");
    pmRyzen_pmUnRegister = (void(*)(pmDispatch_t*))lookup_symbol("_pmUnRegister");
    pmRyzen_cpu_NMI = (void(*)(int))lookup_symbol("_cpu_NMI_interrupt");
    pmRyzen_NMI_enabled = (void(*)(boolean_t))lookup_symbol("_NMIPI_enable");
    pmRyzen_cpu_IPI = (void(*)(boolean_t))lookup_symbol("_i386_cpu_IPI");
    pmRyzen_timeout_TSC = *((uint64_t*)lookup_symbol("_LockTimeOutTSC"));
    
    
    if(*kernelDisp){
        IOLog("Shit, here we go again\n");
        (*pmRyzen_pmUnRegister)(*kernelDisp);
    }
    
    pmCallBacks_t cb;
    pmKextRegister(PM_DISPATCH_VERSION, &pmRyzen_cpuFuncs, &cb);
    
    x86_pkg_t * pkg = cb.GetPkgRoot();
    
    int pkgCount = 0;
    pmRyzen_num_phys = 0;
    pmRyzen_num_logi = 0;
    
    while(pkg){
        pkgCount++;
        x86_core_t *core = pkg->cores;
        
        while (core) {
            pmRyzen_num_phys++;
            x86_lcpu_t *lcpu = core->lcpus;
            
            while (lcpu) {
                pmRyzen_num_logi++;
                IOLog("LCPU: %u master:%u, primary:%u\n", lcpu->cpu_num, lcpu->master, lcpu->primary);
                
                pmRyzen_cpunum_to_lcpu[lcpu->cpu_num] = lcpu;
                
                pmProcessor_t *cpu = &pmRyzen_cpus[lcpu->cpu_num];
                cpu->lcpu = lcpu;
                cpu->stat_idle = 0;
                cpu->stat_exit_idle = 0;
                cpu->arm_flag = 0;

                cpu->cpu_awake = 1;
                
                lcpu = lcpu->next_in_core;
            }
            
            core = core->next_in_pkg;
        }
        
        pkg = pkg->next;
    }
    IOLog("pkg c %d\n", pkgCount);
    
    cb.initComplete();
}

void pmRyzen_stop(){
    
    (*pmRyzen_pmUnRegister)(&pmRyzen_cpuFuncs);
    
    //Make sure all cores exited idle thread.
    for (int i = 0; i < 24; i++) {
        while(pmRyzen_exit_idle(pmRyzen_cpunum_to_lcpu[i])){
            (*pmRyzen_cpu_IPI)(i);
        }
    }
}


uint64_t pmRyzen_machine_idle(uint64_t maxDur){

    
    uint32_t cn = cpu_number();
    //
    
//    if(cn > 2){
//        pmRyzen_wrmsr_safe(pmRyzen_io_service_handle, 0xC0010062, 2);
//    }
    pmRyzen_wrmsr_safe(pmRyzen_io_service_handle, 0xC0010062, 2);
    pmProcessor_t *self = &pmRyzen_cpus[cn];
    self->stat_idle++;
    
    self->cpu_awake = 0;
    self->arm_flag = 0;
    
    
#ifdef PMRYZEN_IDLE_MWAIT

    void* addr = &self->arm_flag;
    uint32_t ps_hint = 0x50;
    __asm__ volatile("wbinvd":::"memory");
    __asm__ volatile("mfence":::"memory");
    __asm__ volatile("clflushopt %0" : "+m" (*(volatile char *)&self->arm_flag));

    __asm__ volatile("mfence;"
                     "movq %0, %%rax;"
                     "xor %%edx, %%edx;"
                     "xor %%ecx, %%ecx;"
                     "monitor;"
                     "xorq %%rax, %%rax;"
                     "movl %1, %%eax;"
                     "movl $0x1, %%ecx;"
                     "mwait;"
                     :
                     : "r"(addr), "r"(ps_hint)
                     : "%ecx", "%edx", "%rax"
                     );
    
#elif PMRYZEN_IDLE_SIMPLE
    
    __asm__ volatile("sti;hlt;");
    
#elif PMRYZEN_IDLE_IO_CSTATE
    
    __asm__ volatile("sti;"
                     "inw $0xf2, %%ax;"
                     :::"%eax");
    
#endif

    
    self->cpu_awake = 1;
    pmRyzen_wrmsr_safe(pmRyzen_io_service_handle, 0xC0010062, 0);

    
    if(!self->arm_flag)
        pmRyzen_exit_idle_false_c++;
    
    return 0;
}

boolean_t pmRyzen_exit_idle(x86_lcpu_t *lcpu){
    

    pmProcessor_t *target = &pmRyzen_cpus[lcpu->cpu_num];

    // Exit if cpu is already awake.
    if(target->cpu_awake) return false;
    
    pmRyzen_exit_idle_c++;
    
#ifdef PMRYZEN_IDLE_MWAIT
    uint64_t start_tsc = rdtsc64();
    do {
        target->arm_flag = 1;
        __asm__ volatile("pause;");
//        asm volatile("clflushopt %0" : "+m" (*(volatile char *)&target->arm_flag));
//        __asm__ volatile("mfence;");
//        break;

        if(rdtsc64() - start_tsc > 0x6000){
            //If we still unable to wake up the processor, send an IPI.
            pmRyzen_exit_idle_ipi_c++;
            return true;
        }
    } while(!target->cpu_awake);
    
    return false;
    
#else
    target->arm_flag = 1;
    pmRyzen_exit_idle_ipi_c++;
    return true;
#endif
}

int pmRyzen_choose_cpu(int startCPU, int endCPU, int preferredCPU){
    if(pmRyzen_cpus[preferredCPU].cpu_awake)
        return preferredCPU+1;
    
    for(int i = startCPU; i < endCPU; i++){
        if(pmRyzen_cpus[i].cpu_awake)
            return i;
    }
    
    return preferredCPU+1;
}
