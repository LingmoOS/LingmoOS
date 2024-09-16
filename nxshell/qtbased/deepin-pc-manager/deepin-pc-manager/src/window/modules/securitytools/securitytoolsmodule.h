// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/interface/moduleinterface.h"

class SecurityToolsWidget;
class SecurityToolsModel;

// 可信保护模块
class TrustedProtectionWidget;

class SecurityToolsModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit SecurityToolsModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~SecurityToolsModule() override;

public:
    void initialize() override;
    void preInitialize() override;
    const QString name() const override;
    void active(int index) override;
    void deactive() override;

private:
    SecurityToolsWidget *m_securityToolsWidget;
    SecurityToolsModel *m_securityToolsModel;
};
