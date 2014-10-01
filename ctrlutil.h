//========================================================================
// ctrlutil.h : structures and definitions used by the ctrlutil.c
//              This code is designed to be shared by the various controls.
//========================================================================


#ifndef _CTRUTIL_H_
   #define _CTRUTIL_H_


// Valid WM_SETWINDOWPARAMS flags:
#define WPM_SETFLAGS   (WPM_TEXT | WPM_CTLDATA)

// Valid WM_QUERYWINDOWPARAMS flags
#define WPM_QUERYFLAGS (WPM_TEXT | WPM_CTLDATA | WPM_CCHTEXT | WPM_CBCTLDATA)

// All controls should typically reserve 8 bytes in the window words.
// The first 4 bytes are reserved for user data (QWL_USER).
// The remaining 4 bytes are reserved for the address of the memory
// used by the control in its procedure.
// Note: this have been redifined just in case we want to port this to
// 64 bit.
#define CB_CTRLWORDS           ((sizeof(PVOID)) << 1)
// Offset to the bytes reserved for the data used by the control
#define QWL_CONTROL            (sizeof(PVOID))

// common data structures

/*
 Control text data structure
 This is limited to usage in static controls, buttons, etc...
 Multiple line edit boxes, lists, etc., should use other structures.
 The following coordinates are measured and stored (some are used only
 by controls which undeline the mnemonic character) :
   ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿ -
   ³ text to be _m_easured ³   cy    -
   ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ -       - ydesc
   |<- xmnemo ->| |
              cxmnemo
   |<----- acx[idx] ------>|

*/

// single line control text (max 512 characters)

#pragma pack(2)
typedef struct {
   USHORT len;        // control text length
   USHORT usflag;     // flag used by CtrlTextDraw
   USHORT cx;         // text box width
   USHORT cy;         // text box height
   SHORT y;           // font descender
   SHORT mnemo;       // offset to the optional mnemonic character
   USHORT xmnemo;     // distance of mnemonic character from the first one
   USHORT cxmnemo;    // width of the mnemonic character
   CHAR ach[1];       // control text data
} CTRLTXT, * PCTRLTXT;
#pragma pack()

// error code used by CtrlTextGet which is called on WM_QUERYWINDOWPARAMS
#define WPM_TEXTERROR 0xffffffff

// structure to cache the control size
#pragma pack(2)
typedef struct {
   SHORT cx, cy;
} SIZES, * PSIZES;
#pragma pack()

// New DT_* (Draw Text) flags used by CtrlTextDraw():
// See the function comments for more details
#define DT_3DTEXTTOP        0x0004
#define DT_3DTEXTBOTTOM     0x0008


// The window hwnd gets updated only if part of it is visible on the screen
// If bchildren is TRUE, its children are updated as well
#define CtrlUpdate(hwnd, bchildren) \
   (WinIsWindowShowing(hwnd)? WinInvalidateRect(hwnd, NULL, (bchildren)) : 0)


// function prototypes:
BOOL CtrlTextSize(HWND hwnd, PCTRLTXT pct);
PCTRLTXT CtrlTextSet(PCTRLTXT pct, PSZ pszText, LONG len, BOOL bmnemo);
ULONG CtrlTextGet(PCTRLTXT pct, PSZ pszBuf, ULONG len, BOOL bmnemo);
ULONG CtrlTextDraw(HPS hps, PCTRLTXT pct, PRECTL pr,
                   LONG clrFgnd, LONG clrBkgnd, LONG clrBktxt);
VOID underlineMnemo(HPS hps, PCTRLTXT pct, PRECTL pr, PPOINTL ppt);
LONG CtrlClrGet(HWND hwnd, ULONG ulid1, ULONG ulid2, LONG ldef, BOOL bi);
BOOL WinIsPointerInWindow(HWND hwnd, PSIZES pszs);
BOOL WinIsMouseMsgInWindow(MPARAM mp, PSIZES pszs);
BOOL WinHalftoneRect(HPS hps, PRECTL pr, LONG clr);
VOID Win3DBorderDraw(HPS hps, PRECTL pr, LONG clrul, LONG clrbr, ULONG cpbrd);

#endif // ifndef _CTRUTIL_H_
