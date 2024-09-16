// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GLOBAL_H
#define GLOBAL_H

#include "DebugTimeManager.h"

#include <QString>

namespace Dr {

/**
启动时间                 POINT-01
关闭时间                 POINT-02
打开文档响应时间          POINT-03    文档名，文档大小
保存文档响应时间          POINT-04    文档名，文档大小
文管中双击文件打开文档时间  POINT-05    文档名，文档大小
**/

#ifdef PERF_ON
#define PERF_PRINT_BEGIN(point, desc) DebugTimeManager::getInstance()->beginPointLinux(point, desc)
#define PERF_PRINT_END(point, desc) DebugTimeManager::getInstance()->endPointLinux(point, desc)
#else
#define PERF_PRINT_BEGIN(point, desc) Q_UNUSED(point) Q_UNUSED(desc)
#define PERF_PRINT_END(point, desc) Q_UNUSED(point) Q_UNUSED(desc)
#endif

/**
 * @brief The FileType enum
 * 支持文档类型
 */
enum FileType {
    Unknown = 0,
    PDF     = 1,
    DJVU    = 2,
    DOCX    = 3,
    PS      = 4,
    DOC     = 5,
    PPTX    = 6
};
FileType fileType(const QString &filePath);

/**
 * @brief The Rotation enum
 * 文档旋转角度
 */
enum Rotation {
    RotateBy0           = 0,
    RotateBy90          = 1,
    RotateBy180         = 2,
    RotateBy270         = 3,
    NumberOfRotations   = 4
};

/**
 * @brief The MouseShape enum
 * 光标形状
 */
enum MouseShape {
    MouseShapeNormal    = 0,
    MouseShapeHand      = 1,
    NumberOfMouseShapes = 2
};

/**
 * @brief The ShowMode enum
 * 显示模式
 */
enum ShowMode {
    DefaultMode         = 0,
    SliderMode          = 1,
    MagniferMode        = 2,
    FullScreenMode      = 3,
    NumberOfShowModes   = 4
};

/**
 * @brief The LayoutMode enum
 * 布局模式
 */
enum LayoutMode {
    SinglePageMode      = 0,
    TwoPagesMode        = 1,
    NumberOfLayoutModes = 2
};

/**
 * @brief The ScaleMode enum
 * 缩放模式
 */
enum ScaleMode {
    ScaleFactorMode      = 0,
    FitToPageWidthMode   = 1,
    FitToPageHeightMode  = 2,
    NumberOfScaleModes   = 3,
    FitToPageDefaultMode = 4,
    FitToPageWorHMode    = 5
};

const QString key_up               = "Up";
const QString key_down             = "Down";
const QString key_left             = "Left";
const QString key_right            = "Right";
const QString key_space            = "Space";           // 空格用于停止启动幻灯片播放
const QString key_pgUp             = "PgUp";            // 上一页
const QString key_pgDown           = "PgDown";          // 下一页
const QString key_delete           = "Del";             // 删除
const QString key_esc              = "Esc";             // 退出全屏\退出放映\退出放大镜
const QString key_f1               = "F1";              // 帮助
const QString key_f5               = "F5";              // 播放幻灯片
const QString key_f11              = "F11";             // 全屏
const QString key_ctrl_1           = "Ctrl+1";          // 适合页面状态
const QString key_ctrl_2           = "Ctrl+2";          // 适合高度
const QString key_ctrl_3           = "Ctrl+3";          // 适合宽度
const QString key_ctrl_d           = "Ctrl+D";          // 添加书签
const QString key_ctrl_f           = "Ctrl+F";          // 搜索
const QString key_ctrl_o           = "Ctrl+O";          // 打开
const QString key_ctrl_e           = "Ctrl+E";          // 导出
const QString key_ctrl_p           = "Ctrl+P";          // 打印
const QString key_ctrl_s           = "Ctrl+S";          // 保存
const QString key_ctrl_m           = "Ctrl+M";          // 打开目标缩略图
const QString key_ctrl_r           = "Ctrl+R";          // 左旋转
const QString key_ctrl_c           = "Ctrl+C";
const QString key_ctrl_x           = "Ctrl+X";
const QString key_ctrl_v           = "Ctrl+V";
const QString key_ctrl_z           = "Ctrl+Z";
const QString key_ctrl_a           = "Ctrl+A";
const QString key_ctrl_equal       = "Ctrl+=";          // 放大
const QString key_ctrl_smaller     = "Ctrl+-";          // 缩小
const QString key_alt_1            = "Alt+1";           // 选择工具
const QString key_alt_2            = "Alt+2";           // 手型工具
const QString key_alt_a            = "Alt+A";           // 添加注释
const QString key_alt_h            = "Alt+H";           // 添加高亮
const QString key_alt_z            = "Alt+Z";           // 放大镜
const QString key_alt_f4           = "Alt+F4";          // 退出应用程序
const QString key_alt_harger       = "Ctrl++";          // 放大
const QString key_ctrl_shift_r     = "Ctrl+Shift+R";    // 右旋转
const QString key_ctrl_shift_s     = "Ctrl+Shift+S";    // 另存为
const QString key_ctrl_shift_slash = "Ctrl+Shift+/";
const QString key_ctrl_home        = "Ctrl+Home";       //第一页
const QString key_ctrl_end         = "Ctrl+End";        //最后一页
}
#endif // GLOBAL_H
