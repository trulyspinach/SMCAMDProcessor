//
//  SMCAMDProcessorUserClient.hpp
//  SMCAMDProcessor
//
//  Created by Qi HaoYan on 2/4/20.
//  Copyright © 2020 Qi HaoYan. All rights reserved.
//

#ifndef SMCAMDProcessorUserClient_hpp
#define SMCAMDProcessorUserClient_hpp


#include <IOKit/IOService.h>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOLib.h>

#include "SMCAMDProcessor.hpp"

class SMCAMDProcessorUserClient : public IOUserClient
{

    OSDeclareDefaultStructors(SMCAMDProcessorUserClient)
    
      
public:
    // IOUserClient methods
    virtual void stop(IOService* provider) override;
    virtual bool start(IOService* provider) override;
    

    
protected:
    
    SMCAMDProcessor *fProvider;
    
    // KPI for supporting access from both 32-bit and 64-bit user processes beginning with Mac OS X 10.5.
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
                                    IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) override;

};

#endif
