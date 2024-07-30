/*
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    SPDX-FileCopyrightText: 2004 Jarosław Staniek <staniek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kkeyserver.h"

#include "kwindowsystem_xcb_debug.h"

#include <private/qtx11extras_p.h>

#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#include <xcb/xcb_keysyms.h>
#define X11_ONLY(arg) arg, // allows to omit an argument

#include <QCoreApplication>

namespace KKeyServer
{
//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------

struct ModInfo {
    int modQt;
    const char *psName;
    QString *sLabel; // this struct is used in static objects, so must use a pointer here.
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------

// Key names with this context are extracted elsewhere,
// no need for I18N_NOOP2's here.
#define KEYCTXT "keyboard-key-name"
static ModInfo g_rgModInfo[4] = {
    {Qt::SHIFT, "Shift", nullptr},
    {Qt::CTRL, "Ctrl", nullptr},
    {Qt::ALT, "Alt", nullptr},
    {Qt::META, "Meta", nullptr},
};

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------
static bool g_bInitializedKKeyLabels;
static bool g_bMacLabels;

static void intializeKKeyLabels()
{
    g_rgModInfo[0].sLabel = new QString(QCoreApplication::translate("KKeyServer", (g_rgModInfo[0].psName), KEYCTXT));
    g_rgModInfo[1].sLabel = new QString(QCoreApplication::translate("KKeyServer", (g_rgModInfo[1].psName), KEYCTXT));
    g_rgModInfo[2].sLabel = new QString(QCoreApplication::translate("KKeyServer", (g_rgModInfo[2].psName), KEYCTXT));
    g_rgModInfo[3].sLabel = new QString(QCoreApplication::translate("KKeyServer", (g_rgModInfo[3].psName), KEYCTXT));
    g_bMacLabels = (*g_rgModInfo[2].sLabel == QLatin1String("Command"));
    g_bInitializedKKeyLabels = true;
}

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

static QString modToString(uint mod, bool bUserSpace)
{
    if (bUserSpace && !g_bInitializedKKeyLabels) {
        intializeKKeyLabels();
    }

    QString s;
    for (int i = 3; i >= 0; i--) {
        if (mod & g_rgModInfo[i].modQt) {
            if (!s.isEmpty()) {
                s += QLatin1Char('+');
            }
            s += (bUserSpace) ? *g_rgModInfo[i].sLabel : QLatin1String(g_rgModInfo[i].psName);
        }
    }
    return s;
}

QString modToStringUser(uint mod)
{
    return modToString(mod, true);
}

uint stringUserToMod(const QString &mod)
{
    for (int i = 3; i >= 0; i--) {
        if (mod.toLower() == g_rgModInfo[i].sLabel->toLower()) {
            return g_rgModInfo[i].modQt;
        }
    }
    return 0;
}

bool isShiftAsModifierAllowed(int keyQt)
{
    // remove any modifiers
    keyQt &= ~Qt::KeyboardModifierMask;

    // Shift only works as a modifier with certain keys. It's not possible
    // to enter the SHIFT+5 key sequence for me because this is handled as
    // '%' by qt on my keyboard.
    // The working keys are all hardcoded here :-(
    if (keyQt >= Qt::Key_F1 && keyQt <= Qt::Key_F35) {
        return true;
    }

    // Returns false if not a unicode code point
    if (QChar::isLetter(keyQt)) {
        return true;
    }

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Backspace:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Escape:
    case Qt::Key_Print:
    case Qt::Key_ScrollLock:
    case Qt::Key_Pause:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Insert:
    case Qt::Key_Delete:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Enter:
    case Qt::Key_SysReq:
    case Qt::Key_CapsLock:
    case Qt::Key_NumLock:
    case Qt::Key_Help:
    case Qt::Key_Back:
    case Qt::Key_Forward:
    case Qt::Key_Stop:
    case Qt::Key_Refresh:
    case Qt::Key_Favorites:
    case Qt::Key_LaunchMedia:
    case Qt::Key_OpenUrl:
    case Qt::Key_HomePage:
    case Qt::Key_Search:
    case Qt::Key_VolumeDown:
    case Qt::Key_VolumeMute:
    case Qt::Key_VolumeUp:
    case Qt::Key_BassBoost:
    case Qt::Key_BassUp:
    case Qt::Key_BassDown:
    case Qt::Key_TrebleUp:
    case Qt::Key_TrebleDown:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaStop:
    case Qt::Key_MediaPrevious:
    case Qt::Key_MediaNext:
    case Qt::Key_MediaRecord:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
    case Qt::Key_LaunchMail:
    case Qt::Key_Calculator:
    case Qt::Key_Memo:
    case Qt::Key_ToDoList:
    case Qt::Key_Calendar:
    case Qt::Key_PowerDown:
    case Qt::Key_ContrastAdjust:
    case Qt::Key_Standby:
    case Qt::Key_MonBrightnessUp:
    case Qt::Key_MonBrightnessDown:
    case Qt::Key_KeyboardLightOnOff:
    case Qt::Key_KeyboardBrightnessUp:
    case Qt::Key_KeyboardBrightnessDown:
    case Qt::Key_PowerOff:
    case Qt::Key_WakeUp:
    case Qt::Key_Eject:
    case Qt::Key_ScreenSaver:
    case Qt::Key_WWW:
    case Qt::Key_Sleep:
    case Qt::Key_LightBulb:
    case Qt::Key_Shop:
    case Qt::Key_History:
    case Qt::Key_AddFavorite:
    case Qt::Key_HotLinks:
    case Qt::Key_BrightnessAdjust:
    case Qt::Key_Finance:
    case Qt::Key_Community:
    case Qt::Key_AudioRewind:
    case Qt::Key_BackForward:
    case Qt::Key_ApplicationLeft:
    case Qt::Key_ApplicationRight:
    case Qt::Key_Book:
    case Qt::Key_CD:
    case Qt::Key_Clear:
    case Qt::Key_ClearGrab:
    case Qt::Key_Close:
    case Qt::Key_Copy:
    case Qt::Key_Cut:
    case Qt::Key_Display:
    case Qt::Key_DOS:
    case Qt::Key_Documents:
    case Qt::Key_Excel:
    case Qt::Key_Explorer:
    case Qt::Key_Game:
    case Qt::Key_Go:
    case Qt::Key_iTouch:
    case Qt::Key_LogOff:
    case Qt::Key_Market:
    case Qt::Key_Meeting:
    case Qt::Key_MenuKB:
    case Qt::Key_MenuPB:
    case Qt::Key_MySites:
    case Qt::Key_News:
    case Qt::Key_OfficeHome:
    case Qt::Key_Option:
    case Qt::Key_Paste:
    case Qt::Key_Phone:
    case Qt::Key_Reply:
    case Qt::Key_Reload:
    case Qt::Key_RotateWindows:
    case Qt::Key_RotationPB:
    case Qt::Key_RotationKB:
    case Qt::Key_Save:
    case Qt::Key_Send:
    case Qt::Key_Spell:
    case Qt::Key_SplitScreen:
    case Qt::Key_Support:
    case Qt::Key_TaskPane:
    case Qt::Key_Terminal:
    case Qt::Key_Tools:
    case Qt::Key_Travel:
    case Qt::Key_Video:
    case Qt::Key_Word:
    case Qt::Key_Xfer:
    case Qt::Key_ZoomIn:
    case Qt::Key_ZoomOut:
    case Qt::Key_Away:
    case Qt::Key_Messenger:
    case Qt::Key_WebCam:
    case Qt::Key_MailForward:
    case Qt::Key_Pictures:
    case Qt::Key_Music:
    case Qt::Key_Battery:
    case Qt::Key_Bluetooth:
    case Qt::Key_WLAN:
    case Qt::Key_UWB:
    case Qt::Key_AudioForward:
    case Qt::Key_AudioRepeat:
    case Qt::Key_AudioRandomPlay:
    case Qt::Key_Subtitle:
    case Qt::Key_AudioCycleTrack:
    case Qt::Key_Time:
    case Qt::Key_Select:
    case Qt::Key_View:
    case Qt::Key_TopMenu:
    case Qt::Key_Suspend:
    case Qt::Key_Hibernate:
    case Qt::Key_Launch0:
    case Qt::Key_Launch1:
    case Qt::Key_Launch2:
    case Qt::Key_Launch3:
    case Qt::Key_Launch4:
    case Qt::Key_Launch5:
    case Qt::Key_Launch6:
    case Qt::Key_Launch7:
    case Qt::Key_Launch8:
    case Qt::Key_Launch9:
    case Qt::Key_LaunchA:
    case Qt::Key_LaunchB:
    case Qt::Key_LaunchC:
    case Qt::Key_LaunchD:
    case Qt::Key_LaunchE:
    case Qt::Key_LaunchF:
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
        return true;

    default:
        return false;
    }
}

// #define KKEYSERVER_DEBUG 1

//---------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------

struct Mod {
    int m_mod;
};

//---------------------------------------------------------------------
// Array Structures
//---------------------------------------------------------------------

struct X11ModInfo {
    int modQt;
    int modX;
};

struct SymVariation {
    uint sym, symVariation;
    bool bActive;
};

struct SymName {
    uint sym;
    const char *psName;
};

struct TransKey {
    int keySymQt;
    uint keySymX;
};

//---------------------------------------------------------------------
// Arrays
//---------------------------------------------------------------------
// clang-format off

static X11ModInfo g_rgX11ModInfo[4] = {
    { Qt::SHIFT,   X11_ONLY(ShiftMask) },
    { Qt::CTRL,    X11_ONLY(ControlMask) },
    { Qt::ALT,     X11_ONLY(Mod1Mask) },
    { Qt::META,    X11_ONLY(Mod4Mask) }
};

// These are the X equivalents to the Qt keycodes 0x1000 - 0x1026
static const TransKey g_rgQtToSymX[] = {
    { Qt::Key_Escape,     XK_Escape },
    { Qt::Key_Tab,        XK_Tab },
    { Qt::Key_Backtab,    XK_ISO_Left_Tab },
    { Qt::Key_Backspace,  XK_BackSpace },
    { Qt::Key_Return,     XK_Return },
    { Qt::Key_Insert,     XK_Insert },
    { Qt::Key_Delete,     XK_Delete },
    { Qt::Key_Pause,      XK_Pause },
#ifdef sun
    { Qt::Key_Print,      XK_F22 },
#else
    { Qt::Key_Print,      XK_Print },
#endif
    { Qt::Key_SysReq,     XK_Sys_Req },
    { Qt::Key_Home,       XK_Home },
    { Qt::Key_End,        XK_End },
    { Qt::Key_Left,       XK_Left },
    { Qt::Key_Up,         XK_Up },
    { Qt::Key_Right,      XK_Right },
    { Qt::Key_Down,       XK_Down },
    { Qt::Key_PageUp,      XK_Prior },
    { Qt::Key_PageDown,       XK_Next },
    { Qt::Key_Shift,      XK_Shift_L },
    { Qt::Key_Shift,      XK_Shift_R },
    { Qt::Key_Shift,      XK_Shift_Lock },
    { Qt::Key_Control,    XK_Control_L },
    { Qt::Key_Control,    XK_Control_R },
    { Qt::Key_Meta,       XK_Meta_L },
    { Qt::Key_Meta,       XK_Meta_R },
    { Qt::Key_Alt,        XK_Alt_L },
    { Qt::Key_Alt,        XK_Alt_R },
    { Qt::Key_CapsLock,   XK_Caps_Lock },
    { Qt::Key_NumLock,    XK_Num_Lock },
    { Qt::Key_ScrollLock, XK_Scroll_Lock },
    { Qt::Key_F1,         XK_F1 },
    { Qt::Key_F2,         XK_F2 },
    { Qt::Key_F3,         XK_F3 },
    { Qt::Key_F4,         XK_F4 },
    { Qt::Key_F5,         XK_F5 },
    { Qt::Key_F6,         XK_F6 },
    { Qt::Key_F7,         XK_F7 },
    { Qt::Key_F8,         XK_F8 },
    { Qt::Key_F9,         XK_F9 },
    { Qt::Key_F10,        XK_F10 },
    { Qt::Key_F11,        XK_F11 },
    { Qt::Key_F12,        XK_F12 },
    { Qt::Key_F13,        XK_F13 },
    { Qt::Key_F14,        XK_F14 },
    { Qt::Key_F15,        XK_F15 },
    { Qt::Key_F16,        XK_F16 },
    { Qt::Key_F17,        XK_F17 },
    { Qt::Key_F18,        XK_F18 },
    { Qt::Key_F19,        XK_F19 },
    { Qt::Key_F20,        XK_F20 },
    { Qt::Key_F21,        XK_F21 },
    { Qt::Key_F22,        XK_F22 },
    { Qt::Key_F23,        XK_F23 },
    { Qt::Key_F24,        XK_F24 },
    { Qt::Key_F25,        XK_F25 },
    { Qt::Key_F26,        XK_F26 },
    { Qt::Key_F27,        XK_F27 },
    { Qt::Key_F28,        XK_F28 },
    { Qt::Key_F29,        XK_F29 },
    { Qt::Key_F30,        XK_F30 },
    { Qt::Key_F31,        XK_F31 },
    { Qt::Key_F32,        XK_F32 },
    { Qt::Key_F33,        XK_F33 },
    { Qt::Key_F34,        XK_F34 },
    { Qt::Key_F35,        XK_F35 },
    { Qt::Key_Super_L,    XK_Super_L },
    { Qt::Key_Super_R,    XK_Super_R },
    { Qt::Key_Menu,       XK_Menu },
    { Qt::Key_Hyper_L,    XK_Hyper_L },
    { Qt::Key_Hyper_R,    XK_Hyper_R },
    { Qt::Key_Help,       XK_Help },
    //{ Qt::Key_Direction_L, XK_Direction_L }, These keys don't exist in X11
    //{ Qt::Key_Direction_R, XK_Direction_R },

    { Qt::Key_Space,      XK_KP_Space },
    { Qt::Key_Tab,        XK_KP_Tab },
    { Qt::Key_Enter,      XK_KP_Enter },
    { Qt::Key_Home,       XK_KP_Home },
    { Qt::Key_Left,       XK_KP_Left },
    { Qt::Key_Up,         XK_KP_Up },
    { Qt::Key_Right,      XK_KP_Right },
    { Qt::Key_Down,       XK_KP_Down },
    { Qt::Key_PageUp,     XK_KP_Prior },
    { Qt::Key_PageDown,   XK_KP_Next },
    { Qt::Key_End,        XK_KP_End },
    { Qt::Key_Clear,      XK_KP_Begin },
    { Qt::Key_Insert,     XK_KP_Insert },
    { Qt::Key_Delete,     XK_KP_Delete },
    { Qt::Key_Equal,      XK_KP_Equal },
    { Qt::Key_Asterisk,   XK_KP_Multiply },
    { Qt::Key_Plus,       XK_KP_Add },
    { Qt::Key_Comma,      XK_KP_Separator },
    { Qt::Key_Minus,      XK_KP_Subtract },
    { Qt::Key_Period,     XK_KP_Decimal },
    { Qt::Key_Slash,      XK_KP_Divide },

// the next lines are taken on 01/2024 from X.org (X11/XF86keysym.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_ModeLock              0x1008ff01  /* Mode Switch Lock */

