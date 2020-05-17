//
//  ISSuperIONCT6683.cpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 5/17/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#include "ISSuperIONCT668X.hpp"

ISSuperIONCT668X::ISSuperIONCT668X(int psel, uint16_t addr, uint16_t chipIntel){
    lpcPortSel = psel;
    chipAddr = addr;
    
    activeFansOnSystem = CHIP_MAX_NUMFAN;
    
    //backup default ctrl mode
//    for (int i = 0; i < activeFansOnSystem; i++) {
//        fanDefaultControlMode[i] = readByte(kFAN_CTRL_MODE_REGS[i]);
//    }
}

ISSuperIONCT668X* ISSuperIONCT668X::getDevice(uint16_t *chipIntel){
    
    i386_ioport_t regport = 0;
    uint8_t deviceID=0, revision=0;
    bool found = false;
    int portSel = 0;
    IOLog("probe NCT668X\n");
    
    for (; portSel < 2; portSel++) {
        regport = ISLPCPort::kREGISTER_PORTS[portSel];
        
        //open port
        outb(regport, CHIP_SIO_OPEN);
        outb(regport, CHIP_SIO_OPEN);
        
        
        deviceID = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_ID_REG);
        revision = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_REVISION_REG);
        found = false;
        
        switch (((deviceID << 8) | revision) & 0xfff0) {
            case CHIP_NCT6681:
            case CHIP_NCT6683:
                found = true;
                IOLog("NCT668X chip identified\n");
                break;
                
            default:
                
                break;
        }
        
        
        if(found) break;
        else{
            //close port
            outb(regport, CHIP_SIO_CLOSE);
            //668X needs additional step to close port.
            outb(regport, 0x02);
            outb(regport, 0x02);
        }
    }
    *chipIntel = (deviceID << 8) | revision;
    if(!found) return nullptr;
    
    IOLog("SMC Chip id:%X revision:%X \n", deviceID, revision);
    ISLPCPort::select(portSel, CHIP_HWM_LDN);
    
    uint16_t devAddr = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER) & (~7);
    
    IOLog("Chip address: 0x%X\n", devAddr);
    
    //Now that the present of chip is confirmed, disable IO address space lock.
    uint8_t conf = 0;
    switch (*chipIntel) {
        //in short these is all of what we currentlt supports.
        case CHIP_NCT6681:
        case CHIP_NCT6683:
            conf = ISLPCPort::readByte(portSel, 0x30);
            if(conf & 0x01){
                ISLPCPort::writeByte(portSel, 0x30, conf & 0x01);
            }
            break;
            
        default:
            break;
    }
    
    //close port
    outb(regport, CHIP_SIO_CLOSE);
    outb(regport, 0x02);
    outb(regport, 0x02);
    
    return new ISSuperIONCT668X(portSel, devAddr, *chipIntel);
}

uint8_t ISSuperIONCT668X::readByte(uint16_t addr){
    outb(chipAddr + EC_BANK_REG_OFFSET, 0xff);
    outb(chipAddr + EC_BANK_REG_OFFSET, addr >> 8);
    outb(chipAddr + EC_INDEX_REG_OFFSET, addr & 0xff);
    return inb(chipAddr + EC_DAT_REG_OFFSET);
}

uint16_t ISSuperIONCT668X::readWord(uint16_t addr){
    return (readByte(addr) << 8) | readByte(addr + 1);
}

void ISSuperIONCT668X::writeByte(uint16_t addr, uint8_t val){
    outb(chipAddr + EC_BANK_REG_OFFSET, 0xff);
    outb(chipAddr + EC_BANK_REG_OFFSET, addr >> 8);
    outb(chipAddr + EC_INDEX_REG_OFFSET, addr & 0xff);
    outb(chipAddr + EC_DAT_REG_OFFSET, val);
}

int ISSuperIONCT668X::getNumberOfFans(){
    return activeFansOnSystem;
}

const char *ISSuperIONCT668X::getReadableStringForFan(int fan){
    if(fan > activeFansOnSystem) return nullptr;
    //TODO: label fans
    return kFAN_READABLE_STRS[0];
}

uint32_t ISSuperIONCT668X::getRPMForFan(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanRPMs[fan];
}

bool ISSuperIONCT668X::getFanAutoControlMode(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanControlMode[fan] != 0;
}

uint8_t ISSuperIONCT668X::getFanThrottle(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanThrottles[fan];
}

void ISSuperIONCT668X::updateFanRPMS(){
   
    for (int i = 0; i < activeFansOnSystem; i++) {
        int v = (int)readWord(FAN_RPM_REGS(i));
        fanRPMs[i] = v;
//        IOLog("fan %d: %d\n", i, (int)v);
    }
}

void ISSuperIONCT668X::updateFanControl(){
    for (int i = 0; i < 8; i++) {
//        fanControlMode[i] = readByte(kFAN_CTRL_MODE_REGS(i));
//        IOLog("fan ctrl %d: %d\n", i, (int)v);
        
        fanControlMode[i] = 1;//TODO: are there control modes in this chip?
        fanThrottles[i] = readByte(FAN_PWM_REGS(i));
//        IOLog("fan pwm %d: %d\n", i, (int)v);
    }
}

void ISSuperIONCT668X::overrideFanControl(int fan, uint8_t thr){
    if(fan >= activeFansOnSystem) return;
    writeByte(FAN_CFG_CTRL_REG, FAN_CFG_REQ);
    IOSleep(2);
    writeByte(FAN_PWMCMD_REGS(fan), thr);
    writeByte(FAN_CFG_CTRL_REG, FAN_CFG_DONE);
}

void ISSuperIONCT668X::setDefaultFanControl(int fan){
    if(fan >= activeFansOnSystem) return;
    
}
