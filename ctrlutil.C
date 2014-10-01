//==========================================================================
// ctrlutil.c : OS/2 PM controls utility functions
// 10-01-2002 * by Alessandro Cantatore * v. 0.1
//==========================================================================


#pragma strings(readonly)

#include "dllmain.h"
#include <string.h>

/*
 ToDo:
 design APIs for measuring and painting different kinds of text:
 single line with or without mnemonic character
 single line with ellipsis (no mnemonic)
 single line with tabs (no mnemonic)
 multiple lines with or without mnemonic character (static text)
 multiple lines with tables (no mnemonic)
 rich text (simple xml/html - no mnemonic)
 colored text (monospaced font with different colors: for simple editors,
               programmers' editor, email-usenet messages, etc.)
*/


//===========================================================================
// CtrlTextSize : measures the control text size when the text is single
//                line with or without a mnemonic style and character.
//                CtrlTextSizeEllps() is provided to prevent text clipping
//                by susbstituting the text end or other parts of the text
//                with ellipsis.
//                CtrlTextSizeML() is provided for multiline text with
//                or without mnemonic character.
//                Note:
//                this procedure should be called in response to:
//                   - WM_CREATE if the control has text
//                   - WM_SETWINDOWPARAMS if coming from WinSetWindowText
//                   - WM_PRESPARAMSCHANGED if a new font has been set
//                this procedure works properly only if the character
//                angle, direction and shear are set to the default
//                values (i.e. have not been changed via:
//                GpiSetCharAngle, GpiSetCharDirection or GpiSetCharShear
// Parameters --------------------------------------------------------------
// HWND hwnd    : control window handle
// PCTRLTXT pct : control text (single line with mnemonic) data
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

BOOL CtrlTextSize(HWND hwnd, PCTRLTXT pct) {
   POINTL aptl[3];
   HPS hps;
   // do some parameters validity check
   if (!hwnd || !pct) return FALSE;
   // initialize the text box width since this value is later modified via +=
   pct->cx = 0;
   // missing text: set the textbox width to 0 and return success
   if (!pct->len) return TRUE;
   // get the presentation space
   if (NULLHANDLE != (hps = WinGetPS(hwnd))) {
      // if mnemonic style and a mnemonic character is present calculate
      // its width and distance from the start of the line
      if (pct->mnemo >= 0) {
         // if the mnemonic character is not the first text character,
         // get the width of the text preceding it
         if (pct->mnemo) {
            if (!GpiQueryTextBox(hps, pct->mnemo, pct->ach, 3, aptl))
               goto error;
            pct->cx = pct->xmnemo = aptl[2].x - aptl[1].x;
         } /* endif */
         // get the width of the mnemonic character
         if (!GpiQueryTextBox(hps, 1, pct->ach + pct->mnemo, 3, aptl))
            goto error;
         pct->cx += (pct->cxmnemo = aptl[2].x - aptl[1].x);
         // if the mnemonic character was not the last character in the
         // string, get the width of the remaining string
         if ((pct->mnemo + 1) < pct->len) {
            if (!GpiQueryTextBox(hps, pct->len - pct->mnemo - 1,
                                 pct->ach + pct->mnemo + 1, 3, aptl))
               goto error;
            pct->cx += aptl[2].x - aptl[1].x;
         } /* endif */
      // single line without mnemonic style or mnemonic character
      } else {
         if (!GpiQueryTextBox(hps, pct->len, pct->ach, 3, aptl))
            goto error;
         pct->cx = aptl[2].x - aptl[1].x;
      } /* endif */
      // common to both styles:
      pct->cy = aptl[0].y - aptl[1].y;  // font height
      pct->y = - aptl[1].y;             // lMaxDescender of the current font
      WinReleasePS(hps);
      return TRUE;
   } /* endif */
   // in case of error sets to 0 the line width and returns FALSE
   error:
   WinReleasePS(hps);
   pct->cx = pct->cy = 0;
   pct->len = 0;
   return FALSE;
}