/* Backlight controls. */
#define XF86XK_MonBrightnessUp       0x1008ff02  /* Monitor/panel brightness */
#define XF86XK_MonBrightnessDown     0x1008ff03  /* Monitor/panel brightness */
#define XF86XK_KbdLightOnOff         0x1008ff04  /* Keyboards may be lit     */
#define XF86XK_KbdBrightnessUp       0x1008ff05  /* Keyboards may be lit     */
#define XF86XK_KbdBrightnessDown     0x1008ff06  /* Keyboards may be lit     */
#define XF86XK_MonBrightnessCycle    0x1008ff07  /* Monitor/panel brightness */

/*
 * Keys found on some "Internet" keyboards.
 */
#define XF86XK_Standby               0x1008ff10  /* System into standby mode   */
#define XF86XK_AudioLowerVolume      0x1008ff11  /* Volume control down        */
#define XF86XK_AudioMute             0x1008ff12  /* Mute sound from the system */
#define XF86XK_AudioRaiseVolume      0x1008ff13  /* Volume control up          */
#define XF86XK_AudioPlay             0x1008ff14  /* Start playing of audio >   */
#define XF86XK_AudioStop             0x1008ff15  /* Stop playing audio         */
#define XF86XK_AudioPrev             0x1008ff16  /* Previous track             */
#define XF86XK_AudioNext             0x1008ff17  /* Next track                 */
#define XF86XK_HomePage              0x1008ff18  /* Display user's home page   */
#define XF86XK_Mail                  0x1008ff19  /* Invoke user's mail program */
#define XF86XK_Start                 0x1008ff1a  /* Start application          */
#define XF86XK_Search                0x1008ff1b  /* Search                     */
#define XF86XK_AudioRecord           0x1008ff1c  /* Record audio application   */

/* These are sometimes found on PDA's (e.g. Palm, PocketPC or elsewhere)   */
#define XF86XK_Calculator            0x1008ff1d  /* Invoke calculator program  */
#define XF86XK_Memo                  0x1008ff1e  /* Invoke Memo taking program */
#define XF86XK_ToDoList              0x1008ff1f  /* Invoke To Do List program  */
#define XF86XK_Calendar              0x1008ff20  /* Invoke Calendar program    */
#define XF86XK_PowerDown             0x1008ff21  /* Deep sleep the system      */
#define XF86XK_ContrastAdjust        0x1008ff22  /* Adjust screen contrast     */
#define XF86XK_RockerUp              0x1008ff23  /* Rocker switches exist up   */
#define XF86XK_RockerDown            0x1008ff24  /* and down                   */
#define XF86XK_RockerEnter           0x1008ff25  /* and let you press them     */

/* Some more "Internet" keyboard symbols */
#define XF86XK_Back                  0x1008ff26  /* Like back on a browser     */
#define XF86XK_Forward               0x1008ff27  /* Like forward on a browser  */
#define XF86XK_Stop                  0x1008ff28  /* Stop current operation     */
#define XF86XK_Refresh               0x1008ff29  /* Refresh the page           */
#define XF86XK_PowerOff              0x1008ff2a  /* Power off system entirely  */
#define XF86XK_WakeUp                0x1008ff2b  /* Wake up system from sleep  */
#define XF86XK_Eject                 0x1008ff2c  /* Eject device (e.g. DVD)    */
#define XF86XK_ScreenSaver           0x1008ff2d  /* Invoke screensaver         */
#define XF86XK_WWW                   0x1008ff2e  /* Invoke web browser         */
#define XF86XK_Sleep                 0x1008ff2f  /* Put system to sleep        */
#define XF86XK_Favorites             0x1008ff30  /* Show favorite locations    */
#define XF86XK_AudioPause            0x1008ff31  /* Pause audio playing        */
#define XF86XK_AudioMedia            0x1008ff32  /* Launch media collection app */
#define XF86XK_MyComputer            0x1008ff33  /* Display "My Computer" window */
#define XF86XK_VendorHome            0x1008ff34  /* Display vendor home web site */
#define XF86XK_LightBulb             0x1008ff35  /* Light bulb keys exist       */
#define XF86XK_Shop                  0x1008ff36  /* Display shopping web site   */
#define XF86XK_History               0x1008ff37  /* Show history of web surfing */
#define XF86XK_OpenURL               0x1008ff38  /* Open selected URL           */
#define XF86XK_AddFavorite           0x1008ff39  /* Add URL to favorites list   */
#define XF86XK_HotLinks              0x1008ff3a  /* Show "hot" links            */
#define XF86XK_BrightnessAdjust      0x1008ff3b  /* Invoke brightness adj. UI   */
#define XF86XK_Finance               0x1008ff3c  /* Display financial site      */
#define XF86XK_Community             0x1008ff3d  /* Display user's community    */
#define XF86XK_AudioRewind           0x1008ff3e  /* "rewind" audio track        */
#define XF86XK_BackForward           0x1008ff3f  /* ??? */
#define XF86XK_Launch0               0x1008ff40  /* Launch Application          */
#define XF86XK_Launch1               0x1008ff41  /* Launch Application          */
#define XF86XK_Launch2               0x1008ff42  /* Launch Application          */
#define XF86XK_Launch3               0x1008ff43  /* Launch Application          */
#define XF86XK_Launch4               0x1008ff44  /* Launch Application          */
#define XF86XK_Launch5               0x1008ff45  /* Launch Application          */
#define XF86XK_Launch6               0x1008ff46  /* Launch Application          */
#define XF86XK_Launch7               0x1008ff47  /* Launch Application          */
#define XF86XK_Launch8               0x1008ff48  /* Launch Application          */
#define XF86XK_Launch9               0x1008ff49  /* Launch Application          */
#define XF86XK_LaunchA               0x1008ff4a  /* Launch Application          */
#define XF86XK_LaunchB               0x1008ff4b  /* Launch Application          */
#define XF86XK_LaunchC               0x1008ff4c  /* Launch Application          */
#define XF86XK_LaunchD               0x1008ff4d  /* Launch Application          */
#define XF86XK_LaunchE               0x1008ff4e  /* Launch Application          */
#define XF86XK_LaunchF               0x1008ff4f  /* Launch Application          */

