#ifndef __USE_FILE_OFFSET64
        #define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
        #define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
        #define _LARGEFILE64_SOURCE
#endif
#ifndef _FILE_OFFSET_BIT
        #define _FILE_OFFSET_BIT 64
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "box86version.h"
#include "debug.h"
#include "box86context.h"
#include "fileutils.h"
#include "elfloader.h"
#include "box86stack.h"
#include "x86emu.h"
#include "x86run.h"
#include "x86trace.h"
#include "librarian.h"
#include "library.h"

int box86_log = LOG_INFO;//LOG_NONE;
#ifdef DYNAREC
int box86_dynarec_log = LOG_NONE;
int box86_dynarec = 1;
int box86_dynarec_linker = 1;
int box86_dynarec_forced = 0;
#endif
int dlsym_error = 0;
int trace_xmm = 0;
int trace_emm = 0;
#ifdef HAVE_TRACE
uint64_t start_cnt = 0;
#ifdef DYNAREC
int box86_dynarec_trace = 0;
#endif
#endif
#ifdef PANDORA
int x11color16 = 0;
#endif
int x11threads = 0;
char* libGL = NULL;

FILE* ftrace = NULL;

void LoadLogEnv()
{
    const char *p = getenv("BOX86_LOG");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0'+LOG_NONE && p[1]<='0'+LOG_DEBUG)
                box86_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP"))
                box86_log = LOG_DUMP;
        }
        printf_log(LOG_INFO, "Debug level is %d\n", box86_log);
    }
#ifdef DYNAREC
    p = getenv("BOX86_DYNAREC_LOG");
    if(p) {
        if(strlen(p)==1) {
            if((p[0]>='0'+LOG_NONE) && (p[0]<='0'+LOG_DUMP))
                box86_dynarec_log = p[0]-'0';
        } else {
            if(!strcasecmp(p, "NONE"))
                box86_dynarec_log = LOG_NONE;
            else if(!strcasecmp(p, "INFO"))
                box86_dynarec_log = LOG_INFO;
            else if(!strcasecmp(p, "DEBUG"))
                box86_dynarec_log = LOG_DEBUG;
            else if(!strcasecmp(p, "DUMP"))
                box86_dynarec_log = LOG_DUMP;
        }
        printf_log(LOG_INFO, "Dynarec log level is %d\n", box86_dynarec_log);
    }
    p = getenv("BOX86_DYNAREC");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec is %s\n", box86_dynarec?"On":"Off");
    }
    p = getenv("BOX86_DYNAREC_LINKER");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec_linker = p[0]-'0';
        }
        printf_log(LOG_INFO, "Dynarec Linker is %s\n", box86_dynarec_linker?"On":"Off");
    }
    p = getenv("BOX86_DYNAREC_FORCED");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='1')
                box86_dynarec_forced = p[0]-'0';
        }
        if(box86_dynarec_forced)
        printf_log(LOG_INFO, "Dynarec is Forced on all addresses\n");
    }
#endif
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_XMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                trace_xmm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_EMM");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                trace_emm = p[0]-'0';
        }
    }
    p = getenv("BOX86_TRACE_START");
    if(p) {
        char* p2;
        start_cnt = strtoll(p, &p2, 10);
        printf_log(LOG_INFO, "Will start trace only after %llu instructions\n", start_cnt);
    }
#ifdef DYNAREC
    p = getenv("BOX86_DYNAREC_TRACE");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                box86_dynarec_trace = p[0]-'0';
            if(box86_dynarec_trace)
                printf_log(LOG_INFO, "Dynarec generated code will also print a trace\n");
        }
    }
#endif
#endif
    p = getenv("BOX86_TRACE_FILE");
    if(p) {
        ftrace = fopen64(p, "w");
        if(!ftrace) {
            ftrace = stdout;
            printf_log(LOG_INFO, "Cannot open trace file \"%s\" for writing (error=%s)\n", p, strerror(errno));
        } else {
            printf("BOX86 Trace redirected to \"%s\"\n", p);
        }
    }
    p = getenv("BOX86_DLSYM_ERROR");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                dlsym_error = p[0]-'0';
        }
    }