//===========================================================================
// CtrlTextSet : (re)allocates storage for the control text initializing it
//               if bmnemo is TRUE, stores the position of the mnemonic
//               character and delete the mnemonic tag character '~'
//               This function can only be used for simple text controls.
//               Complex window controls like multiple line edit windows,
//               list boxes, etc. should define other kind of functions.
//               The maximum string length is 512. Longer text strings are
//               truncated.
// Parameters --------------------------------------------------------------
// PCTRLTXT pct : previous data to be freed
// PSZ pszText  : text being assigned to the control
// INT len      : length of the text being set (-1 to autocalcolate it)
// BOOL bmnemo  : the control has the mnemonic style set
// Return value ------------------------------------------------------------
// PCTRLTXT pct : new text data or NULL in case of error
//===========================================================================

PCTRLTXT CtrlTextSet(PCTRLTXT pct, PSZ pszText, LONG len, BOOL bmnemo) {
   PSZ psz;
   if (len < 0) len = pszText? strlen(pszText) : 0;
   if (len > 512) len = 512;
   if (pct) memfree(pct);
   if (NULL != (pct = (PCTRLTXT)memalloc(len + sizeof(CTRLTXT)))) {
      // if a mnemonic tag character (~) is found it is stripped
      if (len && bmnemo && (NULL != (psz = strchr(pszText, '~')))) {
         len--;
         // if the mnemonic tag is not the first character copy the
         // part of the string preceding it
         if (0 != (pct->mnemo = psz - pszText))
            memcpy(pct->ach, pszText, pct->mnemo);
         // if the mnemonic tag is not the last character copies the
         // rest of the string
         if (*(++psz))
            memcpy(pct->ach + pct->mnemo, psz, len - pct->mnemo);
      } else {
         pct->mnemo = -1; // no mnemonic character found
         if (len) memcpy(pct->ach, pszText, len);
      } /* endif */
      pct->len = len;
      pct->ach[len] = 0;
   } /* endif */
   return pct;
}


//===========================================================================
// CtrlTextGet : copies the control text into a buffer
//               the bmnemo option is not essential and probably it might
//               be better to completely remove it and the associated code
//               It might be possible, though to define some new flags for
//               WNDPARAMS structure: WPM_TEXTMNEMO, WPM_CCHTEXTMNEMO
//               to get, via some new API the text and the text length
//               including the mnemonic character. But probably this is
//               not really necessary.
// Parameters --------------------------------------------------------------
// PCTRLTXT pct : control text data
// PSZ pszBuf   : buffer to which the control text has to be copied
// UINT len     : buffer size (including space for the termination character)
// BOOL bmnemo  : TRUE/FALSE copy/ignore the mnemonic tag '~' (if present)
// Return value ------------------------------------------------------------
// UINT : number of characters copied into pszText (excluding termination)
//===========================================================================

ULONG CtrlTextGet(PCTRLTXT pct, PSZ pszBuf, ULONG len, BOOL bmnemo) {
   ULONG cbCopy = 0;
   // checks the parameters
   if (!pct) return 0;     // the control doesn't have text
   if (!pszBuf || !len) return WPM_TEXTERROR; // wrong parameters
   // decreases len since must leave space for the termination character
   --len;
   // if requested finds the mnmeonic character tag
   if (bmnemo && (pct->mnemo >= 0)) {
      // copies the part preceding the mnemonic prefix if present
      if (pct->mnemo) {
         cbCopy = min(pct->mnemo, len);
         memcpy(pszBuf, pct->ach, cbCopy);
      } /* endif */
      // copy the mnemonic tag character
      if (pct->mnemo < len) pszBuf[cbCopy++] = '~';
      // if the buffer is not yet filled copies the rest
      if (pct->mnemo < len) {
         ULONG cbRest = min(len - cbCopy, pct->len - pct->mnemo);
         memcpy(pszBuf + cbCopy, pct->ach + pct->mnemo, cbRest);
         cbCopy += cbRest;
      } /* endif */
   } else {
      cbCopy = min(pct->len, len);
      memcpy(pszBuf, pct->ach, cbCopy);
   } /* endif */
   pszBuf[cbCopy] = 0;
   return cbCopy;
}


