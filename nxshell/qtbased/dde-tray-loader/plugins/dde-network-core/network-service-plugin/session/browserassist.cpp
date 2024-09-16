// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "browserassist.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

using namespace network::sessionservice;

#define dockService "com.deepin.dde.Dock"
#define dockPath "/com/deepin/dde/Dock"
#define dockInterface "com.deepin.dde.Dock"

BrowserAssist::BrowserAssist(QObject *parent)
    : QObject(parent)
    , m_dockIsRegister(false)
    , m_process(new QProcess(this))
{
    init();
}

BrowserAssist::~BrowserAssist()
{
}

void BrowserAssist::openUrl(const QString &url)
{
    static BrowserAssist browserassist;
    if (browserassist.desktopIsPrepare()) {
        browserassist.openUrlAddress(url);
    } else if (!browserassist.m_cacheUrls.contains(url)) {
        browserassist.m_cacheUrls << url;
    }
}

void BrowserAssist::init()
{
    // 先获取任务栏的服务是否启动，如果任务栏已经启动了，就认为已经进入了桌面环境，可以打开网页了
    m_dockIsRegister = QDBusConnection::sessionBus().interface()->isServiceRegistered(dockInterface);
    if (!m_dockIsRegister) {
        // 如果服务未启动，则等待服务启动
        QDBusServiceWatcher *serviceWatcher = new QDBusServiceWatcher(this);
        serviceWatcher->setConnection(QDBusConnection::sessionBus());
        qWarning() << dockInterface << "service is not register";
        serviceWatcher->addWatchedService(dockInterface);
        connect(serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &BrowserAssist::onServiceRegistered);
    }
}

bool BrowserAssist::desktopIsPrepare() const
{
    return m_dockIsRegister;
}

void BrowserAssist::openUrlAddress(const QString &url)
{
    // 调用xdg-open来打开网页
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    env.insert("DISPLAY", getDisplayEnvironment());
    if (m_process->isOpen()) {
        m_process->close();
    }
    m_process->setProcessEnvironment(env);
    m_process->start("xdg-open", QStringList() << url);
    m_process->waitForFinished();
}

QString BrowserAssist::getDisplayEnvironment() const
{
    QString displayenv = qgetenv("DISPLAY");
    if (!displayenv.isEmpty())
        return displayenv;

    // 有时候获取到的环境变量为空，此时就需要从systemd接口中来获取环境变量了
    QDBusInterface systemdInterface("org.freedesktop.systemd1", "/org/freedesktop/systemd1", "org.freedesktop.systemd1.Manager");
    QStringList environments = systemdInterface.property("Environment").toStringList();
    for (const QString &env : environments) {
        QStringList envArray = env.split("=");
        if (envArray.size() < 2)
            continue;
        if (envArray.first() == "DISPLAY")
            return envArray[1];
    }

    return QString();
}

void BrowserAssist::onServiceRegistered(const QString &service)
{
    // 服务启动后，查询列表中所有的连接，并打开
    if (service != dockInterface)
        return;

    m_dockIsRegister = true;

    // 只有等到任务栏已经启动，此时认为已经完全进入桌面，才让它打开网页
    // 但是有时候任务栏虽然已经启动了，但是偶尔会出现桌面并未完全显示出来，所以这里统一让它延迟5秒再打开网页
    QTimer::singleShot(5000, this, [ this ] {
        for (const QString &url : m_cacheUrls) {
            openUrlAddress(url);
        }
        m_cacheUrls.clear();
    });
}
