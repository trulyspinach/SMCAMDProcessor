//
//  AMDRyzenCPUPowerManagementUserClient.hpp
//  AMDRyzenCPUPowerManagement
//
//  Created by trulyspinach on 2/4/20.
//

#ifndef AMDRyzenCPUPowerManagementUserClient_hpp
#define AMDRyzenCPUPowerManagementUserClient_hpp

//Support for macOS 10.13
#include <Library/LegacyIOService.h>
#include "Headers/LegacyHeaders/LegacyIOUserClient.h"

#include <sys/proc.h>

//#include <IOKit/IOService.h>
//#include <IOKit/IOUserClient.h>
//#include <IOKit/IOLib.h>

#include "AMDRyzenCPUPowerManagement.hpp"

extern "C" {
    proc_t get_bsdtask_info(task_t t);
};



class AMDRyzenCPUPMUserClient : public IOUserClient
{

    OSDeclareDefaultStructors(AMDRyzenCPUPMUserClient)
    
      
public:
    
    bool initWithTask(task_t owningTask, void *securityToken, UInt32 type, OSDictionary *properties) override;
    
    // IOUserClient methods
    virtual void stop(IOService* provider) override;
    virtual bool start(IOService* provider) override;
    
    
    
protected:
    
    AMDRyzenCPUPowerManagement *fProvider;
    void *token;
    bool clientAuthorizedByUser = false;
    
    bool hasPrivilege();
    
    // KPI for supporting access from both 32-bit and 64-bit user processes beginning with Mac OS X 10.5.
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments* arguments,
                                    IOExternalMethodDispatch* dispatch, OSObject* target, void* reference) override;
    
    
    char taskProcessBinaryName[32]{};
};

#endif
