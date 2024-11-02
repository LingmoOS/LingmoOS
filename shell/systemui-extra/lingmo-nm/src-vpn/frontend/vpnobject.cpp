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
#include "vpnobject.h"
#include <QApplication>

#include "lingmostylehelper/lingmostylehelper.h"

vpnObject::vpnObject(QMainWindow *parent) : QMainWindow(parent)
{
    initUI();
    initTrayIcon();
    initDbusConnnect();
}

vpnObject::~vpnObject()
{
    if (m_vpnGsettings != nullptr) {
        delete m_vpnGsettings;
        m_vpnGsettings = nullptr;
    }
}

void vpnObject::initUI()
{
    m_vpnPage = new VpnPage(nullptr);
    m_vpnPage->update();

}

void vpnObject::initTrayIcon()
{
    m_vpnTrayIcon = new QSystemTrayIcon(this);
    m_vpnTrayIcon->setToolTip(QString(tr("vpn tool")));
    m_vpnTrayIcon->setIcon(QIcon::fromTheme("lingmo-vpn-symbolic"));
    m_vpnTrayIcon->setVisible(true);
    initVpnIconVisible();
    connect(m_vpnTrayIcon, &QSystemTrayIcon::activated, this, &vpnObject::onTrayIconActivated);
}

void vpnObject::initVpnIconVisible()
{
    if (QGSettings::isSchemaInstalled(QByteArray(GSETTINGS_VPNICON_VISIBLE))) {
        m_vpnGsettings = new QGSettings(QByteArray(GSETTINGS_VPNICON_VISIBLE));
        if (m_vpnGsettings->keys().contains(QString(VISIBLE))) {
            m_vpnTrayIcon->setVisible(m_vpnGsettings->get(VISIBLE).toBool());
            connect(m_vpnGsettings, &QGSettings::changed, this, [=]() {
                m_vpnTrayIcon->setVisible(m_vpnGsettings->get(VISIBLE).toBool());
            });
        }
    }
}

/**
 * @brief vpnObject::onTrayIconActivated 点击托盘图标的槽函数
 */
void vpnObject::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
        case QSystemTrayIcon::Trigger:
            if(m_vpnPage->isActiveWindow()) {
                m_vpnPage->hide();
            } else {
                onShowMainWindow();
            }
            break;

        default:
            break;
    }
}

void vpnObject::getVirtualList(QVector<QStringList> &vector)
{
    vector.clear();
    if (nullptr != m_vpnPage) {
        m_vpnPage->getVirtualList(vector);
    }
}

//Vpn连接删除
void vpnObject::deleteVpn(const QString &connUuid)
{
    m_vpnPage->deleteVpn(connUuid);
}

//Vpn连接断开
void vpnObject::activateVpn(const QString& connUuid)
{
    m_vpnPage->activateVpn(connUuid);
}
void vpnObject::deactivateVpn(const QString& connUuid)
{
    m_vpnPage->deactivateVpn(connUuid);
}

void vpnObject::showDetailPage(const QString& connUuid)
{
    m_vpnPage->showDetailPage(connUuid);
}


void vpnObject::onShowMainWindow()
{
    kdk::LingmoUIStyleHelper::self()->removeHeader(m_vpnPage);
    m_vpnPage->showUI();
}

void vpnObject::initDbusConnnect()
{
    connect(m_vpnPage, &VpnPage::activateFailed, this, &vpnObject::activateFailed);
    connect(m_vpnPage, &VpnPage::deactivateFailed, this, &vpnObject::deactivateFailed);

    connect(m_vpnPage, &VpnPage::vpnAdd, this, &vpnObject::vpnAdd);
    connect(m_vpnPage, &VpnPage::vpnRemove, this, &vpnObject::vpnRemove);
    connect(m_vpnPage, &VpnPage::vpnUpdate, this, &vpnObject::vpnUpdate);
    connect(m_vpnPage, &VpnPage::vpnActiveConnectionStateChanged, this, &vpnObject::vpnActiveConnectionStateChanged);
    //模式切换
    QDBusConnection::sessionBus().connect(QString("com.lingmo.statusmanager.interface"),
                                         QString("/"),
                                         QString("com.lingmo.statusmanager.interface"),
                                         QString("mode_change_signal"), this, SLOT(onTabletModeChanged(bool)));
}

void vpnObject::onTabletModeChanged(bool mode)
{
    qDebug() << "TabletMode change" << mode;
    Q_UNUSED(mode)
    //模式切换时，隐藏主界面
    m_vpnPage->hide();
}

bool vpnObject::launchApp(QString desktopFile)
{
    QDBusInterface appManagerDbusInterface(LINGMO_APP_MANAGER_NAME,
                                             LINGMO_APP_MANAGER_PATH,
                                             LINGMO_APP_MANAGER_INTERFACE,
                                             QDBusConnection::sessionBus());

    if (!appManagerDbusInterface.isValid()) {
        qWarning()<<"appManagerDbusInterface init error";
        return false;
    } else {
        QDBusReply<bool> reply = appManagerDbusInterface.call("LaunchApp", desktopFile);
        return reply;
    }
}

void vpnObject::runExternalApp() {
    if (!launchApp("nm-connection-editor.desktop")){
        QString cmd = "nm-connection-editor";
        QProcess process(this);
        process.startDetached(cmd);
    }
}

void vpnObject::showVpnAddWidget()
{
#ifdef VPNDETAIL
    if (m_vpnAddPage == nullptr) {
        m_vpnAddPage = new vpnAddPage();
        connect(m_vpnAddPage, &vpnAddPage::closed, [&] () {m_vpnAddPage = nullptr;});
        m_vpnAddPage->show();
        m_vpnAddPage->centerToScreen();
    }
    m_vpnAddPage->raise();
#else
    runExternalApp();
    return;
#endif
}
