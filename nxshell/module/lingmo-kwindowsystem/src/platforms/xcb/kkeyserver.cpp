/*
    Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>

    Win32 port:
    Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kkeyserver_x11.h"
#include "kkeyserver.h"

#include "platforms/xcb/kwindowsystem_xcb_debug.h"
#include <QDebug>

#include <QX11Info>
# define XK_MISCELLANY
# define XK_XKB_KEYS
# include <X11/X.h>
# include <X11/Xlib.h>
# include <X11/Xutil.h>
# include <X11/keysymdef.h>
#include <xcb/xcb_keysyms.h>
# define X11_ONLY(arg) arg, //allows to omit an argument

// #define KKEYSERVER_DEBUG 1

namespace KKeyServer
{

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
    //{ Qt::Key_Shift,      0 },
    //{ Qt::Key_Control,    0 },
    //{ Qt::Key_Meta,       0 },
    //{ Qt::Key_Alt,        0 },
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

// the next lines are taken on 10/2009 from X.org (X11/XF86keysym.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_MonBrightnessUp     0x1008FF02
#define XF86XK_MonBrightnessDown   0x1008FF03
#define XF86XK_KbdLightOnOff       0x1008FF04
#define XF86XK_KbdBrightnessUp     0x1008FF05
#define XF86XK_KbdBrightnessDown   0x1008FF06
#define XF86XK_Standby             0x1008FF10
#define XF86XK_AudioLowerVolume    0x1008FF11
#define XF86XK_AudioMute           0x1008FF12
#define XF86XK_AudioRaiseVolume    0x1008FF13
#define XF86XK_AudioPlay           0x1008FF14
#define XF86XK_AudioStop           0x1008FF15
#define XF86XK_AudioPrev           0x1008FF16
#define XF86XK_AudioNext           0x1008FF17
#define XF86XK_HomePage            0x1008FF18
#define XF86XK_Mail                0x1008FF19
#define XF86XK_Start               0x1008FF1A
#define XF86XK_Search              0x1008FF1B
#define XF86XK_AudioRecord         0x1008FF1C
#define XF86XK_Calculator          0x1008FF1D
#define XF86XK_Memo                0x1008FF1E
#define XF86XK_ToDoList            0x1008FF1F
#define XF86XK_Calendar            0x1008FF20
#define XF86XK_PowerDown           0x1008FF21
#define XF86XK_ContrastAdjust      0x1008FF22
#define XF86XK_Back                0x1008FF26
#define XF86XK_Forward             0x1008FF27
#define XF86XK_Stop                0x1008FF28
#define XF86XK_Refresh             0x1008FF29
#define XF86XK_PowerOff            0x1008FF2A
#define XF86XK_WakeUp              0x1008FF2B
#define XF86XK_Eject               0x1008FF2C
#define XF86XK_ScreenSaver         0x1008FF2D
#define XF86XK_WWW                 0x1008FF2E
#define XF86XK_Sleep               0x1008FF2F
#define XF86XK_Favorites           0x1008FF30
#define XF86XK_AudioPause          0x1008FF31
#define XF86XK_AudioMedia          0x1008FF32
#define XF86XK_MyComputer          0x1008FF33
#define XF86XK_LightBulb           0x1008FF35
#define XF86XK_Shop                0x1008FF36
#define XF86XK_History             0x1008FF37
#define XF86XK_OpenURL             0x1008FF38
#define XF86XK_AddFavorite         0x1008FF39
#define XF86XK_HotLinks            0x1008FF3A
#define XF86XK_BrightnessAdjust    0x1008FF3B
#define XF86XK_Finance             0x1008FF3C
#define XF86XK_Community           0x1008FF3D
#define XF86XK_AudioRewind         0x1008FF3E
#define XF86XK_BackForward         0x1008FF3F
#define XF86XK_Launch0             0x1008FF40
#define XF86XK_Launch1             0x1008FF41
#define XF86XK_Launch2             0x1008FF42
#define XF86XK_Launch3             0x1008FF43
#define XF86XK_Launch4             0x1008FF44
#define XF86XK_Launch5             0x1008FF45
#define XF86XK_Launch6             0x1008FF46
#define XF86XK_Launch7             0x1008FF47
#define XF86XK_Launch8             0x1008FF48
#define XF86XK_Launch9             0x1008FF49
#define XF86XK_LaunchA             0x1008FF4A
#define XF86XK_LaunchB             0x1008FF4B
#define XF86XK_LaunchC             0x1008FF4C
#define XF86XK_LaunchD             0x1008FF4D
#define XF86XK_LaunchE             0x1008FF4E
#define XF86XK_LaunchF             0x1008FF4F
#define XF86XK_ApplicationLeft     0x1008FF50
#define XF86XK_ApplicationRight    0x1008FF51
#define XF86XK_Book                0x1008FF52
#define XF86XK_CD                  0x1008FF53
#define XF86XK_Calculater          0x1008FF54
#define XF86XK_Clear               0x1008FF55
#define XF86XK_ClearGrab           0x1008FE21
#define XF86XK_Close               0x1008FF56
#define XF86XK_Copy                0x1008FF57
#define XF86XK_Cut                 0x1008FF58
#define XF86XK_Display             0x1008FF59
#define XF86XK_DOS                 0x1008FF5A
#define XF86XK_Documents           0x1008FF5B
#define XF86XK_Excel               0x1008FF5C
#define XF86XK_Explorer            0x1008FF5D
#define XF86XK_Game                0x1008FF5E
#define XF86XK_Go                  0x1008FF5F
#define XF86XK_iTouch              0x1008FF60
#define XF86XK_LogOff              0x1008FF61
#define XF86XK_Market              0x1008FF62
#define XF86XK_Meeting             0x1008FF63
#define XF86XK_MenuKB              0x1008FF65
#define XF86XK_MenuPB              0x1008FF66
#define XF86XK_MySites             0x1008FF67
#define XF86XK_News                0x1008FF69
#define XF86XK_OfficeHome          0x1008FF6A
#define XF86XK_Option              0x1008FF6C
#define XF86XK_Paste               0x1008FF6D
#define XF86XK_Phone               0x1008FF6E
#define XF86XK_Reply               0x1008FF72
#define XF86XK_Reload              0x1008FF73
#define XF86XK_RotateWindows       0x1008FF74
#define XF86XK_RotationPB          0x1008FF75
#define XF86XK_RotationKB          0x1008FF76
#define XF86XK_Save                0x1008FF77
#define XF86XK_Send                0x1008FF7B
#define XF86XK_Spell               0x1008FF7C
#define XF86XK_SplitScreen         0x1008FF7D
#define XF86XK_Support             0x1008FF7E
#define XF86XK_TaskPane            0x1008FF7F
#define XF86XK_Terminal            0x1008FF80
#define XF86XK_Tools               0x1008FF81
#define XF86XK_Travel              0x1008FF82
#define XF86XK_Video               0x1008FF87
#define XF86XK_Word                0x1008FF89
#define XF86XK_Xfer                0x1008FF8A
#define XF86XK_ZoomIn              0x1008FF8B
#define XF86XK_ZoomOut             0x1008FF8C
#define XF86XK_Away                0x1008FF8D
#define XF86XK_Messenger           0x1008FF8E
#define XF86XK_WebCam              0x1008FF8F
#define XF86XK_MailForward         0x1008FF90
#define XF86XK_Pictures            0x1008FF91
#define XF86XK_Music               0x1008FF92
#define XF86XK_Battery             0x1008FF93
#define XF86XK_Bluetooth           0x1008FF94
#define XF86XK_WLAN                0x1008FF95
#define XF86XK_UWB                 0x1008FF96
#define XF86XK_AudioForward        0x1008FF97
#define XF86XK_AudioRepeat         0x1008FF98
#define XF86XK_AudioRandomPlay     0x1008FF99
#define XF86XK_Subtitle            0x1008FF9A
#define XF86XK_AudioCycleTrack     0x1008FF9B
#define XF86XK_Time                0x1008FF9F
#define XF86XK_Select              0x1008FFA0
#define XF86XK_View                0x1008FFA1
#define XF86XK_TopMenu             0x1008FFA2
#define XF86XK_Suspend             0x1008FFA7
#define XF86XK_Hibernate           0x1008FFA8
#define XF86XK_TouchpadToggle      0x1008FFA9
#define XF86XK_TouchpadOn          0x1008FFB0
#define XF86XK_TouchpadOff         0x1008FFB1
#define XF86XK_AudioMicMute        0x1008FFB2
// end of XF86keysyms.h

    // All of the stuff below really has to match qxcbkeyboard.cpp in Qt!
    { Qt::Key_Back,       XF86XK_Back },
    { Qt::Key_Forward,    XF86XK_Forward },
    { Qt::Key_Stop,       XF86XK_Stop },
    { Qt::Key_Refresh,    XF86XK_Refresh },
    { Qt::Key_Favorites,  XF86XK_Favorites },
    { Qt::Key_LaunchMedia, XF86XK_AudioMedia },
    { Qt::Key_OpenUrl,    XF86XK_OpenURL },
    { Qt::Key_HomePage,   XF86XK_HomePage },
    { Qt::Key_Search,     XF86XK_Search },
    { Qt::Key_VolumeDown, XF86XK_AudioLowerVolume },
    { Qt::Key_VolumeMute, XF86XK_AudioMute },
    { Qt::Key_VolumeUp,   XF86XK_AudioRaiseVolume },
    { Qt::Key_MediaPlay,  XF86XK_AudioPlay },
    { Qt::Key_MediaStop,  XF86XK_AudioStop },
    { Qt::Key_MediaPrevious,  XF86XK_AudioPrev },
    { Qt::Key_MediaNext,  XF86XK_AudioNext },
    { Qt::Key_MediaRecord, XF86XK_AudioRecord },
    { Qt::Key_LaunchMail, XF86XK_Mail },
    { Qt::Key_Launch0,    XF86XK_MyComputer },
    { Qt::Key_Launch1,    XF86XK_Calculator },
    { Qt::Key_Memo,    XF86XK_Memo },
    { Qt::Key_ToDoList,    XF86XK_ToDoList },
    { Qt::Key_Calendar,    XF86XK_Calendar },
    { Qt::Key_PowerDown,    XF86XK_PowerDown },
    { Qt::Key_ContrastAdjust,    XF86XK_ContrastAdjust },
    { Qt::Key_Standby,    XF86XK_Standby },
    { Qt::Key_MonBrightnessUp,  XF86XK_MonBrightnessUp },
    { Qt::Key_MonBrightnessDown,  XF86XK_MonBrightnessDown },
    { Qt::Key_KeyboardLightOnOff,  XF86XK_KbdLightOnOff },
    { Qt::Key_KeyboardBrightnessUp,  XF86XK_KbdBrightnessUp },
    { Qt::Key_KeyboardBrightnessDown,  XF86XK_KbdBrightnessDown },
    { Qt::Key_PowerOff,  XF86XK_PowerOff },
    { Qt::Key_WakeUp,  XF86XK_WakeUp },
    { Qt::Key_Eject,  XF86XK_Eject },
    { Qt::Key_ScreenSaver,  XF86XK_ScreenSaver },
    { Qt::Key_WWW,  XF86XK_WWW },
    { Qt::Key_Sleep,  XF86XK_Sleep },
    { Qt::Key_LightBulb,  XF86XK_LightBulb },
    { Qt::Key_Shop,  XF86XK_Shop },
    { Qt::Key_History,  XF86XK_History },
    { Qt::Key_AddFavorite,  XF86XK_AddFavorite },
    { Qt::Key_HotLinks,  XF86XK_HotLinks },
    { Qt::Key_BrightnessAdjust,  XF86XK_BrightnessAdjust },
    { Qt::Key_Finance,  XF86XK_Finance },
    { Qt::Key_Community,  XF86XK_Community },
    { Qt::Key_AudioRewind,  XF86XK_AudioRewind },
    { Qt::Key_BackForward,  XF86XK_BackForward },
    { Qt::Key_ApplicationLeft,  XF86XK_ApplicationLeft },
    { Qt::Key_ApplicationRight,  XF86XK_ApplicationRight },
    { Qt::Key_Book,  XF86XK_Book },
    { Qt::Key_CD,  XF86XK_CD },
    { Qt::Key_Calculator,  XF86XK_Calculater },
    { Qt::Key_Clear,  XF86XK_Clear },
    { Qt::Key_ClearGrab,  XF86XK_ClearGrab },
    { Qt::Key_Close,  XF86XK_Close },
    { Qt::Key_Copy,  XF86XK_Copy },
    { Qt::Key_Cut,  XF86XK_Cut },
    { Qt::Key_Display,  XF86XK_Display },
    { Qt::Key_DOS,  XF86XK_DOS },
    { Qt::Key_Documents,  XF86XK_Documents },
    { Qt::Key_Excel,  XF86XK_Excel },
    { Qt::Key_Explorer,  XF86XK_Explorer },
    { Qt::Key_Game,  XF86XK_Game },
    { Qt::Key_Go,  XF86XK_Go },
    { Qt::Key_iTouch,  XF86XK_iTouch },
    { Qt::Key_LogOff,  XF86XK_LogOff },
    { Qt::Key_Market,  XF86XK_Market },
    { Qt::Key_Meeting,  XF86XK_Meeting },
    { Qt::Key_MenuKB,  XF86XK_MenuKB },
    { Qt::Key_MenuPB,  XF86XK_MenuPB },
    { Qt::Key_MySites,  XF86XK_MySites },
    { Qt::Key_News,  XF86XK_News },
    { Qt::Key_OfficeHome,  XF86XK_OfficeHome },
    { Qt::Key_Option,  XF86XK_Option },
    { Qt::Key_Paste,  XF86XK_Paste },
    { Qt::Key_Phone,  XF86XK_Phone },
    { Qt::Key_Reply,  XF86XK_Reply },
    { Qt::Key_Reload,  XF86XK_Reload },
    { Qt::Key_RotateWindows,  XF86XK_RotateWindows },
    { Qt::Key_RotationPB,  XF86XK_RotationPB },
    { Qt::Key_RotationKB,  XF86XK_RotationKB },
    { Qt::Key_Save,  XF86XK_Save },
    { Qt::Key_Send,  XF86XK_Send },
    { Qt::Key_Spell,  XF86XK_Spell },
    { Qt::Key_SplitScreen,  XF86XK_SplitScreen },
    { Qt::Key_Support,  XF86XK_Support },
    { Qt::Key_TaskPane,  XF86XK_TaskPane },
    { Qt::Key_Terminal,  XF86XK_Terminal },
    { Qt::Key_Tools,  XF86XK_Tools },
    { Qt::Key_Travel,  XF86XK_Travel },
    { Qt::Key_Video,  XF86XK_Video },
    { Qt::Key_Word,  XF86XK_Word },
    { Qt::Key_Xfer,  XF86XK_Xfer },
    { Qt::Key_ZoomIn,  XF86XK_ZoomIn },
    { Qt::Key_ZoomOut,  XF86XK_ZoomOut },
    { Qt::Key_Away,  XF86XK_Away },
    { Qt::Key_Messenger,  XF86XK_Messenger },
    { Qt::Key_WebCam,  XF86XK_WebCam },
    { Qt::Key_MailForward,  XF86XK_MailForward },
    { Qt::Key_Pictures,  XF86XK_Pictures },
    { Qt::Key_Music,  XF86XK_Music },
    { Qt::Key_Battery,  XF86XK_Battery },
    { Qt::Key_Bluetooth,  XF86XK_Bluetooth },
    { Qt::Key_WLAN,  XF86XK_WLAN },
    { Qt::Key_UWB,  XF86XK_UWB },
    { Qt::Key_AudioForward,  XF86XK_AudioForward },
    { Qt::Key_AudioRepeat,  XF86XK_AudioRepeat },
    { Qt::Key_AudioRandomPlay,  XF86XK_AudioRandomPlay },
    { Qt::Key_Subtitle,  XF86XK_Subtitle },
    { Qt::Key_AudioCycleTrack,  XF86XK_AudioCycleTrack },
    { Qt::Key_Time,  XF86XK_Time },
    { Qt::Key_Select,  XF86XK_Select },
    { Qt::Key_View,  XF86XK_View },
    { Qt::Key_TopMenu,  XF86XK_TopMenu },
    { Qt::Key_Bluetooth,  XF86XK_Bluetooth },
    { Qt::Key_Suspend,  XF86XK_Suspend },
    { Qt::Key_Hibernate,  XF86XK_Hibernate },
    { Qt::Key_TouchpadToggle, XF86XK_TouchpadToggle },
    { Qt::Key_TouchpadOn, XF86XK_TouchpadOn },
    { Qt::Key_TouchpadOff, XF86XK_TouchpadOff },
    { Qt::Key_MicMute, XF86XK_AudioMicMute },
    { Qt::Key_Launch2,    XF86XK_Launch0 },
    { Qt::Key_Launch3,    XF86XK_Launch1 },
    { Qt::Key_Launch4,    XF86XK_Launch2 },
    { Qt::Key_Launch5,    XF86XK_Launch3 },
    { Qt::Key_Launch6,    XF86XK_Launch4 },
    { Qt::Key_Launch7,    XF86XK_Launch5 },
    { Qt::Key_Launch8,    XF86XK_Launch6 },
    { Qt::Key_Launch9,    XF86XK_Launch7 },
    { Qt::Key_LaunchA,    XF86XK_Launch8 },
    { Qt::Key_LaunchB,    XF86XK_Launch9 },
    { Qt::Key_LaunchC,    XF86XK_LaunchA },
    { Qt::Key_LaunchD,    XF86XK_LaunchB },
    { Qt::Key_LaunchE,    XF86XK_LaunchC },
    { Qt::Key_LaunchF,    XF86XK_LaunchD },
};

//---------------------------------------------------------------------
// Debugging
//---------------------------------------------------------------------
#ifndef NDEBUG
inline void checkDisplay()
{
    // Some non-GUI apps might try to use us.
    if (!QX11Info::display()) {
        qCCritical(LOG_KKEYSERVER_X11) << "QX11Info::display() returns 0.  I'm probably going to crash now." << endl;
        qCCritical(LOG_KKEYSERVER_X11) << "If this is a KApplication initialized without GUI stuff, change it to be "
                    "initialized with GUI stuff." << endl;
    }
}
#else // NDEBUG
# define checkDisplay()
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
    XModifierKeymap *xmk = XGetModifierMapping(QX11Info::display());

    int min_keycode, max_keycode;
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

                keySymX = XKeycodeToKeysym(QX11Info::display(), xmk->modifiermap[xmk->max_keypermod * i + j], k);

                switch (keySymX) {
                case XK_Alt_L:
                case XK_Alt_R:       g_alt_mask |= mask; break;

                case XK_Super_L:
                case XK_Super_R:     g_super_mask |= mask; break;

                case XK_Hyper_L:
                case XK_Hyper_R:     g_hyper_mask |= mask; break;

                case XK_Meta_L:
                case XK_Meta_R:      g_meta_mask |= mask; break;

                case XK_Num_Lock:    g_modXNumLock |= mask; break;
                case XK_Scroll_Lock: g_modXScrollLock |= mask; break;
                case XK_Mode_switch: g_modXModeSwitch |= mask; break;
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
    } return g_alt_mask;
}
uint modXMeta()
{
    if (!g_bInitializedMods) {
        initializeMods();
    } return g_meta_mask;
}

uint modXNumLock()
{
    if (!g_bInitializedMods) {
        initializeMods();
    } return g_modXNumLock;
}
uint modXLock()
{
    return LockMask;
}
uint modXScrollLock()
{
    if (!g_bInitializedMods) {
        initializeMods();
    } return g_modXScrollLock;
}
uint modXModeSwitch()
{
    if (!g_bInitializedMods) {
        initializeMods();
    } return g_modXModeSwitch;
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
        if (sym == XKeycodeToKeysym(QX11Info::display(), code, 0))
            ;
        else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 1)) {
            mod = Qt::SHIFT;
        } else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 2)) {
            mod = MODE_SWITCH;
        } else if (sym == XKeycodeToKeysym(QX11Info::display(), code, 3)) {
            mod = Qt::SHIFT | MODE_SWITCH;
        }
    }
    return mod;
}

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

    for (uint i = 0; i < sizeof(g_rgQtToSymX) / sizeof(TransKey); i++) {
        if (g_rgQtToSymX[i].keySymQt == symQt) {
            if ((keyQt & Qt::KeypadModifier) && !is_keypad_key(g_rgQtToSymX[i].keySymX))
                continue;
            *keySym = g_rgQtToSymX[i].keySymX;
            return true;
        }
    }

    *keySym = 0;
    if (symQt != Qt::Key_Shift && symQt != Qt::Key_Control && symQt != Qt::Key_Alt &&
            symQt != Qt::Key_Meta && symQt != Qt::Key_Direction_L && symQt != Qt::Key_Direction_R) {
        // qCDebug(LOG_KKEYSERVER_X11) << "Sym::initQt( " << QString::number(keyQt,16) << " ): failed to convert key.";
    }
    return false;
}

bool symXModXToKeyQt(uint32_t keySym, uint16_t modX, int *keyQt)
{
    int keyModQt = 0;
    *keyQt = Qt::Key_unknown;

    if (keySym >= XK_KP_0 && keySym <= XK_KP_9) {
        // numeric keypad keys
        *keyQt = Qt::Key_0 + ((int)keySym - XK_KP_0);
    }
    else if (keySym < 0x1000) {
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
        for (uint i = 0; i < sizeof(g_rgQtToSymX) / sizeof(TransKey); i++)
            if (g_rgQtToSymX[i].keySymX == keySym) {
                *keyQt = g_rgQtToSymX[i].keySymQt;
                break;
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

#ifndef KWINDOWSYSTEM_NO_DEPRECATED
bool symXToKeyQt(uint keySym, int *keyQt)
{
    return symXModXToKeyQt(keySym, 0, keyQt) & ~Qt::KeyboardModifierMask;
}
#endif

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
    *sym = (uint) keySym;
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
    XLookupString((XKeyEvent *) e, buffer, 15, &keySym, nullptr);
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

    if ((e->state & KKeyServer::modXNumLock()) && is_keypad_key(keySym1) ) {
        if ((e->state & XCB_MOD_MASK_SHIFT))
            keySymX = keySym0;
        else
            keySymX = keySym1;
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
