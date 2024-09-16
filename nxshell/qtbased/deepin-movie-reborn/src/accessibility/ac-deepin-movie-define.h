// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作

//底部工具栏
#define BOTTOM_TOOL_BOX "bottomToolBox"
#define BOTTOM_WIDGET "bottomWidget"
#define BOTTOM_TOOL_BUTTON_WIDGET "bottomToolButtonWidget"

#define PLAY_BUTTOB_BOX "playButtonBox"
#define PROGBAR_WIDGET "progBarWidget"
#define MOVIE_PROGRESS_WIDGET "MovieProgressBarWidget"//进度条窗口
#define PROGBAR_SLIDER "progBarSlider"       //进度条

#define PREV_BUTTON "prevBtn" // 上一个视频按钮
#define PLAY_BUTTON "playBtn" // 播放、暂停按钮
#define NEXT_BUTTON "nextBtn" // 下一个视频按钮

#define FS_BUTTON "fullscreenButtion" // 全屏
#define VOLUME_BUTTON "volumeButton" // 音量
#define MIRVAST_BUTTON "mircastButton" // 投屏
#define PLAYLIST_BUTTON "playlistButton" // 播放列表按钮

//音量条
#define VOLUME_SLIDER_WIDGET "volumeSliderWidget"
#define VOLUME_SLIDER "volumeSlider"
#define SLIDER "slider"
#define MUTE_BTN "muteButton"



//播放列表
#define PLAYLIST_WIDGET "playListWidget"   //播放列表窗口
#define LEFT_WIDGET "leftWidget" // 左侧
#define CLEAR_PLAYLIST_BUTTON "clearButton" // 清空播放列表按钮

#define RIGHT_LIST_WIDGET "rightListWidget"//右侧文件列表
#define PLAYLIST "playlist"
#define FILE_LIST "fileList" //文件列表
#define PLAYITEM_WIDGET "playItemWidget" //单个视频项
#define PLAYITEN_CLOSE_BUTTON "playItemCloseBtn" // 关闭选中项




//movieInfo Dialog
#define MOVIE_INFO_DIALOG "movieInfoDialog"//影片信息窗口
#define MOVIE_INFO_SCROLL_AREA "movieInfoScrollArea"
#define SCROLL_AREA_VIEWPORT "scrollAreaViewport"
#define MOVIE_INFO_SCROLL_CONTENT "scrollContentWidget"

#define MOVIEINFO_CLOSE_BUTTON "movieInfoCloseButton" //关闭窗口按钮
#define FILM_INFO_WIDGET "filmInfoWidget"  //影片信息
#define CODEC_INFO_WIDGET "codecInfoWidget"  //编码信息
#define AUDIO_INFO_WIDGET "audioInfoWidget" //音频信息



////顶部菜单栏
#define TITLEBAR "titleBar"
//#define SETTINGS_DIALOG QObject::tr("settingsDialog")



////播放页面右键菜单
//#define PLAY_WIDGET_RIGHT_CLICK_MENU QObject::tr("playPageRightClickMenu")

//#define PLAY_MODE_MENU QObject::tr("playModeMenu")

//#define FRAME_MENU QObject::tr("frameMenu")

//#define SOUND_MENU QObject::tr("soundMenu")
//#define SOUND_CHANNEL_MENU QObject::tr("soundChannelMenu")
//#define SOUND_TRACK_MENU QObject::tr("soundTrackMenu")

//#define SUNTITLE_MENU QObject::tr("subtitleMenu")
//#define SUNTITLE_SELECT_MENU QObject::tr("subtitleSelectMenu")
//#define SUNTITLE_ENCODING_MENU QObject::tr("subtitleEncodingMenu")

//#define SCREENSHOT_MENU_MENU QObject::tr("screenshotMenu")

////投屏界面
#define MIRCAST_SUCCESSED          0
#define MIRCAST_EXIT              -1
#define MIRCAST_CONNECTION_FAILED -3
#define MIRCAST_DISCONNECTIONED   -4



#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
