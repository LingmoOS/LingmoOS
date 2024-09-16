// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../namespace.h"
#include "frameproxyinterface.h"

#include <QtCore>

DEF_NAMESPACE_BEGIN

// ModuleInterface作为每个规范每个Module的接口，每个Module实现必须实现其所有虚函数。
class ModuleInterface
{
public:
    ModuleInterface(FrameProxyInterface *frameProxy)
        : m_frameProxy(frameProxy)
    {
    }

    virtual ~ModuleInterface() { }

    // preInitialize会在模块初始化时被调用，用于模块在准备阶段进行资源的初始化；
    // preInitialize不允许进行高资源的操作；
    virtual void preInitialize() { }

    // initialize初始化相应的模块，参数proxy用于Moudle向Frame信息查询和主动调用；
    // 返回Module的id；
    // initialize的时候不能做资源占用较高的操作；
    virtual void initialize() = 0;

    ///
    /// \brief name
    /// your module name
    /// \return
    ///
    virtual const QString name() const = 0;

    ///
    /// \brief active
    /// 当模块第一次被点击进入时，active会被调用
    virtual void active(int index);

    ///
    /// \brief active
    /// 当模块被销毁时，deactive会被调用
    virtual void deactive();

protected:
    FrameProxyInterface *m_frameProxy = nullptr;
};

DEF_NAMESPACE_END
