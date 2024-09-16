// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DESKTOP_ACCESSIBLE_UI_DEFINE_H
#define DESKTOP_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

//打开使用accessibility
#define ENABLE_ACCESSIBILITY

#ifdef ENABLE_ACCESSIBILITY
#define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename) classObj->setAccessibleName(accessiblename);
#else
#define AC_SET_ACCESSIBLE_NAME(classObj,accessiblename)
#endif

#define AC_SET_OBJECT_NAME(classObj,objectname) classObj->setObjectName(objectname);

// 使用宏定义，方便国际化操作
#define INSTALLACCESSIBLEFACTORY
#define OPENACCESSIBLE

#define OPEN_IMAGE ("Open Image") // 打开图片按钮
#define NOT_FOUND_IMAGE ("Image file not found") // 图片未找到lab

#define Thumbnail_Label ("ThumbnailLabel") // 缩略图lab
#define Thumbnail_Widget ("ThumbnailWidget") // 缩略图widegt

#define Lock_Widget ("LockWidget") // 现实不支持格式或损坏图片窗口

#define ADAPT_BUTTON ("AdaptBtn")//１：１视图
#define PRE_BUTTON ("PreviousButton")//上一张
#define NEXT_BUTTON ("NextButton")//下一张
#define ADAPT_SCREEN_BUTTON ("AdaptScreenBtn") //适应屏幕按钮
#define CLOCKWISE_ROTATION ("Clockwise rotation") //顺时针旋转
#define COUNTER_CLOCKWISE_ROTATION ("Counter clockwise rotation") //逆时针旋转
#define TRASH_BUTTON ("trashbtn") //删除按钮
#define TTBCONTENT_WIDGET ("ttbcontent") //缩略图区域

#define VIEW_PANEL_WIDGET ("viewpanel") //图片显示区域
#define VIEW_PANEL_STACK ("viewpanel stack") //图片显示区域堆栈窗口
#define IMAGE_LIST_WIDGET  ("myimagelistwidget") //TTB中缩略图
#define IMAGE_VIEW ("image view") //图片显示控件

#define PANEL_STACK ("panelstack") //装有图片显示区域的视图窗口
#define MAIN_WIDGET ("mainwidget") //主界面
#define MAIN_WIDOW ("mainwindow") //主窗口
#define CENTER_WIDGET ("centerwidget") //中心界面

#define SLIDE_SHOW_WIDGET ("slide show widget") //幻灯片界面
#define SLIDE_SHOW_WIDGET_BUTTOM_BAR ("slide show widget buttom bar") //幻灯片底部工具栏
#define SLIDE_SHOW_PRE_BUTTON ("slide show previous button") //幻灯片底部工具栏上一张按钮
#define SLIDE_SHOW_NEXT_BUTTON ("slide show next button") //幻灯片底部工具栏下一张按钮
#define SLIDE_SHOW_START_PAUSE_BUTTON ("slide show start pause button") //幻灯片底部工具栏开始暂停按钮
#define SLIDE_SHOW_CANCEL_BUTTON ("slide show cancel button") //幻灯片底部工具栏退出按钮
//新的幻灯片宏定义
#define Slider_Pre_Button ("slide show previous button") //幻灯片底部工具栏上一张按钮
#define Slider_Next_Button ("slide show next button") //幻灯片底部工具栏下一张按钮
#define Slider_Play_Pause_Button ("slide show start pause button") //幻灯片底部工具栏开始暂停按钮
#define Slider_Exit_Button ("slide show cancel button") //幻灯片底部工具栏退出按钮

#define TOP_TOOL_BAR ("top tool bar") //顶部工具栏
#define TITLE_TEXT ("title text") //顶部标题栏
#define TITLE_BAR ("title bar") //顶部菜单
//#define SHADOW_LINE ("shadow line") //阴影线

#define EXTENSION_PANEL ("ExtensionPanel") //图片信息窗口
#define CONTENT_WIDGET ("content widget") //图片信息界面
#define CONTENT_TITLE_BAR ("title bar") //图片信息标题栏
#define CONTENT_SCROLL_AREA ("scroll area") //图片信息滚动区域

#define BUTTOM_TOOL_BAR ("buttom_tool_bar") //底部区域

#define IMAGE_WIDGET ("imageinfo widget") //图片信息显示界面

#define RENAME_WIDGET ("rename widget") //重命名界面
#define INPUT_EDIT ("input edit") //重命名输入框
#define OK_BUTTON ("Confirm") //重命名Ok按钮
#define CANCEL_BUTTON ("Cancel") //重命名cancel按钮
#define RENAME_CONTENT ("rename content") //重命名cancel按钮



#define SC_VIEW_SHORTCUT ("sc_view_shortcut") //重命名cancel按钮
//其他
#define TOAST_OBJECT ("toast")  //toast
#define THEME_WIDGET ("theme widget")  //themewidget

#define NO_SCALE_RADIOBUTTON ("No scaling")
#define FITTOIMAGE_RADIOBUTTON ("Fit page to image")
#define FITTOPAGE_RADIOBUTTON ("Fit image to page")
#define SCALE_RADIOBUTTON ("Scale to")

#define TTL_CONTENTS ("ttl content") //TTLContent

#define IMAGE_LIST_OBJECT ("img list") //imglist

#define SLIDER_SHOW_MENU ("slidershow_menu") //缩略图右键菜单

#define MOREPIC_UP_BUTTON ("morepic_up_button") //多页图向上按键
#define MOREPIC_DOWN_BUTTON ("morepic_down_button") //多页图向下按键

#endif // DESKTOP_ACCESSIBLE_UI_DEFINE_H
