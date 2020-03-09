#include "SMCAMDProcessor.hpp"
#include <string.h>

OSDefineMetaClassAndStructors(SMCAMDProcessor, IOService);

bool ADDPR(debugEnabled) = false;
uint32_t ADDPR(debugPrintDelay) = 0;

bool SMCAMDProcessor::init(OSDictionary *dictionary){
    IOLog("AMDCPUSupport v%s, %u, init\n", kMODULE_VERSION, (uint32_t)strlen(kMODULE_VERSION));
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
    
    CPUInfo::getCpuid(1, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuFamily = ((cpuid_eax >> 20) & 0xff) + ((cpuid_eax >> 8) & 0xf);
    cpuModel = ((cpuid_eax >> 16) & 0xf) + ((cpuid_eax >> 4) & 0xf);
    
    //Only 17h Family are supported offically by now.
    cpuSupportedByCurrentVersion = (cpuFamily == 0x17)? 1 : 0;
    IOLog("AMDCPUSupport::start Family %02Xh, Model %02Xh\n", cpuFamily, cpuModel);
    
    CPUInfo::getCpuid(0x80000005, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuCacheL1_perCore = (cpuid_ecx >> 24) + (cpuid_ecx >> 24);
    
    
    CPUInfo::getCpuid(0x80000006, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpuCacheL2_perCore = (cpuid_ecx >> 16);
    cpuCacheL3 = (cpuid_edx >> 18) * 512;
    IOLog("AMDCPUSupport::start L1: %u, L2: %u, L3: %u\n",
          cpuCacheL1_perCore, cpuCacheL2_perCore, cpuCacheL3);
    
    
    CPUInfo::getCpuid(0x80000007, 0, &cpuid_eax, &cpuid_ebx, &cpuid_ecx, &cpuid_edx);
    cpbSupported = (cpuid_edx >> 9) & 0x1;
    
    
    if(!CPUInfo::getCpuTopology(cpuTopology)){
        IOLog("AMDCPUSupport::start unable to get CPU Topology.\n");
    }
    IOLog("AMDCPUSupport::start got %hhu CPU(s): Physical Count: %hhu, Logical Count %hhu.\n",
          cpuTopology.packageCount, cpuTopology.totalPhysical(), cpuTopology.totalLogical());
    
    totalNumberOfPhysicalCores = cpuTopology.totalPhysical();
    totalNumberOfLogicalCores = cpuTopology.totalLogical();
    
    
    workLoop = IOWorkLoop::workLoop();
    timerEventSource = IOTimerEventSource::timerEventSource(this, [](OSObject *object, IOTimerEventSource *sender) {
        SMCAMDProcessor *provider = OSDynamicCast(SMCAMDProcessor, object);
        
        //Run initialization
        if(!provider->serviceInitialized){
            //Disable interrupts and sync all processor cores.
            mp_rendezvous_no_intrs([](void *obj) {
                auto provider = static_cast<SMCAMDProcessor*>(obj);
                
                
                uint64_t hwConfig;
                if(!provider->read_msr(kMSR_HWCR, &hwConfig))
                    panic("AMDCPUSupport::start: wtf?");
                
                hwConfig |= (1 << 30);
//                hwConfig &= ~(1 << 30);
                provider->write_msr(kMSR_HWCR, hwConfig);
                
                
                uint32_t cpu_num = cpu_number();
                
                // Ignore hyper-threaded cores
                uint8_t package = provider->cpuTopology.numberToPackage[cpu_num];
                uint8_t logical = provider->cpuTopology.numberToLogical[cpu_num];

                if (logical >= provider->cpuTopology.physicalCount[package])
                    return;
                uint8_t physical = provider->cpuTopology.numberToPhysicalUnique(cpu_num);
                
                //Read PStateDef generated by EFI.
                provider->dumpPstate(physical);
                
                //Init performance frequency counter.
                uint64_t APERF, MPERF;
                if(!provider->read_msr(kMSR_APERF, &APERF) || !provider->read_msr(kMSR_MPERF, &MPERF))
                    panic("AMDCPUSupport::start: wtf?");
                
                provider->lastAPERF_PerCore[physical] = APERF;
                provider->lastMPERF_PerCore[physical] = MPERF;
                
                
//                IOLog("core: %llu\n", hwConfig);
                
                //Make all cores P0 state by default.
                provider->PStateCtl = 0;
                
            }, provider);
            
            provider->serviceInitialized = true;
            provider->timerEventSource->setTimeoutMS(1);
            return;
        }
        
        
        mp_rendezvous_no_intrs([](void *obj) {
            auto provider = static_cast<SMCAMDProcessor*>(obj);
            uint32_t cpu_num = cpu_number();
            
            // Ignore hyper-threaded cores
            uint8_t package = provider->cpuTopology.numberToPackage[cpu_num];
            uint8_t logical = provider->cpuTopology.numberToLogical[cpu_num];
            if (logical >= provider->cpuTopology.physicalCount[package])
                return;
            uint8_t physical = provider->cpuTopology.numberToPhysicalUnique(cpu_num);
            
            
            provider->calculateEffectiveFrequency(physical);
            provider->updateInstructionDelta(physical);
            
        }, provider);
        
        if(provider->PPMEnabled) provider->updatePowerControl();
        
        //Read stats from package.
        provider->updatePackageTemp();
        provider->updatePackageEnergy();
        
        
        uint32_t now = uint32_t(getCurrentTimeNs() / 1000000); //ms
        uint32_t newInt = max(now - provider->timeOfLastMissedRequest,
                              provider->estimatedRequestTimeInterval);
        
        provider->actualUpdateTimeInterval = now - provider->timeOfLastUpdate;
        provider->timeOfLastUpdate = now;
        provider->updateTimeInterval = min(provider->PPMEnabled? 320 : 1200, max(50, newInt));
        
        provider->timerEventSource->setTimeoutMS(provider->updateTimeInterval);
//        IOLog("est time: %d\n", provider->estimatedRequestTimeInterval);
//        IOLog("update time: %d\n", provider->updateTimeInterval);
//        IOLog("Core %d: %llu\n", 0, (uint64_t)(provider->PStateCur_perCore[0]));
//        for (int i = 0; i < provider->totalNumberOfPhysicalCores; i++) {
//            IOLog("Core %d: %llu\n", i, (uint64_t)(provider->PStateCur_perCore[i]));
//        }
    });
    

    
    
    IOLog("AMDCPUSupport::start trying to init PCI service...\n");
    if(!getPCIService()){
        IOLog("AMDCPUSupport::start no PCI support found, failing...\n");
        return false;
    }
    
    
    lastUpdateTime = getCurrentTimeNs();
    
    workLoop->addEventSource(timerEventSource);
    timerEventSource->setTimeoutMS(1);
    
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

bool SMCAMDProcessor::write_msr(uint32_t addr, uint64_t value){
    wrmsr64(addr, value);
    return true;
}

void SMCAMDProcessor::registerRequest(){
    uint32_t now = (uint32_t)(getCurrentTimeNs() / 1000000);
    
    estimatedRequestTimeInterval = now - timeOfLastMissedRequest;
    timeOfLastMissedRequest = now;
}

void SMCAMDProcessor::updateClockSpeed(uint8_t physical){
    
    uint64_t msr_value_buf = 0;
    bool err = !read_msr(kMSR_HARDWARE_PSTATE_STATUS, &msr_value_buf);
    if(err) panic("AMDCPUSupport::updateClockSpeed: fucked up");
    
    //Convert register value to clock speed.
    uint32_t eax = (uint32_t)(msr_value_buf & 0xffffffff);
    
    // MSRC001_0293
    // CurHwPstate [24:22]
    // CurCpuVid [21:14]
    // CurCpuDfsId [13:8]
    // CurCpuFid [7:0]
    float curCpuDfsId = (float)((eax >> 8) & 0x3f);
    float curCpuFid = (float)(eax & 0xff);
    
    float clock = curCpuFid / curCpuDfsId * 200.0f;
    
//    PStateCur_perCore[physical] = curHwPstate;
    effFreq_perCore[physical] = clock;
    
    //    IOLog("AMDCPUSupport::updateClockSpeed: %u\n", curHwPstate);
}

void SMCAMDProcessor::calculateEffectiveFrequency(uint8_t physical){
    
    uint64_t APERF, MPERF;
    if(!read_msr(kMSR_APERF, &APERF) || !read_msr(kMSR_MPERF, &MPERF))
        panic("AMDCPUSupport::calculateEffectiveFrequency: fucked up");
    
    uint64_t lastAPERF = lastAPERF_PerCore[physical];
    uint64_t lastMPERF = lastMPERF_PerCore[physical];
    
    //If an overflow of either the MPERF or APERF register occurs between the read of last MPERF and the
    //read of last APERF, the effective frequency calculated in is invalid.
    //Yeah, so we will do nothing.
    if(APERF <= lastAPERF || MPERF <= lastMPERF) return;
    
    float freqP0 = PStateDefClock_perCore[physical][0];
    
    uint64_t deltaAPERF = APERF - lastAPERF;
    deltaAPERF_PerCore[physical] = deltaAPERF;
    deltaMPERF_PerCore[physical] = MPERF - lastMPERF;
    float effFreq = ((float)deltaAPERF / (float)(MPERF - lastMPERF)) * freqP0;
    
    //    IOLog("AMDCPUSupport::calculateEffectiveFrequency: core %u, %u\n", physical, (uint32_t)freqP0);
    
    effFreq_perCore[physical] = effFreq;
    
    lastAPERF_PerCore[physical] = APERF;
    lastMPERF_PerCore[physical] = MPERF;
}

void SMCAMDProcessor::updateInstructionDelta(uint8_t physical){
    uint64_t insCount;
    
    if(!read_msr(kMSR_PERF_IRPC, &insCount))
        panic("AMDCPUSupport::updateInstructionDelta: fucked up");
    
    
    //Skip if overflowed
    if(lastInstructionDelta_perCore[physical] > insCount) return;
    
    uint64_t delta = insCount - lastInstructionDelta_perCore[physical];
    instructionDelta_PerCore[physical] = insCount - lastInstructionDelta_perCore[physical];
    
    lastInstructionDelta_perCore[physical] = insCount;
    
    //write_msr(kMSR_PERF_IRPC, 0);
    
    
    //Calculate load index
    float estimatedInstRet = (effFreq_perCore[physical] * 1000000);
    estimatedInstRet = estimatedInstRet * (actualUpdateTimeInterval * 0.001);
    float index = (float)delta / estimatedInstRet;
    
    float growth = 3200;
    loadIndex_PerCore[physical] = log10f(min(index,1) * growth) / log10f(growth);
}

void SMCAMDProcessor::applyPowerControl(){
    mp_rendezvous_no_intrs([](void *obj) {
        auto provider = static_cast<SMCAMDProcessor*>(obj);
        provider->write_msr(kMSR_PSTATE_CTL, (uint64_t)(provider->PStateCtl & 0x7));
    }, this);
}

void SMCAMDProcessor::updatePowerControl(){
    //Passive PM
    float loadMax = 0;
    bool inc = false;
    int shouldStep = PStateCtl;
    for (int i = 0; i < totalNumberOfPhysicalCores; i++) {
        if(loadIndex_PerCore[i] >= PStateStepUpRatio){
            shouldStep = max(shouldStep - 1, 0);
            inc = true;
            break;
        }
        
        loadMax = max(loadMax, loadIndex_PerCore[i]);
    }
    if(!inc && loadMax <= PStateStepDownRatio)
        shouldStep = min(PStateCtl + 1, PStateEnabledLen-1);
    
    if(shouldStep == PStateCtl) return;
    
    PStateCtl = shouldStep;
    
    applyPowerControl();
}

void SMCAMDProcessor::setCPBState(bool enabled){
    if(!cpbSupported) return;
    
    uint64_t hwConfig;
    if(!read_msr(kMSR_HWCR, &hwConfig))
        panic("AMDCPUSupport::setCPBState: wtf?");
    
    if(enabled){
        hwConfig &= ~(1 << 25);
    } else {
        hwConfig |= (1 << 25);
    }
    
    //A bit hacky but at least works for now.
    void* args[] = {this, &hwConfig};
    
    mp_rendezvous_no_intrs([](void *obj) {
        auto v = static_cast<uint64_t*>(*((uint64_t**)obj+1));
        auto provider = static_cast<SMCAMDProcessor*>(obj);
        provider->write_msr(kMSR_HWCR, *v);
    }, args);
}

bool SMCAMDProcessor::getCPBState(){
    uint64_t hwConfig;
    if(!read_msr(kMSR_HWCR, &hwConfig))
        panic("AMDCPUSupport::start: wtf?");
    
    return !((hwConfig >> 25) & 0x1);
}

void SMCAMDProcessor::updatePackageTemp(){
    IOPCIAddressSpace space;
    space.bits = 0x00;
    
    fIOPCIDevice->configWrite32(space, (UInt8)kFAMILY_17H_PCI_CONTROL_REGISTER, (UInt32)kF17H_M01H_THM_TCON_CUR_TMP);
    uint32_t temperature = fIOPCIDevice->configRead32(space, kFAMILY_17H_PCI_CONTROL_REGISTER + 4);
    
    
    bool tempOffsetFlag = (temperature & kF17H_TEMP_OFFSET_FLAG) != 0;
    temperature = (temperature >> 21) * 125;
    
    float offset = 0.0f;
    float t = temperature * 0.001f;
    
    // Offset table: https://github.com/torvalds/linux/blob/master/drivers/hwmon/k10temp.c#L78
    uint32_t totalNumberOfPhysicalCores = cpuTopology.totalPhysical();
    
    if(tempOffsetFlag) {
        if (totalNumberOfPhysicalCores == 6) // 1600X,1700X,1800X
            offset = -20.0f;
        else if (totalNumberOfPhysicalCores == 8) //  2700X
            offset = -10.0f;
        else if  (totalNumberOfPhysicalCores == 12 || totalNumberOfPhysicalCores == 32) // TR1900,2900
            offset = -27.0f;
        
        if (tempOffsetFlag)
            t += -49.0f;
    }
    
    if (offset < 0)
        t += offset;
    
    
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

void SMCAMDProcessor::dumpPstate(uint8_t physical){
    
    uint8_t len = 0;
    for (uint32_t i = 0; i < kMSR_PSTATE_LEN; i++) {
        uint64_t msr_value_buf = 0;
        bool err = !read_msr(kMSR_PSTATE_0 + i, &msr_value_buf);
        if(err) panic("AMDCPUSupport::dumpPstate: fucked up");
        
        uint32_t eax = (uint32_t)(msr_value_buf & 0xffffffff);
        
        // CpuVid [21:14]
        // CpuDfsId [13:8]
        // CpuFid [7:0]
        int curCpuDfsId = (int)((eax >> 8) & 0x3f);
        int curCpuFid = (int)(eax & 0xff);
        
        float clock = (float)((float)curCpuFid / (float)curCpuDfsId * 200.0);
        
        PStateDef_perCore[physical][i] = msr_value_buf;
        PStateDefClock_perCore[physical][i] = clock;
        
        if(msr_value_buf & ((uint64_t)1 << 63)) len++;
        //        IOLog("a: %llu", msr_value_buf);
    }
    
    PStateEnabledLen = max(PStateEnabledLen, len);
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