//===========================================================================
// CtrlTextDraw : draws the control text when the text is single line
//                with or without mnemonic character.
//                Alignment flags are passed via the 'usres' member of
//                the TXTMNDAT structure.
//                Note: the palette must be set to RGB mode.
// Parameters --------------------------------------------------------------
// HPS hps       : presentation space handle
// PCTRLTXT pct  : control text with mnemonic data
//                 The member usflag must be used to define text alignment.
//                 The ordinary text alignment flags (DT_LEFT, DT_CENTER,
//                 DT_RIGHT, DT_TOP, DT_VCENTER, DT_BOTTOM) are used for
//                 this purpose. Other valid flags are:
//                 DT_UNDERSCORE, DT_STRIKEOUT, DT_HALFTONE, DT_ERASERECT.
//                 DT_3DTEXTTOP and DT_3DTEXTBOTTOM (defined as 0x0004
//                 and 0x0008) are used to draw 3D text.
//                 3D text is rendered by drawing a first text string
//                 shifted 1 pixel right and down (DT_3DTEXTTOP)
//                 or 1 pixel left and up (DT_3DTEXTBOTTOM) with the
//                 clrBktxt color, overpainting it with the text at the
//                 correct position (using clrFgnd as color).
//                 To determine if it is necessary to underline the mnemonic
//                 character the text checks the value of the ptd->pct->mnemo
//                 member.
// PRECTL pr     : rectangle within which the text should be aligned and
//                 drawn.
//                 Note: on return pr->xRight and pr->yTop are decreased
//                 by 1 so it is possible to call Win3DBorderDraw()
//                 without modifying those values.
// LONG clrFgnd  : foreground text color (black/dark gray for disabled text)
// LONG clrBkgnd : background color (if DT_ERASERECT is specified)
// LONG clrBktxt : optional text color of the background text (typically
//                 white for disabled text, used to produce a 3D appearance)
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

ULONG CtrlTextDraw(HPS hps, PCTRLTXT pct, PRECTL pr,
                   LONG clrFgnd, LONG clrBkgnd, LONG clrBktxt) {
   POINTL pt;     // text coordinates
   // check the parameters
   if (!hps || !pct || !pr) return FALSE;
   // if DT_ERASERECT has been specified draws a solid rectangle
   if (pct->usflag & DT_ERASERECT) WinFillRect(hps, pr, clrBkgnd);
   // if the text is NULL returns TRUE
   if (!pct->len) return TRUE;
   // calculates the coordinates of the text string according to
   // the alignment flags
   // horizontal alignment:
   if (pct->usflag & DT_CENTER) {
      pt.x = ((pr->xRight - pr->xLeft - pct->cx) >> 1) + pr->xLeft;
   } else if (pct->usflag & DT_RIGHT) {
      pt.x = (pr->xRight - pr->xLeft - pct->cx) + pr->xLeft;
   } else {
      pt.x = pr->xLeft;
   } /* endif */
   // vertical alignment
   if (pct->usflag & DT_VCENTER) {
      pt.y = ((pr->yTop - pr->yBottom - pct->cy) >> 1) + pr->yBottom + pct->y;
   } else if (pct->usflag & DT_BOTTOM) {
      pt.y = pr->yBottom + pct->y;
   } else {
      pt.y = pr->yTop - pct->cy + pct->y;
   } /* endif */
   // adjust the clipping rectangle
   pr->xRight --, pr->yTop--;
   // process 3D attributes
   if (pct->usflag & DT_3DTEXTTOP) {
      pt.x++, pt.y--;
      GpiSetColor(hps, clrBktxt);
      GpiCharStringPosAt(hps, &pt, pr,
             CHS_CLIP | ((pct->usflag & (DT_UNDERSCORE | DT_STRIKEOUT)) << 5),
             pct->len, pct->ach, NULL);
      if (pct->mnemo >= 0) underlineMnemo(hps, pct, pr, &pt);
      pt.x--, pt.y++;
   } else if (pct->usflag & DT_3DTEXTBOTTOM) {
      pt.x --, pt.y++; 
      GpiSetColor(hps, clrBktxt);
      GpiCharStringPosAt(hps, &pt, pr,
             CHS_CLIP | ((pct->usflag & (DT_UNDERSCORE | DT_STRIKEOUT)) << 5),
             pct->len, pct->ach, NULL);
      if (pct->mnemo >= 0) underlineMnemo(hps, pct, pr, &pt);
      pt.x++, pt.y--;
   } /* endif */
   // draws the text
   GpiSetColor(hps, clrFgnd);
   GpiCharStringPosAt(hps, &pt, pr,
             CHS_CLIP | ((pct->usflag & (DT_UNDERSCORE | DT_STRIKEOUT)) << 5),
             pct->len, pct->ach, NULL);
   if (pct->mnemo >= 0) underlineMnemo(hps, pct, pr, &pt);
   // if DT_HALFTONE style, overlays an halftoned rectangle
   if (pct->usflag & DT_HALFTONE) {
      GpiMove(hps, (PPOINTL)pr);
      GpiSetPattern(hps, PATSYM_HALFTONE);
      GpiSetColor(hps, clrBkgnd);
      GpiBox(hps, DRO_FILL, (PPOINTL)pr + 1, 0, 0);
   } /* endif */
   return TRUE;
}


