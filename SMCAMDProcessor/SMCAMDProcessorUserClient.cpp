//
//  SMCAMDProcessorUserClient.cpp
//  SMCAMDProcessor
//
//  Created by Qi HaoYan on 2/4/20.
//  Copyright © 2020 Qi HaoYan. All rights reserved.
//

#include "SMCAMDProcessorUserClient.hpp"



OSDefineMetaClassAndStructors(SMCAMDProcessorUserClient, IOUserClient);


bool SMCAMDProcessorUserClient::start(IOService *provider){
    
    IOLog("AMDCPUSupportUserClient::start\n");
    
    bool success = IOService::start(provider);
    
    if(success){
        fProvider = OSDynamicCast(SMCAMDProcessor, provider);
    }
    
    return success;
}

void SMCAMDProcessorUserClient::stop(IOService *provider){
    IOLog("AMDCPUSupportUserClient::stop\n");
    
    fProvider = nullptr;
    IOService::stop(provider);
}

uint64_t multiply_two_numbers(uint64_t number_one, uint64_t number_two){
    uint64_t number_three = 0;
    for(uint32_t i = 0; i < number_two; i++){
        number_three = number_three + number_one;
    }
    return number_three;
}

IOReturn SMCAMDProcessorUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments,
                                                 IOExternalMethodDispatch *dispatch, OSObject *target, void *reference){
    
    IOLog("AMDCPUSupportUserClient::externalMethod selector:%d\n", selector);
    
    
    switch (selector) {
        case 0: {
            // multiply_two_numbers
            uint64_t r = multiply_two_numbers(arguments->scalarInput[0], arguments->scalarInput[1]);
            arguments->scalarOutput[0] = r;
            arguments->scalarOutputCount = 1;
            
            IOLog("AMDCPUSupportUserClient::multiply_two_numbers r:%llu\n", r);
            
            break;
        }
        case 1: {
            // read_msr
//            IOLog("AMDCPUSupportUserClient::read_msr: got raw address %llu\n", arguments->scalarInput[0]);
            uint32_t msr_addr = (uint32_t)(arguments->scalarInput[0]);
            uint64_t msr_value_buf = 0;
            bool err = !fProvider->read_msr(msr_addr, &msr_value_buf);
            if(err){
                IOLog("AMDCPUSupportUserClient::read_msr: failed at address %u\n", msr_addr);
            } else {
                arguments->scalarOutput[0] = msr_value_buf;
                arguments->scalarOutputCount = 1;
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
                dataOut[i] = fProvider->MSR_HARDWARE_PSTATE_STATUS_perCore[i];
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
        
        //Get all data like this: [float power, float temperature, float clock_core_1, 2, 3 .....]
        //Yes, i am too lazy to write a struct
        case 4: {
            uint32_t numPhyCores = fProvider->totalNumberOfPhysicalCores;
            arguments->scalarOutputCount = 1;
            arguments->scalarOutput[0] = numPhyCores;
            
            arguments->structureOutputSize = (numPhyCores + 2) * sizeof(float);

            float *dataOut = (float*) arguments->structureOutput;
            
            dataOut[0] = (float)fProvider->uniPackageEnegry;
            dataOut[1] = fProvider->PACKAGE_TEMPERATURE_perPackage[0];
            
            for(uint32_t i = 0; i < numPhyCores; i++){
                dataOut[i + 2] = fProvider->MSR_HARDWARE_PSTATE_STATUS_perCore[i];
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
