#ifndef kernel_resolver_h
#define kernel_resolver_h

#include <mach/mach_types.h>
#include <mach-o/loader.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <vm/vm_kern.h>
#include <sys/sysctl.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    void *lookup_symbol(const char *symbol);
    
#ifdef __cplusplus
}
#endif

#endif /* kernel_resolver_h */

