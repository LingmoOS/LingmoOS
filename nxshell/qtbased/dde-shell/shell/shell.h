// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <QObject>

DS_BEGIN_NAMESPACE

class Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent = nullptr);
    void installDtkInterceptor();
    void disableQmlCache();
    void setFlickableWheelDeceleration(const int &value);
    void dconfigsMigrate();

private:
    bool dconfigMigrate(const QString &newConf, const QString &oldConf);
};

DS_END_NAMESPACE
