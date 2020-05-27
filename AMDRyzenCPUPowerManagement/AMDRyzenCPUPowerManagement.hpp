#ifndef AMDRyzenCPUPowerManagement_h
#define AMDRyzenCPUPowerManagement_h

//Support for macOS 10.13
#include <Library/LegacyIOService.h>

#include <math.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>


#include <i386/proc_reg.h>
#include <libkern/libkern.h>

#include <Headers/kern_efi.hpp>

#include <Headers/kern_util.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_time.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>


#include "symresolver/kernel_resolver.h"

#include "SuperIO/ISSuperIONCT668X.hpp"
#include "SuperIO/ISSuperIONCT67XXFamily.hpp"
#include "SuperIO/ISSuperIOIT86XXE.hpp"

#include <i386/cpuid.h>

#define OC_OEM_VENDOR_VARIABLE_NAME        u"oem-vendor"
#define OC_OEM_BOARD_VARIABLE_NAME         u"oem-board"

#define BASEBOARD_STRING_MAX 64

#define kNrOfPowerStates 2
#define kIOPMPowerOff 0

extern "C" {
#include "pmAMDRyzen.h"

#include "Headers/osfmk/i386/pmCPU.h"
#include "Headers/osfmk/i386/cpu_topology.h"
    

int cpu_number(void);
void mp_rendezvous_no_intrs(void (*action_func)(void *), void *arg);

void mp_rendezvous(void (*setup_func)(void *),
                  void (*action_func)(void *),
                  void (*teardown_func)(void *),
                  void *arg);

void i386_deactivate_cpu(void);
//    int wrmsr_carefully(uint32_t msr, uint64_t val);


void pmRyzen_wrmsr_safe(void *, uint32_t, uint64_t);
uint64_t pmRyzen_rdmsr_safe(void *, uint32_t);

};


/**
 * Offset table: https://github.com/torvalds/linux/blob/master/drivers/hwmon/k10temp.c#L78
 */
typedef struct tctl_offset {
    uint8_t model;
    char const *id;
    int offset;
} TempOffset;


