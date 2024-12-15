// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "connectivityprocesser.h"
#include "connectivitychecker.h"
#include "settingconfig.h"

#include <QDebug>

using namespace network::systemservice;

ConnectivityProcesser::ConnectivityProcesser(SystemIPConflict *ipConflict, QObject *parent)
    : QObject(parent)
    , m_ipConflictHandler(ipConflict)
{
    SettingConfig *config = SettingConfig::instance();
    connect(config, &SettingConfig::enableConnectivityChanged, this, &ConnectivityProcesser::onEnableConnectivityChanged);
    m_checker.reset(createConnectivityChecker(config->enableConnectivity()));
}

ConnectivityProcesser::~ConnectivityProcesser()
{
}

void ConnectivityProcesser::checkConnectivity()
{
    m_checker->checkConnectivity();
}

network::service::Connectivity ConnectivityProcesser::connectivity() const
{
    return m_checker->connectivity();
}

QString ConnectivityProcesser::portalUrl() const
{
    return m_checker->portalUrl();
}

ConnectivityChecker *ConnectivityProcesser::createConnectivityChecker(bool enableConnectivity)
{
    ConnectivityChecker *checker = nullptr;
    if (enableConnectivity) {
        qCDebug(DSM) << "uses local connectivity checker";
        LocalConnectionvityChecker *localChecker = new LocalConnectionvityChecker(m_ipConflictHandler, this);
        connect(localChecker, &LocalConnectionvityChecker::portalDetected, this, &ConnectivityProcesser::portalDetected);
        checker = localChecker;
    } else {
        qCDebug(DSM) << "uses nm connectivity checker";
        checker = new NMConnectionvityChecker(this);
    }

    connect(checker, &ConnectivityChecker::connectivityChanged, this, &ConnectivityProcesser::connectivityChanged);

    return checker;
}

void ConnectivityProcesser::onEnableConnectivityChanged(bool enableConnectivity)
{
    m_checker.reset(createConnectivityChecker(enableConnectivity));
}
