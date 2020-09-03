//
//  pmRyzenSymbolTable.c
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 9/2/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//



#ifndef pmRyzenSymbolTable_h
#define pmRyzenSymbolTable_h

typedef struct pmRyzen_symtable {
    void* _wrmsr_carefully;
    void* _KUNCUserNotificationDisplayAlert;
    void* _cpu_to_processor;
    void* _tscFreq;
    void* _pmDispatch;
    void* _pmUnRegister;
    void* _cpu_NMI_interrupt;
    void* _NMIPI_enable;
    void* _i386_cpu_IPI;
} pmRyzen_symtable_t;

#endif
