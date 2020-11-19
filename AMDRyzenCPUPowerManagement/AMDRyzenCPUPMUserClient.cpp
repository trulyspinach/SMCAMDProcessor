//
//  AMDRyzenCPUPowerManagementUserClient.cpp
//  AMDRyzenCPUPowerManagement
//
//  Created by Qi HaoYan on 2/4/20.
//  Copyright © 2020 Qi HaoYan. All rights reserved.
//

#include "AMDRyzenCPUPMUserClient.hpp"



OSDefineMetaClassAndStructors(AMDRyzenCPUPMUserClient, IOUserClient);


bool AMDRyzenCPUPMUserClient::initWithTask(task_t owningTask,
                                             void *securityToken,
                                             UInt32 type,
                                             OSDictionary *properties){
    
    if(!IOUserClient::initWithTask(owningTask, securityToken, type, properties)){
        return false;
    }
    
    token = securityToken;
    
    proc_t proc = (proc_t)get_bsdtask_info(owningTask);
    proc_name(proc_pid(proc), taskProcessBinaryName, 32);
    clientAuthorizedByUser = false;
    

    
    return true;

}

bool AMDRyzenCPUPMUserClient::start(IOService *provider){
    
    IOLog("AMDCPUSupportUserClient::start\n");
    
    bool success = IOService::start(provider);
    
    if(success){
        fProvider = OSDynamicCast(AMDRyzenCPUPowerManagement, provider);
    }
    
    return success;
}

void AMDRyzenCPUPMUserClient::stop(IOService *provider){
    IOLog("AMDCPUSupportUserClient::stop\n");
    
    fProvider = nullptr;
    IOService::stop(provider);
}

//this is a meme, not getting the joke? nvm.
uint64_t multiply_two_numbers(uint64_t number_one, uint64_t number_two){
    uint64_t number_three = 0;
    for(uint32_t i = 0; i < number_two; i++){
        number_three = number_three + number_one;
    }
    return number_three;
}

bool AMDRyzenCPUPMUserClient::hasPrivilege(){
    if(fProvider->disablePrivilegeCheck) return true;
    if(clientHasPrivilege(token, kIOClientPrivilegeAdministrator) == kIOReturnSuccess) return true;
    if(clientAuthorizedByUser) return true;
    
    char buf[128];
    snprintf(buf, 128,
             "A process is trying to make changes to your system.\nAffected process name: %s\n\nAuthorize?",
             taskProcessBinaryName);
    
    unsigned int rf;
    (*(fProvider->kunc_alert))(0, 0, NULL, NULL, NULL,
                  "AMDRyzenCPUPowerManagement", buf, "Deny", "Until Process Terminate", "Once", &rf);
    
    
    if(rf == 1){
        clientAuthorizedByUser = true;
        return true;
    }
    
    if(rf == 2){
        return true;
    }
    
    return false;
}

IOReturn AMDRyzenCPUPMUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
                                                 IOExternalMethodDispatch *dispatch,
                                                   OSObject *target, void *reference){
    
    if (fProvider->kextloadAlerts) {
        unsigned int rf;
        
        char buf[128];
        snprintf(buf, 128,
                 "Kext alert detected: %d",
                 fProvider->kextloadAlerts);
        
        (*(fProvider->kunc_alert))(0, 0, NULL, NULL, NULL,
                      "AMDRyzenCPUPowerManagement", buf, "Ok", "Ok and Clear Alert", "WTF?", &rf);
        if(rf == 1){
            fProvider->kextloadAlerts = 0;
        }
    }
    
    fProvider->registerRequest();
    
    switch (selector) {
            
        //Get PStateDef raw values for core 0
        case 0: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->kMSR_PSTATE_LEN) * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->kMSR_PSTATE_LEN; i++){
                dataOut[i] = fProvider->PStateDef_perCore[i];
            }
            
            break;
        }
            
            
        //Get PStateDef floating point clock values for core 0
        case 1: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->kMSR_PSTATE_LEN) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->kMSR_PSTATE_LEN; i++){
                dataOut[i] = fProvider->PStateDefClock_perCore[i];
            }
            
            break;
        }
            
        case 2: {
            
            uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;

            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;
            
            arguments->structureOutputSize = numPhyCores * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;

            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i] = fProvider->effFreq_perCore[i];
            }
            
            break;
        }
        
        case 3: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 1 * sizeof(float);
            
            float *dataOut = (float*) arguments->structureOutput;
            dataOut[0] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            break;
        }
        
        //Get all data like this: [power, temp, pstateCur, clock_core_1, 2, 3 .....]
        //Yes, i am too lazy to write a struct
        case 4: {
            uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;
            
            arguments->structureOutputSize = (numPhyCores + 3) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            dataOut[0] = (float)fProvider->uniPackageEnergy;
            dataOut[1] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            dataOut[2] = fProvider->PStateCtl;
            
            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i + 3] = fProvider->effFreq_perCore[i];
            }
            
            break;
        }
            
        //Get per core raw load index
        case 5: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 1 * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = 0;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfLogicalCores; i++){
                dataOut[0] += fProvider->instructionDelta_PerCore[i];
            }
            
            break;
        }
            
        //Get per core load index
        case 6: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->totalNumberOfPhysicalCores) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            int lcpu_percore = fProvider->totalNumberOfLogicalCores / fProvider->totalNumberOfPhysicalCores;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfPhysicalCores; i++){
                float l = pmRyzen_avgload_pcpu(i * lcpu_percore);
//                dataOut[i] = fProvider->loadIndex_PerCore[i];
                dataOut[i] = l;
            }
            
            break;
        }
            
        //Get basic CPUID
        //[Family, Model, Physical, Logical, L1_perCore, L2_perCore, L3]
        case 7: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (8) * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = (uint64_t)fProvider->cpuFamily;
            dataOut[1] = (uint64_t)fProvider->cpuModel;
            dataOut[2] = (uint64_t)fProvider->totalNumberOfPhysicalCores;
            dataOut[3] = (uint64_t)fProvider->totalNumberOfLogicalCores;
            dataOut[4] = (uint64_t)fProvider->cpuCacheL1_perCore;
            dataOut[5] = (uint64_t)fProvider->cpuCacheL2_perCore;
            dataOut[6] = (uint64_t)fProvider->cpuCacheL3;
            dataOut[7] = (uint64_t)fProvider->cpuSupportedByCurrentVersion;
            
            break;
        }
        
        //Get AMDRyzenCPUPowerManagement Version String
        case 8: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = sizeof(xStringify(MODULE_VERSION));
            char *dataOut = (char*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < arguments->structureOutputSize; i++){
                dataOut[i] = xStringify(MODULE_VERSION)[i];
            }
            
            break;
        }
        
        //Get PState
        case 9: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = fProvider->PStateCtl;
            
            break;
        }
        
        //Set PState
        case 10: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            fProvider->PStateCtl = (uint8_t)arguments->scalarInput[0];
            fProvider->applyPowerControl();
            break;
        }
            
        //Get CPB
        case 11: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = 2 * sizeof(uint64_t);
            
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)fProvider->cpbSupported;
            dataOut[1] = (uint64_t)fProvider->getCPBState();
            break;
        }
        
        //Set CPB
        case 12: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            if(!fProvider->cpbSupported)
                return kIOReturnNoDevice;
            
            fProvider->setCPBState(arguments->scalarInput[0]==1?true:false);
            
            break;
        }
            
        //Get PPM
        case 13: {
            arguments->scalarOutputCount = 0;
                
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
                
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)(fProvider->getPMPStateLimit() == 0 ? 0 : 1);
            break;
        }
            
        //Set PPM
        case 14: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
                
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
                
            boolean_t enabled = arguments->scalarInput[0]==1?true:false;
            
            fProvider->setPMPStateLimit(enabled ? 1 : 0);
            
            break;
        }
            
        //Set PStateDef
        case 15: {
            if(!hasPrivilege())
                return kIOReturnNotPrivileged;
            
            if(arguments->scalarInputCount != 8)
                return kIOReturnBadArgument;
            
            
            fProvider->writePstate(arguments->scalarInput);
            
            break;
        }
            
        //get board info
        case 16: {
            //Let's give that one more try :)
            if(!fProvider->boardInfoValid)
                fProvider->fetchOEMBaseBoardInfo();
            
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = fProvider->boardInfoValid ? 1 : 0;
            
            arguments->structureOutputSize = 128;

            char *dataOut = (char*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < 64; i++){
                dataOut[i] = fProvider->boardVender[i];
            }
            
            for(uint32_t i = 0; i < 64; i++){
                dataOut[i+64] = fProvider->boardName[i];
            }
            
            break;
        }
            
        case 17: {
            arguments->scalarOutputCount = 0;
                
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
                
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)(fProvider->getHPcpus());
            break;
        }
        
        //Get LPM
        case 18: {
            arguments->scalarOutputCount = 0;
                
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
                
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;

            dataOut[0] = (uint64_t)(fProvider->getPMPStateLimit() == 2 ? 1 : 0);
            break;
        }
            
        //Set LPM
        case 19: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
                
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
                
            boolean_t enabled = arguments->scalarInput[0]==1?true:false;
            
            fProvider->setPMPStateLimit(enabled ? 2 : 1);
            
            break;
        }
        
        //Try load SMC driver
        case 90: {
            
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 2 * sizeof(uint64_t);
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            if(fProvider->superIO != nullptr){
                dataOut[0] = (uint64_t)(1);
                dataOut[1] = (uint64_t)(fProvider->savedSMCChipIntel);
                break;
            }
            
            uint16_t ci = 0;
            bool found = fProvider->initSuperIO(&ci);
            
            dataOut[0] = (uint64_t)(found ? 1 : 0);
            dataOut[1] = (uint64_t)(ci);
            break;
        }
        
        //SMC load number of fans
        case 91: {
            
            if(!fProvider->superIO)
                return kIOReturnNoDevice;

            
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 1 * sizeof(uint64_t);
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            dataOut[0] = (uint64_t)(fProvider->superIO->getNumberOfFans());
            break;
        }
        
        //SMC load readable desc for fan
        case 92: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;

            arguments->scalarOutputCount = 0;
                
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
                
            
            const char *str = fProvider->superIO->getReadableStringForFan((int)arguments->scalarInput[0]);
            arguments->structureOutputSize = (uint32_t)strlen(str);
            
            char *dataOut = (char*) arguments->structureOutput;
            strcpy(dataOut, str, strlen(str));
            
            
            break;
        }
            
        //SMC fan rpms
        case 93: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;
            
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = fProvider->superIO->getNumberOfFans() * sizeof(uint64_t);
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            fProvider->superIO->updateFanRPMS();
            for (int i = 0; i < fProvider->superIO->getNumberOfFans(); i++) {
                dataOut[i] = fProvider->superIO->getRPMForFan(i);
            }
            
            break;
        }
            
        default: {
            IOLog("AMDCPUSupportUserClient::externalMethod: invalid method.\n");
            break;
        }
        
        //SMC fan throttles and control mode
        case 94: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;
            
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = fProvider->superIO->getNumberOfFans() * sizeof(uint64_t);
            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            fProvider->superIO->updateFanControl();
            for (int i = 0; i < fProvider->superIO->getNumberOfFans(); i++) {
                dataOut[i] = fProvider->superIO->getFanThrottle(i) << 8 | (fProvider->superIO->getFanAutoControlMode(i) ? 1 : 0);
            }
            
            break;
        }
        
        //SMC fan overrride control
        case 95: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;
            
            if(!hasPrivilege())
                return kIOReturnNotPrivileged;
            
            if(arguments->scalarInputCount != 2)
                return kIOReturnBadArgument;
            
            int fanSel = (int)arguments->scalarInput[0];
            uint8_t pwm = (uint8_t)arguments->scalarInput[1];
            
            fProvider->superIO->overrideFanControl(fanSel, pwm);
            
            break;
        }
        
        //SMC fan default control
        case 96: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;
            
            if(!hasPrivilege())
                return kIOReturnNotPrivileged;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            int fanSel = (int)arguments->scalarInput[0];
            
            fProvider->superIO->setDefaultFanControl(fanSel);
            
            break;
        }
        
        //SMC Secret Undocumented feature (⁎⁍̴̛ᴗ⁍̴̛⁎)
        case 97: {
            if(!fProvider->superIO)
                return kIOReturnNoDevice;
            
            if(!hasPrivilege())
                return kIOReturnNotPrivileged;
            
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
            
            int numFan = fProvider->superIO->getNumberOfFans();
            for (int i = 0; i < numFan; i++) {
                if(arguments->scalarInput[0])
                    fProvider->superIO->overrideFanControl(i, 0xff);
                else
                    fProvider->superIO->setDefaultFanControl(i);
            }
            
            break;
        }
    }
    
    return kIOReturnSuccess;
}
