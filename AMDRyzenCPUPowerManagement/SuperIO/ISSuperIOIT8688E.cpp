//
//  ISSuperIOIT8688E.cpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Maurice on 25.05.20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#include "ISSuperIOIT8688E.hpp"

ISSuperIOIT8688E::ISSuperIOIT8688E(int psel, uint16_t addr, uint16_t chipIntel)
{
    lpcPortSel = psel;
    chipAddr = addr;

    switch (chipIntel)
    {
        case CHIP_IT8688E:
        default:
            activeFansOnSystem = 5;
            break;
        case CHIP_IT8686E:
            activeFansOnSystem = 5;
            break;
    }

    // backup default ctrl mode
    for (int i = 0; i < activeFansOnSystem; i++)
    {
        fanDefaultControlMode[i] = readByte(kFAN_PWM_CTRL_REGS[i]);
        fanDefaultExtControlMode[i] = readByte(kFAN_PWM_CTRL_EXT_REGS[i]);
    }
}

ISSuperIOIT8688E* ISSuperIOIT8688E::getDevice(uint16_t* chipIntel)
{

    i386_ioport_t regport = 0;
    uint8_t deviceID = 0, revision = 0;
    bool found = false;
    int portSel = 0;
    IOLog("probe IT8XXXE\n");

    for (; portSel < 2; portSel++)
    {
        regport = ISLPCPort::kREGISTER_PORTS[portSel];

        if (regport != 0x2E && regport != 0x4E)
        {
            break;
        }

        // open port
        outb(regport, 0x87);
        outb(regport, 0x01);
        outb(regport, 0x55);

        if (regport == 0x4E)
        {
            outb(regport, 0xAA);
        }
        else
        {
            outb(regport, 0x55);
        }

        deviceID = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_ID_REG);
        revision = ISLPCPort::readByte(portSel, ISLPCPort::kCHIP_REVISION_REG);

        switch ((deviceID << 8) | revision)
        {
            case CHIP_IT8688E:
                found = true;
                IOLog("IT8688E chip identified\n");
                break;
            case CHIP_IT8686E:
                found = true;
                IOLog("IT8686E Chip identfiied\n");
            default:
                break;
        }

        if (found)
        {
            break;
        }
        else
        {
            // close port
            if (regport != 0x4E)
            {
                outb(regport, 0x02);
            }
        }
    }

    *chipIntel = (deviceID << 8) | revision;
    if (!found)
        return nullptr;

    IOLog("SMC Chip id:%X revision:%X \n", deviceID, revision);
    ISLPCPort::select(portSel, CHIP_ENVIRONMENT_CONTROLLER_LDN);

    uint16_t devAddr = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER);

    // verify addr
    IOSleep(100);
    if (ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER) != devAddr)
    {
        IOLog("IT8688E address verify failed");
    }

    ISLPCPort::select(portSel, CHIP_GPIO_LDN);
    uint16_t gpioAddress = ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER + 2);

    // verify gpio addr
    IOSleep(100);
    if (ISLPCPort::readWord(portSel, ISLPCPort::kBASE_ADDRESS_REGISTER + 2) != gpioAddress)
    {
        IOLog("IT8688E gpio address verify failed");
    }

    // close port
    if (regport != 0x4E)
    {
        outb(regport, 0x02);
    }

    return new ISSuperIOIT8688E(portSel, devAddr, *chipIntel);  //TODO: Add GPIO Addr
}

uint8_t ISSuperIOIT8688E::readByte(uint16_t addr)
{
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xFF);
    return inb(chipAddr + CHIP_DAT_REG_OFFSET);
}

uint16_t ISSuperIOIT8688E::readWord(uint16_t addr)
{
    return (readByte(addr) << 8) | readByte(addr + 1);
}

void ISSuperIOIT8688E::writeByte(uint16_t addr, uint8_t val)
{
    outb(chipAddr + CHIP_ADDR_REG_OFFSET, addr & 0xFF);
    outb(chipAddr + CHIP_DAT_REG_OFFSET, val);
}

int ISSuperIOIT8688E::getNumberOfFans()
{
    return activeFansOnSystem;
}

const char* ISSuperIOIT8688E::getReadableStringForFan(int fan)
{
    if (fan > activeFansOnSystem)
        return nullptr;
    return kFAN_READABLE_STRS[fan];
}

uint32_t ISSuperIOIT8688E::getRPMForFan(int fan)
{
    if (fan > activeFansOnSystem)
        return 0;
    return fanRPMs[fan];
}

bool ISSuperIOIT8688E::getFanAutoControlMode(int fan)
{
    if (fan > activeFansOnSystem)
        return 0;
    return fanControlMode[fan] != 0;
}

uint8_t ISSuperIOIT8688E::getFanThrottle(int fan)
{
    if (fan > activeFansOnSystem)
        return 0;
    return fanControlMode[fan];
}

void ISSuperIOIT8688E::updateFanRPMS()
{
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
            fanRPMs[i] = 0;
        }
    }
}

void ISSuperIOIT8688E::updateFanControl()
{
    for (int i = 0; i < activeFansOnSystem; i++)
    {
        fanControlMode[i] = readByte(kFAN_PWM_CTRL_EXT_REGS[i]);
    }
}

void ISSuperIOIT8688E::overrideFanControl(int fan, uint8_t thr)
{
    if (fan >= activeFansOnSystem)
        return;
    writeByte(kFAN_MAIN_CTRL_REG, (readByte(kFAN_MAIN_CTRL_REG) | (1 << fan)));
    writeByte(kFAN_PWM_CTRL_REGS[fan], (fanDefaultControlMode[fan] & 0x7F));
    writeByte(kFAN_PWM_CTRL_EXT_REGS[fan], thr);
}

void ISSuperIOIT8688E::setDefaultFanControl(int fan)
{
    if (fan >= activeFansOnSystem)
        return;
    writeByte(kFAN_MAIN_CTRL_REG, (readByte(kFAN_MAIN_CTRL_REG) ^ (1 << fan)));
    writeByte(kFAN_MAIN_CTRL_REG,
              (readByte(kFAN_MAIN_CTRL_REG) ^
               (1 << fan)));  // Fan 0 only goes back to auto mode when MAIN_CTRL_REG is switched twice
    writeByte(kFAN_PWM_CTRL_REGS[fan], fanDefaultControlMode[fan]);
    writeByte(kFAN_PWM_CTRL_EXT_REGS[fan], fanDefaultExtControlMode[fan]);
}
