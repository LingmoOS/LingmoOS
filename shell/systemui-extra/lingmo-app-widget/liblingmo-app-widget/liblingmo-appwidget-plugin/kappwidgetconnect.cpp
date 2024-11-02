/*
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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#include "kappwidgetconnect.h"
#include <kappwidgetmanager.h>
#include <kappwidgetcommon.h>
#include <QDBusPendingReply>
#include <QDebug>
#include <QGSettings>

using namespace AppWidget;
KAppWidgetConnect::KAppWidgetConnect(QObject *parent) : QObject(parent)
{
}

KAppWidgetConnect::~KAppWidgetConnect()
{
    KAppWidgetManager::getInstance()->unregisterAppWidget(m_id);
    if(m_gsettingmap.count() > 0) {
        qDeleteAll(m_gsettingmap);
        m_gsettingmap.clear();
    }
}

void KAppWidgetConnect::onClickedEvent(const QString &widgetname, const QVariant &value)
{
    qDebug() << __FILE__ << __FUNCTION__ <<  widgetname << " send clicked signal";
    QDBusVariant variant;
    variant.setVariant(value);
    Q_EMIT clickedSignal(widgetname, variant);
}

void KAppWidgetConnect::onDefineEvent(const QString &eventname, const QString &widgetname, const QVariant &value)
{
    qDebug() << __FILE__ << __FUNCTION__ << widgetname << "Send DefineEvent Signal" << value;
    QDBusVariant variant;
    variant.setVariant(value);
    Q_EMIT defineEventSignal(eventname, widgetname, variant);
}

void KAppWidgetConnect::registerAppWidget(const QString &appname, const QString &username)
{
    initConnToManager(appname, username);
    registerAppWidgetDbus(username);
    KAppWidgetManager::getInstance()->notifyProviderUpdate(m_id);
}

void KAppWidgetConnect::initConnToManager(const QString &appname, const QString &username)
{
    qDebug() << __FILE__ << __FUNCTION__ << "Register Appwidget Service: " << username << "," << appname;
    m_appname = appname;
    m_username = username;
    m_id = KAppWidgetManager::getInstance()->registerAppWidget(m_appname, m_username);
    updateAppWidget();
}

void KAppWidgetConnect::registerAppWidgetDbus(const QString &username)
{
    if (m_id == -1) {
        qCritical() << __FILE__ << __FUNCTION__ << "Init AppWidget Id Error: id = " << m_id;
    }
    QString object = KAppWidgetCommon::getAppWidgetDbusObject(username, m_id);
    QString objectpath = KAppWidgetCommon::getAppWidgetDbusPath(username, m_id);
    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService(object)) {
        qDebug() << "Regist Dbus Service success";
        sessionBus.registerObject(objectpath, this, QDBusConnection :: ExportAllSlots
                                  | QDBusConnection :: ExportAllSignals
                                  | QDBusConnection::ExportAllInvokables);
    } else {
        qCritical() << "Register DBus Service Error:" << sessionBus.lastError().message();
    }
}

QMap<QString, QVariant> KAppWidgetConnect::getData()
{
    QMap<QString, QVariant> reply = KAppWidgetManager::getInstance()->getAppWidgetData(m_id);
    qDebug() << __FILE__ << __FUNCTION__ << ": " << reply;
    return reply;
}

void KAppWidgetConnect::updateAppWidget()
{
    Q_EMIT updatesignal();
}

void KAppWidgetConnect::updateUserStatus(const QString &status)
{
    Q_EMIT userStatusChanged(status);
}

bool KAppWidgetConnect::creatGsetting(const QString &gsetting)
{
    if(gsetting.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is null";
        return false;
    }
    if(m_gsettingmap.contains(gsetting)) {
        qWarning() << __FILE__ << __FUNCTION__ << gsetting << " gsetting isalready exists";
        return false;
    }
    QByteArray id(gsetting.toLatin1());
    if(QGSettings::isSchemaInstalled(id)) {
        QGSettings *gsettings = new QGSettings(id);
        m_gsettingmap.insert(gsetting, gsettings);
        connect(gsettings, &QGSettings::changed, [ = ](const QString & key) {
            qDebug() << "gsettings changed" << key;
            emit gsettingChanged(key, gsettings->get(key));
        });
        return true;
    } else {
        qDebug() << __FILE__ << __FUNCTION__ << gsetting << " gsetting is not install";
        return false;
    }
    return true;

}

bool KAppWidgetConnect::deleteGsetting(const QString &gsetting)
{
    if(gsetting.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "parameter is null";
        return false;
    }
    if(!m_gsettingmap.contains(gsetting)) {
        qWarning() << __FILE__ << __FUNCTION__ << gsetting << " gsetting is not exists";
        return false;
    }
    QGSettings *gsettings = m_gsettingmap.value(gsetting);

    gsettings->disconnect();
    gsettings->deleteLater();
    m_gsettingmap.remove(gsetting);
    return true;

}

QVariant KAppWidgetConnect::getGsettingValue(const QString &gsetting , const QString &key)
{
    if(gsetting.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "first parameter is null";
        return QVariant();
    }
    if(key.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "second parameter is null";
        return QVariant();
    }
    QVariant value;
    QByteArray id(gsetting.toLatin1());
    if(QGSettings::isSchemaInstalled(id)) {
         QGSettings *gsettings = new QGSettings(id);
         value = gsettings->get(key);
         gsettings->deleteLater();

    }else {
        qWarning() << __FILE__ << __FUNCTION__ << gsetting << " gsetting is uninstall";
    }
    return value;

}
