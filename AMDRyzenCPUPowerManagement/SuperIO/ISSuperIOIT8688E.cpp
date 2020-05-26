//
//  ISSuperIOIT8688E.cpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Maurice on 25.05.20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#include "ISSuperIOIT8688E.hpp"

ISSuperIOIT8688E::ISSuperIOIT8688E(int psel, uint16_t addr, uint16_t chipIntel){
    lpcPortSel = psel;
    chipAddr = addr;
    
    switch (chipIntel) {
        case CHIP_IT8688E:
        default:
            activeFansOnSystem = 5;
            break;
        case CHIP_IT8686E:
            activeFansOnSystem = 5;
    }
    
    //backup default ctrl mode
    for (int i = 0; i < activeFansOnSystem; i++) {
        fanDefaultControlMode[i] = readByte(kFAN_CTRL_MODE_REGS[i]);
    }
}

ISSuperIOIT8688E* ISSuperIOIT8688E::getDevice(uint16_t *chipIntel){
    
    i386_ioport_t regport = 0;
    uint8_t deviceID=0, revision=0;
    bool found = false;
    int portSel = 0;
    IOLog("probe IT8688E\n");
    
    for (; portSel < 2; portSel++) {
        regport = ISLPCPort::kREGISTER_PORTS[portSel];
        
        if (regport != 0x2E && regport != 0x4E)
        {
            break;
        }
        
        //open port
        outb(regport, 0x87);
        outb(regport, 0x01);
        outb(regport, 0x55);
        
        if (regport == 0x4E) {
          outb(regport, 0xAA);
        } else {
          outb(regport, 0x55);
        }
        
        
        deviceID = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_ID_REG);
        revision = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_REVISION_REG);
        found = false;
        
        switch ((deviceID << 8) | revision) {
            case CHIP_IT8688E:
                found = true;
                IOLog("IT8688E chip identified\n");
                break;
            case CHIP_IT8686E:
                found = true;
                IOLog("IT8686E Chip identified\n");
                break;
            default:
                
                break;
        }
        
        
        if(found) break;
        else{
            //close port
            if (regport != 0x4E) {
              outb(regport, 0x02);
            }
        }
    }
    *chipIntel = (deviceID << 8) | revision;
    if(!found) return nullptr;
    
    IOLog("SMC Chip id:%X revision:%X \n", deviceID, revision);
    ISLPCPort::select(portSel, CHIP_ENVIRONMENT_CONTROLLER_LDN);
    
    uint16_t devAddr = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER);
    
    //verify addr
    IOSleep(100);
    if(ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER) != devAddr){
        IOLog("IT8688E address verify failed");
    }
    
    IOLog("Chip address: 0x%X\n", devAddr);
    IOLog("Chip version: 0x%X\n", (uint8_t)(ISLPCPort::readByte(portSel, CHIP_VERSION_REGISTER) & 0x0F));

    ISLPCPort::select(portSel, CHIP_GPIO_LDN);
    uint16_t gpioAddress = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER + 2);
    
    //verify gpio addr
    IOSleep(100);
    if(ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER + 2) != gpioAddress){
        IOLog("IT8688E gpio address verify failed");
    }
    
    //close port
    if (regport != 0x4E) {
      outb(regport, 0x02);
    }
    
    return new ISSuperIOIT8688E(portSel, devAddr, *chipIntel); // ToDo Add GPIO Addr
}

uint8_t ISSuperIOIT8688E::readByte(uint16_t addr){
    IOLog("ReadByte Addr: 0x%X\n", addr);
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xFF);
    return inb(chipAddr + CHIP_DAT_REG_OFFSET);
}

uint16_t ISSuperIOIT8688E::readWord(uint16_t addr){
    IOLog("ReadWord Addr: 0x%X\n", addr);
    return (readByte(addr) << 8) | readByte(addr + 1);
}

void ISSuperIOIT8688E::writeByte(uint16_t addr, uint8_t val){
    IOLog("WriteByte Addr: 0x%X, 0x%X\n", addr, val);
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xFF);
    outb(chipAddr + CHIP_DAT_REG_OFFSET, val);
}

int ISSuperIOIT8688E::getNumberOfFans(){
    return activeFansOnSystem;
}

const char *ISSuperIOIT8688E::getReadableStringForFan(int fan){
    if(fan > activeFansOnSystem) return nullptr;
    return kFAN_READABLE_STRS[fan];
}

uint32_t ISSuperIOIT8688E::getRPMForFan(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanRPMs[fan];
}

bool ISSuperIOIT8688E::getFanAutoControlMode(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanControlMode[fan] != 0;
}

uint8_t ISSuperIOIT8688E::getFanThrottle(int fan){
    if(fan > activeFansOnSystem) return 0;
    return fanThrottles[fan];
}

void ISSuperIOIT8688E::updateFanRPMS(){
   
    for (int i = 0; i < activeFansOnSystem; i++)
    {
        int value = readByte(kFAN_RPM_REGS[i]);
        value |= readByte(kFAN_RPM_EXT_REGS[i]) << 8;

        if (value > 0x3f)
        {
          fanRPMs[i] = (value < 0xffff) ? 1.35e6f / (value * 2) : 0;
        }
        else
        {
          IOLog("Fan RPM else condition");
          fanRPMs[i] = 0;
        }
        IOLog("fan %d: %d\n", i, (int)fanRPMs[i]);
    }
}

void ISSuperIOIT8688E::updateFanControl(){
    for (int i = 0; i < activeFansOnSystem; i++)
    {
        uint16_t value = readByte(kFAN_PWM_CTRL_REGS[i]);

        if ((value & 0x80) > 0)
        {
           fanControlMode[i] = 255;
        }
        else
        {
            value = readByte(kFAN_PWM_CTRL_EXT_REGS[i]);
            fanControlMode[i] = value * 100.0f / 0xFF;
        }
        
        IOLog("fan pwm %d:\n", fanControlMode[i]);
    }
}

void ISSuperIOIT8688E::overrideFanControl(int fan, uint8_t thr){
    if(fan >= activeFansOnSystem) return;

    writeByte(kFAN_MAIN_CTRL_REG, (readByte(kFAN_MAIN_CTRL_REG) | (1 << fan)));
    writeByte(kFAN_PWM_CTRL_REGS[fan], (fanDefaultControlMode[fan] & 0x7F));
    writeByte(kFAN_PWM_CTRL_EXT_REGS[fan], thr);
}

void ISSuperIOIT8688E::setDefaultFanControl(int fan){
    if(fan >= activeFansOnSystem) return;
    writeByte(kFAN_CTRL_MODE_REGS[fan], fanDefaultControlMode[fan]);
}
