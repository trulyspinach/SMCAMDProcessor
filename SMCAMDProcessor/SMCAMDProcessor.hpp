
#ifndef SMCAMDProcessor_h
#define SMCAMDProcessor_h


//Support for macOS 10.13
#include <Library/LegacyIOService.h>
#include <IOKit/IOLib.h>

#include <AMDRyzenCPUPowerManagement.hpp>

#undef EFIAPI   // must place here!
#include <VirtualSMCSDK/kern_vsmcapi.hpp>
#include <VirtualSMCSDK/AppleSmc.h>
#define EFIAPI

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
    
public:
    
    virtual bool init(OSDictionary *dictionary = 0) override;
    virtual void free(void) override;
    
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    
    
private:
    
    bool setupKeysVsmc();
    AMDRyzenCPUPowerManagement *fProvider;
    
};


#endif
