#ifndef SMCAMDProcessor_h
#define SMCAMDProcessor_h

//Support for macOS 10.13
#include <Library/LegacyIOService.h>

//#include <IOKit/IOService.h>
//#include <IOKit/IOLib.h>

#include <math.h>
#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOTimerEventSource.h>

#include <i386/proc_reg.h>
#include <libkern/libkern.h>


#include <Headers/kern_util.hpp>
#include <Headers/kern_cpu.hpp>
#include <Headers/kern_time.hpp>

#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>

#include "KeyImplementations.hpp"

#include "symresolver/kernel_resolver.h"


extern "C" {
    int cpu_number(void);
    void mp_rendezvous_no_intrs(void (*action_func)(void *), void *arg);

    void
    mp_rendezvous(void (*setup_func)(void *),
                  void (*action_func)(void *),
                  void (*teardown_func)(void *),
                  void *arg);

//    int wrmsr_carefully(uint32_t msr, uint64_t val);
};



class SMCAMDProcessor : public IOService {
    OSDeclareDefaultStructors(SMCAMDProcessor)
    
    /**
     *  VirtualSMC service registration notifier
     */
    IONotifier *vsmcNotifier {nullptr};
    
    static bool vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier);
    
    /**
     *  Registered plugin instance
     */
    VirtualSMCAPI::Plugin vsmcPlugin {
        xStringify(PRODUCT_NAME),
        parseModuleVersion(xStringify(MODULE_VERSION)),
        VirtualSMCAPI::Version,
    };
    
    
public:
    
    static constexpr char const *kMODULE_VERSION = xStringify(MODULE_VERSION);
    
    
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
    static constexpr uint32_t kMSR_PWR_UNIT = 0xC0010299;
    static constexpr uint32_t kMSR_MPERF = 0x000000E7;
    static constexpr uint32_t kMSR_APERF = 0x000000E8;
    static constexpr uint32_t kMSR_PERF_CTL_0 = 0xC0010000;
    static constexpr uint32_t kMSR_PERF_CTR_0 = 0xC0010004;
    static constexpr uint32_t kMSR_PERF_IRPC = 0xC00000E9;
    

    
    /**
     *  Key name index mapping
     */
    static constexpr size_t MaxIndexCount = sizeof("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") - 1;
    static constexpr const char *KeyIndexes = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    
    
    /**
     *  Supported SMC keys
     */
    static constexpr SMC_KEY KeyPC0C = SMC_MAKE_IDENTIFIER('P','C','0','C');
    static constexpr SMC_KEY KeyPC0G = SMC_MAKE_IDENTIFIER('P','C','0','G');
    static constexpr SMC_KEY KeyPC0R = SMC_MAKE_IDENTIFIER('P','C','0','R');
    static constexpr SMC_KEY KeyPC3C = SMC_MAKE_IDENTIFIER('P','C','3','C');
    static constexpr SMC_KEY KeyPCAC = SMC_MAKE_IDENTIFIER('P','C','A','C');
    static constexpr SMC_KEY KeyPCAM = SMC_MAKE_IDENTIFIER('P','C','A','M');
    static constexpr SMC_KEY KeyPCEC = SMC_MAKE_IDENTIFIER('P','C','E','C');
    static constexpr SMC_KEY KeyPCGC = SMC_MAKE_IDENTIFIER('P','C','G','C');
    static constexpr SMC_KEY KeyPCGM = SMC_MAKE_IDENTIFIER('P','C','G','M');
    static constexpr SMC_KEY KeyPCPC = SMC_MAKE_IDENTIFIER('P','C','P','C');
    static constexpr SMC_KEY KeyPCPG = SMC_MAKE_IDENTIFIER('P','C','P','G');
    static constexpr SMC_KEY KeyPCPR = SMC_MAKE_IDENTIFIER('P','C','P','R');
    static constexpr SMC_KEY KeyPSTR = SMC_MAKE_IDENTIFIER('P','S','T','R');
    static constexpr SMC_KEY KeyPCPT = SMC_MAKE_IDENTIFIER('P','C','P','T');
    static constexpr SMC_KEY KeyPCTR = SMC_MAKE_IDENTIFIER('P','C','T','R');
    static constexpr SMC_KEY KeyTCxD(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'D'); }
    static constexpr SMC_KEY KeyTCxE(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'E'); }
    static constexpr SMC_KEY KeyTCxF(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'F'); }
    static constexpr SMC_KEY KeyTCxG(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'G'); }
    static constexpr SMC_KEY KeyTCxJ(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'J'); }
    static constexpr SMC_KEY KeyTCxH(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'H'); }
    static constexpr SMC_KEY KeyTCxP(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'P'); }
    static constexpr SMC_KEY KeyTCxT(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'T'); }
    static constexpr SMC_KEY KeyTCxp(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'p'); }
    
    static constexpr SMC_KEY KeyTCxC(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'C'); }
    static constexpr SMC_KEY KeyTCxc(size_t i) { return SMC_MAKE_IDENTIFIER('T','C',KeyIndexes[i],'c'); }
    

    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    

    bool read_msr(uint32_t addr, uint64_t *value);
    bool write_msr(uint32_t addr, uint64_t value);
    
    
    void updateClockSpeed(uint8_t physical);
    void calculateEffectiveFrequency(uint8_t physical);
    void updateInstructionDelta(uint8_t physical);
    void applyPowerControl();
    void updatePowerControl();
    
    void setCPBState(bool enabled);
    bool getCPBState();
    
    void updatePackageTemp();
    void updatePackageEnergy();
    
    void registerRequest();
    
    void dumpPstate(uint8_t cpu_num);
    void writePstate(const uint64_t *buf);
    
    uint32_t totalNumberOfPhysicalCores;
    uint32_t totalNumberOfLogicalCores;
    
    uint8_t cpuFamily;
    uint8_t cpuModel;
    uint8_t cpuSupportedByCurrentVersion;
    
    //Cache size in KB
    uint32_t cpuCacheL1_perCore;
    uint32_t cpuCacheL2_perCore;
    uint32_t cpuCacheL3;
    
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
    
    bool PPMEnabled = false;
    float PStateStepUpRatio = 0.36;
    float PStateStepDownRatio = 0.05;
    
    uint8_t PStateCur_perCore[CPUInfo::MaxCpus];
    uint8_t PStateCtl = 0;
    uint64_t PStateDef_perCore[CPUInfo::MaxCpus][8];
    uint8_t PStateEnabledLen = 0;
    float PStateDefClock_perCore[CPUInfo::MaxCpus][8];
    bool cpbSupported;
    
    
    uint64_t lastUpdateTime;
    uint64_t lastUpdateEnergyValue;
    
    double uniPackageEnegry;
    
    
    bool disablePrivilegeCheck = false;

    
    
private:
    
    IOWorkLoop *workLoop;
    IOTimerEventSource *timerEventSource;
    IOSimpleLock *mpLock {nullptr};
    
    bool serviceInitialized = false;
    
    uint32_t updateTimeInterval = 1000;
    uint32_t actualUpdateTimeInterval = 1;
    uint32_t timeOfLastUpdate = 0;
    uint32_t estimatedRequestTimeInterval = 0;
    uint32_t timeOfLastMissedRequest = 0;
    
    
    int (*wrmsr_carefully)(uint32_t, uint32_t, uint32_t) {nullptr};
    
    CPUInfo::CpuTopology cpuTopology {};
    
    IOPCIDevice *fIOPCIDevice;
    
    bool setupKeysVsmc();
    bool getPCIService();
    
};
#endif
