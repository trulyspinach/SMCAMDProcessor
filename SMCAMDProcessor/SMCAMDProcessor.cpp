#include "SMCAMDProcessor.hpp"


#include "KeyImplementations.hpp"

OSDefineMetaClassAndStructors(SMCAMDProcessor, IOService);


bool SMCAMDProcessor::setupKeysVsmc(){
    
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    
    
    
    bool suc = true;

    // [PCPR] type [sp9s] 73703936 len [ 2]: CPU package total power (SMC) (PCPR)
    suc &= VirtualSMCAPI::addKey(KeyPCPR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PSTR] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // System Total Power Consumed (Delayed 1 Second) in watts
    // This is a modern key present in e.g. MacBookPro12,1. Has many types.
    // suc &= VirtualSMCAPI::addKey(KeyPSTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new EnergyPackage(fProvider, 0)));

    // [PCPT] type [spa5] 73706135 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU package total power (PECI) in watts
    // This is a modern key present in e.g. MacPro6,1.
    suc &= VirtualSMCAPI::addKey(KeyPCPT, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PCTR] type [spa5] 73706135 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU total power (PC0C+PC0G) in watts
    // Also said that it may be comprised of PC0C+PC0G+PC0I+PC0M+PC0S.
    // This is a modern key present in e.g. MacPro6,1.
    // Only one key, regardless of CPU count.
    suc &= VirtualSMCAPI::addKey(KeyPCTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    //CPU 温度

    // [TC0D] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU die temperature in C°, 1 per physical CPU
    // This is a legacy key present in e.g. Macmini6,2.
    // No Mac models with more than 1 CPU were released with this key.
    // suc &= VirtualSMCAPI::addKey(KeyTCxD(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));

    // [TC0E] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU PECI Die filtered temp in C°, 1 per physical CPU
    // This is a modern key present in e.g. iMac16,1.
    // Filtered temperature is also called virtual temperature.
    suc &= VirtualSMCAPI::addKey(KeyTCxE(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));

    // [TC0F] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU PECI Die filtered and adjusted temp for fan/power control in C°, 1 per physical CPU
    // This is a modern key present in e.g. iMac16,1.
    suc &= VirtualSMCAPI::addKey(KeyTCxF(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));

    // [TC0G] type [sp78] 73703738 len [ 2] attr [C1] -> ATTR_PRIVATE_WRITE|ATTR_WRITE|ATTR_READ
    // CPU PECI Die temperature adjustment in C°, 1 per physical CPU
    // This is a modern key only present in Macmini7,1.
    // Writable, might be of some use in overclocking(?)
    suc &= VirtualSMCAPI::addKey(KeyTCxG(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));

    // [TC0H] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Heatsink Temperature, 1 per physical CPU
    // This is a legacy key present in e.g. iMac12,2.
    // No Mac models with more than 1 CPU were released with this key.
    // suc &= VirtualSMCAPI::addKey(KeyTCxH(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));

    // [TC0J] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU PECI die temp max error filtered output used in TC0F=TC0E+TC0G in C°, 1 per physical CPU
    // This is a modern key present in e.g. iMac16,1
    // No Mac models with more than 1 CPU were released with this key, probably appeared in later updates.
    suc &= VirtualSMCAPI::addKey(KeyTCxJ(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));

    // [TC0P] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Proximity Temperature in C°, 1 per physical CPU
    // This is a modern key present in e.g. MacPro6,1
    // MacPro6,1 has 2 keys due to two CPUs present
    suc &= VirtualSMCAPI::addKey(KeyTCxP(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));

    // [TC0T] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU PECI Die temp Trend in C°, 1 per physical CPU
    // This is a modern key present in e.g. iMac16,1
    // No Mac models with more than 1 CPU were released with this key.
    suc &= VirtualSMCAPI::addKey(KeyTCxT(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));

    // [TC0p] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Proximity raw temp in C°, 1 per physical CPU
    // This is a modern key present in e.g. iMac16,1
    // Compared to [TC0P] the value is not filtered anyhow.
    suc &= VirtualSMCAPI::addKey(KeyTCxp(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));

    // [PC0G] type [sp96] 73703936 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU AXG low-side power in watts (IGPU)
    // This is a modern key present in e.g. iMac17,1.
    // VirtualSMCAPI::addKey(KeyPC0G, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PC0I] type [sp96] 73703936 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU I/O high-side power in watts
    // This is a modern key present in e.g. iMac17,1.
    // VirtualSMCAPI::addKey(KeyPC0I, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));


    // [PC0M] type [sp96] 73703936 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU I/O high-side power in watts
    // This is a modern key present in e.g. iMac17,1.
    // VirtualSMCAPI::addKey(KeyPC0M, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PC0R] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // Average CPU High side power (IC0R * VD0R) in watts
    // Also known as PBUS CPU Highside or 
    // This is a modern key present in e.g. iMac17,1.
    // VirtualSMCAPI::addKey(KeyPC0R, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new EnergyPackage(fProvider, 0)));

    // [PC0S] type [sp96] 73703936 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU System Agent power in watts
    // This is a modern key present in e.g. iMac17,1.
    // VirtualSMCAPI::addKey(KeyPC0S, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PCAC] type [flt ] 666C7420 len [ 4] attr [C4] -> ATTR_ATOMIC|ATTR_WRITE|ATTR_READ
    // CPU core in watts
    // This is a modern key present in e.g. MacBookPro13,2.
    // VirtualSMCAPI::addKey(KeyPCAC, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnergyPackage(fProvider, 0)));


    // [PCAM] type [flt ] 666C7420 len [ 4] attr [C4] -> ATTR_ATOMIC|ATTR_WRITE|ATTR_READ
    // CPU core (IMON) in watts
    // This is a modern key present in e.g. MacBookPro13,2.
    // VirtualSMCAPI::addKey(KeyPCAM, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnergyPackage(fProvider, 0)));

    // [PCPC] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU package core power (PECI) in watts
    // This is a modern key present in e.g. MacBookPro12,1. Has many types.
    // VirtualSMCAPI::addKey(KeyPCPC, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new EnergyPackage(fProvider, 0)));


    // [PCPG] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU package Gfx power (PECI) in watts
    // This is a modern key present in e.g. MacBookPro12,1. Has many types.
    // VirtualSMCAPI::addKey(KeyPCPG, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new EnergyPackage(fProvider, 0)));


    // [PCSC] type [flt ] 666C7420 len [ 4] attr [C4] -> ATTR_ATOMIC|ATTR_WRITE|ATTR_READ
    // CPU VCCSA Power in watts
    // This is a modern key present in e.g. MacBookPro13,1. Replaces [PC2C] most likely.
    // VirtualSMCAPI::addKey(KeyPCSC, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnergyPackage(fProvider, 0)));

    

    // [PCGC] type [flt ] 666C7420 len [ 4] attr [C4] -> ATTR_ATOMIC|ATTR_WRITE|ATTR_READ
    // Intel GPU (IMON) power in watts
    // This is a modern key present in e.g. iMac18,1 (updated??). Replaces [PC2C] most likely.
    // VirtualSMCAPI::addKey(KeyPCGC, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnergyPackage(fProvider, 0)));

    // [PCGM] type [flt ] 666C7420 len [ 4] attr [C4] -> ATTR_ATOMIC|ATTR_WRITE|ATTR_READ
    // Intel GPU (IMON) power in watts
    // This is a modern key present in e.g. MacBookPro13,2. Replaces [PC2C] most likely.
    // VirtualSMCAPI::addKey(KeyPCGM, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnergyPackage(fProvider, 0)));
    
    // Since AMD cpu dont have temperature MSR for each core, we simply report the same package temperature for all cores.
    // [TC0C] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Core Temperature from PECI in C°, 1 per physical core
    // This is a legacy key present in e.g. iMac11,3.
    // No Mac models with more than 4 CPU cores were released with this key.

    // [TC0c] type [sp78] 73703738 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Core Temperature from PECI in C°, 1 per physical core
    // This is a modern key present in e.g. iMacPro1,1.
    // No Mac models with more than 18 CPU cores were released with this key, but no dumps with more than 10 cores are online.
    //Most likely the numeration follows with alphabetic characters.

    // [PC0C] type [spa5] 73706135 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Core power in watts
    // Basically comprised of IC0C * VC0C.
    // This is a modern key present in e.g. MacPro6,1.
    // Only one key, regardless of CPU count, [PC1C] is a different key.
    // VirtualSMCAPI::addKey(KeyPC0C, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, 0)));

    // [PC0c] type [ui16] 75693136 len [ 2] attr [C0] -> ATTR_WRITE|ATTR_READ
    // CPU Raw Package power, raw ADC input value.
    // This is a legacy key present in e.g. MacBookAir3,1.
    for(int core = 0; core <= fProvider->totalNumberOfPhysicalCores; core++){
        // VirtualSMCAPI::addKey(KeyTCxC(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, core)));
        VirtualSMCAPI::addKey(KeyTCxc(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, core)));
        // VirtualSMCAPI::addKey(KeyPCxC(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, core)));
        // VirtualSMCAPI::addKey(KeyPCxc(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnergyPackage(fProvider, core)));
    }
    VirtualSMCAPI::addKey(KeyTGxD(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    VirtualSMCAPI::addKey(KeyTGxP(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    // VirtualSMCAPI::addKey(KeyTGxd(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    // VirtualSMCAPI::addKey(KeyTGxp(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    // VirtualSMCAPI::addKey(KeyTGDD, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    // VirtualSMCAPI::addKey(KeyTCGC, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(fProvider, 0)));
    
    if(!suc){
        IOLog("AMDCPUSupport::setupKeysVsmc: VirtualSMCAPI::addKey returned false. \n");
    } else {
        IOLog("AMDCPUSupport::setupKeysVsmc: VirtualSMCAPI::addKey succeed!. \n");
    }
    
    return suc;
}

bool SMCAMDProcessor::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
    if (sensors && vsmc) {
        IOLog("AMDCPUSupport: got vsmc notification\n");
        auto &plugin = static_cast<SMCAMDProcessor *>(sensors)->vsmcPlugin;
        auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
        if (ret == kIOReturnSuccess) {
            IOLog("AMDCPUSupport: submitted plugin\n");
            return true;
        } else if (ret != kIOReturnUnsupported) {
            IOLog("AMDCPUSupport: plugin submission failure %X\n", ret);
        } else {
            IOLog("AMDCPUSupport: plugin submission to non vsmc\n");
        }
    } else {
        IOLog("AMDCPUSupport: got null vsmc notification\n");
    }
    return false;
}

bool SMCAMDProcessor::init(OSDictionary *dictionary){
    return IOService::init(dictionary);
}

void SMCAMDProcessor::free(void){
    
}

bool SMCAMDProcessor::start(IOService *provider){
    
    if(!IOService::start(provider))
        return false;
    
    fProvider = OSDynamicCast(AMDRyzenCPUPowerManagement, provider);
    if(!fProvider)
        return false;
    
    
    IOLog("SMCAMDProcessor: inited, registering VirtualSMC keys...\n");
    
    setupKeysVsmc();
    
    return true;
}

void SMCAMDProcessor::stop(IOService *provider){
    
}

