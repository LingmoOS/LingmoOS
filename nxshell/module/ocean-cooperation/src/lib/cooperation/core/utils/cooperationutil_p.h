// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONUTIL_P_H
#define COOPERATIONUTIL_P_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include "gui/mainwindow.h"

#include <QObject>

class SessionManager;
namespace cooperation_core {

class MainWindow;
class CooperationUtil;
class CooperationUtilPrivate : public QObject
{
    Q_OBJECT
public:
    explicit CooperationUtilPrivate(CooperationUtil *qq);
    ~CooperationUtilPrivate();

public:
    CooperationUtil *q { nullptr };
    QSharedPointer<MainWindow> window { nullptr };
    bool isOnline { false };
};

}

#endif   // COOPERATIONUTIL_P_H
