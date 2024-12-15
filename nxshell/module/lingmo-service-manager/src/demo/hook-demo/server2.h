// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

class Service2 : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.services.demo")
public:
    explicit Service2(QObject *parent = 0)
        : QObject(parent)
    {
    }

public slots:

    void SetData(QString data)
    {
        qInfo() << "Data:" << data;
        // dosomething
    }
};