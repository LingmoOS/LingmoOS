// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QString>
#include <syslog.h>

#include "daemon.h"
#include "booster-dtkdeclarative.h"

#include <DAppLoader>
#include <DWindowManagerHelper>
#include <QImageReader>
#include <QQmlComponent>
#include <QQuickView>

DQUICK_USE_NAMESPACE

BEGIN_NAMESPACE
const string DeclarativeBooster::m_boosterType  = "dtkqml";

const string & DeclarativeBooster::boosterType() const
{
    return m_boosterType;
}

void DeclarativeBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    auto app = new QGuiApplication(initialArgc, initialArgv);
    Q_UNUSED(app) // delete in DeclarativeBooster::preload
    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

int DeclarativeBooster::launchProcess()
{
    setEnvironmentBeforeLaunch();

    // make booster specific initializations unless booster is in boot mode
    if (!bootMode())
        preinit();

    // Close syslog
    closelog();

    string appName = m_appData->fileName();

    appName = appName.substr(appName.find_last_of("/") + 1);
    DAppLoader appLoader(QString::fromStdString(appName));

    // 默认开启高分辨率图标支持
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    int argc = m_appData->argc();
    char **argv = const_cast<char**>(m_appData->argv());

    return appLoader.exec(argc, argv);
}

bool DeclarativeBooster::preload()
{
    // 此处销毁了 QGuiApplication 对象会存在一个问题：
    // 目前的做法是通过 DAppLoader 类型去解析插件类型，之后再通过 createApplication() 创建 QGuiApplication 对象；
    // 如果 qml 应用在插件的构造函数中(也就是在使用createApplication之前)去连接(connect)信号和槽，就去出现问题。

    // 销毁在initialize中创建的QGuiApplication对象
    // 这个对象仅是为了preload中创建窗口使用，preload之后应当销毁它
    QScopedPointer<QGuiApplication> appClearer(qGuiApp);
    Q_UNUSED(appClearer)

    QQuickView window;
    window.create();

    // 初始化图片解码插件，在龙芯和申威上，Qt程序冷加载图片解码插件几乎耗时1s
    Q_UNUSED(QImageReader::supportedImageFormats());

    return true;
}
END_NAMESPACE

int main(int argc, char **argv)
{
    // NOTE(lxz): booster will delete in daemon fork function
    DeepinTurbo::DeclarativeBooster* booster = new DeepinTurbo::DeclarativeBooster;

    DeepinTurbo::Daemon d(argc, argv);
    d.run(booster);
}
