// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../namespace.h"

class QWidget;
class QString;

DEF_NAMESPACE_BEGIN
class ModuleInterface;

class FrameProxyInterface
{
public:
    // Module request to into next page
    // 推窗口进显示栈
    virtual void pushWidget(ModuleInterface *const inter, QWidget *const w) = 0;
    // 将窗口出显示栈
    virtual void popWidget(ModuleInterface *const inter) = 0;
    // 将窗口出显示栈, 并删除
    virtual void popAndDelWidget(ModuleInterface *const inter) = 0;
    // 设置模块可见
    virtual void setModuleVisible(ModuleInterface *const inter, const bool visible) = 0;
    // 设置当前显示模块
    virtual void setCurrentModule(int iModuleIdx, int iPageIdx) = 0;
    // 获取当前显示栈的索引
    virtual ModuleInterface *getCurrentModule() = 0;
    // 通过模块名称获取模块索引
    virtual int getModuleIndex(const QString &name) = 0;
    // 其它模块设置后退按钮状状态
    virtual void setBackForwardButtonStatus(bool status) = 0;
};

DEF_NAMESPACE_END
