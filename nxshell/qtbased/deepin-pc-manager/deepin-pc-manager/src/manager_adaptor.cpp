// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "manager_adaptor.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class ManagerAdaptor
 */

ManagerAdaptor::ManagerAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

ManagerAdaptor::~ManagerAdaptor()
{
    // destructor
}

void ManagerAdaptor::ExitApp()
{
    // handle method call com.deepin.pc.manager.ExitApp
    QMetaObject::invokeMethod(parent(), "ExitApp");
}

void ManagerAdaptor::Show()
{
    // handle method call com.deepin.pc.manager.Show
    QMetaObject::invokeMethod(parent(), "Show");
}

void ManagerAdaptor::ShowModule(const QString &module)
{
    // handle method call com.deepin.pc.manager.ShowModule
    QMetaObject::invokeMethod(parent(), "ShowModule", Q_ARG(QString, module));
}

void ManagerAdaptor::ShowPage(const QString &module, const QString &page)
{
    // handle method call com.deepin.pc.manager.ShowPage
    QMetaObject::invokeMethod(parent(), "ShowPage", Q_ARG(QString, module), Q_ARG(QString, page));
}

