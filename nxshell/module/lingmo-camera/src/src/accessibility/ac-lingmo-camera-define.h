// Copyright (C) 2020 UOS Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作
//mainwindow


#define BUTTON_OPTION_PIC_LINE_EDIT QObject::tr("OptionPicLineEditBtn")
#define BUTTON_OPTION_VIDEO_LINE_EDIT QObject::tr("OptionVideoLineEditBtn")
#define OPTION_FORMAT_LABER QObject::tr("OptionFormatLabel")
#define OPTION_FRAME QObject::tr("OptionFrame")
#define PIC_OPTION QObject::tr("PicOption")
#define VIDEO_OPTION QObject::tr("VideoOption")
#define OPTION_PIC_SELECTABLE_LINE_EDIT QObject::tr("OptionPicSelectableLineEdit")
#define OPTION_VIDEO_SELECTABLE_LINE_EDIT QObject::tr("OptionVideoSelectableLineEdit")
#define OPTION_INVALID_DIALOG QObject::tr("OptionInvalidDialog")
#define SETTING_DIALOG QObject::tr("SettingDialog")

#define MAIN_WINDOW QObject::tr("MainWindow")
#define TITLEBAR QObject::tr("TitleBar")
#define TITLE_MUNE QObject::tr("TitleMenu")
#define VIDEO_PREVIEW_WIDGET QObject::tr("VideoPreviewWidget")

//右侧按钮
#define CAMERA_SWITCH_BTN QObject::tr("SelectBtn")  //摄像头切换
#define BUTTON_PICTURE_VIDEO QObject::tr("PicVdBtn")    //拍照
#define BUTTON_TAKE_VIDEO_END QObject::tr("TakeVdEndBtn")  //结束拍摄
#define THUMBNAIL_PREVIEW QObject::tr("ThumbnailPreview")  //预览缩略图
#define MODE_SWITCH_BOX QObject::tr("ModeSwitchBox")
#define SWITCH_BTN_RECOD QObject::tr("SwitchRecordBtn")
//左侧按钮
#define LEFT_BTNS_BOX QObject::tr("LeftBtnsBox")
#define UNFOLD_BTN QObject::tr("UnfoldBtn")
#define FOLD_BTN QObject::tr("FoldBtn")
#define FLASHLITE_UNFOLD_BTN QObject::tr("FlashlightUnfoldBtn")
#define FLASHLITE_FOLD_BTN QObject::tr("FlashlightFoldBtn")
#define FLASHLITE_ON_BTN QObject::tr("FlashlightOnBtn")
#define FLASHLITE_OFF_BTN QObject::tr("FlashlightOffBtn")
#define DELAY_UNFOLD_BTN QObject::tr("DelayUnfoldBtn")
#define DELAY_FOLD_BTN QObject::tr("DelayFoldBtn")
#define DELAY_NODELAY_BTN QObject::tr("DelayNoDelayBtn")
#define DELAY_3S_BTN QObject::tr("Delay3SecBtn")
#define DELAY_6S_BTN QObject::tr("Delay6SecBtn")
#define FILTERS_UNFOLD_BTN QObject::tr("FiltersUnfoldBtn")
#define FILTERS_FOLD_BTN QObject::tr("FiltersFoldBtn")
#define FILTERS_CLOSE_BTN QObject::tr("FiltersCloseBtn")
#define EXPOSURE_BTN QObject::tr("ExposureBtn")
#define EXPOSURE_SLIDER QObject::tr("ExposureSlider")

//缩略图快捷键
#define SHORTCUT_CALLMENU QObject::tr("ShortcutCallMenu")
#define SHORTCUT_COPY QObject::tr("ShortcutCopy")
#define SHORTCUT_DELETE QObject::tr("ShortcutDelete")
#define SHORTCUT_OPENFOLDER QObject::tr("ShortcutOpenFolder")
#define SHORTCUT_PRINT QObject::tr("ShortcutPrint")


#define SHORTCUT_VIEW QObject::tr("ViewShortCut")   //快捷键预览
#define SHORTCUT_SPACE QObject::tr("SpaceShortCut")


#define CLOSE_DIALOG QObject::tr("CloseDlg")

//imageitem
#define PRINT_DIALOG QObject::tr("PrintDialog")

//closedialog
#define CANCEL_BUTTON QObject::tr("CancelBtn")
#define CLOSE_BUTTON QObject::tr("CloseBtn")



#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
