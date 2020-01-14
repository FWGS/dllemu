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
}
#endif