#ifdef PANDORA
    p = getenv("BOX86_X11COLOR16");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                x11color16 = p[0]-'0';
        }
        printf_log(LOG_INFO, "Try to adjust X11 Color (32->16bits) : %s\n", x11color16?"Yes":"No");
    }
#endif
    p = getenv("BOX86_X11THREADS");
    if(p) {
        if(strlen(p)==1) {
            if(p[0]>='0' && p[1]<='0'+1)
                x11threads = p[0]-'0';
        }
        if(x11threads)
            printf_log(LOG_INFO, "Try to Call XInitThreads if libX11 is loaded\n");
    }
    p = getenv("BOX86_LIBGL");
    if(p)
        libGL = strdup(p);
    if(!libGL) {
        p = getenv("SDL_VIDEO_GL_DRIVER");
        if(p)
            libGL = strdup(p);
    }
    if(libGL) {
        printf_log(LOG_INFO, "BOX86 using \"%s\" as libGL.so.1\n", p);
    }
}

void LoadEnvPath(path_collection_t *col, const char* defpath, const char* env)
{
    const char* p = getenv(env);
    if(p) {
        printf_log(LOG_INFO, "%s: ", env);
        ParseList(p, col, 1);
    } else {
        printf_log(LOG_INFO, "Using default %s: ", env);
        ParseList(defpath, col, 1);
    }
    if(LOG_INFO<=box86_log) {
        for(int i=0; i<col->size; i++)
            printf_log(LOG_INFO, "%s%s", col->paths[i], (i==col->size-1)?"\n":":");
    }
}

int CountEnv(const char** env)
{
    // count, but remove all BOX86_* environnement
    // also remove PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    const char** p = env;
    int c = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_", 6)!=0)
            if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0))
                ++c;
        ++p;
    }
    return c+2;
}
int GatherEnv(char*** dest, const char** env, const char* prog)
{
    // Add all but BOX86_* environnement
    // and PATH and LD_LIBRARY_PATH
    // but add 2 for default BOX86_PATH and BOX86_LD_LIBRARY_PATH
    const char** p = env;    
    int idx = 0;
    int path = 0;
    int ld_path = 0;
    while(*p) {
        if(strncmp(*p, "BOX86_PATH=", 11)==0) {
            (*dest)[idx++] = strdup(*p+6);
            path = 1;
        } else if(strncmp(*p, "BOX86_LD_LIBRARY_PATH=", 22)==0) {
            (*dest)[idx++] = strdup(*p+6);
            ld_path = 1;
        } else if(strncmp(*p, "_=", 2)==0) {
            int l = strlen(prog);
            char tmp[l+3];
            strcpy(tmp, "_=");
            strcat(tmp, prog);
            (*dest)[idx++] = strdup(tmp);
        } else if(strncmp(*p, "BOX86_", 6)!=0) {
            if(!(strncmp(*p, "PATH=", 5)==0 || strncmp(*p, "LD_LIBRARY_PATH=", 16)==0)) {
                (*dest)[idx++] = strdup(*p);
            }
        }
        ++p;
    }
    if(!path) {
        (*dest)[idx++] = strdup("BOX86_PATH=.:bin");
    }
    if(!ld_path) {
        (*dest)[idx++] = strdup("BOX86_LD_LIBRARY_PATH=.:lib");
    }
    return 0;
}


