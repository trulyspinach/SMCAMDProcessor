//
//  pmAMDRyzen.h
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 3/27/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#ifndef pmAMDRyzen_h
#define pmAMDRyzen_h


#include <mach/mach_types.h>
#include <IOKit/IOLib.h>


#include "Headers/osfmk/i386/pmCPU.h"
#include "Headers/osfmk/i386/cpu_topology.h"

#include "symresolver/kernel_resolver.h"

#include <i386/proc_reg.h>


#define MOD_NAME pmAMDRyzen
#define XNU_MAX_CPU 64

extern int cpu_number(void);

extern x86_lcpu_t *pmRyzen_cpunum_to_lcpu[XNU_MAX_CPU];

extern uint32_t pmRyzen_num_phys;
extern uint32_t pmRyzen_num_logi;

extern uint64_t pmRyzen_exit_idle_c;
extern uint64_t pmRyzen_exit_idle_ipi_c;
extern uint64_t pmRyzen_exit_idle_false_c;

extern void pmRyzen_wrmsr_safe(void *, uint32_t, uint64_t);


typedef struct pmProcessor{
    x86_lcpu_t *lcpu;
    uint64_t stat_idle;
    uint64_t stat_exit_idle;
#ifdef PMRYZEN_IDLE_MWAIT
    char pad1[64*64];
#endif
    uint64_t arm_flag;
#ifdef PMRYZEN_IDLE_MWAIT
    char pad2[64*64];
#endif
    uint64_t cpu_awake;
} pmProcessor_t;

void pmRyzen_init(void*);
void pmRyzen_stop(void);

uint64_t pmRyzen_machine_idle(uint64_t);

boolean_t pmRyzen_exit_idle(x86_lcpu_t *);

int pmRyzen_choose_cpu(int,int,int);


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
