// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#define DEEPIN_VOICE_NOTE "deepin-voice-note"

//Default main window size
#define DEFAULT_WINDOWS_WIDTH 980
#define DEFAULT_WINDOWS_HEIGHT 634

#define MIN_WINDOWS_WIDTH 680
#define MIN_WINDOWS_HEIGHT 300

//TitleBar height
#define VNOTE_TITLEBAR_HEIGHT 50

//LeftView width
#define VNOTE_LEFTVIEW_W 200
#define VNOTE_LEFTVIEW_MIN_W 133

//FolderSelectDialog Width
#define VNOTE_SELECTDIALOG_W 448

//MiddleView width
#define VNOTE_MIDDLEVIEW_W 260
#define VNOTE_MIDDLEVIEW_MIN_W 133

//SearchEdit Size
#define VNOTE_SEARCHBAR_H 36
#define VNOTE_SEARCHBAR_W 350
#define VNOTE_SEARCHBAR_MIN_W 200

//StandIcon path
#define STAND_ICON_PAHT ":/icons/deepin/builtin/"

//Audio device polling time in milliseconds
#define AUDIO_DEV_CHECK_TIME 1000

//********App setting data key****************
#define VNOTE_MAINWND_SZ_KEY "old._app_main_wnd_sz_key_"
#define VNOTE_EXPORT_TEXT_PATH_KEY "old._app_export_text_path_key"
#define VNOTE_EXPORT_VOICE_PATH_KEY "old._app_export_voice_path_key"
#define VNOTE_AUDIO_SELECT "base.audiosource.select"
#define VNOTE_FOLDER_SORT "base.folder_sort.folder_sort_data"
#define VNOTE_NOTEPAD_LIST_SHOW "base.notepadlist.show"
#define VNOTE_NOTEPAD_ENCRYPTION_KEY "base.encryption.key"
//********************************************

//Time format
#define VNOTE_TIME_FMT "yyyy-MM-dd HH:mm:ss.zzz"

// Uncomment follow line if need debug layout
//#define VNOTE_LAYOUT_DEBUG

// Note meta-data parser config
// Comment:Can't change this config when selected
// Now support
//      1.(xml version)  VN_XML_METADATA_PARSER
//      2.(json version) VN_JSON_METADATA_PARSER
// Default: json verson

#define VN_JSON_METADATA_PARSER

//Audio to text file lenght limit
//20 minutes
#define MAX_A2T_AUDIO_LEN_MS (20 * 60 * 1000)

//Limit shortcut key response time
//to 300ms
#define MIN_STKEY_RESP_TIME 300

//Enable/Disable import old data
#define IMPORT_OLD_VERSION_DATA

#define NOTES_DRAG_KEY "notes_drag"
#define NOTEPAD_DRAG_KEY "notepad_drag"

#include <sys/time.h>
#include <QtMath>
#define TM(s, e) ( \
    (qAbs((e.tv_sec - s.tv_sec) * 1000 + (e.tv_usec - s.tv_usec) / 1000)))

#define UPT(s, e) ((s) = (e))

#endif // GLOBALDEF_H