#define XF86XK_ApplicationLeft       0x1008ff50  /* switch to application, left */
#define XF86XK_ApplicationRight      0x1008ff51  /* switch to application, right*/
#define XF86XK_Book                  0x1008ff52  /* Launch bookreader           */
#define XF86XK_CD                    0x1008ff53  /* Launch CD/DVD player        */
#define XF86XK_Calculater            0x1008ff54  /* Launch Calculater           */
#define XF86XK_Clear                 0x1008ff55  /* Clear window, screen        */
#define XF86XK_Close                 0x1008ff56  /* Close window                */
#define XF86XK_Copy                  0x1008ff57  /* Copy selection              */
#define XF86XK_Cut                   0x1008ff58  /* Cut selection               */
#define XF86XK_Display               0x1008ff59  /* Output switch key           */
#define XF86XK_DOS                   0x1008ff5a  /* Launch DOS (emulation)      */
#define XF86XK_Documents             0x1008ff5b  /* Open documents window       */
#define XF86XK_Excel                 0x1008ff5c  /* Launch spread sheet         */
#define XF86XK_Explorer              0x1008ff5d  /* Launch file explorer        */
#define XF86XK_Game                  0x1008ff5e  /* Launch game                 */
#define XF86XK_Go                    0x1008ff5f  /* Go to URL                   */
#define XF86XK_iTouch                0x1008ff60  /* Logitech iTouch- don't use  */
#define XF86XK_LogOff                0x1008ff61  /* Log off system              */
#define XF86XK_Market                0x1008ff62  /* ??                          */
#define XF86XK_Meeting               0x1008ff63  /* enter meeting in calendar   */
#define XF86XK_MenuKB                0x1008ff65  /* distinguish keyboard from PB */
#define XF86XK_MenuPB                0x1008ff66  /* distinguish PB from keyboard */
#define XF86XK_MySites               0x1008ff67  /* Favourites                  */
#define XF86XK_New                   0x1008ff68  /* New (folder, document...    */
#define XF86XK_News                  0x1008ff69  /* News                        */
#define XF86XK_OfficeHome            0x1008ff6a  /* Office home (old Staroffice)*/
#define XF86XK_Open                  0x1008ff6b  /* Open                        */
#define XF86XK_Option                0x1008ff6c  /* ?? */
#define XF86XK_Paste                 0x1008ff6d  /* Paste                       */
#define XF86XK_Phone                 0x1008ff6e  /* Launch phone; dial number   */
#define XF86XK_Q                     0x1008ff70  /* Compaq's Q - don't use      */
#define XF86XK_Reply                 0x1008ff72  /* Reply e.g., mail            */
#define XF86XK_Reload                0x1008ff73  /* Reload web page, file, etc. */
#define XF86XK_RotateWindows         0x1008ff74  /* Rotate windows e.g. xrandr  */
#define XF86XK_RotationPB            0x1008ff75  /* don't use                   */
#define XF86XK_RotationKB            0x1008ff76  /* don't use                   */
#define XF86XK_Save                  0x1008ff77  /* Save (file, document, state */
#define XF86XK_ScrollUp              0x1008ff78  /* Scroll window/contents up   */
#define XF86XK_ScrollDown            0x1008ff79  /* Scrool window/contentd down */
#define XF86XK_ScrollClick           0x1008ff7a  /* Use XKB mousekeys instead   */
#define XF86XK_Send                  0x1008ff7b  /* Send mail, file, object     */
#define XF86XK_Spell                 0x1008ff7c  /* Spell checker               */
#define XF86XK_SplitScreen           0x1008ff7d  /* Split window or screen      */
#define XF86XK_Support               0x1008ff7e  /* Get support (??)            */
#define XF86XK_TaskPane              0x1008ff7f  /* Show tasks */
#define XF86XK_Terminal              0x1008ff80  /* Launch terminal emulator    */
#define XF86XK_Tools                 0x1008ff81  /* toolbox of desktop/app.     */
#define XF86XK_Travel                0x1008ff82  /* ?? */
#define XF86XK_UserPB                0x1008ff84  /* ?? */
#define XF86XK_User1KB               0x1008ff85  /* ?? */
#define XF86XK_User2KB               0x1008ff86  /* ?? */
#define XF86XK_Video                 0x1008ff87  /* Launch video player       */
#define XF86XK_WheelButton           0x1008ff88  /* button from a mouse wheel */
#define XF86XK_Word                  0x1008ff89  /* Launch word processor     */
#define XF86XK_Xfer                  0x1008ff8a
#define XF86XK_ZoomIn                0x1008ff8b  /* zoom in view, map, etc.   */
#define XF86XK_ZoomOut               0x1008ff8c  /* zoom out view, map, etc.  */

#define XF86XK_Away                  0x1008ff8d  /* mark yourself as away     */
#define XF86XK_Messenger             0x1008ff8e  /* as in instant messaging   */
#define XF86XK_WebCam                0x1008ff8f  /* Launch web camera app.    */
#define XF86XK_MailForward           0x1008ff90  /* Forward in mail           */
#define XF86XK_Pictures              0x1008ff91  /* Show pictures             */
#define XF86XK_Music                 0x1008ff92  /* Launch music application  */

#define XF86XK_Battery               0x1008ff93  /* Display battery information */
#define XF86XK_Bluetooth             0x1008ff94  /* Enable/disable Bluetooth    */
#define XF86XK_WLAN                  0x1008ff95  /* Enable/disable WLAN         */
#define XF86XK_UWB                   0x1008ff96  /* Enable/disable UWB	    */

#define XF86XK_AudioForward          0x1008ff97  /* fast-forward audio track    */
#define XF86XK_AudioRepeat           0x1008ff98  /* toggle repeat mode          */
#define XF86XK_AudioRandomPlay       0x1008ff99  /* toggle shuffle mode         */
#define XF86XK_Subtitle              0x1008ff9a  /* cycle through subtitle      */
#define XF86XK_AudioCycleTrack       0x1008ff9b  /* cycle through audio tracks  */
#define XF86XK_CycleAngle            0x1008ff9c  /* cycle through angles        */
#define XF86XK_FrameBack             0x1008ff9d  /* video: go one frame back    */
#define XF86XK_FrameForward          0x1008ff9e  /* video: go one frame forward */
#define XF86XK_Time                  0x1008ff9f  /* display, or shows an entry for time seeking */
#define XF86XK_Select                0x1008ffa0  /* Select button on joypads and remotes */
#define XF86XK_View                  0x1008ffa1  /* Show a view options/properties */
#define XF86XK_TopMenu               0x1008ffa2  /* Go to a top-level menu in a video */

#define XF86XK_Red                   0x1008ffa3  /* Red button                  */
#define XF86XK_Green                 0x1008ffa4  /* Green button                */
#define XF86XK_Yellow                0x1008ffa5  /* Yellow button               */
#define XF86XK_Blue                  0x1008ffa6  /* Blue button                 */

#define XF86XK_Suspend               0x1008ffa7  /* Sleep to RAM                */
#define XF86XK_Hibernate             0x1008ffa8  /* Sleep to disk               */
#define XF86XK_TouchpadToggle        0x1008ffa9  /* Toggle between touchpad/trackstick */
#define XF86XK_TouchpadOn            0x1008ffb0  /* The touchpad got switched on */
#define XF86XK_TouchpadOff           0x1008ffb1  /* The touchpad got switched off */

#define XF86XK_AudioMicMute          0x1008ffb2  /* Mute the Mic from the system */

#define XF86XK_Keyboard              0x1008ffb3  /* User defined keyboard related action */

#define XF86XK_WWAN                  0x1008ffb4  /* Toggle WWAN (LTE, UMTS, etc.) radio */
#define XF86XK_RFKill                0x1008ffb5  /* Toggle radios on/off */

#define XF86XK_AudioPreset           0x1008ffb6  /* Select equalizer preset, e.g. theatre-mode */

#define XF86XK_RotationLockToggle    0x1008ffb7  /* Toggle screen rotation lock on/off */

#define XF86XK_FullScreen            0x1008ffb8  /* Toggle fullscreen */

/* Keys for special action keys (hot keys) */
/* Virtual terminals on some operating systems */
#define XF86XK_Switch_VT_1           0x1008fe01
#define XF86XK_Switch_VT_2           0x1008fe02
#define XF86XK_Switch_VT_3           0x1008fe03
#define XF86XK_Switch_VT_4           0x1008fe04
#define XF86XK_Switch_VT_5           0x1008fe05
#define XF86XK_Switch_VT_6           0x1008fe06
#define XF86XK_Switch_VT_7           0x1008fe07
#define XF86XK_Switch_VT_8           0x1008fe08
#define XF86XK_Switch_VT_9           0x1008fe09
#define XF86XK_Switch_VT_10          0x1008fe0a
#define XF86XK_Switch_VT_11          0x1008fe0b
#define XF86XK_Switch_VT_12          0x1008fe0c

#define XF86XK_Ungrab                0x1008fe20  /* force ungrab               */
#define XF86XK_ClearGrab             0x1008fe21  /* kill application with grab */
#define XF86XK_Next_VMode            0x1008fe22  /* next video mode available  */
#define XF86XK_Prev_VMode            0x1008fe23  /* prev. video mode available */
#define XF86XK_LogWindowTree         0x1008fe24  /* print window tree to log   */
#define XF86XK_LogGrabInfo           0x1008fe25  /* print all active grabs to log */


/*
 * Reserved range for evdev symbols: 0x10081000-0x10081FFF
 *
 * Key syms within this range must match the Linux kernel
 * input-event-codes.h file in the format:
 *     XF86XK_CamelCaseKernelName	_EVDEVK(kernel value)
 * For example, the kernel
 *   #define KEY_MACRO_RECORD_START	0x2b0
 * effectively ends up as:
 *   #define XF86XK_MacroRecordStart	0x100812b0
 *
 * For historical reasons, some keysyms within the reserved range will be
 * missing, most notably all "normal" keys that are mapped through default
 * XKB layouts (e.g. KEY_Q).
 *
 * CamelCasing is done with a human control as last authority, e.g. see VOD
 * instead of Vod for the Video on Demand key.
 *
 * The format for #defines is strict:
 *
 * #define XF86XK_FOO<tab...>_EVDEVK(0xABC)<tab><tab> |* kver KEY_FOO *|
 *
 * Where
 * - alignment by tabs
 * - the _EVDEVK macro must be used
 * - the hex code must be in uppercase hex
 * - the kernel version (kver) is in the form v5.10
 * - kver and key name are within a slash-star comment (a pipe is used in
 *   this example for technical reasons)
 * These #defines are parsed by scripts. Do not stray from the given format.
 *
 * Where the evdev keycode is mapped to a different symbol, please add a
 * comment line starting with Use: but otherwise the same format, e.g.
 *  Use: XF86XK_RotationLockToggle	_EVDEVK(0x231)		   v4.16 KEY_ROTATE_LOCK_TOGGLE
 *
 */