//===========================================================================
// underlineMnemo : utility function used by CtrlTextDraw()
//                  underlines the mnemonic character
// Parameters --------------------------------------------------------------
// HPS hps      : presentation space handle
// PCTRLTXT pct : control text data
// PRECTL pr    : clipping rectangle
// PPOINTL ppt  : starting point
// Return value ------------------------------------------------------------
// VOID (no error check is performed)
//===========================================================================

VOID underlineMnemo(HPS hps, PCTRLTXT pct, PRECTL pr, PPOINTL ppt) {
   POINTL pt0, pt1;
   INT i;
   // calculates the starting point of the line
   pt0.x = ppt->x + pct->xmnemo;
   pt0.y = pt1.y = ppt->y - ((pct->cy + 4) >> 4) - 2;
   pt1.x = pt0.x + pct->cxmnemo;
   i = (pct->cy >> 5) + 1;
   // checks if the line is inside the rectangle
   if ((pt1.x <= pr->xLeft) || (pt0.x >= pr->xRight) ||
       ((pt0.y + i) <= pr->yBottom) || (pt0.y >= pr->yTop))
      return;
   // clip the coordinates within pr boundaries
   if (pt0.x < pr->xLeft) pt0.x = pr->xLeft;
   if (pt1.x > pr->xRight) pt1.x = pr->xRight;
   while (pt0.y < pr->yBottom) i--, pt0.y++, pt1.y++;
   while (i--) {
      GpiMove(hps, &pt0);
      GpiLine(hps, &pt1);
      pt0.y++, pt1.y++;
   } /* endwhile */
}


//===========================================================================
// CtrlClrGet : gets the RGB value of the color used to paint a control.
//              First checks the presentation parameters and then
//               mnemonic prefix ~ if needed.
// Parameters --------------------------------------------------------------
// HWND hwnd   : control handle
// ULONG ulid1 : presentation parameter ID
// ULONG ulid2 : presentation parameter INDEX
// LONG ldef   : default color as SYSCLR_* or RGB value
// BOOL bi     : optional inheritance flag -> TRUE inherit/FALSE no inherit
// Return value ------------------------------------------------------------
// LONG : color as a 24 bit RGB value
//===========================================================================

LONG CtrlClrGet(HWND hwnd, ULONG ulid1, ULONG ulid2, LONG ldef, BOOL bi) {
   LONG lclr = 0;
   bi = bi? 0 : QPF_NOINHERIT;
   // first checks for presentation parameters
   if (WinQueryPresParam(hwnd, ulid1, ulid2, NULL, 4, (PVOID)&lclr,
                         bi | QPF_PURERGBCOLOR | QPF_ID2COLORINDEX))
      return lclr;
   // if ldef refers to a valid SYSCLR_* index gets the RGB value
   if ((ldef >= SYSCLR_SHADOWHILITEBGND) && (ldef <= SYSCLR_HELPHILITE))
      return WinQuerySysColor(HWND_DESKTOP, ldef, 0L);
   return ldef;
}


