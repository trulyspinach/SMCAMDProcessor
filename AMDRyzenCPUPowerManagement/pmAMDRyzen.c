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



void(*pmRyzen_pmUnRegister)(pmDispatch_t*) = 0;


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
    .exitIdle = 0,
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
    .pmChooseCPU = 0,
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
    
    if(*kernelDisp){
        IOLog("Shit, here we go again\n");
        (*pmRyzen_pmUnRegister)(*kernelDisp);
    }
    
    pmCallBacks_t cb;
    pmKextRegister(PM_DISPATCH_VERSION, &pmRyzen_cpuFuncs, &cb);

    x86_pkg_t * pkg = cb.GetPkgRoot();

    int pkgCount = 0;

    while(pkg){
        pkgCount++;
        
        x86_core_t *core = pkg->cores;
        while (core) {

            x86_lcpu_t *lcpu = core->lcpus;
            while (lcpu) {
                IOLog("LCPU: %u master:%u, primary:%u\n", lcpu->cpu_num, lcpu->master, lcpu->primary);
                
                pmRyzen_cpunum_to_lcpu[lcpu->cpu_num] = lcpu;
                lcpu = lcpu->next_in_core;
            }
            
            core = core->next_in_pkg;
        }

        pkg = pkg->next;
    }
    IOLog("c %d\n", pkgCount);
    
    cb.initComplete();
}

void pmRyzen_stop(){
    (*pmRyzen_pmUnRegister)(&pmRyzen_cpuFuncs);
}

uint64_t pmRyzen_machine_idle(uint64_t maxDur){
    __asm__ volatile ("sti");
    __asm__ volatile ("hlt");
//    IOLog("IDLE\n");
    
    return 0;
}