#define _EVDEVK(_v) (0x10081000 + _v)
/* Use: XF86XK_Eject                    _EVDEVK(0x0a2)             KEY_EJECTCLOSECD */
/* Use: XF86XK_New                      _EVDEVK(0x0b5)     v2.6.14 KEY_NEW */
/* Use: XK_Redo                         _EVDEVK(0x0b6)     v2.6.14 KEY_REDO */
/* KEY_DASHBOARD has been mapped to LaunchB in xkeyboard-config since 2011 */
/* Use: XF86XK_LaunchB                  _EVDEVK(0x0cc)     v2.6.28 KEY_DASHBOARD */
/* Use: XF86XK_Display                  _EVDEVK(0x0e3)     v2.6.12 KEY_SWITCHVIDEOMODE */
/* Use: XF86XK_KbdLightOnOff            _EVDEVK(0x0e4)     v2.6.12 KEY_KBDILLUMTOGGLE */
/* Use: XF86XK_KbdBrightnessDown        _EVDEVK(0x0e5)     v2.6.12 KEY_KBDILLUMDOWN */
/* Use: XF86XK_KbdBrightnessUp          _EVDEVK(0x0e6)     v2.6.12 KEY_KBDILLUMUP */
/* Use: XF86XK_Send                     _EVDEVK(0x0e7)     v2.6.14 KEY_SEND */
/* Use: XF86XK_Reply                    _EVDEVK(0x0e8)     v2.6.14 KEY_REPLY */
/* Use: XF86XK_MailForward              _EVDEVK(0x0e9)     v2.6.14 KEY_FORWARDMAIL */
/* Use: XF86XK_Save                     _EVDEVK(0x0ea)     v2.6.14 KEY_SAVE */
/* Use: XF86XK_Documents                _EVDEVK(0x0eb)     v2.6.14 KEY_DOCUMENTS */
/* Use: XF86XK_Battery                  _EVDEVK(0x0ec)     v2.6.17 KEY_BATTERY */
/* Use: XF86XK_Bluetooth                _EVDEVK(0x0ed)     v2.6.19 KEY_BLUETOOTH */
/* Use: XF86XK_WLAN                     _EVDEVK(0x0ee)     v2.6.19 KEY_WLAN */
/* Use: XF86XK_UWB                      _EVDEVK(0x0ef)     v2.6.24 KEY_UWB */
/* Use: XF86XK_Next_VMode               _EVDEVK(0x0f1)     v2.6.23 KEY_VIDEO_NEXT */
/* Use: XF86XK_Prev_VMode               _EVDEVK(0x0f2)     v2.6.23 KEY_VIDEO_PREV */
/* Use: XF86XK_MonBrightnessCycle       _EVDEVK(0x0f3)     v2.6.23 KEY_BRIGHTNESS_CYCLE */
#define XF86XK_BrightnessAuto           _EVDEVK(0x0f4)  /* v3.16   KEY_BRIGHTNESS_AUTO */
#define XF86XK_DisplayOff               _EVDEVK(0x0f5)  /* v2.6.23 KEY_DISPLAY_OFF */
/* Use: XF86XK_WWAN                     _EVDEVK(0x0f6)     v3.13   KEY_WWAN */
/* Use: XF86XK_RFKill                   _EVDEVK(0x0f7)     v2.6.33 KEY_RFKILL */
/* Use: XF86XK_AudioMicMute             _EVDEVK(0x0f8)     v3.1    KEY_MICMUTE */
#define XF86XK_Info                     _EVDEVK(0x166)  /*         KEY_INFO */
/* Use: XF86XK_CycleAngle               _EVDEVK(0x173)             KEY_ANGLE */
/* Use: XF86XK_FullScreen               _EVDEVK(0x174)     v5.1    KEY_FULL_SCREEN */
#define XF86XK_AspectRatio              _EVDEVK(0x177)  /* v5.1    KEY_ASPECT_RATIO */
#define XF86XK_DVD                      _EVDEVK(0x185)  /*         KEY_DVD */
#define XF86XK_Audio                    _EVDEVK(0x188)  /*         KEY_AUDIO */
/* Use: XF86XK_Video                    _EVDEVK(0x189)             KEY_VIDEO */
/* Use: XF86XK_Calendar                 _EVDEVK(0x18d)             KEY_CALENDAR */
#define XF86XK_ChannelUp                _EVDEVK(0x192)  /*         KEY_CHANNELUP */
#define XF86XK_ChannelDown              _EVDEVK(0x193)  /*         KEY_CHANNELDOWN */
/* Use: XF86XK_AudioRandomPlay          _EVDEVK(0x19a)             KEY_SHUFFLE */
#define XF86XK_Break                    _EVDEVK(0x19b)  /*         KEY_BREAK */
#define XF86XK_VideoPhone               _EVDEVK(0x1a0)  /* v2.6.20 KEY_VIDEOPHONE */
/* Use: XF86XK_Game                     _EVDEVK(0x1a1)     v2.6.20 KEY_GAMES */
/* Use: XF86XK_ZoomIn                   _EVDEVK(0x1a2)     v2.6.20 KEY_ZOOMIN */
/* Use: XF86XK_ZoomOut                  _EVDEVK(0x1a3)     v2.6.20 KEY_ZOOMOUT */
#define XF86XK_ZoomReset                _EVDEVK(0x1a4)  /* v2.6.20 KEY_ZOOMRESET */
/* Use: XF86XK_Word                     _EVDEVK(0x1a5)     v2.6.20 KEY_WORDPROCESSOR */
#define XF86XK_Editor                   _EVDEVK(0x1a6)  /* v2.6.20 KEY_EDITOR */
/* Use: XF86XK_Excel                    _EVDEVK(0x1a7)     v2.6.20 KEY_SPREADSHEET */
#define XF86XK_GraphicsEditor           _EVDEVK(0x1a8)  /* v2.6.20 KEY_GRAPHICSEDITOR */
#define XF86XK_Presentation             _EVDEVK(0x1a9)  /* v2.6.20 KEY_PRESENTATION */
#define XF86XK_Database                 _EVDEVK(0x1aa)  /* v2.6.20 KEY_DATABASE */
/* Use: XF86XK_News                     _EVDEVK(0x1ab)     v2.6.20 KEY_NEWS */
#define XF86XK_Voicemail                _EVDEVK(0x1ac)  /* v2.6.20 KEY_VOICEMAIL */
#define XF86XK_Addressbook              _EVDEVK(0x1ad)  /* v2.6.20 KEY_ADDRESSBOOK */
/* Use: XF86XK_Messenger                _EVDEVK(0x1ae)     v2.6.20 KEY_MESSENGER */
#define XF86XK_DisplayToggle            _EVDEVK(0x1af)  /* v2.6.20 KEY_DISPLAYTOGGLE */
#define XF86XK_SpellCheck               _EVDEVK(0x1b0)  /* v2.6.24 KEY_SPELLCHECK */
/* Use: XF86XK_LogOff                   _EVDEVK(0x1b1)     v2.6.24 KEY_LOGOFF */
/* Use: XK_dollar                       _EVDEVK(0x1b2)     v2.6.24 KEY_DOLLAR */
/* Use: XK_EuroSign                     _EVDEVK(0x1b3)     v2.6.24 KEY_EURO */
/* Use: XF86XK_FrameBack                _EVDEVK(0x1b4)     v2.6.24 KEY_FRAMEBACK */
/* Use: XF86XK_FrameForward             _EVDEVK(0x1b5)     v2.6.24 KEY_FRAMEFORWARD */
#define XF86XK_ContextMenu              _EVDEVK(0x1b6)  /* v2.6.24 KEY_CONTEXT_MENU */
#define XF86XK_MediaRepeat              _EVDEVK(0x1b7)  /* v2.6.26 KEY_MEDIA_REPEAT */
#define XF86XK_10ChannelsUp             _EVDEVK(0x1b8)  /* v2.6.38 KEY_10CHANNELSUP */
#define XF86XK_10ChannelsDown           _EVDEVK(0x1b9)  /* v2.6.38 KEY_10CHANNELSDOWN */
#define XF86XK_Images                   _EVDEVK(0x1ba)  /* v2.6.39 KEY_IMAGES */
#define XF86XK_NotificationCenter       _EVDEVK(0x1bc)  /* v5.10   KEY_NOTIFICATION_CENTER */
#define XF86XK_PickupPhone              _EVDEVK(0x1bd)  /* v5.10   KEY_PICKUP_PHONE */
#define XF86XK_HangupPhone              _EVDEVK(0x1be)  /* v5.10   KEY_HANGUP_PHONE */
#define XF86XK_Fn                       _EVDEVK(0x1d0)  /*         KEY_FN */
#define XF86XK_Fn_Esc                   _EVDEVK(0x1d1)  /*         KEY_FN_ESC */
#define XF86XK_FnRightShift             _EVDEVK(0x1e5)  /* v5.10   KEY_FN_RIGHT_SHIFT */
/* Use: XK_braille_dot_1                _EVDEVK(0x1f1)     v2.6.17 KEY_BRL_DOT1 */
/* Use: XK_braille_dot_2                _EVDEVK(0x1f2)     v2.6.17 KEY_BRL_DOT2 */
/* Use: XK_braille_dot_3                _EVDEVK(0x1f3)     v2.6.17 KEY_BRL_DOT3 */
/* Use: XK_braille_dot_4                _EVDEVK(0x1f4)     v2.6.17 KEY_BRL_DOT4 */
/* Use: XK_braille_dot_5                _EVDEVK(0x1f5)     v2.6.17 KEY_BRL_DOT5 */
/* Use: XK_braille_dot_6                _EVDEVK(0x1f6)     v2.6.17 KEY_BRL_DOT6 */
/* Use: XK_braille_dot_7                _EVDEVK(0x1f7)     v2.6.17 KEY_BRL_DOT7 */
/* Use: XK_braille_dot_8                _EVDEVK(0x1f8)     v2.6.17 KEY_BRL_DOT8 */
/* Use: XK_braille_dot_9                _EVDEVK(0x1f9)     v2.6.23 KEY_BRL_DOT9 */
/* Use: XK_braille_dot_1                _EVDEVK(0x1fa)     v2.6.23 KEY_BRL_DOT10 */
#define XF86XK_Numeric0                 _EVDEVK(0x200)  /* v2.6.28 KEY_NUMERIC_0 */
#define XF86XK_Numeric1                 _EVDEVK(0x201)  /* v2.6.28 KEY_NUMERIC_1 */
#define XF86XK_Numeric2                 _EVDEVK(0x202)  /* v2.6.28 KEY_NUMERIC_2 */
#define XF86XK_Numeric3                 _EVDEVK(0x203)  /* v2.6.28 KEY_NUMERIC_3 */
#define XF86XK_Numeric4                 _EVDEVK(0x204)  /* v2.6.28 KEY_NUMERIC_4 */
#define XF86XK_Numeric5                 _EVDEVK(0x205)  /* v2.6.28 KEY_NUMERIC_5 */
#define XF86XK_Numeric6                 _EVDEVK(0x206)  /* v2.6.28 KEY_NUMERIC_6 */
#define XF86XK_Numeric7                 _EVDEVK(0x207)  /* v2.6.28 KEY_NUMERIC_7 */
#define XF86XK_Numeric8                 _EVDEVK(0x208)  /* v2.6.28 KEY_NUMERIC_8 */
#define XF86XK_Numeric9                 _EVDEVK(0x209)  /* v2.6.28 KEY_NUMERIC_9 */
#define XF86XK_NumericStar              _EVDEVK(0x20a)  /* v2.6.28 KEY_NUMERIC_STAR */
#define XF86XK_NumericPound             _EVDEVK(0x20b)  /* v2.6.28 KEY_NUMERIC_POUND */
#define XF86XK_NumericA                 _EVDEVK(0x20c)  /* v4.1    KEY_NUMERIC_A */
#define XF86XK_NumericB                 _EVDEVK(0x20d)  /* v4.1    KEY_NUMERIC_B */
#define XF86XK_NumericC                 _EVDEVK(0x20e)  /* v4.1    KEY_NUMERIC_C */
#define XF86XK_NumericD                 _EVDEVK(0x20f)  /* v4.1    KEY_NUMERIC_D */
#define XF86XK_CameraFocus              _EVDEVK(0x210)  /* v2.6.33 KEY_CAMERA_FOCUS */
#define XF86XK_WPSButton                _EVDEVK(0x211)  /* v2.6.34 KEY_WPS_BUTTON */
/* Use: XF86XK_TouchpadToggle           _EVDEVK(0x212)     v2.6.37 KEY_TOUCHPAD_TOGGLE */
/* Use: XF86XK_TouchpadOn               _EVDEVK(0x213)     v2.6.37 KEY_TOUCHPAD_ON */
/* Use: XF86XK_TouchpadOff              _EVDEVK(0x214)     v2.6.37 KEY_TOUCHPAD_OFF */
#define XF86XK_CameraZoomIn             _EVDEVK(0x215)  /* v2.6.39 KEY_CAMERA_ZOOMIN */
#define XF86XK_CameraZoomOut            _EVDEVK(0x216)  /* v2.6.39 KEY_CAMERA_ZOOMOUT */
#define XF86XK_CameraUp                 _EVDEVK(0x217)  /* v2.6.39 KEY_CAMERA_UP */
#define XF86XK_CameraDown               _EVDEVK(0x218)  /* v2.6.39 KEY_CAMERA_DOWN */
#define XF86XK_CameraLeft               _EVDEVK(0x219)  /* v2.6.39 KEY_CAMERA_LEFT */
#define XF86XK_CameraRight              _EVDEVK(0x21a)  /* v2.6.39 KEY_CAMERA_RIGHT */
#define XF86XK_AttendantOn              _EVDEVK(0x21b)  /* v3.10   KEY_ATTENDANT_ON */
#define XF86XK_AttendantOff             _EVDEVK(0x21c)  /* v3.10   KEY_ATTENDANT_OFF */
#define XF86XK_AttendantToggle          _EVDEVK(0x21d)  /* v3.10   KEY_ATTENDANT_TOGGLE */
#define XF86XK_LightsToggle             _EVDEVK(0x21e)  /* v3.10   KEY_LIGHTS_TOGGLE */
#define XF86XK_ALSToggle                _EVDEVK(0x230)  /* v3.13   KEY_ALS_TOGGLE */
/* Use: XF86XK_RotationLockToggle       _EVDEVK(0x231)     v4.16   KEY_ROTATE_LOCK_TOGGLE */
#define XF86XK_Buttonconfig             _EVDEVK(0x240)  /* v3.16   KEY_BUTTONCONFIG */
#define XF86XK_Taskmanager              _EVDEVK(0x241)  /* v3.16   KEY_TASKMANAGER */
#define XF86XK_Journal                  _EVDEVK(0x242)  /* v3.16   KEY_JOURNAL */
#define XF86XK_ControlPanel             _EVDEVK(0x243)  /* v3.16   KEY_CONTROLPANEL */
#define XF86XK_AppSelect                _EVDEVK(0x244)  /* v3.16   KEY_APPSELECT */
#define XF86XK_Screensaver              _EVDEVK(0x245)  /* v3.16   KEY_SCREENSAVER */
#define XF86XK_VoiceCommand             _EVDEVK(0x246)  /* v3.16   KEY_VOICECOMMAND */
#define XF86XK_Assistant                _EVDEVK(0x247)  /* v4.13   KEY_ASSISTANT */
/* Use: XK_ISO_Next_Group               _EVDEVK(0x248)     v5.2    KEY_KBD_LAYOUT_NEXT */
#define XF86XK_EmojiPicker              _EVDEVK(0x249)  /* v5.13   KEY_EMOJI_PICKER */
#define XF86XK_Dictate                  _EVDEVK(0x24a)  /* v5.17   KEY_DICTATE */
#define XF86XK_CameraAccessEnable       _EVDEVK(0x24b)  /* v6.2    KEY_CAMERA_ACCESS_ENABLE */
#define XF86XK_CameraAccessDisable      _EVDEVK(0x24c)  /* v6.2    KEY_CAMERA_ACCESS_DISABLE */
#define XF86XK_CameraAccessToggle       _EVDEVK(0x24d)  /* v6.2    KEY_CAMERA_ACCESS_TOGGLE */
#define XF86XK_BrightnessMin            _EVDEVK(0x250)  /* v3.16   KEY_BRIGHTNESS_MIN */
#define XF86XK_BrightnessMax            _EVDEVK(0x251)  /* v3.16   KEY_BRIGHTNESS_MAX */
#define XF86XK_KbdInputAssistPrev       _EVDEVK(0x260)  /* v3.18   KEY_KBDINPUTASSIST_PREV */
#define XF86XK_KbdInputAssistNext       _EVDEVK(0x261)  /* v3.18   KEY_KBDINPUTASSIST_NEXT */
#define XF86XK_KbdInputAssistPrevgroup  _EVDEVK(0x262)  /* v3.18   KEY_KBDINPUTASSIST_PREVGROUP */
#define XF86XK_KbdInputAssistNextgroup  _EVDEVK(0x263)  /* v3.18   KEY_KBDINPUTASSIST_NEXTGROUP */
#define XF86XK_KbdInputAssistAccept     _EVDEVK(0x264)  /* v3.18   KEY_KBDINPUTASSIST_ACCEPT */
#define XF86XK_KbdInputAssistCancel     _EVDEVK(0x265)  /* v3.18   KEY_KBDINPUTASSIST_CANCEL */
#define XF86XK_RightUp                  _EVDEVK(0x266)  /* v4.7    KEY_RIGHT_UP */
#define XF86XK_RightDown                _EVDEVK(0x267)  /* v4.7    KEY_RIGHT_DOWN */
#define XF86XK_LeftUp                   _EVDEVK(0x268)  /* v4.7    KEY_LEFT_UP */
#define XF86XK_LeftDown                 _EVDEVK(0x269)  /* v4.7    KEY_LEFT_DOWN */
#define XF86XK_RootMenu                 _EVDEVK(0x26a)  /* v4.7    KEY_ROOT_MENU */
#define XF86XK_MediaTopMenu             _EVDEVK(0x26b)  /* v4.7    KEY_MEDIA_TOP_MENU */
#define XF86XK_Numeric11                _EVDEVK(0x26c)  /* v4.7    KEY_NUMERIC_11 */
#define XF86XK_Numeric12                _EVDEVK(0x26d)  /* v4.7    KEY_NUMERIC_12 */
#define XF86XK_AudioDesc                _EVDEVK(0x26e)  /* v4.7    KEY_AUDIO_DESC */
#define XF86XK_3DMode                   _EVDEVK(0x26f)  /* v4.7    KEY_3D_MODE */
#define XF86XK_NextFavorite             _EVDEVK(0x270)  /* v4.7    KEY_NEXT_FAVORITE */
#define XF86XK_StopRecord               _EVDEVK(0x271)  /* v4.7    KEY_STOP_RECORD */
#define XF86XK_PauseRecord              _EVDEVK(0x272)  /* v4.7    KEY_PAUSE_RECORD */
#define XF86XK_VOD                      _EVDEVK(0x273)  /* v4.7    KEY_VOD */
#define XF86XK_Unmute                   _EVDEVK(0x274)  /* v4.7    KEY_UNMUTE */
#define XF86XK_FastReverse              _EVDEVK(0x275)  /* v4.7    KEY_FASTREVERSE */
#define XF86XK_SlowReverse              _EVDEVK(0x276)  /* v4.7    KEY_SLOWREVERSE */
#define XF86XK_Data                     _EVDEVK(0x277)  /* v4.7    KEY_DATA */
#define XF86XK_OnScreenKeyboard         _EVDEVK(0x278)  /* v4.12   KEY_ONSCREEN_KEYBOARD */
#define XF86XK_PrivacyScreenToggle      _EVDEVK(0x279)  /* v5.5    KEY_PRIVACY_SCREEN_TOGGLE */
#define XF86XK_SelectiveScreenshot      _EVDEVK(0x27a)  /* v5.6    KEY_SELECTIVE_SCREENSHOT */
#define XF86XK_NextElement              _EVDEVK(0x27b)  /* v5.18   KEY_NEXT_ELEMENT */
#define XF86XK_PreviousElement          _EVDEVK(0x27c)  /* v5.18   KEY_PREVIOUS_ELEMENT */
#define XF86XK_AutopilotEngageToggle    _EVDEVK(0x27d)  /* v5.18   KEY_AUTOPILOT_ENGAGE_TOGGLE */
#define XF86XK_MarkWaypoint             _EVDEVK(0x27e)  /* v5.18   KEY_MARK_WAYPOINT */
#define XF86XK_Sos                      _EVDEVK(0x27f)  /* v5.18   KEY_SOS */
#define XF86XK_NavChart                 _EVDEVK(0x280)  /* v5.18   KEY_NAV_CHART */
#define XF86XK_FishingChart             _EVDEVK(0x281)  /* v5.18   KEY_FISHING_CHART */
#define XF86XK_SingleRangeRadar         _EVDEVK(0x282)  /* v5.18   KEY_SINGLE_RANGE_RADAR */
#define XF86XK_DualRangeRadar           _EVDEVK(0x283)  /* v5.18   KEY_DUAL_RANGE_RADAR */
#define XF86XK_RadarOverlay             _EVDEVK(0x284)  /* v5.18   KEY_RADAR_OVERLAY */
#define XF86XK_TraditionalSonar         _EVDEVK(0x285)  /* v5.18   KEY_TRADITIONAL_SONAR */
#define XF86XK_ClearvuSonar             _EVDEVK(0x286)  /* v5.18   KEY_CLEARVU_SONAR */
#define XF86XK_SidevuSonar              _EVDEVK(0x287)  /* v5.18   KEY_SIDEVU_SONAR */
#define XF86XK_NavInfo                  _EVDEVK(0x288)  /* v5.18   KEY_NAV_INFO */
/* Use: XF86XK_BrightnessAdjust         _EVDEVK(0x289)     v5.18   KEY_BRIGHTNESS_MENU */
#define XF86XK_Macro1                   _EVDEVK(0x290)  /* v5.5    KEY_MACRO1 */
#define XF86XK_Macro2                   _EVDEVK(0x291)  /* v5.5    KEY_MACRO2 */
#define XF86XK_Macro3                   _EVDEVK(0x292)  /* v5.5    KEY_MACRO3 */
#define XF86XK_Macro4                   _EVDEVK(0x293)  /* v5.5    KEY_MACRO4 */
#define XF86XK_Macro5                   _EVDEVK(0x294)  /* v5.5    KEY_MACRO5 */
#define XF86XK_Macro6                   _EVDEVK(0x295)  /* v5.5    KEY_MACRO6 */
#define XF86XK_Macro7                   _EVDEVK(0x296)  /* v5.5    KEY_MACRO7 */
#define XF86XK_Macro8                   _EVDEVK(0x297)  /* v5.5    KEY_MACRO8 */
#define XF86XK_Macro9                   _EVDEVK(0x298)  /* v5.5    KEY_MACRO9 */
#define XF86XK_Macro10                  _EVDEVK(0x299)  /* v5.5    KEY_MACRO10 */
#define XF86XK_Macro11                  _EVDEVK(0x29a)  /* v5.5    KEY_MACRO11 */
#define XF86XK_Macro12                  _EVDEVK(0x29b)  /* v5.5    KEY_MACRO12 */
#define XF86XK_Macro13                  _EVDEVK(0x29c)  /* v5.5    KEY_MACRO13 */
#define XF86XK_Macro14                  _EVDEVK(0x29d)  /* v5.5    KEY_MACRO14 */
#define XF86XK_Macro15                  _EVDEVK(0x29e)  /* v5.5    KEY_MACRO15 */
#define XF86XK_Macro16                  _EVDEVK(0x29f)  /* v5.5    KEY_MACRO16 */
#define XF86XK_Macro17                  _EVDEVK(0x2a0)  /* v5.5    KEY_MACRO17 */
#define XF86XK_Macro18                  _EVDEVK(0x2a1)  /* v5.5    KEY_MACRO18 */
#define XF86XK_Macro19                  _EVDEVK(0x2a2)  /* v5.5    KEY_MACRO19 */
#define XF86XK_Macro20                  _EVDEVK(0x2a3)  /* v5.5    KEY_MACRO20 */
#define XF86XK_Macro21                  _EVDEVK(0x2a4)  /* v5.5    KEY_MACRO21 */
#define XF86XK_Macro22                  _EVDEVK(0x2a5)  /* v5.5    KEY_MACRO22 */
#define XF86XK_Macro23                  _EVDEVK(0x2a6)  /* v5.5    KEY_MACRO23 */
#define XF86XK_Macro24                  _EVDEVK(0x2a7)  /* v5.5    KEY_MACRO24 */
#define XF86XK_Macro25                  _EVDEVK(0x2a8)  /* v5.5    KEY_MACRO25 */
#define XF86XK_Macro26                  _EVDEVK(0x2a9)  /* v5.5    KEY_MACRO26 */
#define XF86XK_Macro27                  _EVDEVK(0x2aa)  /* v5.5    KEY_MACRO27 */
#define XF86XK_Macro28                  _EVDEVK(0x2ab)  /* v5.5    KEY_MACRO28 */
#define XF86XK_Macro29                  _EVDEVK(0x2ac)  /* v5.5    KEY_MACRO29 */
#define XF86XK_Macro30                  _EVDEVK(0x2ad)  /* v5.5    KEY_MACRO30 */
#define XF86XK_MacroRecordStart         _EVDEVK(0x2b0)  /* v5.5    KEY_MACRO_RECORD_START */
#define XF86XK_MacroRecordStop          _EVDEVK(0x2b1)  /* v5.5    KEY_MACRO_RECORD_STOP */
#define XF86XK_MacroPresetCycle         _EVDEVK(0x2b2)  /* v5.5    KEY_MACRO_PRESET_CYCLE */
#define XF86XK_MacroPreset1             _EVDEVK(0x2b3)  /* v5.5    KEY_MACRO_PRESET1 */
#define XF86XK_MacroPreset2             _EVDEVK(0x2b4)  /* v5.5    KEY_MACRO_PRESET2 */
#define XF86XK_MacroPreset3             _EVDEVK(0x2b5)  /* v5.5    KEY_MACRO_PRESET3 */
#define XF86XK_KbdLcdMenu1              _EVDEVK(0x2b8)  /* v5.5    KEY_KBD_LCD_MENU1 */
#define XF86XK_KbdLcdMenu2              _EVDEVK(0x2b9)  /* v5.5    KEY_KBD_LCD_MENU2 */
#define XF86XK_KbdLcdMenu3              _EVDEVK(0x2ba)  /* v5.5    KEY_KBD_LCD_MENU3 */
#define XF86XK_KbdLcdMenu4              _EVDEVK(0x2bb)  /* v5.5    KEY_KBD_LCD_MENU4 */
#define XF86XK_KbdLcdMenu5              _EVDEVK(0x2bc)  /* v5.5    KEY_KBD_LCD_MENU5 */
#undef _EVDEVK
// end of XF86keysyms.h

    // All of the stuff below really has to match qxcbcommon.cpp in Qt!
    { Qt::Key_Back,                  XF86XK_Back },
    { Qt::Key_Forward,               XF86XK_Forward },
    { Qt::Key_Stop,                  XF86XK_Stop },
    { Qt::Key_Refresh,               XF86XK_Refresh },
    { Qt::Key_Favorites,             XF86XK_Favorites },
    { Qt::Key_LaunchMedia,           XF86XK_AudioMedia },
    { Qt::Key_OpenUrl,               XF86XK_OpenURL },
    { Qt::Key_HomePage,              XF86XK_HomePage },
    { Qt::Key_Search,                XF86XK_Search },
    { Qt::Key_VolumeDown,            XF86XK_AudioLowerVolume },
    { Qt::Key_VolumeMute,            XF86XK_AudioMute },
    { Qt::Key_VolumeUp,              XF86XK_AudioRaiseVolume },
    { Qt::Key_MediaPlay,             XF86XK_AudioPlay },
    { Qt::Key_MediaStop,             XF86XK_AudioStop },
    { Qt::Key_MediaPrevious,         XF86XK_AudioPrev },
    { Qt::Key_MediaNext,             XF86XK_AudioNext },
    { Qt::Key_MediaRecord,           XF86XK_AudioRecord },
    { Qt::Key_MediaPause,            XF86XK_AudioPause },
    { Qt::Key_LaunchMail,            XF86XK_Mail },
    { Qt::Key_LaunchMedia,           XF86XK_MyComputer },
    { Qt::Key_Memo,                  XF86XK_Memo },
    { Qt::Key_ToDoList,              XF86XK_ToDoList },
    { Qt::Key_Calendar,              XF86XK_Calendar },
    { Qt::Key_PowerDown,             XF86XK_PowerDown },
    { Qt::Key_ContrastAdjust,        XF86XK_ContrastAdjust },
    { Qt::Key_Standby,               XF86XK_Standby },
    { Qt::Key_MonBrightnessUp,       XF86XK_MonBrightnessUp },
    { Qt::Key_MonBrightnessDown,     XF86XK_MonBrightnessDown },
    { Qt::Key_KeyboardLightOnOff,    XF86XK_KbdLightOnOff },
    { Qt::Key_KeyboardBrightnessUp,  XF86XK_KbdBrightnessUp },
    { Qt::Key_KeyboardBrightnessDown,XF86XK_KbdBrightnessDown },
    { Qt::Key_PowerOff,              XF86XK_PowerOff },
    { Qt::Key_WakeUp,                XF86XK_WakeUp },
    { Qt::Key_Eject,                 XF86XK_Eject },
    { Qt::Key_ScreenSaver,           XF86XK_ScreenSaver },
    { Qt::Key_WWW,                   XF86XK_WWW },
    { Qt::Key_Sleep,                 XF86XK_Sleep },
    { Qt::Key_LightBulb,             XF86XK_LightBulb },
    { Qt::Key_Shop,                  XF86XK_Shop },
    { Qt::Key_History,               XF86XK_History },
    { Qt::Key_AddFavorite,           XF86XK_AddFavorite },
    { Qt::Key_HotLinks,              XF86XK_HotLinks },
    { Qt::Key_BrightnessAdjust,      XF86XK_BrightnessAdjust },
    { Qt::Key_Finance,               XF86XK_Finance },
    { Qt::Key_Community,             XF86XK_Community },
    { Qt::Key_AudioRewind,           XF86XK_AudioRewind },
    { Qt::Key_BackForward,           XF86XK_BackForward },
    { Qt::Key_ApplicationLeft,       XF86XK_ApplicationLeft },
    { Qt::Key_ApplicationRight,      XF86XK_ApplicationRight },
    { Qt::Key_Book,                  XF86XK_Book },
    { Qt::Key_CD,                    XF86XK_CD },
    { Qt::Key_Calculator,            XF86XK_Calculater },
    { Qt::Key_Calculator,            XF86XK_Calculator },
    { Qt::Key_Clear,                 XF86XK_Clear },
    { Qt::Key_ClearGrab,             XF86XK_ClearGrab },
    { Qt::Key_Close,                 XF86XK_Close },
    { Qt::Key_Copy,                  XF86XK_Copy },
    { Qt::Key_Cut,                   XF86XK_Cut },
    { Qt::Key_Display,               XF86XK_Display },
    { Qt::Key_DOS,                   XF86XK_DOS },
    { Qt::Key_Documents,             XF86XK_Documents },
    { Qt::Key_Excel,                 XF86XK_Excel },
    { Qt::Key_Explorer,              XF86XK_Explorer },
    { Qt::Key_Game,                  XF86XK_Game },
    { Qt::Key_Go,                    XF86XK_Go },
    { Qt::Key_iTouch,                XF86XK_iTouch },
    { Qt::Key_LogOff,                XF86XK_LogOff },
    { Qt::Key_Market,                XF86XK_Market },
    { Qt::Key_Meeting,               XF86XK_Meeting },
    { Qt::Key_MenuKB,                XF86XK_MenuKB },
    { Qt::Key_MenuPB,                XF86XK_MenuPB },
    { Qt::Key_MySites,               XF86XK_MySites },
    { Qt::Key_New,                   XF86XK_New },
    { Qt::Key_News,                  XF86XK_News },
    { Qt::Key_OfficeHome,            XF86XK_OfficeHome },
    { Qt::Key_Open,                  XF86XK_Open },
    { Qt::Key_Option,                XF86XK_Option },
    { Qt::Key_Paste,                 XF86XK_Paste },
    { Qt::Key_Phone,                 XF86XK_Phone },
    { Qt::Key_Reply,                 XF86XK_Reply },
    { Qt::Key_Reload,                XF86XK_Reload },
    { Qt::Key_RotateWindows,         XF86XK_RotateWindows },
    { Qt::Key_RotationPB,            XF86XK_RotationPB },
    { Qt::Key_RotationKB,            XF86XK_RotationKB },
    { Qt::Key_Save,                  XF86XK_Save },
    { Qt::Key_Send,                  XF86XK_Send },
    { Qt::Key_Spell,                 XF86XK_Spell },
    { Qt::Key_SplitScreen,           XF86XK_SplitScreen },
    { Qt::Key_Support,               XF86XK_Support },
    { Qt::Key_TaskPane,              XF86XK_TaskPane },
    { Qt::Key_Terminal,              XF86XK_Terminal },
    { Qt::Key_Tools,                 XF86XK_Tools },
    { Qt::Key_Travel,                XF86XK_Travel },
    { Qt::Key_Video,                 XF86XK_Video },
    { Qt::Key_Word,                  XF86XK_Word },
    { Qt::Key_Xfer,                  XF86XK_Xfer },
    { Qt::Key_ZoomIn,                XF86XK_ZoomIn },
    { Qt::Key_ZoomOut,               XF86XK_ZoomOut },
    { Qt::Key_Away,                  XF86XK_Away },
    { Qt::Key_Messenger,             XF86XK_Messenger },
    { Qt::Key_WebCam,                XF86XK_WebCam },
    { Qt::Key_MailForward,           XF86XK_MailForward },
    { Qt::Key_Pictures,              XF86XK_Pictures },
    { Qt::Key_Music,                 XF86XK_Music },
    { Qt::Key_Battery,               XF86XK_Battery },
    { Qt::Key_Bluetooth,             XF86XK_Bluetooth },
    { Qt::Key_WLAN,                  XF86XK_WLAN },
    { Qt::Key_UWB,                   XF86XK_UWB },
    { Qt::Key_AudioForward,          XF86XK_AudioForward },
    { Qt::Key_AudioRepeat,           XF86XK_AudioRepeat },
    { Qt::Key_AudioRandomPlay,       XF86XK_AudioRandomPlay },
    { Qt::Key_Subtitle,              XF86XK_Subtitle },
    { Qt::Key_AudioCycleTrack,       XF86XK_AudioCycleTrack },
    { Qt::Key_Time,                  XF86XK_Time },
    { Qt::Key_Select,                XF86XK_Select },
    { Qt::Key_View,                  XF86XK_View },
    { Qt::Key_TopMenu,               XF86XK_TopMenu },
    { Qt::Key_Red,                   XF86XK_Red },
    { Qt::Key_Green,                 XF86XK_Green },
    { Qt::Key_Yellow,                XF86XK_Yellow },
    { Qt::Key_Blue,                  XF86XK_Blue },
    { Qt::Key_Bluetooth,             XF86XK_Bluetooth },
    { Qt::Key_Suspend,               XF86XK_Suspend },
    { Qt::Key_Hibernate,             XF86XK_Hibernate },
    { Qt::Key_TouchpadToggle,        XF86XK_TouchpadToggle },
    { Qt::Key_TouchpadOn,            XF86XK_TouchpadOn },
    { Qt::Key_TouchpadOff,           XF86XK_TouchpadOff },
    { Qt::Key_MicMute,               XF86XK_AudioMicMute },
    { Qt::Key_Launch0,               XF86XK_Launch0 },
    { Qt::Key_Launch1,               XF86XK_Launch1 },
    { Qt::Key_Launch2,               XF86XK_Launch2 },
    { Qt::Key_Launch3,               XF86XK_Launch3 },
    { Qt::Key_Launch4,               XF86XK_Launch4 },
    { Qt::Key_Launch5,               XF86XK_Launch5 },
    { Qt::Key_Launch6,               XF86XK_Launch6 },
    { Qt::Key_Launch7,               XF86XK_Launch7 },
    { Qt::Key_Launch8,               XF86XK_Launch8 },
    { Qt::Key_Launch9,               XF86XK_Launch9 },
    { Qt::Key_LaunchA,               XF86XK_LaunchA },
    { Qt::Key_LaunchB,               XF86XK_LaunchB },
    { Qt::Key_LaunchC,               XF86XK_LaunchC },
    { Qt::Key_LaunchD,               XF86XK_LaunchD },
    { Qt::Key_LaunchE,               XF86XK_LaunchE },
    { Qt::Key_LaunchF,               XF86XK_LaunchF },
};
// clang-format on

