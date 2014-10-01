//===========================================================================
// superclass.c : simple example of a static bar control implemented via
//                superclassing
// 10-01-2002 * by Alessandro Cantatore * v. 0.1
//===========================================================================


#include "dllmain.h"
#include "superclass.h"


// Global variables
PFNWP pfnWcStatic;
ULONG cbWcStatic;

// Control data (for internal use)
#pragma pack(2)

typedef struct {
   USHORT style;   // valid only for the BARS_VERTICAL and BARS_AUTOSIZE
   USHORT thkns;   // bar control thickness
   SIZES sz;       // control size
   PCTRLTXT pct;   // control text data (with mnemonic)
} BAR, * PBAR;

#pragma pack()

// function prototypes
MRESULT EXPENTRY testDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
BOOL BarRegister(HAB hab);
MRESULT EXPENTRY BarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID BarPaint(HPS hps, HWND hwnd, PBAR pbar);

// macro to get the inner control data
#define BarData(hwnd) \
   ((PBAR)WinQueryWindowPtr((hwnd), cbWcStatic))

// colors used by the bar control:
#define ICLR_BKGND         0       // light gray (caption background)
#define ICLR_FGND          1       // black (caption foreground)
#define ICLR_HILITE        2       // white (light border)
#define ICLR_SHADE         3       // dark gray (used as dark border)
#define C_ICLRS            4       // color count

// macro to get the color values

#define BarColors(hwnd, pclr)                                       \
{                                                                   \
   (pclr)[ICLR_BKGND] = CtrlClrGet((hwnd), PP_BACKGROUNDCOLOR,      \
       PP_BACKGROUNDCOLORINDEX, SYSCLR_DIALOGBACKGROUND, TRUE);     \
   (pclr)[ICLR_FGND] = CtrlClrGet((hwnd), PP_FOREGROUNDCOLOR,       \
       PP_FOREGROUNDCOLORINDEX, SYSCLR_WINDOWSTATICTEXT, TRUE);     \
   (pclr)[ICLR_HILITE] = CtrlClrGet((hwnd), PP_BORDERLIGHTCOLOR, 0, \
       SYSCLR_BUTTONLIGHT, FALSE);                                  \
   (pclr)[ICLR_SHADE] = CtrlClrGet((hwnd), PP_BORDERDARKCOLOR, 0,   \
       SYSCLR_BUTTONDARK, FALSE);                                   \
}    


//===========================================================================
// Test executable - main procedure
// Parameters --------------------------------------------------------------
// VOID
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID main(VOID) {
   HAB hab;
   HMQ hmq;
   QMSG qmsg;
   HWND hwnd;
   hmq = WinCreateMsgQueue((hab = WinInitialize(0)), 0);
   BarRegister(hab);   
   hwnd = WinLoadDlg(HWND_DESKTOP, 0, testDlgProc, 0, 100, NULL);
   if (hwnd) {
      while (WinGetMsg(hab, &qmsg, NULLHANDLE, 0, 0))
         WinDispatchMsg(hab, &qmsg);
   } /* endif */
   WinDestroyWindow(hwnd);
   WinDestroyMsgQueue(hmq);
   WinTerminate(hab);
}


//===========================================================================
// Test dialog procedure
// Parameters --------------------------------------------------------------
// ordinary window procedure parameters
// Return value ------------------------------------------------------------
// MRESULT
//===========================================================================

MRESULT EXPENTRY testDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   switch (msg) {
      case WM_CLOSE:
         WinPostMsg(hwnd, WM_QUIT, MPVOID, MPVOID);
         break;
      default:
         return WinDefDlgProc(hwnd, msg, mp1, mp2);
   } /* endswitch */
   return MRFALSE;
}


//==========================================================================
// BarRegister : register the Bar control
// Parameters --------------------------------------------------------------
// HAB hab : anchor block handle
// Return value ------------------------------------------------------------
// BOOL : TRUE (success) or FALSE (error)
//==========================================================================

