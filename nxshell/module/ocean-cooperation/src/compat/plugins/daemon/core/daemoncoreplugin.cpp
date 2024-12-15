// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "daemoncoreplugin.h"
#include "common/commonutils.h"

#include "service/servicemanager.h"

#include <QThreadPool>

using namespace daemon_core;
using namespace lingmo_cross;

void daemonCorePlugin::initialize()
{
    CommonUitls::initLog();
    CommonUitls::manageDaemonProcess("backend");
}

bool daemonCorePlugin::start()
{
    QThreadPool::globalInstance()->setMaxThreadCount(32);
    ServiceManager *manager = new ServiceManager(this);
    manager->startRemoteServer();
    return true;
}
