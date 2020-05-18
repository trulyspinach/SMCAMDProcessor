//
//  ISLPCPort.h
//  SMCAMDProcessor
//
//  Created by Qi HaoYan on 5/14/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#ifndef ISLPCPort_h
#define ISLPCPort_h

#include <mach/mach_types.h>

class ISLPCPort {
    
    
public:
    
    static constexpr i386_ioport_t kCHIP_ID_REG = 0x20;
    static constexpr i386_ioport_t kCHIP_REVISION_REG = 0x21;
    static constexpr i386_ioport_t kCHIP_DEV_SEL_REG = 0x07;
    static constexpr i386_ioport_t kBASE_ADDRESS_REGISTER = 0x60;
    
    static constexpr i386_ioport_t kREGISTER_PORTS[] = {0x4E, 0x2E};
    static constexpr i386_ioport_t kVALUE_PORTS[] = {0x4F, 0x2F};
    
    
    static uint8_t readByte(int portSelect, uint8_t reg){
        outb(kREGISTER_PORTS[portSelect], reg);
        return inb(kVALUE_PORTS[portSelect]);
    }
    
    static uint16_t readWord(int portSelect, uint8_t reg){
        uint16_t w = (readByte(portSelect, reg) << 8) | readByte(portSelect, reg + 1);
        return w;
    }
    
    static void writeByte(int portSelect, uint8_t reg, uint8_t val){
        outb(kREGISTER_PORTS[portSelect], reg);
        outb(kVALUE_PORTS[portSelect], val);
    }
    
    static void select(int portSelect, uint8_t devNum){
        outb(kREGISTER_PORTS[portSelect], kCHIP_DEV_SEL_REG);
        outb(kVALUE_PORTS[portSelect], devNum);
    }
};

#endif /* ISLPCPort_h */