BOOL BarRegister(HAB hab) {
   CLASSINFO ci;
   if (WinQueryClassInfo(hab, WC_STATIC, &ci)) {
      // store the default class procedure in the global variables
      pfnWcStatic = ci.pfnWindowProc;
      cbWcStatic = ci.cbWindowData;
      return WinRegisterClass(hab, WC_BAR, BarProc,
                              ci.flClassStyle & ~CS_PUBLIC,
                              ci.cbWindowData + sizeof(PVOID));
   }                           
   return FALSE;
}


//===========================================================================
// BarProc: Bar control window procedure implemented as WC_STATIC superclass
// Parameters --------------------------------------------------------------
// ordinary window procedure parameters
// Return value ------------------------------------------------------------
// MRESULT
//===========================================================================

MRESULT EXPENTRY BarProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2) {
   switch (msg) {
      PBAR pbar;
      
      // Checks the mnemonic character -------------------------------------
      // If the control is not disabled checks the input character
      case WM_MATCHMNEMONIC:
         if (!(WinStyle(hwnd) & WS_DISABLED) &&
             (NULL != (pbar = BarData(hwnd))) && pbar->pct &&
             (pbar->pct->mnemo >= 0)) {
            HAB hab = WinHAB(hwnd); 
            return (MRESULT)
                   (WinUpperChar(hab, 0, 0, (ULONG)mp1) ==
                    WinUpperChar(hab, 0, 0,
                            (ULONG)pbar->pct->ach[pbar->pct->mnemo]));
         } /* endif */                        
         return MRFALSE;
      /* end case WM_MATCHMNEMONIC */
      
      // control painting --------------------------------------------------
      // WC_STATIC procedure is completely replaced here since we want to
      // paint the control ourselves
      case WM_PAINT:
         if (NULL != (pbar = BarData(hwnd))) {
            HPS hps;
            if (NULLHANDLE != (hps = WinBeginPaint(hwnd, NULLHANDLE, NULL))) {
               BarPaint(hps, hwnd, pbar);
               WinEndPaint(hps);
            } /* endif */
         } /* endif */
         return MRFALSE;
      /* end case WM_PAINT */
      
      // Window resizing ---------------------------------------------------
      // if autosize, modify the control size, otherwise store the new size
      case WM_ADJUSTWINDOWPOS:
         if ((((PSWP)mp1)->fl & SWP_SIZE) &&
            (NULL != (pbar = BarData(hwnd)))) {
            if (pbar->style & BARS_AUTOSIZE) {
               if (pbar->style & BARS_VERTICAL) {
                  ((PSWP)mp1)->cx = pbar->sz.cx;
               } else {                                   // horizontal bar
                  ((PSWP)mp1)->cy = pbar->sz.cy;
               } /* endif */
            } else {
               pbar->sz.cx = ((PSWP)mp1)->cx;
               pbar->sz.cy = ((PSWP)mp1)->cy;
            } /* endif */
         } /* end if */
         return MRFALSE;
         /* end case WM_ADJUSTWINDOWPOS */
         
      // Set the window text and other data --------------------------------
      // This message is caused by a WinSetWindowText() call. It may also
      // be sent directly if the control allow to change some data (for
      // instance icon handle, bitmap handle, etc.) via the WPM_CTLDATA
      // flag. As stated by the PM documentation, the only valid flags are:
      // WPM_TEXT and WPM_CTLDATA.
      case WM_SETWINDOWPARAMS:
         if (((PWNDPARAMS)mp1)->fsStatus & WPM_TEXT) {
            // updates the control text
            if ((NULL != (pbar = BarData(hwnd))) &&
                !(pbar->style & BARS_VERTICAL) &&
                (NULL != (pbar->pct = CtrlTextSet(pbar->pct,
                                       ((PWNDPARAMS)mp1)->pszText,
                                       ((PWNDPARAMS)mp1)->cchText,
                                       (WinStyle(hwnd) & BARS_MNEMONIC)))) &&
                CtrlTextSize(hwnd, pbar->pct)) {
               // redraws the control
               CtrlUpdate(hwnd, FALSE);
               return MRTRUE;
            } /* endif */
         // control data to set the bar thickness
         } else if (((PWNDPARAMS)mp1)->fsStatus & WPM_CTLDATA) {
            if (((PWNDPARAMS)mp1)->cbCtlData && ((PWNDPARAMS)mp1)->pCtlData) {
               return (MRESULT)WinBarThicknessSet(hwnd,
                                     *((PUSHORT)((PWNDPARAMS)mp1)->pCtlData));
            } /* endif */
         } /* endif */
         // invalid fsStatus flags: returns FALSE
         return MRFALSE;
         //break;
      /* end case WM_SETWINDOWPARAMS */
      
      // Checks the control text or the control data -----------------------
      // This message is caused by a WinQueryWindowTextLength() or by a
      // WinQueryWindowText() call, or may sent directly to query the control
      // data via the WPM_CTLDATA or WPM_CBCTLDATA flags.
      // As stated by the PM documentation the only valid fsStatus flags
      // are: WPM_TEXT, WPM_CCHTEXT, WPM_CTLDATA and WPM_CBCTLDATA.
      // The flags in fsStatus must be cleared as each item is processed.
      // If the call is successful, fsStatus is 0. If any item has not been 
      // processed, the flag for that item is still set. 
      // Note: in this implementeation the mnemonic tag character is not
      //        returned
      case WM_QUERYWINDOWPARAMS:
         // If there are any valid flags:
         if ((((PWNDPARAMS)mp1)->fsStatus & WPM_QUERYFLAGS) &&
             (NULL != (pbar = BarData(hwnd)))) {
            if (!(pbar->style & BARS_VERTICAL)) {
               // The control text has been queried
               if (((PWNDPARAMS)mp1)->fsStatus & WPM_TEXT) {
                  ((PWNDPARAMS)mp1)->cchText = CtrlTextGet(pbar->pct,
                                                ((PWNDPARAMS)mp1)->pszText,
                                                ((PWNDPARAMS)mp1)->cchText, 0);
                  if (((PWNDPARAMS)mp1)->cchText == WPM_TEXTERROR) {
                     ((PWNDPARAMS)mp1)->cchText = 0;
                  } else {
                     ((PWNDPARAMS)mp1)->fsStatus &= ~(WPM_TEXT | WPM_CCHTEXT);
                  } /* endif */
               // The control text length has been queried
               } else if (((PWNDPARAMS)mp1)->fsStatus & WPM_CCHTEXT) {
                  ((PWNDPARAMS)mp1)->cchText = pbar->pct->len;
                  ((PWNDPARAMS)mp1)->fsStatus &= ~WPM_CCHTEXT;
               } /* endif */
            } /* endif */   
            // The control data have been queried 
            if (((PWNDPARAMS)mp1)->fsStatus & WPM_CTLDATA) {
               // set appropriately ((PWNDPARAMS)mp1)->pCtlData
               // and reset the fsStatus flags if successful:
               if (((PWNDPARAMS)mp1)->cbCtlData &&
                   ((PWNDPARAMS)mp1)->pCtlData) {
                  *((PUSHORT)((PWNDPARAMS)mp1)->pCtlData) = pbar->thkns;
                  ((PWNDPARAMS)mp1)->fsStatus &=
                                             ~(WPM_CTLDATA | WPM_CBCTLDATA);
               } /* endif */
            // The control data size has been queriedd
            } else if (((PWNDPARAMS)mp1)->fsStatus & WPM_CBCTLDATA) {
               ((PWNDPARAMS)mp1)->cbCtlData = 2;
               ((PWNDPARAMS)mp1)->fsStatus &= ~WPM_CBCTLDATA;
            } /* endif */
         } /* endif */
         return (MRESULT)!((PWNDPARAMS)mp1)->fsStatus;
      /* end case WM_QUERYWINDOWPARAMS */
      
      // If the font has been changed measures the new text box ------------
      case WM_PRESPARAMCHANGED:
         if (((LONG)mp1 == PP_FONTNAMESIZE) &&
             (NULL != (pbar = BarData(hwnd))) &&
             !(pbar->style & BARS_VERTICAL) && pbar->pct) {
             CtrlTextSize(hwnd, pbar->pct);
             // if the BARS_AUTOSIZE flag is set resizes the control
             if (pbar->style & BARS_AUTOSIZE) {
                pbar->sz.cy = pbar->pct->cy + 2;
                WinSetWindowPos(hwnd, 0, 0, 0, pbar->sz.cx, pbar->sz.cy,
                                SWP_SIZE | SWP_NOADJUST);
             } /* endif */
         } /* endif */
         CtrlUpdate(hwnd, FALSE);
         return MRFALSE;
      /* end case WM_PRESPARAMCHANGED */
      
      // Set the bar thickness ---------------------------------------------
      // this also causes the BARS_AUTOSIZE style to be set
      case BARM_SETTHICKNESS:
         if (!mp1 || (NULL == (pbar = BarData(hwnd)))) return MRFALSE;
         pbar->style |= BARS_AUTOSIZE;
         pbar->thkns = ((ULONG)mp1) & 0x7e;
         if (pbar->style & BARS_VERTICAL) {
            pbar->sz.cx = pbar->thkns;
         } else {
            pbar->sz.cy = max(pbar->thkns, (pbar->pct? pbar->pct->cy + 2: 0));
         } /* endif */
         WinSetWindowPos(hwnd, 0, 0, 0, pbar->sz.cx, pbar->sz.cy,
                         SWP_SIZE | SWP_NOADJUST);
         return MRTRUE;
      /* end case BARM_SETTHICKNESS */
      
      // Query the bar thickness -------------------------------------------
      case BARM_QUERYTHICKNESS:
         if (NULL == (pbar = BarData(hwnd))) return (MRESULT)0xffff;
         return (MRESULT)pbar->thkns;
      /* end case BARM_QUERYTHICKNESS */
      
      // Window creation ---------------------------------------------------
      case WM_CREATE:
         // Checks if the control style is horizontal or vertical
         pbar = (PBAR)memalloc(sizeof(BAR));
         if (!pbar) return MRTRUE;
         WinSetWindowPtr(hwnd, cbWcStatic, (PVOID)pbar);
         // store the direction and BARS_AUTOSIZE styles
         pbar->style = ((PCREATESTRUCT)mp2)->flStyle;
         // if it is an horizontal bar stores the text
         if (pbar->style & BARS_VERTICAL) {
            pbar->pct = NULL;
         } else {
            pbar->pct = CtrlTextSet(NULL, ((PCREATESTRUCT)mp2)->pszText,
                                    -1, pbar->style & BARS_MNEMONIC);
            // if the control has any text, measures the text box size
            if (!pbar->pct ||
                (pbar->pct->len && !CtrlTextSize(hwnd, pbar->pct))) {
               memfree(pbar);
               return MRTRUE;
            } /* endif */
         } /* endif */
         // We reset the CREATESTRUCT pszText member to NULL since we don't
         // want to store the text both in our procedure and in the WC_STATIC
         // procedure
         ((PCREATESTRUCT)mp2)->pszText = NULL;
         // Set the control thickness: if a non-default thickness is defined
         // automatically sets the AUTOSIZE style!!!
         if (((PCREATESTRUCT)mp2)->pCtlData) {
            pbar->thkns = *((PUSHORT)((PCREATESTRUCT)mp2)->pCtlData) & 0x7e;
            pbar->style |= BARS_AUTOSIZE;
         } else {
            pbar->thkns = (pbar->style & BARS_THICK) ? 4 : 2;
         } /* endif */
         // if the BARS_AUTOSIZE style flag is set, resize the control
         if (pbar->style & BARS_AUTOSIZE) {
            if (pbar->style & BARS_VERTICAL) {
               ((PCREATESTRUCT)mp2)->cx = pbar->thkns;
            } else {
               ((PCREATESTRUCT)mp2)->cy = max(pbar->thkns, pbar->pct->cy + 2);
            } /* endif */
            WinSetWindowPos(hwnd, 0, 0, 0,
                          ((PCREATESTRUCT)mp2)->cx, ((PCREATESTRUCT)mp2)->cy,
                          SWP_SIZE | SWP_NOADJUST);
         } /* endif */
         pbar->sz.cx = ((PCREATESTRUCT)mp2)->cx;
         pbar->sz.cy = ((PCREATESTRUCT)mp2)->cy;
         break;
         /* end case WM_CREATE */
      // Window destruction ------------------------------------------------
      // All the allocated resources are freed:
      case WM_DESTROY:
         if (NULL != (pbar = BarData(hwnd))) {
            if (pbar->pct) memfree(pbar->pct);
            memfree(pbar);
            memheapmin();
         } /* endif */
         break;
      /* end case WM_DESTROY */
       
   } /* endswitch */
   // all other messages go to the original WC_STATIC procedure
   return pfnWcStatic(hwnd, msg, mp1, mp2);
}


