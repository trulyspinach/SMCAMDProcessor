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
#include <IOKit/IOLib.h>

#define KERNEL_BASE 0xffffff8000200000

struct nlist_64 {
    union {
        uint32_t  n_strx;   /* index into the string table */
    } n_un;
    uint8_t n_type;         /* type flag, see below */
    uint8_t n_sect;         /* section number or NO_SECT */
    uint16_t n_desc;        /* see <mach-o/stab.h> */
    uint64_t n_value;       /* value of this symbol (or stab offset) */
};

struct segment_command_64 *find_segment_64(struct mach_header_64 *mh, const char *segname);
struct load_command *find_load_command(struct mach_header_64 *mh, uint32_t cmd);
void *find_symbol(struct mach_header_64 *mh, const char *name);

void *lookup_symbol(const char *symbol)
{
    int64_t slide = 0;
    vm_offset_t slide_address = 0;
    vm_kernel_unslide_or_perm_external((unsigned long long)(void *)printf, &slide_address);
    slide = (unsigned long long)(void *)printf - slide_address;
    int64_t base_address = slide + KERNEL_BASE;
    
    IOLog("%s: aslr slide: 0x%0llx\n", __func__, slide);
    IOLog("%s: base address: 0x%0llx\n", __func__, base_address);
    
    return find_symbol((struct mach_header_64 *)base_address, symbol);
}

struct segment_command_64 *
find_segment_64(struct mach_header_64 *mh, const char *segname)
{
    struct load_command *lc;
    struct segment_command_64 *seg, *foundseg = NULL;
    
    /* first load command begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == LC_SEGMENT_64) {
            /* evaluate segment */
            seg = (struct segment_command_64 *)lc;
            if (strcmp(seg->segname, segname) == 0) {
                foundseg = seg;
                break;
            }
        }
        
        /* next load command */
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    return foundseg;
}

struct load_command *
find_load_command(struct mach_header_64 *mh, uint32_t cmd)
{
    struct load_command *lc, *foundlc = NULL;
    
    /* first load command begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == cmd) {
            foundlc = (struct load_command *)lc;
            break;
        }
        
        /* next load command*/
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    return foundlc;
}

void *
find_symbol(struct mach_header_64 *mh, const char *name)
{
    struct symtab_command *symtab = NULL;
    struct segment_command_64 *linkedit = NULL;
    struct nlist_64 *nl = NULL;
    void *strtab = NULL;
    void *addr = NULL;
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
    
    symtab = (struct symtab_command *)find_load_command(mh, LC_SYMTAB);
    if (!symtab) {
        IOLog("%s: couldn't find LC_SYMTAB\n", __func__);
        return NULL;
    }
    
    /* walk the symbol table until we find a match */
    int64_t strtab_addr = (int64_t)(linkedit->vmaddr - linkedit->fileoff) + symtab->stroff;
    int64_t symtab_addr = (int64_t)(linkedit->vmaddr - linkedit->fileoff) + symtab->symoff;
    
    strtab = (void *)strtab_addr;
    for (i = 0, nl = (struct nlist_64 *)symtab_addr;
         i < symtab->nsyms;
         i++, nl = (struct nlist_64 *)((int64_t)nl + sizeof(struct nlist_64)))
    {
        char *str = (char *)strtab + nl->n_un.n_strx;
        
        if (strcmp(str, name) == 0) {
            IOLog("%s: symbol %s at address %llx\n", __func__, str, (uint64_t)nl->n_value);
            addr = (void *)nl->n_value;
        }
    }
    
    return addr;
}
