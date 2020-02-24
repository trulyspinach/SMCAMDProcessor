//
//  KeyImplementations.cpp
//  SMCAMDProcessor
//
//  Created by Qi HaoYan on 2/12/20.
//  Copyright © 2020 Qi HaoYan. All rights reserved.
//

#include "KeyImplementations.hpp"


SMC_RESULT TempPackage::readAccess() {
    uint16_t *ptr = reinterpret_cast<uint16_t *>(data);
    *ptr = VirtualSMCAPI::encodeSp(type, (double)provider->PACKAGE_TEMPERATURE_perPackage[0]);

    return SmcSuccess;
}

SMC_RESULT TempCore::readAccess() {
    uint16_t *ptr = reinterpret_cast<uint16_t *>(data);
    *ptr = VirtualSMCAPI::encodeSp(type, (double)provider->PACKAGE_TEMPERATURE_perPackage[0]);

    return SmcSuccess;
}

SMC_RESULT EnegryPackage::readAccess(){
    if (type == SmcKeyTypeFloat)
        *reinterpret_cast<uint32_t *>(data) = VirtualSMCAPI::encodeFlt(provider->uniPackageEnegry);
    else
        *reinterpret_cast<uint16_t *>(data) = VirtualSMCAPI::encodeSp(type, provider->uniPackageEnegry);
    
    return SmcSuccess;
}
