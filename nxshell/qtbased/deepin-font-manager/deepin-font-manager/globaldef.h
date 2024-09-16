// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBALDEF_H
#define GLOBALDEF_H
//窗口默认大小
#define DEFAULT_WINDOWS_WIDTH 820
#define DEFAULT_WINDOWS_HEIGHT 580
#define MIN_WINDOWS_WIDTH 680
#define MIN_WINDOWS_HEIGHT 300

// Tool Search bar
#define FTM_TITLE_FIXED_WIDTH 58
#define FTM_SEARCH_BAR_W 358

// Left navigation bar
#define FTM_LEFT_SIDE_BAR_WIDTH 160
// State bar
#define FTM_SBAR_HEIGHT 56
#define FTM_SBAR_COMPACT_HEIGHT 40

//Alt+M rightMenuPositionParam //SP3--Alt+M右键菜单--位置参数
#define POSITION_PARAM_X 158
#define POSITION_PARAM_Y 50

#define FTM_SBAR_TXT_EDIT_H 44
#define FTM_SBAR_TXT_EDIT_W 384  // Not used,Expanding now

#define FTM_SBAR_SLIDER_H 40
#define FTM_SBAR_SLIDER_W 199

#define FTM_SBAR_FSIZE_LABEL_H 40
#define FTM_SBAR_FSIZE_LABEL_W 60

// File manager binary name
#define DEEPIN_FILE_MANAGE_NAME "dde-file-manager"

// Left side bar debug test data
#define FTM_DEBUG_DATA_ON

// Uncomment follow line if need debug layout
//#define FTM_DEBUG_LAYOUT_COLOR

#define FTM_ENABLE_DISABLE_CACHE_DIR "/usr/local/.deep_fontmanger_cache"

#define FTM_REJECT_FONT_CONF_FILENAME  QString("71-DeepInFontManager-Reject.conf")

//Font Manager Theme Key
#define FTM_THEME_KEY "FontManagerTheme"

//Font Manager MainWindow Size
#define FTM_MWSIZE_H_KEY "FontManagerSizeH"
#define FTM_MWSIZE_W_KEY "FontManagerSizeW"

//Font Manager MainWindow Status
#define FTM_MWSTATUS_KEY "FontManagerStatus"

//Deepin manaul binary name
#define DEEPIN_MANUAL_NAME "dman"

//Font manager binary name
#define DEEPIN_FONT_MANAGER "deepin-font-manager"

//Font MIME
#define FONT_FILE_MIME "font/collection;font/ttf;font/otf;application/x-font-otf;application/x-font-ttf;application/x-font-type1;"

//默认触摸屏一次触摸时间
#define  DEFAULT_PRESSTIME_LENGTH 500
#endif  // GLOBALDEF_H
