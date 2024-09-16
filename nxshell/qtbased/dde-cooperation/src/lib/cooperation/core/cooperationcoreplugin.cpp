// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationcoreplugin.h"
#include "utils/cooperationutil.h"
#include "utils/historymanager.h"
#include "discover/discovercontroller.h"
#include "net/networkutil.h"
#include "net/transferwrapper.h"
#include "net/helper/transferhelper.h"
#include "net/helper/sharehelper.h"
#include "discover/deviceinfo.h"


#include "common/commonutils.h"
#include "configs/settings/configmanager.h"
#include "singleton/singleapplication.h"

#include <QCommandLineParser>
#include <QCommandLineOption>
#ifdef __linux__
#    include "base/reportlog/reportlogmanager.h"
#    include <DFeatureDisplayDialog>
#    include <QFile>
DWIDGET_USE_NAMESPACE
#endif

using namespace cooperation_core;
using namespace deepin_cross;

CooperaionCorePlugin::CooperaionCorePlugin(QObject *parent)
    : QObject(parent)
{
    initialize();
}

CooperaionCorePlugin::~CooperaionCorePlugin()
{
}

void CooperaionCorePlugin::initialize()
{
    CommonUitls::initLog();
    CommonUitls::loadTranslator();
    dMain = QSharedPointer<MainWindow>::create();

    onlyTransfer = qApp->property("onlyTransfer").toBool();
    if (onlyTransfer) {
        auto appName = qApp->applicationName();
        qApp->setApplicationName(MainAppName);
        ConfigManager::instance();
        qApp->setApplicationName(appName);
    }

#ifdef linux
    ReportLogManager::instance()->init();
#endif

    CooperationUtil::instance();
}

bool CooperaionCorePlugin::isMinilize()
{
    QCommandLineParser parser;
    // 添加自定义选项和参数"-m"
    QCommandLineOption option("m", "Launch with minimize UI");
    parser.addOption(option);

    // 解析命令行参数
    const auto &args = qApp->arguments();
    if (args.size() != 2 || !args.contains("-m"))
        return false;

    parser.process(args);
    return parser.isSet(option);;
}

bool CooperaionCorePlugin::start()
{
    CooperationUtil::instance()->mainWindow(dMain);

    TransferHelper::instance()->registBtn();
    if (onlyTransfer) {
        // transfer deepend cooperation, not need network & share. bind the sendfile command.
        connect(TransferHelper::instance(), &TransferHelper::deliverMessage, qApp, &SingleApplication::onDeliverMessage);
    } else {
        DiscoverController::instance();
        NetworkUtil::instance();

        ShareHelper::instance()->registConnectBtn();

        // bind search&refresh click and other status
        connect(dMain.get(), &MainWindow::searchDevice, NetworkUtil::instance(), &NetworkUtil::trySearchDevice);
        connect(dMain.get(), &MainWindow::refreshDevices, DiscoverController::instance(), &DiscoverController::startDiscover);

        // bind storage setting
        connect(CooperationUtil::instance(), &CooperationUtil::storageConfig, NetworkUtil::instance(), &NetworkUtil::updateStorageConfig);

        // bind device change with UI
        connect(CooperationUtil::instance(), &CooperationUtil::onlineStateChanged, dMain.get(), &MainWindow::onlineStateChanged);
        connect(DiscoverController::instance(), &DiscoverController::deviceOnline, dMain.get(), &MainWindow::addDevice);
        connect(DiscoverController::instance(), &DiscoverController::deviceOffline, dMain.get(), &MainWindow::removeDevice);
        connect(DiscoverController::instance(), &DiscoverController::discoveryFinished, dMain.get(), &MainWindow::onDiscoveryFinished);

        // bridge: update the connected history after discover finish.
        connect(DiscoverController::instance(), &DiscoverController::discoveryFinished, HistoryManager::instance(), &HistoryManager::refreshHistory);
        connect(HistoryManager::instance(), &HistoryManager::historyConnected, DiscoverController::instance(), &DiscoverController::updateHistoryDevices);

        DiscoverController::instance()->init(); // init zeroconf and regist

        // start network status listen after all ready
        CooperationUtil::instance()->initNetworkListener();

        // start local ipc listen for transfer app
        TransferWrapper::instance()->listen(qAppName());

#ifdef __linux__
        if (CommonUitls::isFirstStart()) {
            DFeatureDisplayDialog *dlg = qApp->featureDisplayDialog();
            auto btn = dlg->getButton(0);
            connect(btn, &QAbstractButton::clicked, qApp, &SingleApplication::helpActionTriggered);
            CooperationUtil::instance()->showFeatureDisplayDialog(dlg);
        }
#endif
    }

    if (isMinilize()) {
        dMain->minimizedAPP();
    } else {
        dMain->show();
    }

    return true;
}

void CooperaionCorePlugin::stop()
{
    NetworkUtil::instance()->stop();
}