//===========================================================================
// WinIsPointerInWindow : checks if the mouse pointer is currently on the
//                        window hwnd
// Parameters --------------------------------------------------------------
// HWND hwnd : window handle
// Return value ------------------------------------------------------------
// BOOL : TRUE / FALSE -> pointer inside / pointer ouside or error
//===========================================================================

BOOL WinIsPointerInWindow(HWND hwnd, PSIZES pszs) {
   POINTL pt;
   return (WinQueryPointerPos(HWND_DESKTOP, &pt) &&
           WinMapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1) &&
       (pt.x >= 0) && (pt.x < pszs->cx) && (pt.y >= 0) && (pt.y < pszs->cy));
}


//===========================================================================
// WinIsMouseMsgInWindow : checks if the mouse coordinates it gets from
//                         the first parameter of a mouse message are
//                         within the window rectangle
// Parameters --------------------------------------------------------------
// MPARAM mp : message parameter
// PSIZES pszs : control size
// Return value ------------------------------------------------------------
// BOOL : TRUE / FALSE -> pointer inside / pointer ouside or error
//===========================================================================

BOOL WinIsMouseMsgInWindow(MPARAM mp, PSIZES pszs) {
   SHORT x = MOUSEX(mp);
   SHORT y = MOUSEY(mp);
   return ((x >= 0) && (x < pszs->cx) && (y >= 0) && (y < pszs->cy));
}


//===========================================================================
// WinHalftoneRect : overlays the rectangle pr with an halftone pattern
//                   Note: this routine draws the halftone rectangle via
//                   a pattern. The presentation space pattern is not reset
//                   to its previous value!
// Parameters --------------------------------------------------------------
// HPS hps     : presentation space handle
// PRECTL pr   : address of rectangle to be filled with the pattern
//               since the rectangle enclose the upper right corner
//               coordinates, pr->xRight and pr->yTop must be previously
//               decreased by 1 pixel otherwise this routine will paint
//               one line outside the upper border of the rectangle and
//               one line outside the right border of the rectangle
// LONG clr    : pattern color
// Return value ------------------------------------------------------------
// BOOL : TRUE/FALSE -> success/error
//===========================================================================

BOOL WinHalftoneRect(HPS hps, PRECTL pr, LONG clr) {
   return GpiMove(hps, (PPOINTL)pr) &&
          GpiSetPattern(hps, (((pr->xLeft & 1) == (pr->yBottom & 1))?
                              PATSYM_HALFTONE : PATSYM_DENSE5)) &&
          GpiSetColor(hps, clr) &&
          GpiBox(hps, DRO_FILL, (PPOINTL)pr + 1, 0, 0);
}


//===========================================================================
// Win3DBorderDraw : draws a 3D border within the rectangle pr
//               the bottom/left corner of the rectangle is moved up and
//               and right of cpbrd pixels, while the upper/right corner
//               is moved down and left, so the resulting rectangle
//               width and height are decreased of cpbrd * 2 pixels
// Parameters --------------------------------------------------------------
// HPS hps     : presentation space handle
// PRECTL pr   : address of rectangle to be drawn with a 3D border
// ULONG clrul : upper-left border color
// ULONG clrbr : bottom-right border color
// ULONG cpbrd : border thickness in pixels
// Return value ------------------------------------------------------------
// VOID
//===========================================================================

VOID Win3DBorderDraw(HPS hps, PRECTL pr, LONG clrul, LONG clrbr, ULONG cpbrd) {
   POINTL apt[2];
   while (cpbrd--) {
      // bordo sinistro e superiore
      GpiSetColor(hps, clrul);
      PointSet(apt, pr->xLeft, pr->yBottom);
      GpiMove(hps, apt);
      PointSet(apt, pr->xLeft, pr->yTop);
      PointSet(apt + 1, pr->xRight, pr->yTop);
      GpiPolyLine(hps, 2, apt);
      // bordo destro e inferiore
      GpiSetColor(hps, clrbr);       
      PointSet(apt, apt[1].x, apt[1].y);
      GpiMove(hps, apt);
      PointSet(apt, pr->xRight, pr->yBottom);
      PointSet(apt + 1, pr->xLeft + 1, pr->yBottom);
      GpiPolyLine(hps, 2, apt);
      RectInflate(pr, -1, -1);
   } /* endfor */
}  
