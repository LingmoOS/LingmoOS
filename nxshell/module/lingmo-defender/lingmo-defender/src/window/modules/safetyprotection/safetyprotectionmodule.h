// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/interface/moduleinterface.h"
#include "window/modules/common/common.h"

#include <QObject>

DEF_NAMESPACE_BEGIN
class FrameProxyInterface;
class ModuleInterface;
class SafetyProtectionWidget;
DEF_NAMESPACE_END

class SafetyProtectionModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit SafetyProtectionModule(FrameProxyInterface *frameProxy, QObject *parent = nullptr);
    ~SafetyProtectionModule() override;
    // preInitialize会在模块初始化时被调用，用于模块在准备阶段进行资源的初始化；
    // preInitialize不允许进行高资源的操作；
    virtual void preInitialize() override;

    // initialize初始化相应的模块，参数proxy用于Moudle向Frame信息查询和主动调用；
    // 返回Module的id；
    // initialize的时候不能做资源占用较高的操作；
    virtual void initialize() override;

    ///
    /// \brief name
    /// your module name
    /// \return
    ///
    virtual const QString name() const override;

    ///
    /// \brief active
    /// 当模块第一次被点击进入时，active会被调用
    virtual void active(int index) override;

    ///
    /// \brief active
    /// 当模块被销毁时，deactive会被调用
    virtual void deactive() override;

private:
    SafetyProtectionWidget *m_safetyProtectionWidget;
};
