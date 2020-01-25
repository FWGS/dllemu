#include <stdint.h>
#include <stdio.h>
#include "sys/mman.h"
#include "mmap_anon.h"
#if X86EMU
#include "emu/x86emu_private.h"
#include "debug.h"
#include "box86context.h"
int box86_log = LOG_DUMP;
FILE* ftrace = NULL;
int trace_xmm = 0;
int trace_emm = 0;

void* my__IO_2_1_stderr_ = NULL;
void* my__IO_2_1_stdin_  = NULL;
void* my__IO_2_1_stdout_ = NULL;

void* GetGSBase(box86context_t *context)
{
    return NULL;
}

const char* GetNameOffset(void *maplib, void* offs)
{
    /// TODO
    static char s[32];
    snprintf(s, 32, "%p", offs);
    return s;
}

int my_open(x86emu_t* emu, void* pathname, int32_t flags, uint32_t mode)
{
    return -1;
}
int32_t my_accept4(x86emu_t* emu, int32_t fd, void* a, void* l, int32_t flags)
{
    return -1;
}
int32_t my_getrandom(x86emu_t* emu, void* buf, uint32_t buflen, uint32_t flags)
{
    return 42;
}

int my_setcontext(x86emu_t* emu, void* ucp)
{
    return -1;
}

int my_syscall_sigaction(x86emu_t* emu, int signum, const void *act, void *oldact, int sigsetsize)
{
    return -1;
}


uint16_t Pop16(x86emu_t *emu)
{
    uint16_t* st = ((uint16_t*)(R_ESP));
    R_ESP += 2;
    return *st;
}

void Push16(x86emu_t *emu, uint16_t v)
{
    R_ESP -= 2;
    *((uint16_t*)R_ESP) = v;
}
uint32_t Pop32(x86emu_t *emu)
{
    uint32_t* st = ((uint32_t*)(R_ESP));
    R_ESP += 4;
    return *st;
}

void Push32(x86emu_t *emu, uint32_t v)
{
    R_ESP -= 4;
    *((uint32_t*)R_ESP) = v;
}

void PushString(x86emu_t *emu, const char* s)
{
    int sz = strlen(s) + 1;
    // round to 4 bytes boundary
    R_ESP -= sz;
    memcpy((void*)R_ESP, s, sz);
}

#ifdef DYNAREC
#include <sys/mman.h>
#include "dynablock.h"

int box86_dynarec_log = LOG_DUMP;
int box86_dynarec = 1;
int box86_dynarec_linker = 1;
//int box86_dynarec_forced = 0;

typedef struct mmaplist_s {
    void*         block;
    uintptr_t     offset;           // offset in the block
} mmaplist_t;

#define MMAPSIZE (4*1024*1024)       // allocate 4Mo sized blocks

uintptr_t AllocDynarecMap(box86context_t *context, int size)
{
    // make size 0x10 bytes aligned
    size = (size+0x0f)&~0x0f;
    pthread_mutex_lock(&context->mutex_mmap);
    // look for free space
    for(int i=0; i<context->mmapsize; ++i) {
        if(context->mmaplist[i].offset+size < MMAPSIZE) {
            uintptr_t ret = context->mmaplist[i].offset + (uintptr_t)context->mmaplist[i].block;
            context->mmaplist[i].offset+=size;
            pthread_mutex_unlock(&context->mutex_mmap);
            return ret;
        }
    }
    // no luck, add a new one !
    int i = context->mmapsize++;    // yeah, usefull post incrementation
    dynarec_log(LOG_DEBUG, "Ask for DynaRec Block Alloc #%d\n", context->mmapsize);
    context->mmaplist = (mmaplist_t*)realloc(context->mmaplist, context->mmapsize*sizeof(mmaplist_t));
    void* p = mmap(NULL, MMAPSIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(p==MAP_FAILED) {
        dynarec_log(LOG_INFO, "Cannot create memory map of %d byte for dynarec block #%d\n", MMAPSIZE, i);
        --context->mmapsize;
        pthread_mutex_unlock(&context->mutex_mmap);
        return 0;
    }
    context->mmaplist[i].block = p;
    context->mmaplist[i].offset=size;
    pthread_mutex_unlock(&context->mutex_mmap);
    return (uintptr_t)p;
}

dynablocklist_t* GetDynablocksFromAddress(box86context_t *context, uintptr_t addr)
{
    return context->dynablocks;
}

#endif



extern x86emu_t *emu;
/* context used only in trace and log calls, probably not needed */
static box86context_t context;
void InitX86Emu( void )
{
    context.stacksz = 8*1024*1024;
    context.stackalign = 4;
    posix_memalign((void*)&context.stack, context.stackalign, context.stacksz );
    ftrace = stdout;
    emu = NewX86Emu(&context, NULL, (uintptr_t)context.stack, context.stacksz, 0);
    // Setup FS segment here
    emu->segs[_FS] = mmap_anon(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE, 0);
    *(void**)((char*)emu->segs[_FS]+0x18) = emu->segs[_FS];
    *(void**)emu->segs[_FS] = malloc(80000)+40000;
    // not used
//    context.emu = emu;
    SetupX86Emu(emu);
#ifdef DYNAREC
    context.dynablocks = NewDynablockList(0, 0, 0);
#endif
}
#endif