//---------------------------------------------------------------------
// Debugging
//---------------------------------------------------------------------
#ifndef NDEBUG
inline void checkDisplay()
{
    // Some non-GUI apps might try to use us.
    if (!QX11Info::display()) {
        qCCritical(LOG_KKEYSERVER_X11) << "QX11Info::display() returns 0.  I'm probably going to crash now.";
        qCCritical(LOG_KKEYSERVER_X11) << "If this is a KApplication initialized without GUI stuff, change it to be "
                                          "initialized with GUI stuff.";
    }
}
#else // NDEBUG
#define checkDisplay()
#endif

//---------------------------------------------------------------------
// Initialization
//---------------------------------------------------------------------

static bool g_bInitializedMods;
static uint g_modXNumLock, g_modXScrollLock, g_modXModeSwitch, g_alt_mask, g_meta_mask, g_super_mask, g_hyper_mask;

bool initializeMods()
{
    // Reinitialize the masks
    g_modXNumLock = 0;
    g_modXScrollLock = 0;
    g_modXModeSwitch = 0;
    g_alt_mask = 0;
    g_meta_mask = 0;
    g_super_mask = 0;
    g_hyper_mask = 0;

    if (!QX11Info::isPlatformX11()) {
        qCWarning(LOG_KKEYSERVER_X11) << "X11 implementation of KKeyServer accessed from non-X11 platform! This is an application bug.";
        g_bInitializedMods = true;
        return false;
    }

    checkDisplay();
    xcb_key_symbols_t *symbols = xcb_key_symbols_alloc(QX11Info::connection());
    XModifierKeymap *xmk = XGetModifierMapping(QX11Info::display());

    int min_keycode;
    int max_keycode;
    int keysyms_per_keycode = 0;

    XDisplayKeycodes(QX11Info::display(), &min_keycode, &max_keycode);
    XFree(XGetKeyboardMapping(QX11Info::display(), min_keycode, 1, &keysyms_per_keycode));

    for (int i = Mod1MapIndex; i < 8; i++) {
        uint mask = (1 << i);
        uint keySymX = NoSymbol;

        // This used to be only XKeycodeToKeysym( ... , 0 ), but that fails with XFree4.3.99
        // and X.org R6.7 , where for some reason only ( ... , 1 ) works. I have absolutely no
        // idea what the problem is, but searching all possibilities until something valid is
        // found fixes the problem.
        for (int j = 0; j < xmk->max_keypermod; ++j) {
            for (int k = 0; k < keysyms_per_keycode; ++k) {
                keySymX = xcb_key_symbols_get_keysym(symbols, xmk->modifiermap[xmk->max_keypermod * i + j], k);

                switch (keySymX) {
                case XK_Alt_L:
                case XK_Alt_R:
                    g_alt_mask |= mask;
                    break;

                case XK_Super_L:
                case XK_Super_R:
                    g_super_mask |= mask;
                    break;

                case XK_Hyper_L:
                case XK_Hyper_R:
                    g_hyper_mask |= mask;
                    break;

                case XK_Meta_L:
                case XK_Meta_R:
                    g_meta_mask |= mask;
                    break;

                case XK_Num_Lock:
                    g_modXNumLock |= mask;
                    break;
                case XK_Scroll_Lock:
                    g_modXScrollLock |= mask;
                    break;
                case XK_Mode_switch:
                    g_modXModeSwitch |= mask;
                    break;
                }
            }
        }
    }

#ifdef KKEYSERVER_DEBUG
    qCDebug(LOG_KKEYSERVER_X11) << "Alt:" << g_alt_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Meta:" << g_meta_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Super:" << g_super_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Hyper:" << g_hyper_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "NumLock:" << g_modXNumLock;
    qCDebug(LOG_KKEYSERVER_X11) << "ScrollLock:" << g_modXScrollLock;
    qCDebug(LOG_KKEYSERVER_X11) << "ModeSwitch:" << g_modXModeSwitch;
#endif

    // Check if hyper overlaps with super or meta or alt
    if (g_hyper_mask & (g_super_mask | g_meta_mask | g_alt_mask)) {
#ifdef KKEYSERVER_DEBUG
        qCDebug(LOG_KKEYSERVER_X11) << "Hyper conflicts with super, meta or alt.";
#endif
        // Remove the conflicting masks
        g_hyper_mask &= ~(g_super_mask | g_meta_mask | g_alt_mask);
    }

    // Check if super overlaps with meta or alt
    if (g_super_mask & (g_meta_mask | g_alt_mask)) {
#ifdef KKEYSERVER_DEBUG
        qCDebug(LOG_KKEYSERVER_X11) << "Super conflicts with meta or alt.";
#endif
        // Remove the conflicting masks
        g_super_mask &= ~(g_meta_mask | g_alt_mask);
    }

    // Check if meta overlaps with alt
    if (g_meta_mask | g_alt_mask) {
#ifdef KKEYSERVER_DEBUG
        qCDebug(LOG_KKEYSERVER_X11) << "Meta conflicts with alt.";
#endif
        // Remove the conflicting masks
        g_meta_mask &= ~(g_alt_mask);
    }

    if (!g_meta_mask) {
#ifdef KKEYSERVER_DEBUG
        qCDebug(LOG_KKEYSERVER_X11) << "Meta is not set or conflicted with alt.";
#endif
        if (g_super_mask) {
#ifdef KKEYSERVER_DEBUG
            qCDebug(LOG_KKEYSERVER_X11) << "Using super for meta";
#endif
            // Use Super
            g_meta_mask = g_super_mask;
        } else if (g_hyper_mask) {
#ifdef KKEYSERVER_DEBUG
            qCDebug(LOG_KKEYSERVER_X11) << "Using hyper for meta";
#endif
            // User Hyper
            g_meta_mask = g_hyper_mask;
        } else {
            // ???? Nothing left
            g_meta_mask = 0;
        }
    }

#ifdef KKEYSERVER_DEBUG
    qCDebug(LOG_KKEYSERVER_X11) << "Alt:" << g_alt_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Meta:" << g_meta_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Super:" << g_super_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "Hyper:" << g_hyper_mask;
    qCDebug(LOG_KKEYSERVER_X11) << "NumLock:" << g_modXNumLock;
    qCDebug(LOG_KKEYSERVER_X11) << "ScrollLock:" << g_modXScrollLock;
    qCDebug(LOG_KKEYSERVER_X11) << "ModeSwitch:" << g_modXModeSwitch;
#endif

    if (!g_meta_mask) {
        qCWarning(LOG_KKEYSERVER_X11) << "Your keyboard setup doesn't provide a key to use for meta. See 'xmodmap -pm' or 'xkbcomp $DISPLAY'";
    }

    g_rgX11ModInfo[2].modX = g_alt_mask;
    g_rgX11ModInfo[3].modX = g_meta_mask;

    xcb_key_symbols_free(symbols);
    XFreeModifiermap(xmk);
    g_bInitializedMods = true;

    return true;
}