static IOPMPowerState powerStates[kNrOfPowerStates] = {
   {1, kIOPMPowerOff, kIOPMPowerOff, kIOPMPowerOff, 0, 0, 0, 0, 0, 0, 0, 0},
   {1, kIOPMPowerOn, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};


class AMDRyzenCPUPowerManagement : public IOService {
    OSDeclareDefaultStructors(AMDRyzenCPUPowerManagement)
    
public:
    
    char kMODULE_VERSION[12]{};
    
    /**
     *  MSRs supported by AMD 17h CPU from:
     *  https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitorLib/Hardware/Cpu/Amd17Cpu.cs
     * and
     * Processor Programming Reference for AMD 17h CPU,
     *
     */
    
    static constexpr uint32_t kCOFVID_STATUS = 0xC0010071;
    static constexpr uint32_t k17H_M01H_SVI = 0x0005A000;
    static constexpr uint32_t kF17H_M01H_THM_TCON_CUR_TMP = 0x00059800;
    static constexpr uint32_t kF17H_M70H_CCD1_TEMP = 0x00059954;
    static constexpr uint32_t kF17H_TEMP_OFFSET_FLAG = 0x80000;
    static constexpr uint32_t kF18H_TEMP_OFFSET_FLAG = 0x60000;
    static constexpr uint8_t kFAMILY_17H_PCI_CONTROL_REGISTER = 0x60;
    static constexpr uint32_t kMSR_HWCR = 0xC0010015;
    static constexpr uint32_t kMSR_CORE_ENERGY_STAT = 0xC001029A;
    static constexpr uint32_t kMSR_HARDWARE_PSTATE_STATUS = 0xC0010293;
    static constexpr uint32_t kMSR_PKG_ENERGY_STAT = 0xC001029B;
    static constexpr uint32_t kMSR_PSTATE_0 = 0xC0010064;
    static constexpr uint32_t kMSR_PSTATE_LEN = 8;
    static constexpr uint32_t kMSR_PSTATE_STAT = 0xC0010063;
    static constexpr uint32_t kMSR_PSTATE_CTL = 0xC0010062;
    static constexpr uint32_t kMSR_RAPL_PWR_UNIT = 0xC0010299;
    static constexpr uint32_t kMSR_MPERF = 0x000000E7;
    static constexpr uint32_t kMSR_APERF = 0x000000E8;
    static constexpr uint32_t kMSR_PERF_CTL_0 = 0xC0010000;
    static constexpr uint32_t kMSR_PERF_CTR_0 = 0xC0010004;
    static constexpr uint32_t kMSR_PERF_IRPC = 0xC00000E9;
    static constexpr uint32_t kMSR_CSTATE_ADDR = 0xC0010073;
    
    
//    static constexpr uint32_t EF = 0x88;
    
    static constexpr uint32_t kEFI_VARIABLE_NON_VOLATILE = 0x00000001;
    static constexpr uint32_t kEFI_VARIABLE_BOOTSERVICE_ACCESS = 0x00000002;
    static constexpr uint32_t kEFI_VARIABLE_RUNTIME_ACCESS = 0x00000004;
    
    

    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService* whatDevice) override;
    
    void fetchOEMBaseBoardInfo();

    bool read_msr(uint32_t addr, uint64_t *value);
    bool write_msr(uint32_t addr, uint64_t value);
    
    
    void updateClockSpeed(uint8_t physical);
    void calculateEffectiveFrequency(uint8_t physical);
    void updateInstructionDelta(uint8_t physical);
    void applyPowerControl();
    
    void setCPBState(bool enabled);
    bool getCPBState();
    
    void updatePackageTemp();
    void updatePackageEnergy();
    
    void registerRequest();
    
    void dumpPstate();
    void writePstate(const uint64_t *buf);
    
    bool initSuperIO(uint16_t* chipIntel);
    
    uint32_t getPMPStateLimit();
    void setPMPStateLimit(uint32_t);
    
    uint32_t getHPcpus();
    
    uint32_t totalNumberOfPhysicalCores;
    uint32_t totalNumberOfLogicalCores;
    
    uint8_t cpuFamily;
    uint8_t cpuModel;
    uint8_t cpuSupportedByCurrentVersion;
    
    //Cache size in KB
    uint32_t cpuCacheL1_perCore;
    uint32_t cpuCacheL2_perCore;
    uint32_t cpuCacheL3;
    
    char boardVender[BASEBOARD_STRING_MAX]{};
    char boardName[BASEBOARD_STRING_MAX]{};
    bool boardInfoValid = false;
    
    
    /**
     *  Hard allocate space for cached readings.
     */
    float effFreq_perCore[CPUInfo::MaxCpus] {};
    float PACKAGE_TEMPERATURE_perPackage[CPUInfo::MaxCpus];
    
    uint64_t lastMPERF_PerCore[CPUInfo::MaxCpus];
    uint64_t lastAPERF_PerCore[CPUInfo::MaxCpus];
    uint64_t deltaAPERF_PerCore[CPUInfo::MaxCpus];
    uint64_t deltaMPERF_PerCore[CPUInfo::MaxCpus];
    
//    uint64_t lastAPERF_PerCore[CPUInfo::MaxCpus];
    
    uint64_t instructionDelta_PerCore[CPUInfo::MaxCpus];
    uint64_t lastInstructionDelta_perCore[CPUInfo::MaxCpus];
    
    float loadIndex_PerCore[CPUInfo::MaxCpus];
    
    float PStateStepUpRatio = 0.36;
    float PStateStepDownRatio = 0.05;
    
    uint8_t PStateCur_perCore[CPUInfo::MaxCpus];
    uint8_t PStateCtl = 0;
    uint64_t PStateDef_perCore[8];
    uint8_t PStateEnabledLen = 0;
    float PStateDefClock_perCore[8];
    bool cpbSupported;
    
    
    uint64_t lastUpdateTime;
    uint64_t lastUpdateEnergyValue;
    
    double uniPackageEnergy;
    
    bool disablePrivilegeCheck = false;
    uint16_t savedSMCChipIntel = 0;

    kern_return_t (*kunc_alert)(int,unsigned,const char*,const char*,const char*,
                                const char*,const char*,const char*,const char*,const char*,unsigned*) {nullptr};
    
    
    ISSuperIOSMCFamily *superIO{nullptr};
    
private:
    IOWorkLoop *workLoop;
    IOTimerEventSource *timerEventSource;
    
    bool serviceInitialized = false;
    
    uint32_t updateTimeInterval = 1000;
    uint32_t actualUpdateTimeInterval = 1;
    uint32_t timeOfLastUpdate = 0;
    uint32_t estimatedRequestTimeInterval = 0;
    uint32_t timeOfLastMissedRequest = 0;
    
    float tempOffset = 0;
    double pwrTimeUnit = 0;
    double pwrEnergyUnit = 0;
    uint64_t pwrLastTSC = 0;
    
    uint64_t xnuTSCFreq = 1;
    int (*wrmsr_carefully)(uint32_t, uint32_t, uint32_t) {nullptr};
    processor_t(*cpu_to_processor)(int);
    kern_return_t(*processor_shutdown)(processor_t);
    kern_return_t(*processor_startup)(processor_t);
    
    CPUInfo::CpuTopology cpuTopology {};
    
    IOPCIDevice *fIOPCIDevice;
    bool getPCIService();
    bool wentToSleep;
    
    void startWorkLoop();
    void stopWorkLoop();
};
#endif
