// Copyright (C) 2020 UOS Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H
#define SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H

#include <QString>
#include <QObject>

// 使用宏定义，方便国际化操作



// 开始录屏，截图按钮
#define AC_MAINWINDOW_MAINSHOTBTN "main_shot_but" // 截图按钮
#define AC_MAINWINDOW_MAINRECORDBTN "main_record_but" // 录屏按钮

// 截图录屏切换按钮
#define AC_MAINTOOLWIDGET_RECORD_BUT "to_record_but" //切换到录屏功能按钮
#define AC_MAINTOOLWIDGET_SHOT_BUT "to_shot_but"// 切换到截图功能按钮

// 关闭按钮
#define AC_TOOLBARWIDGET_CLOSE_BUTTON_TOOL "toolbar_close_button"// 关闭按钮
#define AC_TOOLBARWIDGET_CLOSE_BUTTON_SIDE "sidebarclose_button"// 关闭按钮

// 底部工具栏按钮
#define AC_SUBTOOLWIDGET_AUDIO_BUTTON "audio_button"// 录屏 音频按钮
#define AC_SUBTOOLWIDGET_KEYBOARD_BUTTON "keyboard_button"// 录屏 显示按键按钮
#define AC_SUBTOOLWIDGET_CAMERA_BUTTON "camera_button"//录屏 摄像头按钮
#define AC_SUBTOOLWIDGET_MOUSE_BUTTON "mouse_button"//录屏 显示鼠标按钮
#define AC_SUBTOOLWIDGET_SHOT_BUTTON "shot_button"//录屏 截图按钮
#define AC_SUBTOOLWIDGET_RECORD_OPTION_BUT "record_option_but" //录屏选项按钮

#define AC_SUBTOOLWIDGET_PINSCREENSHOTS_BUTTON "pinscreenshots_button"// 截图 贴图工具按钮
#define AC_SUBTOOLWIDGET_SCROLLSHOT_BUTTON "scrollshot_button"// 截图 滚动截图别工具按钮
#define AC_SUBTOOLWIDGET_OCR_BUTTON "orc_button"// 截图 文字识别工具按钮
#define AC_SUBTOOLWIDGET_RECT_BUTTON "rect_button"// 截图 矩形工具按钮
#define AC_SUBTOOLWIDGET_CIRCL_BUTTON "circl_button"//截图 椭圆工具按钮
#define AC_SUBTOOLWIDGET_LINE_BUTTON "line_button"// 截图 直线按钮
#define AC_SUBTOOLWIDGET_PEN_BUTTON "pen_button"// 截图 画笔按钮
#define AC_SUBTOOLWIDGET_MOSAIC_BUTTON "mosaic_button"// 截图 模糊按钮
#define AC_SUBTOOLWIDGET_TEXT_BUTTON "text_button"// 截图文本按钮
#define AC_SUBTOOLWIDGET_UNDO_BUTTON "undo_button"// 截图撤销按钮
#define AC_SUBTOOLWIDGET_RECORDER_BUTTON "recorder_button"// 截图 录屏按钮
#define AC_SUBTOOLWIDGET_SHOT_OPTION_BUT "shot_option_but" // 截图 选项按钮


// 边侧工具栏按钮
#define AC_SHOTTOOLWIDGET_THICK_ONE_RECT "thick_one_rect" // 矩形宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_RECT "thick_two_rect" // 矩形宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_RECT "thick_three_rect" //矩形宽度3
#define AC_SHOTTOOLWIDGET_BLUR_RECT "thick_blur_rect"//矩形水滴
#define AC_SHOTTOOLWIDGET_MOSAIC_RECT "thick_mosaic_rect"//矩形马赛克

#define AC_SHOTTOOLWIDGET_THICK_ONE_CIRC "thick_one_circ"// 椭圆宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_CIRC "thick_two_circ"// 椭圆宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_CIRC "thick_three_circ"// 椭圆宽度3
#define AC_SHOTTOOLWIDGET_BLUR_CIRC "thick_blur_circ"//椭圆水滴
#define AC_SHOTTOOLWIDGET_MOSAIC_CIRC "thick_mosaic_circ"//椭圆马赛克

#define AC_SHOTTOOLWIDGET_THICK_ONE_LINE "thick_one_line"// 直线宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_LINE "thick_two_line"// 直线宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_LINE "thick_three_line"// 直线宽度3
#define AC_SHOTTOOLWIDGET_LINE_BUTTON "thick_line_line" // 直线按钮
#define AC_SHOTTOOLWIDGET_ARROW_BUTTON "thick_arrow_line"// 箭头按钮

#define AC_SHOTTOOLWIDGET_THICK_ONE_PEN "thick_one_pen"// 画笔宽度1
#define AC_SHOTTOOLWIDGET_THICK_TWO_PEN "thick_two_pen"// 画笔宽度2
#define AC_SHOTTOOLWIDGET_THICK_THREE_PEN "thick_three_pen"// 画笔宽度3

// 颜色按钮
#define AC_COLORTOOLWIDGET_BLACK_BUT "Black_button"//黑色按钮
#define AC_COLORTOOLWIDGET_GRAY_BUT "Gray_button"//灰色按钮
#define AC_COLORTOOLWIDGET_WHITE_BUT "White_button"//白色按钮
#define AC_COLORTOOLWIDGET_RED_BUT "Red_button" // 红色按钮
#define AC_COLORTOOLWIDGET_ORANGE_BUT "Orange_button"//橙色按钮
#define AC_COLORTOOLWIDGET_YELLOW_BUT "Yellow_button"// 黄色按钮
#define AC_COLORTOOLWIDGET_LIGHTLIME_BUT "LightLime_button"//亮柠檬绿按钮
#define AC_COLORTOOLWIDGET_AQUAMARINE_BUT "Aquamarine_button"//碧蓝色按钮
#define AC_COLORTOOLWIDGET_GREEN_BUT "Green_button"//绿色按钮
#define AC_COLORTOOLWIDGET_BLUE_BUT "Blue_button"// 蓝色按钮
#define AC_COLORTOOLWIDGET_PURPLE_BUT "Purple_button"//紫色按钮
#define AC_COLORTOOLWIDGET_DARKBLUE_BUT "DarkBlue_button"//深蓝按钮

// 字体大小
#define AC_SHOTTOOLWIDGET_TEXT_FONT_SIZE "text_font_size"// 字体大小

#endif // SHOT_RECORDER_ACCESSIBLE_UI_DEFINE_H