//---------------------------------------------------------------------
// Helper functions
//---------------------------------------------------------------------

static bool is_keypad_key(xcb_keysym_t keysym)
{
    return keysym >= XK_KP_Space && keysym <= XK_KP_9;
}

//---------------------------------------------------------------------
// Public functions
//---------------------------------------------------------------------

uint modXShift()
{
    return ShiftMask;
}
uint modXCtrl()
{
    return ControlMask;
}
uint modXAlt()
{
    if (!g_bInitializedMods) {
        initializeMods();
    }
    return g_alt_mask;
}
uint modXMeta()
{
    if (!g_bInitializedMods) {
        initializeMods();
    }
    return g_meta_mask;
}

uint modXNumLock()
{
    if (!g_bInitializedMods) {
        initializeMods();
    }
    return g_modXNumLock;
}
uint modXLock()
{
    return LockMask;
}
uint modXScrollLock()
{
    if (!g_bInitializedMods) {
        initializeMods();
    }
    return g_modXScrollLock;
}
uint modXModeSwitch()
{
    if (!g_bInitializedMods) {
        initializeMods();
    }
    return g_modXModeSwitch;
}

bool keyboardHasMetaKey()
{
    return modXMeta() != 0;
}

uint getModsRequired(uint sym)
{
    if (!QX11Info::isPlatformX11()) {
        qCWarning(LOG_KKEYSERVER_X11) << "X11 implementation of KKeyServer accessed from non-X11 platform! This is an application bug.";
        return 0;
    }
    uint mod = 0;

    // FIXME: This might not be true on all keyboard layouts!
    if (sym == XK_Sys_Req) {
        return Qt::ALT;
    }
    if (sym == XK_Break) {
        return Qt::CTRL;
    }

    if (sym < 0x3000) {
        QChar c(sym);
        if (c.isLetter() && c.toLower() != c.toUpper() && sym == c.toUpper().unicode()) {
            return Qt::SHIFT;
        }
    }

    uchar code = XKeysymToKeycode(QX11Info::display(), sym);
    if (code) {
        // need to check index 0 before the others, so that a null-mod
        //  can take precedence over the others, in case the modified
        //  key produces the same symbol.
        if (sym == XKeycodeToKeysym(QX11Info::display(), code, 0)) {
            ;
        } else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 1)) {
            mod = Qt::SHIFT;
        } else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 2)) {
            mod = MODE_SWITCH;
        } else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 3)) {
            mod = Qt::SHIFT | MODE_SWITCH;
        }
    }
    return mod;
}

