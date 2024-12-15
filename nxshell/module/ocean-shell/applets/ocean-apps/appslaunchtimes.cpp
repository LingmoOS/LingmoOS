// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appslaunchtimes.h"

namespace apps {
AppsLaunchTimesHelper* AppsLaunchTimesHelper::instance()
{
    static AppsLaunchTimesHelper* _instance = nullptr;
    if (_instance == nullptr) {
        _instance = new AppsLaunchTimesHelper;
    }

    return _instance;
}

AppsLaunchTimesHelper::AppsLaunchTimesHelper(QObject *parent)
    : QObject(parent)
    , m_launchTimesConfig(DConfig::create("org.lingmo.ocean.application-manager", "org.lingmo.ocean.am", "", this))
{
    if (m_launchTimesConfig ->isValid()) {
        m_data = m_launchTimesConfig->value("appsLaunchedTimes").toMap();
    }

    connect(m_launchTimesConfig, &DConfig::valueChanged, this, [this](const QString &key){
        if (key == "appsLaunchedTimes") {
            m_data = m_launchTimesConfig->value("appsLaunchedTimes").toMap();
        }
    });
}

void AppsLaunchTimesHelper::setLaunchTimesFor(const QString &desktopId, quint64 launchTimes)
{
    if (launchTimes == 0) {
        m_data.remove(desktopId);
    } else {
        m_data[desktopId] = launchTimes;
    }

    // TODO: write list
    m_launchTimesConfig->setValue("appsLaunchedTimes", m_data);
}

quint64 AppsLaunchTimesHelper::getLaunchedTimesFor(const QString &desktopId)
{
    return m_data.value(desktopId, 0).toULongLong();
}

}