/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#include "lingmo-search-dbus-service.h"
#include <KWindowSystem>
#include <utility>
#include "serviceadaptor.h"

using namespace LingmoUISearch;
void LingmoUISearchDbusServices::showWindow()
{
    QString display = checkDisplay();
    qDebug() << "showWindow called, from display:" << display << "current display: " << m_display;
    if(!display.isEmpty() && display != m_display) {
        Q_EMIT showWindowSignal(display);
        return;
    }
    onShowWindow(m_display);
}

void LingmoUISearchDbusServices::searchKeyword(QString keyword)
{
    QString display = checkDisplay();
    qDebug() << "searchKeyword called, from display:" << display << "current display: " << m_display;
    if(!display.isEmpty() && display != m_display) {
        Q_EMIT searchKeywordSignal(display, keyword);
        return;
    }
    onSearchKeyword(m_display, keyword);
}

void LingmoUISearchDbusServices::mainWindowSwitch()
{
    QString display = checkDisplay();
    qDebug() << "mainWindowSwitch called, from display:" << display << "current display: " << m_display;
    if(!display.isEmpty() && display != m_display) {
        Q_EMIT mainWindowSwitchSignal(display);
        return;
    }
    onMainWindowSwitch(m_display);
}

LingmoUISearchDbusServices::LingmoUISearchDbusServices(MainWindow *m, QObject *parent):
        QObject(parent),
        m_mainWindow(m)
{
    m_mainWindow = m;
    m_display = qApp->property("display").toString();
    //注册服务
    bool isServiceRegistered = QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("com.lingmo.search.service"));
    if(isServiceRegistered) {
        initWatcher();
    } else {
        if(!registerService()) {
            initWatcher();
        }
    }
}

void LingmoUISearchDbusServices::initWatcher()
{
    m_watcher = new QDBusServiceWatcher(QStringLiteral("com.lingmo.search.service"),QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &LingmoUISearchDbusServices::onServiceOwnerChanged);
    connectToService();
}

bool LingmoUISearchDbusServices::registerService()
{
    if(!m_adaptor) {
        m_adaptor = new ServiceAdaptor(this);
    }
    QDBusConnection conn = QDBusConnection::sessionBus();
    auto reply = conn.interface()->registerService(QStringLiteral("com.lingmo.search.service"),
                                                          QDBusConnectionInterface::ReplaceExistingService,
                                                          QDBusConnectionInterface::DontAllowReplacement);
    if (reply.value() == QDBusConnectionInterface::ServiceNotRegistered) {
        return false;
    }

    bool res = QDBusConnection::sessionBus().registerObject("/", this);
    if (!res) {
        QDBusConnection::sessionBus().interface()->unregisterService(QStringLiteral("com.lingmo.search.service"));
    }
    return res;
}

void
LingmoUISearchDbusServices::onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    if (newOwner.isEmpty()) {
        bool success = registerService();
        if (success) {
            disConnectToService();
            m_watcher->deleteLater();
        }
        qDebug() << "try to register service:" << success;
        return;
    }

    uint newOwnerPid = QDBusConnection::sessionBus().interface()->servicePid(newOwner);
    qDebug() << "newOwnerPid:" << newOwnerPid << ", myPid:" << QCoreApplication::applicationPid() << ", display:" << m_display;
}

void LingmoUISearchDbusServices::connectToService()
{
    m_serviceIface = new OrgLingmoUISearchServiceInterface(QStringLiteral("com.lingmo.search.service"), "/", QDBusConnection::sessionBus(), this);
    connect(m_serviceIface, &OrgLingmoUISearchServiceInterface::showWindowSignal, this, &LingmoUISearchDbusServices::onShowWindow);
    connect(m_serviceIface, &OrgLingmoUISearchServiceInterface::searchKeywordSignal, this, &LingmoUISearchDbusServices::onSearchKeyword);
    connect(m_serviceIface, &OrgLingmoUISearchServiceInterface::mainWindowSwitchSignal, this, &LingmoUISearchDbusServices::onMainWindowSwitch);
}

void LingmoUISearchDbusServices::disConnectToService()
{
    if(m_serviceIface) {
        m_serviceIface->disconnect();
        m_serviceIface->deleteLater();
    }
}

void LingmoUISearchDbusServices::onShowWindow(const QString &display)
{
    if(m_display == display) {
        m_mainWindow->bootOptionsFilter("-s");
    }
}

void LingmoUISearchDbusServices::onSearchKeyword(const QString &display, const QString &keyword)
{
    if(m_display == display) {
        m_mainWindow->bootOptionsFilter("-s");
        m_mainWindow->setText(keyword);
    }
}

void LingmoUISearchDbusServices::onMainWindowSwitch(const QString &display)
{
    if(m_display == display) {
        if (m_mainWindow->isVisible()) {
            m_mainWindow->tryHide();
        } else {
            m_mainWindow->bootOptionsFilter("-s");
        }
    }
}

QString LingmoUISearchDbusServices::checkDisplay()
{
    uint pid = 0;
    QDBusReply<uint> pidReply = connection().interface()->servicePid(message().service());
    qDebug() << "caller pid: " << pidReply.value();
    if(pidReply.isValid()) {
        pid = pidReply.value();
    } else {
        return {};
    }
    return LingmoUISearchDbusServices::displayFromPid(pid);;
}

QString LingmoUISearchDbusServices::displayFromPid(uint pid)
{
    QFile environFile(QStringLiteral("/proc/%1/environ").arg(QString::number(pid)));
    if (environFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        const QByteArray DISPLAY = qApp->property("sessionType").toString() == "wayland" ? QByteArrayLiteral("WAYLAND_DISPLAY")
                                                                      : QByteArrayLiteral("DISPLAY");
        const auto lines = environFile.readAll().split('\0');
        for (const QByteArray &line : lines) {
            const int equalsIdx = line.indexOf('=');
            if (equalsIdx <= 0) {
                continue;
            }
            const QByteArray key = line.left(equalsIdx);
            if (key == DISPLAY) {
                const QByteArray value = line.mid(equalsIdx + 1);
                return value;
            }
        }
    }
    return {};
}
