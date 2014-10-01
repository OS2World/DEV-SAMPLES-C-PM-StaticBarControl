//===========================================================================
// superclass.h : simple example of a static bar control implemented via
//                superclassing
// 10-01-2002 * by Alessandro Cantatore * v. 0.1
//===========================================================================


#ifndef _APIEX_BAR_H_
   #define _APIEX_BAR_H_


// class name
#define WC_BAR     "SUPERCLASSED_BAR"
/*
 Bar control styles:
 we plan to use the following DT_* flags:
 #define DT_CENTER                  0x00000100
 #define DT_RIGHT                   0x00000200
 #define DT_MNEMONIC                0x00002000
 so we define :
*/

#define BARS_HORIZONTAL        SS_GROUPBOX  // it must behave as a group box
#define BARS_VERTICAL          (SS_GROUPBOX | 0x10)   // unused SS_* flag
#define BARS_DEPRESSED         0x0000
#define BARS_RAISED            0x0020                 // unused SS_* flag
#define BARS_THICK             DT_ERASERECT // we can consider this available
#define BARS_LEFT              DT_LEFT
#define BARS_CENTER            DT_CENTER
#define BARS_RIGHT             DT_RIGHT
#define BARS_MNEMONIC          DT_MNEMONIC
#define BARS_AUTOSIZE          SS_AUTOSIZE

/*
 orientation :
 BARS_HORIZONTAL     (default) is rendered as an horizontal bar
 BARS_VERTICAL       is rendered as a vertical bar
 appearance :
 BARS_DEPRESSED      (default) paints the bar in a depressed look
 BARS_RAISED         paints a raised bar
 BARS_THICK          paints a thick (4 pixels) bar
 text alignment (valid only for the horizontal bar) :
 BARS_LEFT           (default) left aligned text
 BARS_CENTER         centered text
 BARS_RIGHT          right aligned text
 various :
 BARS_MNEMONIC       displays an underlined character, when the corresponding
                     keystroke is pressed the focus is shifted to the next
                     control
 BARS_AUTOSIZE       sets the bar size to its thickness if the bar style is
                     BARS_VERTICAL or if it is BARS_HORIZONTAL with no text,
                     otherwise sets it to the text height
 Note:
 BARS_HORIZONTAL and BARS_VERTICAL are to be considered static styles
 i.e. a change to the style flags via WinSetWindowULong() will not change
 the control.
 The control thickness can be set only to 2 or 4 pixels. To set it to a
 different thickness you must send the BARM_SETTHICKNESS message.
 BARS_DOUBLE flag stae modifications via WinSetWindowULong() are ignored.
*/

// MESSAGES: it is safe to re-use the WC_STATIC message definitions

// This allow to change the bar thickness (the control is resized if necessary)
// The new control thickness must be set as mp1, while mp2 is ignored
// The return value is BOOL : TRUE/FALSE -> success/error
#define BARM_SETTHICKNESS      SM_SETHANDLE
// This message allow to retrieve the current bar thickness. The current
// thickness is returned as message result, while both mp1 and mp2 are ignored
// The return value is the control thickness or 0xffff in case of error
#define BARM_QUERYTHICKNESS    SM_QUERYHANDLE

// macros
#define WinBarThicknessSet(hwnd, thkns) \
   ((BOOL)(WinSendMsg((hwnd), BARM_SETTHICKNESS, (MPARAM)(thkns), MPVOID)))
#define DlgBarThicknessSet(hwnd, id, thkns) \
   ((BOOL)(WinSendDlgItemMsg((hwnd), (id), BARM_SETTHICKNESS, \
                             (MPARAM)(thkns), MPVOID)))
#define WinBarThickness(hwnd) \
   ((SHORT)(WinSendMsg((hwnd), BARM_QUERYTHICKNESS, MPVOID, MPVOID))) 
#define DlgBarThickness(hwnd, id) \
   ((SHORT)(WinSendDlgItemMsg((hwnd), (id), BARM_QUERYTHICKNESS, \
                              MPVOID, MPVOID)))
                              
// macro used to check the orientation style
#define BarIsVertical(style)   ((style) & 0x10)


#endif // #ifndef _APIEX_BAR_H_
