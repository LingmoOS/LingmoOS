// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transferplugin.h"

#include "common/commonutils.h"
#include "configs/settings/configmanager.h"
#include "gui/mainwindow.h"
#include "helper/transferhelper.h"
#include "utils/cooperationutil.h"

#include <QCommandLineParser>
#include <QCommandLineOption>

using namespace cooperation_transfer;
using namespace cooperation_core;
using namespace deepin_cross;

TransferPlugin::TransferPlugin(QObject *parent)
    : QObject(parent)
{
    initialize();
}

TransferPlugin::~TransferPlugin()
{
}

void TransferPlugin::initialize()
{
    dMain = QSharedPointer<cooperation_core::MainWindow>::create();


    auto appName = qApp->applicationName();
    qApp->setApplicationName(MainAppName);

    ConfigManager::instance();

    CommonUitls::initLog();
    CommonUitls::loadTranslator();

    qApp->setApplicationName(appName);
}

bool TransferPlugin::start()
{
    TransferHelper::instance()->registBtn(dMain.get());

    // bind search&refresh click and other status
    connect(dMain.get(), &MainWindow::searchDevice, TransferHelper::instance(), &TransferHelper::search);
    connect(dMain.get(), &MainWindow::refreshDevices, TransferHelper::instance(), &TransferHelper::refresh);

    connect(CooperationUtil::instance(), &CooperationUtil::onlineStateChanged, dMain.get(), &MainWindow::onlineStateChanged);
    connect(TransferHelper::instance(), &TransferHelper::onlineDevices, dMain.get(), &MainWindow::addDevice);
    connect(TransferHelper::instance(), &TransferHelper::offlineDevice, dMain.get(), &MainWindow::removeDevice);
    connect(TransferHelper::instance(), &TransferHelper::finishDiscovery, dMain.get(), &MainWindow::onDiscoveryFinished);

    // start network status listen after all ready
    CooperationUtil::instance()->initNetworkListener();

    dMain->show();

    return true;
}

void TransferPlugin::stop()
{
}
