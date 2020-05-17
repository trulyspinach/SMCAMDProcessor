//
//  ISSuperIONCT6683.hpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 5/17/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#ifndef ISSuperIONCT6683_hpp
#define ISSuperIONCT6683_hpp

#include <IOKit/IOLib.h>

#include <architecture/i386/pio.h>

#include "ISLPCPort.h"
#include "ISSuperIOSMCFamily.hpp"

#define CHIP_NCT6681 0xb270
#define CHIP_NCT6683 0xc730


#define CHIP_MAX_NUMFAN 16

#define CHIP_SIO_OPEN 0x87
#define CHIP_SIO_CLOSE 0xaa
#define CHIP_HWM_LDN 0x0b

#define CHIP_ADDR_REG_OFFSET 0x05
#define CHIP_DAT_REG_OFFSET 0x06
#define CHIP_BANK_SEL_REG 0x4E

#define EC_BANK_REG_OFFSET  0
#define EC_INDEX_REG_OFFSET 1
#define EC_DAT_REG_OFFSET  2

#define FAN_RPM_REGS(x) (0x140 + (x) * 2)
#define FAN_PWMCMD_REGS(x) (0xa28 + (x))
#define FAN_PWM_REGS(x) (0x160 + (x))

#define FAN_CFG_CTRL_REG 0xa01
#define FAN_CFG_REQ      0x80
#define FAN_CFG_DONE     0x40

class ISSuperIONCT668X : public ISSuperIOSMCFamily {

    
public:
    static constexpr const char *kFAN_READABLE_STRS[] = {
        "Fan",
    };
    
    static ISSuperIONCT668X* getDevice(uint16_t *chipIntel);
    
    
    ISSuperIONCT668X(int psel, uint16_t addr, uint16_t chipIntel);
    
    int fanRPMs[CHIP_MAX_NUMFAN];
    uint8_t fanThrottles[CHIP_MAX_NUMFAN];
    uint8_t fanControlMode[CHIP_MAX_NUMFAN];
    
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
    
    
    int lpcPortSel = 0;
    
    uint16_t chipAddr = 0;
    uint8_t fanDefaultControlMode[CHIP_MAX_NUMFAN];
    
    uint8_t readByte(uint16_t addr);
    uint16_t readWord(uint16_t addr);
    
    void writeByte(uint16_t addr, uint8_t val);
};

#endif /* ISSuperIONCT6683_hpp */
