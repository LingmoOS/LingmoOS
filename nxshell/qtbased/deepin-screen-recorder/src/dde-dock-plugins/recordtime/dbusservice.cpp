// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusservice.h"
#include <QDebug>
DBusService::DBusService(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
}

bool DBusService::onStart()
{
    emit start();
    return true;
}

bool DBusService::onStop()
{
    emit stop();
    return true;
}

bool DBusService::onRecording()
{
    emit recording();
    return true;
}

bool DBusService::onPause()
{
    emit pause();
    return true;
}
