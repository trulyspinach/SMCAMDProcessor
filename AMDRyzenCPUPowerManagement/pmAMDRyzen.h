//
//  pmAMDRyzen.h
//  AMDRyzenCPUPowerManagement
//
//  Created by trulyspinach on 3/27/20.
//

#ifndef pmAMDRyzen_h
#define pmAMDRyzen_h


#include <Headers/mach/mach_types.h>
#include <IOKit/IOLib.h>

#include "Headers/osfmk/i386/pmCPU.h"
#include "Headers/osfmk/i386/cpu_topology.h"

#include "symresolver/kernel_resolver.h"

#include <i386/proc_reg.h>
#include "Headers/pmRyzenSymbolTable.h"

#define MOD_NAME pmARyzen
#define XNU_MAX_CPU 64

#define MSR_PSTATE_CTL 0xC0010062
#define MSR_PSTATE_0 0xC0010064

#define EFF_INTERVAL 0.15
#define PSTATE_LIMIT 1
#define PSTATE_LOW_POWER 2
#define PSTATE_STEPDOWN_THRE 0.25
#define PSTATE_STEPUP_THRE 0.45
#define PSTATE_STEPDOWN_TIME 5
#define PSTATE_STEPDOWN_MP_GAIN 1


extern int cpu_number(void);
extern void mp_rendezvous_no_intrs(void (*action_func)(void *), void *arg);

extern x86_lcpu_t *pmRyzen_cpunum_to_lcpu[XNU_MAX_CPU];

extern uint32_t pmRyzen_num_phys;
extern uint32_t pmRyzen_num_logi;

extern uint64_t pmRyzen_exit_idle_c;
extern uint64_t pmRyzen_exit_idle_ipi_c;
extern uint64_t pmRyzen_exit_idle_false_c;

extern uint32_t pmRyzen_hpcpus;

extern uint32_t pmRyzen_pstatelimit;


extern void pmRyzen_wrmsr_safe(void *, uint32_t, uint64_t);
extern uint64_t pmRyzen_rdmsr_safe(void *, uint32_t);
extern pmRyzen_symtable_t pmRyzen_symtable;


typedef struct pmProcessor{
    x86_lcpu_t *lcpu;

    uint64_t stat_exit_idle;
#ifdef PMRYZEN_IDLE_MWAIT
    char pad1[64*64];
#endif
    uint64_t arm_flag;
#ifdef PMRYZEN_IDLE_MWAIT
    char pad2[64*64];
#endif
    uint64_t cpu_awake;
    
    uint64_t last_idle_tsc;
    uint64_t last_start_tsc;
    uint64_t last_idle_length;
    uint64_t last_running_time;
    
    uint64_t eff_timeacc;
    uint64_t eff_idleacc;
    
    float eff_load;
    uint64_t eff_timeaccd;
    uint64_t eff_idleaccd;
    
    uint32_t ll_count;
    uint8_t PState;
    
} pmProcessor_t;

void pmRyzen_init(void*);
void pmRyzen_stop(void);
void pmRyzen_PState_reset(void);
float pmRyzen_avgload_pcpu(uint32_t);

uint64_t pmRyzen_machine_idle(uint64_t);

boolean_t pmRyzen_exit_idle(x86_lcpu_t *);

int pmRyzen_choose_cpu(int,int,int);

pmProcessor_t* pmRyzen_get_processor(uint32_t);

inline uint32_t pmRyzen_cpu_phys_num(uint32_t cpunum){
    return pmRyzen_cpunum_to_lcpu[cpunum]->core->pcore_num;
}

inline uint32_t pmRyzen_cpu_primary_in_core(uint32_t cpunum){
    return pmRyzen_cpunum_to_lcpu[cpunum]->core->lcpus == pmRyzen_cpunum_to_lcpu[cpunum];
}

inline boolean_t pmRyzen_cpu_is_master(uint32_t cpunum){
    return pmRyzen_cpunum_to_lcpu[cpunum]->master;
}


#endif /* pmAMDRyzen_h */
