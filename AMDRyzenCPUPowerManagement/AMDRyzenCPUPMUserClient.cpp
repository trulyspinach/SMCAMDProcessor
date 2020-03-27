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
    
    
    char buf[128];
    snprintf(buf, 128,
             "A process is trying to make changes to your system.\n\nAffected process name: %s",
             taskProcessBinaryName);
    
    unsigned int rf;
    (*(fProvider->kunc_alert))(0, 0, NULL, NULL, NULL,
                  "AMDRyzenCPUPowerManagement", buf, "Deny", "I'm not sure.", "Authorize", &rf);
    
    
    if(rf == 2) return true;
    
    return false;
}

IOReturn AMDRyzenCPUPMUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
                                                 IOExternalMethodDispatch *dispatch,
                                                   OSObject *target, void *reference){
    
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
            
            dataOut[0] = (float)fProvider->uniPackageEnegry;
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
            
            arguments->structureOutputSize = (fProvider->totalNumberOfPhysicalCores) * sizeof(uint64_t);

            uint64_t *dataOut = (uint64_t*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfPhysicalCores; i++){
                dataOut[i] = fProvider->instructionDelta_PerCore[i];
            }
            
            break;
        }
            
        //Get per core load index
        case 6: {
            arguments->scalarOutputCount = 0;
            
            arguments->structureOutputSize = (fProvider->totalNumberOfPhysicalCores) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            for(uint32_t i = 0; i < fProvider->totalNumberOfPhysicalCores; i++){
                dataOut[i] = fProvider->loadIndex_PerCore[i];
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

            dataOut[0] = (uint64_t)fProvider->PPMEnabled;
            break;
        }
            
        //Set PPM
        case 14: {
            arguments->scalarOutputCount = 0;
            arguments->structureOutputSize = 0;
                
            if(arguments->scalarInputCount != 1)
                return kIOReturnBadArgument;
                
            fProvider->PPMEnabled = arguments->scalarInput[0]==1?true:false;
            
            if(!fProvider->PPMEnabled){
                fProvider->PStateCtl = 0;
                fProvider->applyPowerControl();
            }
            
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
            
        default: {
            IOLog("AMDCPUSupportUserClient::externalMethod: invalid method.\n");
            break;
        }
    }
    
    return kIOReturnSuccess;
}
