#include "SMCAMDProcessor.hpp"


#include "KeyImplementations.hpp"

OSDefineMetaClassAndStructors(SMCAMDProcessor, IOService);


bool SMCAMDProcessor::setupKeysVsmc(){
    
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    
    
    
    bool suc = true;
    
    //    suc &= VirtualSMCAPI::addKey(KeyTCxD(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxE(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxF(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));
    //    suc &= VirtualSMCAPI::addKey(KeyTCxG(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
    //    suc &= VirtualSMCAPI::addKey(KeyTCxH(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxJ(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
    suc &= VirtualSMCAPI::addKey(KeyTCxP(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxT(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxp(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(fProvider, 0)));
    
    
    suc &= VirtualSMCAPI::addKey(KeyPCPR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(fProvider, 0)));
    suc &= VirtualSMCAPI::addKey(KeyPSTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(fProvider, 0)));
    //    suc &= VirtualSMCAPI::addKey(KeyPCPT, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    //    suc &= VirtualSMCAPI::addKey(KeyPCTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    
    
    //    VirtualSMCAPI::addKey(KeyPC0C, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPC0R, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPCAM, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPCPC, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    //
    //    VirtualSMCAPI::addKey(KeyPC0G, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPCGC, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPCGM, vsmcPlugin.data, VirtualSMCAPI::valueWithFlt(0, new EnegryPackage(this, 0)));
    //    VirtualSMCAPI::addKey(KeyPCPG, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    
    //Since AMD cpu dont have temperature MSR for each core, we simply report the same package temperature for all cores.
    //    for(int core = 0; core < totalNumberOfPhysicalCores; core++){
    //        VirtualSMCAPI::addKey(KeyTCxC(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(this, 0, core)));
    //        VirtualSMCAPI::addKey(KeyTCxc(core), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempCore(this, 0, core)));
    //    }
    
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
