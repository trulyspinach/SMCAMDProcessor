#include "SMCAMDProcessor.hpp"


OSDefineMetaClassAndStructors(SMCAMDProcessor, IOService);

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

bool SMCAMDProcessor::init(OSDictionary *dictionary){
    
    IOLog("AMDCPUSupport got inited !!!!!!!!!!!\n");
    
    return IOService::init(dictionary);
}

void SMCAMDProcessor::free(){
    IOService::free();
}

bool SMCAMDProcessor::setupKeysVsmc(){
    
    vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
    
    
    bool suc = true;
    
//    suc &= VirtualSMCAPI::addKey(KeyTCxD(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxE(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxF(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
//    suc &= VirtualSMCAPI::addKey(KeyTCxG(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
//    suc &= VirtualSMCAPI::addKey(KeyTCxH(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxJ(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78));
    suc &= VirtualSMCAPI::addKey(KeyTCxP(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxT(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyTCxp(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TempPackage(this, 0)));
    
    
    suc &= VirtualSMCAPI::addKey(KeyPCPR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
    suc &= VirtualSMCAPI::addKey(KeyPSTR, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp96, new EnegryPackage(this, 0)));
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


bool SMCAMDProcessor::getPCIService(){
    

    OSDictionary *matching_dict = serviceMatching("IOPCIDevice");
    if(!matching_dict){
        IOLog("AMDCPUSupport::getPCIService: serviceMatching unable to generate matching dictonary.\n");
        return false;
    }
    
    //Wait for PCI services to init.
    waitForMatchingService(matching_dict);
    
    OSIterator *service_iter = getMatchingServices(matching_dict);
    IOPCIDevice *service = 0;
    
    if(!service_iter){
        IOLog("AMDCPUSupport::getPCIService: unable to find a matching IOPCIDevice.\n");
        return false;
    }
 
    while (true){
        OSObject *obj = service_iter->getNextObject();
        if(!obj) break;
        
        service = OSDynamicCast(IOPCIDevice, obj);
        break;
    }
    
    if(!service){
        IOLog("AMDCPUSupport::getPCIService: unable to get IOPCIDevice on host system.\n");
        return false;
    }
    
    IOLog("AMDCPUSupport::getPCIService: succeed!\n");
    fIOPCIDevice = service;
    
    return true;
}


bool SMCAMDProcessor::start(IOService *provider){
    
    bool success = IOService::start(provider);
    if(!success){
        IOLog("AMDCPUSupport::start failed to start. :(\n");
        return false;
    }
    registerService();
    
    cpuGeneration = CPUInfo::getGeneration(&cpuFamily, &cpuModel, &cpuStepping);
    
    uint32_t cpuid_eax = 0;
    uint32_t cpuid_ebx = 0;
    uint32_t cpuid_ecx = 0;
    uint32_t cpuid_edx = 0;
    CPUInfo::getCpuid(0, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    IOLog("AMDCPUSupport::start got CPUID: %X %X %X %X\n", cpuid_eax, cpuid_ebx, cpuid_ecx, cpuid_edx);
    
    if(cpuid_ebx != CPUInfo::signature_AMD_ebx
       || cpuid_ecx != CPUInfo::signature_AMD_ecx
       || cpuid_edx != CPUInfo::signature_AMD_edx){
        IOLog("AMDCPUSupport::start no AMD signature detected, failing..\n");
        
        return false;
    }
    
    workLoop = IOWorkLoop::workLoop();
    timerEventSource = IOTimerEventSource::timerEventSource(this, [](OSObject *object, IOTimerEventSource *sender) {
        SMCAMDProcessor *provider = OSDynamicCast(SMCAMDProcessor, object);
        
        
        mp_rendezvous_no_intrs([](void *obj) {
            auto provider = static_cast<SMCAMDProcessor*>(obj);
            
            //Read current clock speed from MSR for each core
            provider->updateClockSpeed();
        }, provider);
        
        //Read stats from package.
        provider->updatePackageTemp();
        provider->updatePackageEnergy();
        
        provider->timerEventSource->setTimeoutMS(1000);
    });
    
    if(!CPUInfo::getCpuTopology(cpuTopology)){
        IOLog("AMDCPUSupport::start unable to get CPU Topology.\n");
    }
    IOLog("AMDCPUSupport::start got %hhu CPU(s): Physical Count: %hhu, Logical Count %hhu.\n",
          cpuTopology.packageCount, cpuTopology.totalPhysical(), cpuTopology.totalLogical());
    
    totalNumberOfPhysicalCores = cpuTopology.totalPhysical();
    
    
    IOLog("AMDCPUSupport::start trying to init PCI service...\n");
    if(!getPCIService()){
        IOLog("AMDCPUSupport::start no PCI support found, failing...\n");
        return false;
    }
    
    
    lastUpdateTime = getCurrentTimeNs();
    
    workLoop->addEventSource(timerEventSource);
    timerEventSource->setTimeoutMS(1000);
    
    IOLog("AMDCPUSupport::start registering VirtualSMC keys...\n");
    setupKeysVsmc();
    
    return success;
}

void SMCAMDProcessor::stop(IOService *provider){
    IOLog("AMDCPUSupport stopped, you have no more support :(\n");
    
    timerEventSource->cancelTimeout();
    
    IOService::stop(provider);
}

bool SMCAMDProcessor::read_msr(uint32_t addr, uint64_t *value){
    
    uint32_t lo, hi;
//    IOLog("AMDCPUSupport lalala \n");
    int err = rdmsr_carefully(addr, &lo, &hi);
//    IOLog("AMDCPUSupport rdmsr_carefully %d\n", err);
    
    if(!err) *value = lo | ((uint64_t)hi << 32);
    
    return err == 0;
}

void SMCAMDProcessor::updateClockSpeed(){
    
    uint32_t cpu_num = cpu_number();
            
    // Ignore hyper-threaded cores
    uint8_t package = cpuTopology.numberToPackage[cpu_num];
    uint8_t logical = cpuTopology.numberToLogical[cpu_num];
    if (logical >= cpuTopology.physicalCount[package])
        return;
            
    uint8_t physical = cpuTopology.numberToPhysicalUnique(cpu_num);
            
    uint64_t msr_value_buf = 0;
    bool err = !read_msr(kMSR_HARDWARE_PSTATE_STATUS, &msr_value_buf);
    if(err) IOLog("AMDCPUSupport::updateClockSpeed: failed somewhere");
            
    //Convert register value to clock speed.
    uint32_t eax = (uint32_t)(msr_value_buf & 0xffffffff);
        
    // MSRC001_0293
    // CurHwPstate [24:22]
    // CurCpuVid [21:14]
    // CurCpuDfsId [13:8]
    // CurCpuFid [7:0]
    int curCpuDfsId = (int)((eax >> 8) & 0x3f);
    int curCpuFid = (int)(eax & 0xff);
    
    float clock = (float)(curCpuFid / (double)curCpuDfsId * 200.0);
    
    MSR_HARDWARE_PSTATE_STATUS_perCore[physical] = clock;
}

void SMCAMDProcessor::updatePackageTemp(){
    IOPCIAddressSpace space;
    space.bits = 0x00;
    
    fIOPCIDevice->configWrite32(space, (UInt8)kFAMILY_17H_PCI_CONTROL_REGISTER, (UInt32)kF17H_M01H_THM_TCON_CUR_TMP);
    uint32_t temperature = fIOPCIDevice->configRead32(space, kFAMILY_17H_PCI_CONTROL_REGISTER + 4);
    
    bool tempOffsetFlag = (temperature & kF17H_TEMP_OFFSET_FLAG) != 0;
    temperature = (temperature >> 21) * 125;

    float offset = 0.0f;

    // Offset table: https://github.com/torvalds/linux/blob/master/drivers/hwmon/k10temp.c#L78
    uint32_t totalNumberOfPhysicalCores = cpuTopology.totalPhysical();
    if (totalNumberOfPhysicalCores == 6) // 1600X,1700X,1800X
        offset = -20.0f;
    else if  (totalNumberOfPhysicalCores == 12 || totalNumberOfPhysicalCores == 32)// Threadripper 19,Threadripper 29
        offset = -27.0f;
    else if (totalNumberOfPhysicalCores == 8) //  2700X
        offset = -10.0f;

    float t = temperature * 0.001f;
    if (tempOffsetFlag)
        t += -49.0f;
    
    temperature += t; // substract the offset
    
    
    PACKAGE_TEMPERATURE_perPackage[0] = t;
}

void SMCAMDProcessor::updatePackageEnergy(){
    
    uint64_t time = getCurrentTimeNs();
    
    uint64_t msr_value_buf = 0;
    read_msr(kMSR_PKG_ENERGY_STAT, &msr_value_buf);
    
    uint32_t enegryValue = (uint32_t)(msr_value_buf & 0xffffffff);
    
    uint64_t enegryDelta = (lastUpdateEnergyValue <= enegryValue) ?
        enegryValue - lastUpdateEnergyValue : UINT64_MAX - lastUpdateEnergyValue;
    
    double e = (0.0000153 * enegryDelta) / ((time - lastUpdateTime) / 1000000000.0);
    uniPackageEnegry = e;
    
    lastUpdateEnergyValue = enegryValue;
    lastUpdateTime = time;
}

EXPORT extern "C" kern_return_t ADDPR(kern_start)(kmod_info_t *, void *) {
    // Report success but actually do not start and let I/O Kit unload us.
    // This works better and increases boot speed in some cases.
    PE_parse_boot_argn("liludelay", &ADDPR(debugPrintDelay), sizeof(ADDPR(debugPrintDelay)));
    ADDPR(debugEnabled) = checkKernelArgument("-amdcpudbg");
    return KERN_SUCCESS;
}

EXPORT extern "C" kern_return_t ADDPR(kern_stop)(kmod_info_t *, void *) {
    // It is not safe to unload VirtualSMC plugins!
    return KERN_FAILURE;
}


#ifdef __MAC_10_15

// macOS 10.15 adds Dispatch function to all OSObject instances and basically
// every header is now incompatible with 10.14 and earlier.
// Here we add a stub to permit older macOS versions to link.
// Note, this is done in both kern_util and plugin_start as plugins will not link
// to Lilu weak exports from vtable.

kern_return_t WEAKFUNC PRIVATE OSObject::Dispatch(const IORPC rpc) {
    PANIC("util", "OSObject::Dispatch smcproc stub called");
}

kern_return_t WEAKFUNC PRIVATE OSMetaClassBase::Dispatch(const IORPC rpc) {
    PANIC("util", "OSMetaClassBase::Dispatch smcproc stub called");
}

#endif
