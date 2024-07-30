/*
    SPDX-FileCopyrightText: 2015 Thomas Lübking <thomas.luebking@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#if (!defined ATOMS_H) || (defined ENUM_CREATE_CHAR_ARRAY)

#undef ENUM_BEGIN
#undef ENUM
#undef ENUM_END
#undef ENUM_COUNT

// the following macros are set in a way so that
// the code below will either construct an enum for "<typ>"
// or a const *char array "<typ>Strings" containing all enum
// symbols as strings, depending on whether ENUM_CREATE_CHAR_ARRAY is
// defined
// The enum gets one extra item "<typ>Count", describing also the
// length of the array

// The header is safe for re-inclusion unless you define ENUM_CREATE_CHAR_ARRAY
// which is therefore undefined after usage

// => You *must* "#define ENUM_CREATE_CHAR_ARRAY 1" *every* time you want to create
// a string array!

// clang-format off

#ifndef ENUM_CREATE_CHAR_ARRAY
#define ATOMS_H
#define ENUM_BEGIN(typ) enum typ {
#define ENUM(nam) nam
#define ENUM_COUNT(typ) , typ##Count
#else
#define ENUM_BEGIN(typ) const char * typ##Strings [] = {
#define ENUM(nam) #nam
#define ENUM_COUNT(typ)
#undef ENUM_CREATE_CHAR_ARRAY
#endif

#define ENUM_END(typ) };

ENUM_BEGIN(KwsAtom)
    ENUM(UTF8_STRING),

    // root window properties
    ENUM(_NET_SUPPORTED),
    ENUM(_NET_SUPPORTING_WM_CHECK),
    ENUM(_NET_CLIENT_LIST),
    ENUM(_NET_CLIENT_LIST_STACKING),
    ENUM(_NET_NUMBER_OF_DESKTOPS),
    ENUM(_NET_DESKTOP_GEOMETRY),
    ENUM(_NET_DESKTOP_VIEWPORT),
    ENUM(_NET_CURRENT_DESKTOP),
    ENUM(_NET_DESKTOP_NAMES),
    ENUM(_NET_ACTIVE_WINDOW),
    ENUM(_NET_WORKAREA),
    ENUM(_NET_VIRTUAL_ROOTS),
    ENUM(_NET_DESKTOP_LAYOUT),
    ENUM(_NET_SHOWING_DESKTOP),

    // root window messages
    ENUM(_NET_CLOSE_WINDOW),
    ENUM(_NET_RESTACK_WINDOW),
    ENUM(_NET_WM_MOVERESIZE),
    ENUM(_NET_MOVERESIZE_WINDOW),

    // application window properties
    ENUM(_NET_WM_NAME),
    ENUM(_NET_WM_VISIBLE_NAME),
    ENUM(_NET_WM_ICON_NAME),
    ENUM(_NET_WM_VISIBLE_ICON_NAME),
    ENUM(_NET_WM_DESKTOP),
    ENUM(_NET_WM_WINDOW_TYPE),
    ENUM(_NET_WM_STATE),
    ENUM(_NET_WM_STRUT),
    ENUM(_NET_WM_STRUT_PARTIAL),
    ENUM(_NET_WM_ICON_GEOMETRY),
    ENUM(_NET_WM_ICON),
    ENUM(_NET_WM_PID),
    ENUM(_NET_WM_USER_TIME),
    ENUM(_NET_WM_HANDLED_ICONS),
    ENUM(_NET_STARTUP_ID),
    ENUM(_NET_WM_ALLOWED_ACTIONS),
    ENUM(WM_WINDOW_ROLE),
    ENUM(_NET_FRAME_EXTENTS),
    ENUM(_NET_WM_WINDOW_OPACITY),
    ENUM(_NET_WM_FULLSCREEN_MONITORS),
    ENUM(_NET_WM_OPAQUE_REGION),
    ENUM(_KDE_NET_WM_DESKTOP_FILE),
    // used to determine whether application window is managed or not
    ENUM(WM_STATE),

    // application window types
    ENUM(_NET_WM_WINDOW_TYPE_NORMAL),
    ENUM(_NET_WM_WINDOW_TYPE_DESKTOP),
    ENUM(_NET_WM_WINDOW_TYPE_DOCK),
    ENUM(_NET_WM_WINDOW_TYPE_TOOLBAR),
    ENUM(_NET_WM_WINDOW_TYPE_MENU),
    ENUM(_NET_WM_WINDOW_TYPE_DIALOG),
    ENUM(_NET_WM_WINDOW_TYPE_UTILITY),
    ENUM(_NET_WM_WINDOW_TYPE_SPLASH),
    ENUM(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU),
    ENUM(_NET_WM_WINDOW_TYPE_POPUP_MENU),
    ENUM(_NET_WM_WINDOW_TYPE_TOOLTIP),
    ENUM(_NET_WM_WINDOW_TYPE_NOTIFICATION),
    ENUM(_NET_WM_WINDOW_TYPE_COMBO),
    ENUM(_NET_WM_WINDOW_TYPE_DND),

    // application window state
    ENUM(_NET_WM_STATE_MODAL),
    ENUM(_NET_WM_STATE_STICKY),
    ENUM(_NET_WM_STATE_MAXIMIZED_VERT),
    ENUM(_NET_WM_STATE_MAXIMIZED_HORZ),
    ENUM(_NET_WM_STATE_SHADED),
    ENUM(_NET_WM_STATE_SKIP_TASKBAR),
    ENUM(_NET_WM_STATE_SKIP_PAGER),
    ENUM(_NET_WM_STATE_HIDDEN),
    ENUM(_NET_WM_STATE_FULLSCREEN),
    ENUM(_NET_WM_STATE_ABOVE),
    ENUM(_NET_WM_STATE_BELOW),
    ENUM(_NET_WM_STATE_DEMANDS_ATTENTION),
    ENUM(_NET_WM_STATE_FOCUSED),
    // KDE-specific atom
    ENUM(_KDE_NET_WM_STATE_SKIP_SWITCHER),

    // allowed actions
    ENUM(_NET_WM_ACTION_MOVE),
    ENUM(_NET_WM_ACTION_RESIZE),
    ENUM(_NET_WM_ACTION_MINIMIZE),
    ENUM(_NET_WM_ACTION_SHADE),
    ENUM(_NET_WM_ACTION_STICK),
    ENUM(_NET_WM_ACTION_MAXIMIZE_VERT),
    ENUM(_NET_WM_ACTION_MAXIMIZE_HORZ),
    ENUM(_NET_WM_ACTION_FULLSCREEN),
    ENUM(_NET_WM_ACTION_CHANGE_DESKTOP),
    ENUM(_NET_WM_ACTION_CLOSE),

    // KDE extensions
    ENUM(_KDE_NET_WM_FRAME_STRUT),
    ENUM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE),
    ENUM(_KDE_NET_WM_WINDOW_TYPE_TOPMENU),
    ENUM(_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY),
    ENUM(_KDE_NET_WM_WINDOW_TYPE_CRITICAL_NOTIFICATION),
    ENUM(_KDE_NET_WM_WINDOW_TYPE_APPLET_POPUP),
    ENUM(_KDE_NET_WM_TEMPORARY_RULES),
    ENUM(_NET_WM_FRAME_OVERLAP),
    ENUM(_KDE_NET_WM_APPMENU_SERVICE_NAME),
    ENUM(_KDE_NET_WM_APPMENU_OBJECT_PATH),

    // deprecated and naming convention violation
    ENUM(_NET_WM_STATE_STAYS_ON_TOP),

    // GTK extensions
    ENUM(_GTK_FRAME_EXTENTS),
    ENUM(_GTK_APPLICATION_ID),
    ENUM(_GTK_SHOW_WINDOW_MENU),

    // application protocols
    ENUM(WM_PROTOCOLS),
    ENUM(WM_TAKE_FOCUS),
    ENUM(WM_DELETE_WINDOW),
    ENUM(_NET_WM_PING),
    ENUM(_NET_WM_SYNC_REQUEST),
    ENUM(_NET_WM_CONTEXT_HELP),

    // ability flags
    ENUM(_NET_WM_FULL_PLACEMENT),
    ENUM(_NET_WM_BYPASS_COMPOSITOR),
    ENUM(_KDE_NET_WM_ACTIVITIES),
    ENUM(_KDE_NET_WM_BLOCK_COMPOSITING),
    ENUM(_KDE_NET_WM_SHADOW)
    ENUM_COUNT(KwsAtom)
ENUM_END(KwsAtom)

#endif // ATOMS_H
