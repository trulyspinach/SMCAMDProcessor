//
//  ISSuperIOIT8688E.hpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Maurice on 25.05.20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#ifndef ISSuperIOIT8688E_hpp
#define ISSuperIOIT8688E_hpp

#include <IOKit/IOLib.h>

#include <architecture/i386/pio.h>

#include "ISLPCPort.h"
#include "ISSuperIOSMCFamily.hpp"

#define CHIP_IT8688E 0x8688
#define CHIP_IT8686E 0x8686

#define IT8688E_MAX_NUMFAN 5

#define CHIP_SIO_OPEN 0x87
#define CHIP_SIO_CLOSE 0x02
#define CHIP_ENVIRONMENT_CONTROLLER_LDN 0x04
#define CHIP_VERSION_REGISTER 0x22
#define CHIP_GPIO_LDN 0x07

#define CHIP_TACHOMETER_DIVISOR_REGISTER 0x0B
#define CHIP_ADDR_REG_OFFSET 0x05
#define CHIP_DAT_REG_OFFSET 0x06


class ISSuperIOIT8688E : public ISSuperIOSMCFamily {

    
public:
    static constexpr const char *kFAN_READABLE_STRS[] = {
        "CPU Fan",
        "System 1 Fan",
        "System 2 Fan",
        "PCH Fan",
        "CPU OPT Fan",
    };
    
    static ISSuperIOIT8688E* getDevice(uint16_t *chipIntel);
    
    
    ISSuperIOIT8688E(int psel, uint16_t addr, uint16_t chipIntel);
    
    int fanRPMs[IT8688E_MAX_NUMFAN];
    uint8_t fanThrottles[IT8688E_MAX_NUMFAN];
    uint8_t fanControlMode[IT8688E_MAX_NUMFAN];
    
    int activeFansOnSystem = 0;
    
    int getNumberOfFans() override;
    const char *getReadableStringForFan(int fan) override;
    
    uint32_t getRPMForFan(int fan) override;
    bool getFanAutoControlMode(int fan) override;
    uint8_t getFanThrottle(int fan) override;
    
    void updateFanRPMS() override;
    void updateFanControl() override;
    
    void overrideFanControl(int fan, uint8_t thr) override;
    void setDefaultFanControl(int fan) override;
    
private:
    
    static constexpr uint16_t kFAN_MAIN_CTRL_REG = 0x13;
    static constexpr uint16_t kFAN_RPM_REGS[] = { 0x0d, 0x0e, 0x0f, 0x80, 0x82 };
    static constexpr uint16_t kFAN_RPM_EXT_REGS[] = { 0x18, 0x19, 0x1a, 0x81, 0x83 };
    static constexpr uint16_t kFAN_PWM_CTRL_REGS[] = { 0x15, 0x16, 0x17, 0x7f, 0xa7 };
    static constexpr uint16_t kFAN_PWM_CTRL_EXT_REGS[] = { 0x63, 0x6b, 0x73, 0x7b, 0xa3 };
    static constexpr uint16_t kFAN_CTRL_MODE_REGS[] = { 0x15, 0x16, 0x17, 0x7f, 0xa7 };
    
    int lpcPortSel = 0;
    
    uint16_t chipAddr = 0;
    uint8_t fanDefaultControlMode[IT8688E_MAX_NUMFAN];
    
    uint8_t readByte(uint16_t addr);
    uint16_t readWord(uint16_t addr);
    
    void writeByte(uint16_t addr, uint8_t val);
};

#endif /* ISSuperIOIT8688E_hpp */
