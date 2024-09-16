// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "manager_adaptor.h"

#include <QObject>
#include "manager_adaptor.h"

class MainWindow;

class DefenderDBusService : public QObject
{
    Q_OBJECT
public:
    explicit DefenderDBusService(QObject *parent = nullptr);

Q_SIGNALS:

public Q_SLOTS:
    void Show();
    MainWindow *parent();
    void ShowModule(const QString &module);
    void ShowPage(const QString &module, const QString &page);
    void ExitApp();

private:
    ManagerAdaptor *m_managerAdaptor;
};
