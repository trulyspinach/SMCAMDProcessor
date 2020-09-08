/*
 * kernel_resolver.c
 * by snare (snare@ho.ax)
 * updates and aslr support by nervegas
 *
 * This is a simple example of how to resolve private symbols in the kernel
 * from within a kernel extension. There are much more efficient ways to 
 * do this, but this should serve as a good starting point.
 *
 * See the following URL for more info: 
 *     http://ho.ax/posts/2012/02/resolving-kernel-symbols/
 */


#include "kernel_resolver.h"


#define KERNEL_BASE 0xffffff8000200000

seg_command_64_t *find_segment_64(mach_header_64_t *mh, const char *segname);
load_command_t *find_load_command(mach_header_64_t *mh, uint32_t cmd);
void *find_symbol(mach_header_64_t *mh, const char *name);

static uint64_t mh_base_addr = 0;

void find_mach_header_addr(uint8_t kc){
    uint64_t slide = 0;
    vm_offset_t slide_address = 0;
    vm_kernel_unslide_or_perm_external((unsigned long long)(void *)printf, &slide_address);
    slide = (uint64_t)(void *)printf - slide_address;
    uint64_t base_address = (uint64_t)slide + KERNEL_BASE;
    
    if(!kc){
        mh_base_addr = base_address;
        return;
    }
    
    mach_header_64_t* mach_header = (mach_header_64_t*)base_address;
    
    load_command_t* lcp = (load_command_t*)(base_address + sizeof(mach_header_64_t));
    for (uint32_t i = 0; i < mach_header->ncmds; i++) {
        if (lcp->cmd == LC_SEGMENT_64) {
            seg_command_64_t *sc = (seg_command_64_t*)lcp;
            if (!strncmp(sc->segname, "__PRELINK_TEXT", sizeof(sc->segname))) {
                mh_base_addr = sc->vmaddr;
                break;
            }
        }
        lcp = (load_command_t*)((uint64_t)lcp + (uint64_t)lcp->cmdsize);
    }
}

void *lookup_symbol(const char *symbol)
{
    
    if(!mh_base_addr) return NULL;
//    IOLog("%s: aslr slide: 0x%0llx\n", __func__, slide);
//    print_pointer((void*)slide);
//    IOLog("%s: base address: 0x%0llx\n", __func__, base_address);
//    print_pointer((void*)base_address);
//
//    IOLog("%s: actual address: 0x%0llx\n", __func__, (uint64_t)actual_header);
//    print_pointer((void*)actual_header);
    
    return find_symbol((mach_header_64_t*)mh_base_addr, symbol);
}

seg_command_64_t *
find_segment_64(mach_header_64_t *mh, const char *segname)
{
    load_command_t *lc;
    seg_command_64_t *seg, *foundseg = NULL;
    size_t segname_len = strlen(segname) + 1;
    /* first load command begins straight after the mach header */
    lc = (load_command_t *)((uint64_t)mh + sizeof(mach_header_64_t));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == LC_SEGMENT_64) {
            /* evaluate segment */
            seg = (seg_command_64_t*)lc;
            if (strncmp(seg->segname, segname, segname_len) == 0) {
                foundseg = seg;
                break;
            }
        }
        
        /* next load command */
        lc = (load_command_t *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    return foundseg;
}

load_command_t *
find_load_command(mach_header_64_t *mh, uint32_t cmd)
{
    load_command_t *lc, *foundlc = NULL;
    
    /* first load command begins straight after the mach header */
    lc = (load_command_t *)((uint64_t)mh + sizeof(mach_header_64_t));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == cmd) {
            foundlc = (load_command_t *)lc;
            break;
        }
        
        /* next load command*/
        lc = (load_command_t *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    return foundlc;
}

void print_pointer(void *ptr){
    uint64_t v = (uint64_t)ptr;
    uint32_t l = v & 0xffffffff;
    uint32_t h = v >> 32;
    IOLog("vh: %u, vl: %u\n", h, l);
}

void *
find_symbol(mach_header_64_t *mh, const char *name)
{
    symtab_command_t *symtab = NULL;
    seg_command_64_t *linkedit = NULL;
    nlist_64_t *nl = NULL;
    char *strtab = NULL;
    void *addr = NULL;
    size_t symlen = strlen(name)+1;
    uint64_t i;

    /* check header (0xfeedfccf) */
    if (mh->magic != MH_MAGIC_64) {
        IOLog("%s: magic number doesn't match - 0x%x\n", __func__, mh->magic);
        return NULL;
    }
    
    /* find the __LINKEDIT segment and LC_SYMTAB command */
    linkedit = find_segment_64(mh, SEG_LINKEDIT);

    if (!linkedit) {
        IOLog("%s: couldn't find __LINKEDIT\n", __func__);
        return NULL;
    }
    
    symtab = (symtab_command_t*)find_load_command(mh, LC_SYMTAB);
    if (!symtab) {
        IOLog("%s: couldn't find LC_SYMTAB\n", __func__);
        return NULL;
    }

    /* walk the symbol table until we find a match */

    print_pointer((void*)linkedit->vmaddr);
    uint64_t strtab_addr = linkedit->vmaddr - linkedit->fileoff + (uint64_t)symtab->stroff;
    uint64_t symtab_addr = linkedit->vmaddr - linkedit->fileoff + (uint64_t)symtab->symoff;
    
    strtab = (char *)strtab_addr;

    for (i = 0, nl = (nlist_64_t*)symtab_addr; i < symtab->nsyms; i++, nl++)
    {
        char *str = strtab + nl->n_un.n_strx;
        if (strncmp(str, name, symlen) == 0) {
            addr = (void *)nl->n_value;
        }
    }
    
    return addr;
}
