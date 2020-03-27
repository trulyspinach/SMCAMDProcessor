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

#define MOD_NAME pmAMDRyzen
#define XNU_MAX_CPU 64

extern x86_lcpu_t *pmRyzen_cpunum_to_lcpu[XNU_MAX_CPU];


void pmRyzen_init(void*);
void pmRyzen_stop();

uint64_t pmRyzen_machine_idle(uint64_t);

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
