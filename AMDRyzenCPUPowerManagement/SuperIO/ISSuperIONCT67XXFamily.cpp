//
//  ISSuperIODevice.cpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 5/14/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#include "ISSuperIONCT67XXFamily.hpp"

ISSuperIONCT67XXFamily::ISSuperIONCT67XXFamily(int psel, uint16_t addr, uint16_t chipIntel){
    lpcPortSel = psel;
    chipAddr = addr;
    
    switch (chipIntel) {
        case CHIP_NCT6779D:
            activeFansOnSystem = 5;
            break;
        
        case CHIP_NCT6797D:
        case CHIP_NCT6798D:
            activeFansOnSystem = 7;
            break;
            
        default:
            activeFansOnSystem = 6;
            break;
    }
    
    //backup default ctrl mode
    for (int i = 0; i < activeFansOnSystem; i++) {
        fanDefaultControlMode[i] = readByte(kFAN_CTRL_MODE_REGS[i]);
    }
}

ISSuperIONCT67XXFamily* ISSuperIONCT67XXFamily::getDevice(uint16_t *chipIntel){
    
    i386_ioport_t regport = 0;
    uint8_t deviceID=0, revision=0;
    bool found = false;
    int portSel = 0;
    IOLog("probe NCT67XX\n");
    
    for (; portSel < 2; portSel++) {
        regport = ISLPCPort::kREGISTER_PORTS[portSel];
        
        //open port
        outb(regport, CHIP_SIO_OPEN);
        outb(regport, CHIP_SIO_OPEN);
        
        
        deviceID = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_ID_REG);
        revision = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_REVISION_REG);
        found = false;
        
        switch ((deviceID << 8) | revision) {
            case CHIP_NCT610XD:
            case CHIP_NCT6771F:
            case CHIP_NCT6776F:
            case CHIP_NCT6779D:
            case CHIP_NCT6791D:
            case CHIP_NCT6792D:
            case CHIP_NCT6792DA:
            case CHIP_NCT6793D:
            case CHIP_NCT6795D:
            case CHIP_NCT6796D:
            case CHIP_NCT6796DR:
            case CHIP_NCT6797D:
            case CHIP_NCT6798D:
                found = true;
                IOLog("NCT67XX chip identified\n");
                break;
                
            default:
                
                break;
        }
        
        
        if(found) break;
        else{
            //close port
            outb(regport, CHIP_SIO_CLOSE);
        }
    }
    *chipIntel = (deviceID << 8) | revision;
    if(!found) return nullptr;
    
    IOLog("SMC Chip id:%X revision:%X \n", deviceID, revision);
    ISLPCPort::select(portSel, CHIP_HWM_LDN);
    
    uint16_t devAddr = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER);
    
    //verify addr
    IOSleep(100);
    if(ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER) != devAddr){
        IOLog("Fuck me your fucking address is not vaild. Go and fuck yourself");
    }
    
    IOLog("Chip address: 0x%X\n", devAddr);
    
    //Now that the present of chip is confirmed, disable IO address space lock.
    uint8_t conf = 0;
    switch (*chipIntel) {
        //in short these is all of what we currentlt supports.
        case CHIP_NCT6791D:
        case CHIP_NCT6792D:
        case CHIP_NCT6792DA:
        case CHIP_NCT6793D:
        case CHIP_NCT6795D:
        case CHIP_NCT6796D:
        case CHIP_NCT6796DR:
        case CHIP_NCT6797D:
        case CHIP_NCT6798D:
            conf = ISLPCPort::readByte(portSel, CHIP_IO_SPACE_LOCK);
            if(conf & 0x10){
                ISLPCPort::writeByte(portSel, CHIP_IO_SPACE_LOCK, conf & ~0x10);
            }
            break;
            
        default:
            break;
    }
    
    //close port
    outb(regport, 0xaa);
    
    return new ISSuperIONCT67XXFamily(portSel, devAddr, *chipIntel);
}

uint8_t ISSuperIONCT67XXFamily::readByte(uint16_t addr){
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, CHIP_BANK_SEL_REG);
    outb(chipAddr + CHIP_DAT_REG_OFFSET, addr >> 8); // select bank
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xff); // select register
    return inb(chipAddr + CHIP_DAT_REG_OFFSET);
}

uint16_t ISSuperIONCT67XXFamily::readWord(uint16_t addr){
    return (readByte(addr) << 8) | readByte(addr + 1);
}

void ISSuperIONCT67XXFamily::writeByte(uint16_t addr, uint8_t val){
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, CHIP_BANK_SEL_REG);
    outb(chipAddr + CHIP_DAT_REG_OFFSET, addr >> 8); // select bank
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xff); // select register
    outb(chipAddr + CHIP_DAT_REG_OFFSET, val);
}

int ISSuperIONCT67XXFamily::getNumberOfFans(){
    return activeFansOnSystem;
}

const char *ISSuperIONCT67XXFamily::getReadableStringForFan(int fan){
    if(fan > activeFansOnSystem) return nullptr;
    return kFAN_READABLE_STRS[fan];
}

uint32_t ISSuperIONCT67XXFamily::getRPMForFan(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanRPMs[fan];
}

bool ISSuperIONCT67XXFamily::getFanAutoControlMode(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanControlMode[fan] != 0;
}

uint8_t ISSuperIONCT67XXFamily::getFanThrottle(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanThrottles[fan];
}

void ISSuperIONCT67XXFamily::updateFanRPMS(){
   
    for (int i = 0; i < activeFansOnSystem; i++) {
        int v = (int)readWord(kFAN_RPM_REGS[i]);
        fanRPMs[i] = v;
//        IOLog("fan %d: %d\n", i, (int)v);
    }
}

void ISSuperIONCT67XXFamily::updateFanControl(){
    for (int i = 0; i < activeFansOnSystem; i++) {
        fanControlMode[i] = readByte(kFAN_CTRL_MODE_REGS[i]);
//        IOLog("fan ctrl %d: %d\n", i, (int)v);
        
        fanThrottles[i] = readByte(kFAN_PWMCMD_REGS[i]);
//        IOLog("fan pwm %d: %d\n", i, (int)v);
    }
}

void ISSuperIONCT67XXFamily::overrideFanControl(int fan, uint8_t thr){
    if(fan >= activeFansOnSystem) return;
    writeByte(kFAN_CTRL_MODE_REGS[fan], 0);
    writeByte(kFAN_PWMCMD_REGS[fan], thr);
}

void ISSuperIONCT67XXFamily::setDefaultFanControl(int fan){
    if(fan >= activeFansOnSystem) return;
    writeByte(kFAN_CTRL_MODE_REGS[fan], fanDefaultControlMode[fan]);
}