#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 0)
bool keyQtToCodeX(int keyQt, int *keyCode)
{
    if (!QX11Info::isPlatformX11()) {
        qCWarning(LOG_KKEYSERVER_X11) << "X11 implementation of KKeyServer accessed from non-X11 platform! This is an application bug.";
        return false;
    }
    int sym;
    uint mod;
    keyQtToSymX(keyQt, &sym);
    keyQtToModX(keyQt, &mod);

    // Get any extra mods required by the sym.
    //  E.g., XK_Plus requires SHIFT on the en layout.
    uint modExtra = getModsRequired(sym);
    // Get the X modifier equivalent.
    if (!sym || !keyQtToModX((keyQt & Qt::KeyboardModifierMask) | modExtra, &mod)) {
        *keyCode = 0;
        return false;
    }

    *keyCode = XKeysymToKeycode(QX11Info::display(), sym);
    return true;
}
#endif

QList<int> keyQtToCodeXs(int keyQt)
{
    QList<int> keyCodes;
    if (!QX11Info::isPlatformX11()) {
        qCWarning(LOG_KKEYSERVER_X11) << "X11 implementation of KKeyServer accessed from non-X11 platform! This is an application bug.";
        return keyCodes;
    }
    uint mod;
    const QList<int> syms(keyQtToSymXs(keyQt));
    keyQtToModX(keyQt, &mod);

    for (int sym : syms) {
        // Get any extra mods required by the sym.
        //  E.g., XK_Plus requires SHIFT on the en layout.
        uint modExtra = getModsRequired(sym);
        // Get the X modifier equivalent.
        if (!sym || !keyQtToModX((keyQt & Qt::KeyboardModifierMask) | modExtra, &mod)) {
            continue;
        }

        keyCodes.append(XKeysymToKeycode(QX11Info::display(), sym));
    }
    return keyCodes;
}

