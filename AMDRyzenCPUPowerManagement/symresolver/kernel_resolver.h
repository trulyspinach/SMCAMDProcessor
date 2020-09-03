#ifndef kernel_resolver_h
#define kernel_resolver_h

#include <IOKit/IOLib.h>
#include <mach/mach_types.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <vm/vm_kern.h>
#include <sys/sysctl.h>

typedef struct mach_header_64 mach_header_64_t;
typedef struct load_command load_command_t;
typedef struct segment_command_64 seg_command_64_t;
typedef struct nlist_64 nlist_64_t;
typedef struct symtab_command symtab_command_t;

#ifdef __cplusplus
extern "C" {
#endif
    
    void *lookup_symbol(const char *symbol);
void print_pointer(void *ptr);
    
#ifdef __cplusplus
}
#endif

#endif /* kernel_resolver_h */