void PrintHelp() {
    printf("\n\nThis is Box86, the Linux 86 emulator with a twist\n");
    printf("\nUsage is box86 path/to/software [args]\n");
    printf("to launch x86 software\n");
    printf("You can also set some environment variables:\n");
    printf(" BOX86_PATH is the box86 version of PATH (default is '.:bin')\n");
    printf(" BOX86_LD_LIBRARY_PATH is the box86 version LD_LIBRARY_PATH (default is '.:lib')\n");
    printf(" BOX86_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed debug info\n");
#ifdef DYNAREC
    printf(" BOX86_DYNAREC_LOG with 0/1/2/3 or NONE/INFO/DEBUG/DUMP to set the printed dynarec info\n");
    printf(" BOX86_DYNAREC with 0/1 to disable or enable Dynarec (On by default)\n");
    printf(" BOX86_DYNAREC_LINKER with 0/1 to disable or enable Dynarec Linker (On by default, use 0 only for easier debug)\n");
#endif
#ifdef HAVE_TRACE
    printf(" BOX86_TRACE with 1 to enable x86 execution trace\n");
    printf("    or with XXXXXX-YYYYYY to enable x86 execution trace only between address\n");
    printf("    or with FunctionName to enable x86 execution trace only in one specific function\n");
    printf("  use BOX86_TRACE_INIT instead of BOX_TRACE to start trace before init of Libs and main program\n\t (function name will probably not work then)\n");
    printf(" BOX86_TRACE_XMM with 1 to enable dump of SSE/SSE2 register along with regular registers\n");
    printf(" BOX86_TRACE_START with N to enable trace after N instructions\n");
#ifdef DYNAREC
    printf(" BOX86_DYNAREC_TRACE with 0/1 to disable or enable Trace on generated code too\n");
#endif
#endif
    printf(" BOX86_TRACE_FILE with FileName to redirect logs in a file");
    printf(" BOX86_DLSYM_ERROR with 1 to log dlsym errors\n");
    printf(" BOX86_LOAD_ADDR=0xXXXXXX try to load at 0xXXXXXX main binary (if binary is a PIE)\n");
    printf(" BOX86_NOSIGSEGV=1 to disable handling of SigSEGV\n");
#ifdef PANDORA
    printf(" BOX86_X11COLOR16=1 to try convert X11 color from 32 bits to 16 bits (to avoid light green on light cyan windows\n");
#endif
    printf(" BOX86_X11THREADS=1 to call XInitThreads when loading X11 (for old Loki games with Loki_Compat lib)");
    printf(" BOX86_LIBGL=libXXXX set the name (and optionnaly full path) for libGL.so.1\n");
    printf(" BOX86_LD_PRELOAD=XXXX[:YYYYY] force loading XXXX (and YYYY...) libraries with the binary\n");
}

