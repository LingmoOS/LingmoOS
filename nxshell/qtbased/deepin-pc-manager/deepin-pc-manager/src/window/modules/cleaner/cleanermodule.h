// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/interface/frameproxyinterface.h"
#include "window/interface/moduleinterface.h"
#include "window/modules/common/common.h"

DEF_NAMESPACE_BEGIN
DEF_ANTIVIRUS_NAMESPACE_BEGIN

class TrashCleanWidget;
class ModuleInterface;
class FrameProxyInterface;
class CleanerDBusAdaptorImpl;
class TrashCleanGreetingWidget;
class TrashCleanResultWidget;

class CleanerModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit CleanerModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~CleanerModule() override;

public:
    void initialize() override;
    void preInitialize() override;
    const QString name() const override;
    void active(int index) override;
    void deactive() override;

private:
    void showResultWidget();

public Q_SLOTS:

private:
    CleanerDBusAdaptorImpl *m_dbusAdaptorImpl;
    TrashCleanWidget *m_widget;
    TrashCleanGreetingWidget *m_greetingWidget;
    TrashCleanResultWidget *m_resutlWidget;
    FrameProxyInterface *m_framProxy;
};

DEF_ANTIVIRUS_NAMESPACE_END
DEF_NAMESPACE_END