#if KWINDOWSYSTEM_BUILD_DEPRECATED_SINCE(6, 0)
bool keyQtToSymX(int keyQt, int *keySym)
{
    int symQt = keyQt & ~Qt::KeyboardModifierMask;

    if (keyQt & Qt::KeypadModifier) {
        if (symQt >= Qt::Key_0 && symQt <= Qt::Key_9) {
            *keySym = XK_KP_0 + (symQt - Qt::Key_0);
            return true;
        }
    } else {
        if (symQt < 0x1000) {
            *keySym = QChar(symQt).toUpper().unicode();
            return true;
        }
    }

    for (const TransKey &tk : g_rgQtToSymX) {
        if (tk.keySymQt == symQt) {
            if ((keyQt & Qt::KeypadModifier) && !is_keypad_key(tk.keySymX)) {
                continue;
            }
            *keySym = tk.keySymX;
            return true;
        }
    }

    *keySym = 0;
    if (symQt != Qt::Key_Shift && symQt != Qt::Key_Control && symQt != Qt::Key_Alt && symQt != Qt::Key_Meta && symQt != Qt::Key_Direction_L
        && symQt != Qt::Key_Direction_R) {
        // qCDebug(LOG_KKEYSERVER_X11) << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key.";
    }
    return false;
}
#endif

QList<int> keyQtToSymXs(int keyQt)
{
    int symQt = keyQt & ~Qt::KeyboardModifierMask;
    QList<int> syms;

    if (keyQt & Qt::KeypadModifier) {
        if (symQt >= Qt::Key_0 && symQt <= Qt::Key_9) {
            syms.append(XK_KP_0 + (symQt - Qt::Key_0));
            return syms;
        }
    } else {
        if (symQt < 0x1000) {
            syms.append(QChar(symQt).toUpper().unicode());
            return syms;
        }
    }

    for (const TransKey &tk : g_rgQtToSymX) {
        if (tk.keySymQt == symQt) {
            if ((keyQt & Qt::KeypadModifier) && !is_keypad_key(tk.keySymX)) {
                continue;
            }
            syms.append(tk.keySymX);
        }
    }
    return syms;
}

bool symXModXToKeyQt(uint32_t keySym, uint16_t modX, int *keyQt)
{
    int keyModQt = 0;
    *keyQt = Qt::Key_unknown;

    if (keySym >= XK_KP_0 && keySym <= XK_KP_9) {
        // numeric keypad keys
        *keyQt = Qt::Key_0 + ((int)keySym - XK_KP_0);
    } else if (keySym < 0x1000) {
        if (keySym >= 'a' && keySym <= 'z') {
            *keyQt = QChar(keySym).toUpper().unicode();
        } else {
            *keyQt = keySym;
        }
    }

    else if (keySym < 0x3000) {
        *keyQt = keySym;
    }

    else {
        for (const TransKey &tk : g_rgQtToSymX) {
            if (tk.keySymX == keySym) {
                *keyQt = tk.keySymQt;
                break;
            }
        }
    }

    if (*keyQt == Qt::Key_unknown) {
        return false;
    }

    if (modXToQt(modX, &keyModQt)) {
        *keyQt |= keyModQt;
        if (is_keypad_key(keySym)) {
            *keyQt |= Qt::KeypadModifier;
        }
        return true;
    }
    return false;
}

bool keyQtToModX(int modQt, uint *modX)
{
    if (!g_bInitializedMods) {
        initializeMods();
    }

    *modX = 0;
    for (int i = 0; i < 4; i++) {
        if (modQt & g_rgX11ModInfo[i].modQt) {
            if (g_rgX11ModInfo[i].modX) {
                *modX |= g_rgX11ModInfo[i].modX;
            } else {
                // The qt modifier has no x equivalent. Return false
                return false;
            }
        }
    }
    return true;
}

bool modXToQt(uint modX, int *modQt)
{
    if (!g_bInitializedMods) {
        initializeMods();
    }

    *modQt = 0;
    for (int i = 0; i < 4; i++) {
        if (modX & g_rgX11ModInfo[i].modX) {
            *modQt |= g_rgX11ModInfo[i].modQt;
            continue;
        }
    }
    return true;
}

bool codeXToSym(uchar codeX, uint modX, uint *sym)
{
    if (!QX11Info::isPlatformX11()) {
        qCWarning(LOG_KKEYSERVER_X11) << "X11 implementation of KKeyServer accessed from non-X11 platform! This is an application bug.";
        return false;
    }
    KeySym keySym;
    XKeyPressedEvent event;

    checkDisplay();

    event.type = KeyPress;
    event.display = QX11Info::display();
    event.state = modX;
    event.keycode = codeX;

    XLookupString(&event, nullptr, 0, &keySym, nullptr);
    *sym = (uint)keySym;
    return true;
}

uint accelModMaskX()
{
    return modXShift() | modXCtrl() | modXAlt() | modXMeta();
}

bool xEventToQt(XEvent *e, int *keyQt)
{
    Q_ASSERT(e->type == KeyPress || e->type == KeyRelease);

    uchar keyCodeX = e->xkey.keycode;
    uint keyModX = e->xkey.state & (accelModMaskX() | MODE_SWITCH);

    KeySym keySym;
    char buffer[16];
    XLookupString((XKeyEvent *)e, buffer, 15, &keySym, nullptr);
    uint keySymX = (uint)keySym;

    // If numlock is active and a keypad key is pressed, XOR the SHIFT state.
    //  e.g., KP_4 => Shift+KP_Left, and Shift+KP_4 => KP_Left.
    if (e->xkey.state & modXNumLock()) {
        uint sym = XKeycodeToKeysym(QX11Info::display(), keyCodeX, 0);
        // TODO: what's the xor operator in c++?
        // If this is a keypad key,
        if (sym >= XK_KP_Space && sym <= XK_KP_9) {
            switch (sym) {
            // Leave the following keys unaltered
            // FIXME: The proper solution is to see which keysyms don't change when shifted.
            case XK_KP_Multiply:
            case XK_KP_Add:
            case XK_KP_Subtract:
            case XK_KP_Divide:
                break;
            default:
                if (keyModX & modXShift()) {
                    keyModX &= ~modXShift();
                } else {
                    keyModX |= modXShift();
                }
            }
        }
    }

    return KKeyServer::symXModXToKeyQt(keySymX, keyModX, keyQt);
}

bool xcbKeyPressEventToQt(xcb_generic_event_t *e, int *keyQt)
{
    if ((e->response_type & ~0x80) != XCB_KEY_PRESS && (e->response_type & ~0x80) != XCB_KEY_RELEASE) {
        return false;
    }
    return xcbKeyPressEventToQt(reinterpret_cast<xcb_key_press_event_t *>(e), keyQt);
}

bool xcbKeyPressEventToQt(xcb_key_press_event_t *e, int *keyQt)
{
    const uint16_t keyModX = e->state & (accelModMaskX() | MODE_SWITCH);

    xcb_key_symbols_t *symbols = xcb_key_symbols_alloc(QX11Info::connection());

    // We might have to use 4,5 instead of 0,1 here when mode_switch is active, just not sure how to test that.
    const xcb_keysym_t keySym0 = xcb_key_press_lookup_keysym(symbols, e, 0);
    const xcb_keysym_t keySym1 = xcb_key_press_lookup_keysym(symbols, e, 1);
    xcb_keysym_t keySymX;

    if ((e->state & KKeyServer::modXNumLock()) && is_keypad_key(keySym1)) {
        if ((e->state & XCB_MOD_MASK_SHIFT)) {
            keySymX = keySym0;
        } else {
            keySymX = keySym1;
        }
    } else {
        keySymX = keySym0;
    }

    bool ok = KKeyServer::symXModXToKeyQt(keySymX, keyModX, keyQt);

    if ((*keyQt & Qt::ShiftModifier) && !KKeyServer::isShiftAsModifierAllowed(*keyQt)) {
        if (*keyQt != Qt::Key_Tab) { // KKeySequenceWidget does not map shift+tab to backtab
            static const int FirstLevelShift = 1;
            keySymX = xcb_key_symbols_get_keysym(symbols, e->detail, FirstLevelShift);
            KKeyServer::symXModXToKeyQt(keySymX, keyModX, keyQt);
        }
        *keyQt &= ~Qt::ShiftModifier;
    }

    xcb_key_symbols_free(symbols);
    return ok;
}

} // end of namespace KKeyServer block
