//========================================================================
// dllmain.h : definitions of global variables, macros, structures
//             These definitions should only be available to the DLL sources
//========================================================================

// header inclusions

#ifndef COMMON_H_
   #define COMMON_H_
   #define INCL_WIN
   #define INCL_GPI
   #define INCL_DOS
//   #define INCL_DOSMISC
//   #define INCL_DOSPROCESS
//   #define INCL_DOSSEMAPHORES
//   #define INCL_DOSMODULEMGR
//   #define INCL_SHLERRORS

   #include <os2.h>
   #include <stdlib.h>
   #include <malloc.h>
   #include <string.h>
   #include "ApiExPM.h"
   #include "ctrlutil.h"

// conditional compilation macros:
//#define MULTITHREADLIB    // comment to compile as single thread library
#define BUILD_TEST_EXE    // builds the program as a test executable


// common usage macros
// aligns n to the next m multiple (where m is a power of 2)
#define RNDUP(n, m)   (((ULONG)(n) + (m) - 1) & ~((m) - 1))
// aligns n to the previous m multiple (where m is a power of 2)
#define RNDDWN(n, m)  ((ULONG)(n) & ~((m) - 1))

// memory allocation management macros
#ifdef BUILD_TEST_EXE
   #define memalloc(cb)          (malloc(cb))
   #define memfree(p)            (free(p))
   #define memheapmin()          (_heapmin())
#else
   #ifdef MULTITHREADLIB
      #define memalloc(cb)          (_mtalloc(cb))
      #define memfree(p)            (_mtfree(p))
      #define memheapmin()          (_mtheapmin())
   #else
      #define memalloc(cb)          (_stalloc(cb))
      #define memfree(p)            (_stfree(p))
      #define memheapmin()          (_stheapmin())
   #endif // ifdef MULTITHREADLIB
#endif // #ifdef BUILD_TEST_EXE

// globals

typedef struct {
   HMODULE hmod;
   CLASSINFO ci;
   HPOINTER hHandClick;
} GLOBALS, * PGLOBALS;

extern GLOBALS g;


// exported APIs
// bar.c
BOOL APIENTRY BarRegister(HAB hab);
BOOL APIENTRY WzBtnRegister(HAB hab);


// other prototypes:

// dllmain.c
VOID heapLock(VOID);
VOID heapUnlock(VOID);
PVOID _stalloc(ULONG cb);
PVOID _mtalloc(ULONG cb);
VOID _stfree(PVOID pv);
VOID _mtfree(PVOID pv);
VOID _stheapmin(VOID);
VOID _mtheapmin(VOID);

#endif
