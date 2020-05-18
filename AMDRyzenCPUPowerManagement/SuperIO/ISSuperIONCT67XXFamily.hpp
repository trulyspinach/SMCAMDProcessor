//
//  ISSuperIODevice.hpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 5/14/20.
//  Copyright © 2020 trulyspinach. All rights reserved.
//

#ifndef ISSuperIODevice_hpp
#define ISSuperIODevice_hpp

#include <IOKit/IOLib.h>

#include <architecture/i386/pio.h>

#include "ISLPCPort.h"
#include "ISSuperIOSMCFamily.hpp"

#define CHIP_NCT610XD 0xC452
#define CHIP_NCT6771F 0xB470
#define CHIP_NCT6776F 0xC330
#define CHIP_NCT6779D 0xC560
#define CHIP_NCT6791D 0xC803
#define CHIP_NCT6792D 0xC911
#define CHIP_NCT6792DA 0xC913
#define CHIP_NCT6793D 0xD121
#define CHIP_NCT6795D 0xD352
#define CHIP_NCT6796D 0xD423
#define CHIP_NCT6796DR 0xD42A
#define CHIP_NCT6797D 0xD451
#define CHIP_NCT6798D 0xD42B

#define NCT67XX_MAX_NUMFAN 7

#define CHIP_SIO_OPEN 0x87
#define CHIP_SIO_CLOSE 0xaa
#define CHIP_HWM_LDN 0x0b

#define CHIP_IO_SPACE_LOCK 0x28
#define CHIP_ADDR_REG_OFFSET 0x05
#define CHIP_DAT_REG_OFFSET 0x06
#define CHIP_BANK_SEL_REG 0x4E


class ISSuperIONCT67XXFamily : public ISSuperIOSMCFamily {

    
public:
    static constexpr const char *kFAN_READABLE_STRS[] = {
        "Pump",
        "CPU",
        "AUX_0",
        "AUX_1",
        "AUX_2",
        "AUX_3",
        "PECI"
    };
    
    static ISSuperIONCT67XXFamily* getDevice(uint16_t *chipIntel);
    
    
    ISSuperIONCT67XXFamily(int psel, uint16_t addr, uint16_t chipIntel);
    
    int fanRPMs[NCT67XX_MAX_NUMFAN];
    uint8_t fanThrottles[NCT67XX_MAX_NUMFAN];
    uint8_t fanControlMode[NCT67XX_MAX_NUMFAN];
    
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
    
    static constexpr uint16_t kFAN_RPM_REGS[] = { 0x4c0, 0x4c2, 0x4c4, 0x4c6, 0x4c8, 0x4ca, 0x4ce };
    static constexpr uint16_t kFAN_PWMCMD_REGS[] = { 0x109, 0x209, 0x309, 0x809, 0x909, 0xA09, 0xB09 };
    static constexpr uint16_t kFAN_CTRL_MODE_REGS[] = { 0x102, 0x202, 0x302, 0x802, 0x902, 0xA02, 0xB02 };
    
    
    
    int lpcPortSel = 0;
    
    uint16_t chipAddr = 0;
    uint8_t fanDefaultControlMode[NCT67XX_MAX_NUMFAN];
    
    uint8_t readByte(uint16_t addr);
    uint16_t readWord(uint16_t addr);
    
    void writeByte(uint16_t addr, uint8_t val);
};

#endif /* ISSuperIODevice_hpp */
