// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef APPLICATIONUPDATENOTIFIER1SERVICE_H
#define APPLICATIONUPDATENOTIFIER1SERVICE_H

#include <QObject>

constexpr auto NotifierServiceName = "org.desktopspec.ApplicationUpdateNotifier1";
constexpr auto NotifierObjectPath = "/org/desktopspec/ApplicationUpdateNotifier1";
constexpr auto NotifierInterfaceName = "org.desktopspec.ApplicationUpdateNotifier1";

class ApplicationUpdateNotifier1Service : public QObject
{
    Q_OBJECT
public:
    ApplicationUpdateNotifier1Service();
    ~ApplicationUpdateNotifier1Service() override = default;
Q_SIGNALS:
    void ApplicationUpdated();
};

#endif