//===========================================================================
// BarPaint: paints the bar control
// Parameters --------------------------------------------------------------
// HPS hps   : control presentation space
// HWND hwnd : control handle
// PBAR pbar : control data
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID BarPaint(HPS hps, HWND hwnd, PBAR pbar) {
   RECTL r;
   LONG aclr[C_ICLRS];
   // get the current style flags
   ULONG style = WinStyle(hwnd);
   // Get the current colors
   BarColors(hwnd, aclr);
   // Set to RGB mode
   GpiCreateLogColorTable(hps, 0, LCOLF_RGB, 0, 0, NULL);
   // Erase the background if needed
   if ((!(pbar->style & BARS_VERTICAL) && (pbar->sz.cy > pbar->thkns)) ||
       ((pbar->style & BARS_VERTICAL) && (pbar->sz.cx > pbar->thkns))) {
      RectSet(&r, 0, 0, pbar->sz.cx, pbar->sz.cy);
      WinFillRect(hps, &r, aclr[ICLR_BKGND]);
   } /* endif */
   // Draws the bar:
   if (pbar->style & BARS_VERTICAL) {
      r.xLeft = ((pbar->sz.cx - pbar->thkns) >> 1) - 1,
      r.xRight = r.xLeft + pbar->thkns - 1,
      r.yBottom = 0,
      r.yTop = pbar->sz.cy - 1;
   } else {
      r.xLeft = 0,
      r.yBottom = ((pbar->sz.cy - pbar->thkns) >> 1) - 1,
      r.xRight = pbar->sz.cx - 1,
      r.yTop = r.yBottom + pbar->thkns - 1;
   } /* endif */
   if (style & BARS_RAISED) {
      Win3DBorderDraw(hps, &r, aclr[ICLR_HILITE], aclr[ICLR_SHADE],
                      pbar->thkns >> 1);
   } else {
      Win3DBorderDraw(hps, &r, aclr[ICLR_SHADE], aclr[ICLR_HILITE], 
                      pbar->thkns >> 1);
   } /* endif */
   // if it is an horizontal bar and there is any text draws it
   if (!(pbar->style & BARS_VERTICAL) && pbar->pct->len) {
      if (style & BARS_CENTER) {
         r.xLeft = max((pbar->thkns << 1) + 2,
                       ((pbar->sz.cx - pbar->pct->cx) >> 1) - 2);
         r.xRight = min(pbar->sz.cx - (pbar->thkns << 1) - 2,
                        r.xLeft + pbar->pct->cx + 3);
      } else if (style & BARS_RIGHT) {
         r.xRight = pbar->sz.cx - (pbar->thkns << 1) - 2;
         r.xLeft = max((pbar->thkns << 1) + 2,
                       r.xRight - pbar->pct->cx - 3);
      } else {
         r.xLeft = (pbar->thkns << 1) + 2;
         r.xRight = min(pbar->sz.cx - (pbar->thkns << 1) - 2,
                        r.xLeft + pbar->pct->cx + 3);
      } /* endif */
      r.yBottom = 0, r.yTop = pbar->sz.cy;
      pbar->pct->usflag = (style & WS_DISABLED) ?
                         DT_HALFTONE | DT_CENTER | DT_VCENTER | DT_ERASERECT :
                         DT_CENTER | DT_VCENTER | DT_ERASERECT;
      CtrlTextDraw(hps, pbar->pct, &r, aclr[ICLR_FGND], aclr[ICLR_BKGND], 0);
   } /* endif */
}
