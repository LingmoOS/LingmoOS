// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <DConfig>

DCORE_USE_NAMESPACE

namespace apps {
class AppsLaunchTimesHelper : public QObject
{
    Q_OBJECT

public:
    void setLaunchTimesFor(const QString &desktopId, quint64 launchTimes);
    quint64 getLaunchedTimesFor(const QString &desktopId);
    static AppsLaunchTimesHelper* instance();

private:
    AppsLaunchTimesHelper(QObject *parent = nullptr);

private:
    DConfig* m_launchTimesConfig;
    QVariantMap m_data;
};
}