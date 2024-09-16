// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONDEFINE_H
#define COMMONDEFINE_H
#include <QString>

#define APPEARANCEDBUS QDBusConnection::sessionBus()

const QString APPEARANCE_SERVICE = "org.deepin.dde.Appearance1";
const QString APPEARANCE_PATH = "/org/deepin/dde/Appearance1";
const QString APPEARANCE_INTERFACE = "org.deepin.dde.Appearance1";

#define APPEARANCESCHEMA    "org.deepin.dde.appearance"
#define XSETTINGSSCHEMA     "com.deepin.xsettings"
#define WRAPBGSCHEMA        "com.deepin.wrap.gnome.desktop.background"
#define GNOMEBGSCHEMA       "org.gnome.desktop.background"
#define WMSCHEMA            "com.deepin.wrap.gnome.desktop.wm.preferences"
#define METACITYSCHEMA      "com.deepin.wrap.gnome.metacity"
#define INTERFACESCHEMA     "com.deepin.wrap.gnome.desktop.interface"

#define ZONEPATH                 "/usr/share/zoneinfo/zone1970.tab"
#define GSKEYBACKGROUND          "pictureUri"
#define GSKEYFONTSIZE            "Font_Size"
#define GSKEYBACKGROUNDURIS      "Background_Uris"
#define GSKEYGLOBALTHEME            "Global_Theme"
#define GSKEYGTKTHEME            "Gtk_Theme"
#define GSKEYICONTHEM            "Icon_Theme"
#define GSKEYCURSORTHEME         "Cursor_Theme"
#define GSKEYFONTMONOSPACE       "Font_Monospace"
#define GSKEYOPACITY             "Opacity"
#define GSKEYQTACTIVECOLOR       "qtActiveColor"
#define GSKEYFONTSTANDARD        "Font_Standard"
#define GSKEYWALLPAPERSLIDESHOW  "Wallpaper_Slideshow"
#define GSKEYWALLPAPERURIS       "Wallpaper_Uris"
#define GSKEYDTKWINDOWRADIUS     "dtkWindowRadius"
#define GSKEYFONTNAME            "gtkFontName"
#define DCKEYALLWALLPAPER        "All_Wallpaper_Uris"
#define DDTKSIZEMODE             "Dtk_Size_Mode"
#define DCOMPACTFONTSIZE         "Compact_Font_Size"

#define GTK2CONFDELIM            "="
#define GTK3GROUPSETTINGS        "Settings"
#define GTK3KEYTHEME             "gtk-theme-name"
#define GTK3KEYICON              "gtk-icon-theme-name"
#define GTK3KEYCURSOR            "gtk-cursor-theme-name"
#define XSKEYTHEME               "themeName"
#define XSKEYICONTHEME           "iconThemeName"
#define XSKEYCURSORNAME          "gtkCursorThemeName"

#define SYNCSERVICENAME         "com.deepin.sync.Daemon"
#define SYNCSERVICEPATH         "/com/deepin/sync/Daemon"
#define SYNCSERVICEINTERFACE    "com.deepin.sync.Daemon"

#define	AUTOGTKTHEME            "deepin-auto"
#define	DEEPIN                  "deepin"
#define	DEEPINDARK              "deepin-dark"
#define QTKEYFONT               "Font"
#define QTKEYICON               "IconThemeName"
#define QTKEYFONTSIZE           "FontSize"
#define QTKEYMONOFONT           "MonoFont"
#define DEFAULTGLOBALTHEME      "flow"
#define DEFAULTICONTHEME        "bloom"
#define	DEFAULTGTKTHEME         "deepin"
#define	DEFAULTCURSORTHEME      "bloom"

#define DEFAULTLANG             "en"
#define DEFAULTLANGDELIM        "|"
#define	DEFAULTNAMEDELIM        ","
#define SPACETYPEMONO           "100"

#define TYPEGTK                 "gtk"
#define TYPEICON                "icon"
#define TYPECURSOR              "cursor"
#define TYPEBACKGROUND          "background"
#define TYPEGREETERBACKGROUND   "greeterbackground"
#define TYPESTANDARDFONT        "standardfont"
#define TYPEMONOSPACEFONT       "monospacefont"
#define TYPEFONTSIZE            "fontsize"
#define TYPEGLOBALTHEME         "globaltheme"
#define TYPEACTIVECOLOR         "activecolor"
#define TYPEDOCKOPACITY         "dockopacity"
#define TYPEWALLPAPER           "wallpaper"
#define TYPEWINDOWOPACITY       "windowopacity"
#define TYPWINDOWRADIUS         "windowradius"
#define TYPEWALLPAPERSLIDESHOW  "wallpaperflideshow"
#define TYPEALLWALLPAPER        "allwallpaperuris"
#define TYPEDTKSIZEMODE         "dtksizemode"
#define TYPECOMPACTFONTSIZE     "compactfontsize"
#define MIMETYPEGLOBAL          "application/x-global-theme"
#define MIMETYPEGTK             "application/x-gtk-theme"
#define MIMETYPEICON            "application/x-icon-theme"
#define MIMETYPECURSOR          "application/x-cursor-theme"
#define MIMEUSERCONFIG          ".config/mimeapps.list"
#define MIMETYPEXCURSOR         "image/x-xcursor"
#define SCHEME_FILE             "file://"
#define SCHEME_FTP              "ftp://"
#define SCHEME_HTTP             "http://"
#define SCHEME_HTTPS            "https://"
#define SCHEME_SMB              "smb://"
#define DEFAULTSTANDARDFONT     "Noto Sans"
#define	DEFAULTMONOSPACEFONT    "Noto Mono"

#define WSPOLICYLOGIN           "login"
#define	WSPOLICYWAKEUP          "wakeup"

#define HOME "HOME"

#define MAX_FILEPATH_LEN 256
#define MAX_LINE_LEN 256

#endif // COMMONDEFINE_H