void *emu;
#include "emu/x86emu_private.h"
#include "mmap_anon.h"
#include "sys/mman.h"
int main(int argc, const char **argv, const char **env) {

    // trying to open and load 1st arg
    if(argc==1) {
        printf("Box86%s%s%s v%d.%d.%d\n", 
        #ifdef HAVE_TRACE
            " with trace",
        #else
            "",
        #endif
        #ifdef DYNAREC
            " with Dynarec",
        #else
            "",
        #endif
        #ifdef USE_FLOAT
            " (float only)",
        #else
            "",
        #endif
            BOX86_MAJOR, BOX86_MINOR, BOX86_REVISION);
        PrintHelp();
        return 1;
    }
}
void init()
{
    ftrace = stdout;

    // init random seed
    srandom(time(NULL));

    // check BOX86_LOG debug level
    LoadLogEnv();
    
    // Create a new context
    box86context_t *context = NewBox86Context(1);
    //context->box86path = strdup(argv[0]);

    const char *p;
    const char* prog = "";
    // check BOX86_LD_LIBRARY_PATH and load it
    LoadEnvPath(&context->box86_ld_lib, ".:lib:lib32:x86", "BOX86_LD_LIBRARY_PATH");
#ifdef PANDORA
    if(FileExist("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/codeblocks/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/mnt/utmp/box86/lib/i386-linux-gnu", 0))
        AddPath("/mnt/utmp/box86/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    //TODO: add relative path to box86 location
#endif
    if(FileExist("/lib/i386-linux-gnu", 0))
        AddPath("/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/i386-linux-gnu", 0))
        AddPath("/usr/lib/i386-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/lib/i686-pc-linux-gnu", 0))
        AddPath("/lib/i686-pc-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib/i686-pc-linux-gnu", 0))
        AddPath("/usr/lib/i686-pc-linux-gnu", &context->box86_ld_lib, 1);
    if(FileExist("/usr/lib32", 0))
        AddPath("/usr/lib32", &context->box86_ld_lib, 1);
    if(getenv("LD_LIBRARY_PATH"))
        AppendList(&context->box86_ld_lib, getenv("LD_LIBRARY_PATH"), 1);   // in case some of the path are for x86 world
    path_collection_t ld_preload = {0};
    if(getenv("BOX86_LD_PRELOAD")) {
        char* p = getenv("BOX86_LD_PRELOAD");
        ParseList(p, &ld_preload, 0);
        if (ld_preload.size && box86_log) {
            printf_log(LOG_INFO, "BOX86 try to Preload ");
            for (int i=0; i<ld_preload.size; ++i)
                printf_log(LOG_INFO, "%s ", ld_preload.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    }
    if(getenv("BOX86_EMULATED_LIBS")) {
        char* p = getenv("BOX86_EMULATED_LIBS");
        ParseList(p, &context->box86_emulated_libs, 0);
        if (context->box86_emulated_libs.size && box86_log) {
            printf_log(LOG_INFO, "BOX86 will force the used of emulated libs for ");
            for (int i=0; i<context->box86_emulated_libs.size; ++i)
                printf_log(LOG_INFO, "%s ", context->box86_emulated_libs.paths[i]);
            printf_log(LOG_INFO, "\n");
        }
    }

    if(getenv("BOX86_NOSIGSEGV")) {
        if (strcmp(getenv("BOX86_NOSIGSEGV"), "1")==0)
            context->no_sigsegv = 1;
            printf_log(LOG_INFO, "BOX86: Disabling handling of SigSEGV\n");
    }
    // check BOX86_PATH and load it
    LoadEnvPath(&context->box86_path, ".:bin", "BOX86_PATH");
    if(getenv("PATH"))
        AppendList(&context->box86_path, getenv("PATH"), 1);   // in case some of the path are for x86 world
    // prepare all other env. var
    /*context->envc = CountEnv(env);
    printf_log(LOG_INFO, "Counted %d Env var\n", context->envc);
    context->envv = (char**)calloc(context->envc, sizeof(char*));
    GatherEnv(&context->envv, env, prog);*/
    if(box86_log>=LOG_DUMP) {
        for (int i=0; i<context->envc; ++i)
            printf_log(LOG_DUMP, " Env[%02d]: %s\n", i, context->envv[i]);
    }
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    p = getenv("BOX86_TRACE_INIT");
    if(p) {
        if (strcmp(p, "0"))
            context->x86trace = 1;
    }
    if(context->x86trace) {
        printf_log(LOG_INFO, "Initializing Zydis lib\n");
        if(InitX86Trace(context)) {
            printf_log(LOG_INFO, "Zydis init failed, no x86 trace activated\n");
            context->x86trace = 0;
        }
    }
#endif
    // lets build argc/argv stuff
    printf_log(LOG_INFO, "Looking for %s\n", prog);
    if(strchr(prog, '/'))
        context->argv[0] = strdup(prog);
    else
        context->argv[0] = ResolveFile(prog, &context->box86_path);

    // special case for LittleInferno that use an old libvorbis
    if(strstr(prog, "LittleInferno.bin.x86")) {
        printf_log(LOG_INFO, "LittleInferno detected, forcing emulated libvorbis\n");
        AddPath("libvorbis.so.0", &context->box86_emulated_libs, 0);
    }

/*    for(int i=1; i<context->argc; ++i)
        context->argv[i] = strdup(argv[i+1]);
    // check if file exist
    if(!context->argv[0]) {
        printf_log(LOG_NONE, "Error: file is not found (check BOX86_PATH)\n");
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(!FileExist(context->argv[0], IS_FILE|IS_EXECUTABLE)) {
        printf_log(LOG_NONE, "Error: %s is not an executable file\n", context->argv[0]);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    context->fullpath = (char*)calloc(PATH_MAX, 1);
    if(!realpath(context->argv[0], context->fullpath))
        strcpy(context->fullpath, context->argv[0]);*/
	CalcStackSize(context);
    context->emu = NewX86Emu(context, context->ep, (uintptr_t)context->stack, context->stacksz, 0);
    context->emu->segs[_FS] = mmap_anon(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_PRIVATE, 0);
    *(void**)((char*)context->emu->segs[_FS]+0x18) = context->emu->segs[_FS];
    *(void**)context->emu->segs[_FS] = malloc(80000)+40000;
    emu = context->emu;
    // stack setup is much more complicated then just that!

    const char **av = {"","",""};
    context->argc = 2;
    context->argv = av;
    context->envc = 2;
    context->envv = av;
   // SetupInitialStack(context);
    // this is probably useless
    SetupX86Emu(context->emu);
//	void *lib = my_dlopen(context->emu, argv[1], 2 );
//	Setup_LDT_Keeper();
    /*void *lib = Loader_LoadLibrary(argv[1]);
    // defered init
    RunDeferedElfInit(context->emu);
	printf("%p\n", lib);
	void *sym1 = Loader_GetProcAddress(lib, "GiveFnptrsToDll");//my_dlsym(context->emu, lib, "GiveFnptrsToDll");
	void *sym2 = Loader_GetProcAddress(lib, "GetEntityAPI2");//my_dlsym(context->emu, lib, "GetEntityAPI2");
	printf("%p %p\n", sym1, sym2);
	void *ptrs[256] = {};
	void *globals[256] = {};

	void *ftable[256] = {};
	int version = 140;
    SetEAX(context->emu, 0);
    SetEBX(context->emu, 0);
    SetEIP(context->emu, sym1);
    ResetFlags(context->emu);
    
	Push32(context->emu,&globals);
	Push32(context->emu,&ptrs);
  //  Run(context->emu, 0);
	DynaCall(context->emu, sym1);
	printf("%p\n", GetEAX(context->emu));

	Push32(context->emu,&version);
	Push32(context->emu,&ftable[0]);
	DynaCall(context->emu, sym2);
//	Run( context->emu, 0);
	printf("%p\n", ftable[28]);
	DynaCall(context->emu, ftable[28]);
	printf("%s\n", GetEAX(context->emu));
    */
	return 0;
}
#if 0
{
    FILE *f = fopen(context->argv[0], "rb");
    if(!f) {
        printf_log(LOG_NONE, "Error: Cannot open %s\n", context->argv[0]);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    elfheader_t *elf_header = LoadAndCheckElfHeader(f, context->argv[0], 1);
    if(!elf_header) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    int mainelf = AddElfHeader(context, elf_header);

    if(CalcLoadAddr(elf_header)) {
        printf_log(LOG_NONE, "Error: reading elf header of %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // allocate memory
    if(AllocElfMemory(context, elf_header, 1)) {
        printf_log(LOG_NONE, "Error: allocating memory for elf %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // Load elf into memory
    if(LoadElfMemory(f, context, elf_header)) {
        printf_log(LOG_NONE, "Error: loading in memory elf %s\n", context->argv[0]);
        fclose(f);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // can close the file now
    fclose(f);
    // get and alloc stack size and align
    if(CalcStackSize(context)) {
        printf_log(LOG_NONE, "Error: allocating stack\n");
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // init x86 emu
    context->emu = NewX86Emu(context, context->ep, (uintptr_t)context->stack, context->stacksz, 0);
    // stack setup is much more complicated then just that!
    SetupInitialStack(context);
    // this is probably useless
    SetupX86Emu(context->emu);
    SetEAX(context->emu, context->argc);
    SetEBX(context->emu, (uint32_t)context->argv);
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE_INIT");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(context->emu, 0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start)
                SetTraceEmu(context->emu, trace_start, trace_end);
        } else {
            if (GetSymbolStartEnd(GetMapSymbol(context->maplib), p, &trace_start, &trace_end)) {
                SetTraceEmu(context->emu, trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(context->emu, 0, 100);  // disabling trace, mostly
            }
        }
    } else {
        p = getenv("BOX86_TRACE");
        if(p)
            if (strcmp(p, "0"))
                SetTraceEmu(context->emu, 0, 1);
    }
#endif
    // export symbols
    AddSymbols(context->maplib, GetMapSymbol(context->maplib), GetWeakSymbol(context->maplib), GetLocalSymbol(context->maplib), elf_header);
    // pre-load lib if needed
    if(ld_preload.size) {
        for (int i=0; i<ld_preload.size; ++i) {
            if(AddNeededLib(context->maplib, NULL, ld_preload.paths[i], context, context->emu)) {
                printf_log(LOG_INFO, "Warning, cannot pre-load lib: \"%s\"\n", ld_preload.paths[i]);
            }            
        }
    }
    // Call librarian to load all dependant elf
    if(LoadNeededLibs(elf_header, context->maplib, NULL, context, context->emu)) {
        printf_log(LOG_NONE, "Error: loading needed libs in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    if(ld_preload.size) {
        for (int i=0; i<ld_preload.size; ++i) {
            if(FinalizeLibrary(GetLib(context->maplib, ld_preload.paths[i]), context->emu)) {
                printf_log(LOG_INFO, "Warning, cannot finalize pre-load lib: \"%s\"\n", ld_preload.paths[i]);
            }            
        }
    }
    if(FinalizeNeededLibs(elf_header, context->maplib, context, context->emu)) {
        printf_log(LOG_NONE, "Error: finalizing needed libs in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // reloc...
    printf_log(LOG_DEBUG, "And now export symbols / relocation for %s...\n", ElfName(elf_header));
    if(RelocateElf(context->maplib, elf_header)) {
        printf_log(LOG_NONE, "Error: relocating symbols in elf %s\n", context->argv[0]);
        FreeBox86Context(&context);
        FreeCollection(&ld_preload);
        return -1;
    }
    // and handle PLT
    RelocateElfPlt(context, context->maplib, elf_header);
    // defered init
    RunDeferedElfInit(context->emu);
    // do some special case check, _IO_2_1_stderr_ and friends, that are setup by libc, but it's already done here, so need to do a copy
    ResetSecialCaseMainElf(elf_header);
    // init...
#ifdef HAVE_TRACE
    p = getenv("BOX86_TRACE");
    if(p) {
        setbuf(stdout, NULL);
        uintptr_t trace_start=0, trace_end=0;
        if (strcmp(p, "1")==0)
            SetTraceEmu(context->emu, 0, 0);
        else if (strchr(p,'-')) {
            if(sscanf(p, "%d-%d", &trace_start, &trace_end)!=2) {
                if(sscanf(p, "0x%X-0x%X", &trace_start, &trace_end)!=2)
                    sscanf(p, "%x-%x", &trace_start, &trace_end);
            }
            if(trace_start)
                SetTraceEmu(context->emu, trace_start, trace_end);
        } else {
            if (GetGlobalSymbolStartEnd(context->maplib, p, &trace_start, &trace_end)) {
                SetTraceEmu(context->emu, trace_start, trace_end);
                printf_log(LOG_INFO, "TRACE on %s only (%p-%p)\n", p, (void*)trace_start, (void*)trace_end);
            } else {
                printf_log(LOG_NONE, "Warning, symbol to Traced (\"%s\") not found, disabling trace\n", p);
                SetTraceEmu(context->emu, 0, 100);  // disabling trace, mostly
            }
        }
    }
#endif
    // get entrypoint
    context->ep = GetEntryPoint(context->maplib, elf_header);
#ifdef RPI
    // before launching emulation, let's check if this is a mojosetup from GOG
    if (strstr(prog, "bin/linux/x86/mojosetup") && getenv("MOJOSETUP_BASE") && getenv("GTK2_RC_FILES")) {
        sanitize_mojosetup_gtk_background();
    }
#endif

    // emulate!
    printf_log(LOG_DEBUG, "Start x86emu on Main\n");
    SetEAX(context->emu, context->argc);
    SetEBX(context->emu, (uint32_t)context->argv);
    SetEIP(context->emu, context->ep);
    ResetFlags(context->emu);
    Run(context->emu, 0);
    // Get EAX
    int ret = GetEAX(context->emu);
    printf_log(LOG_DEBUG, "Emulation finished, EAX=%d\n", ret);


    // all done, free context
    FreeBox86Context(&context);
    if(libGL)
        free(libGL);
    FreeCollection(&ld_preload);

    return ret;
}
#endif
