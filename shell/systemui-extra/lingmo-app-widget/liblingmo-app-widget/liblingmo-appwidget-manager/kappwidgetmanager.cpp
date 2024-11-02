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

#include "kappwidgetmanager.h"

#include <kappwidgetcommon.h>

#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDebug>
#include <QDBusMetaType>
namespace AppWidget
{
KAppWidgetManager::KAppWidgetManager(QObject *parent)
    :QDBusInterface(KAppWidgetCommon::getManagerDbusObject(),
                    KAppWidgetCommon::getManagerDbusPath(),
                    KAppWidgetCommon::getManagerDbusInterface(),QDBusConnection::sessionBus(),parent)
{
    qDBusRegisterMetaType<QMap<QString, QString>>();
}

QStringList KAppWidgetManager::getAppWidgetList()
{
    qDebug() << "KAppWidgetManager::getAppWidgetList";
    QStringList appWidgetList;
    QDBusMessage response = call("getAppWidgetList");
    if (response.type() == QDBusMessage::ErrorMessage ) {
        qCritical() <<  "ERROR" << response.errorMessage();
        return QStringList();
    }
    if (response.type() == QDBusMessage::ReplyMessage) {
       appWidgetList = response.arguments().takeFirst().toStringList();
    }
    return appWidgetList;
}
QString KAppWidgetManager::getAppWidgetUiFile(const QString &appwidgetname)
{
    qDebug() << "KAppWidgetManager::getAppWidgetUiFile";
    if(appwidgetname.isEmpty()) {
        qWarning() << "parameter is empty";
        return QString();
    }
    QString uiFile;
    QDBusMessage response = call("getAppWidgetUiFile", appwidgetname);
    if (response.type() == QDBusMessage::ErrorMessage ) {
        qCritical() <<  "ERROR" << response.errorMessage();
        return QString();
    }
    if (response.type() == QDBusMessage::ReplyMessage) {
       uiFile = response.arguments().takeFirst().toString();
    }
    return uiFile;
}
QString KAppWidgetManager::getAppWidgetConfig(const QString &appwidgetname,
                                                        const AppWidgetConfig& config)
{
    qDebug() << "KAppWidgetManager::getAppWidgetConfig";
    if(appwidgetname.isEmpty()) {
        qWarning() << "parameter is empty";
        return QString();
    }
    QString uiFile;
    QDBusMessage response = call("getAppWidgetConfig", appwidgetname, enumToSTring(config));
    if (response.type() == QDBusMessage::ErrorMessage ) {
        qCritical() <<  "ERROR" << response.errorMessage();
        return QString();
    }
    if (response.type() == QDBusMessage::ReplyMessage) {
       uiFile = response.arguments().takeFirst().toString();
    }
    return uiFile;
}
QMap<QString, QVariant> KAppWidgetManager::getAppWidgetConfigs(const QString &appwidgetname)
{
    qDebug() << "KAppWidgetManager::getAppWidgetConfigs";
    if(appwidgetname.isEmpty()) {
        qWarning() << "parameter is empty";
        return QMap<QString, QVariant>();
    }
    QMap<QString, QVariant> confs;
    QDBusPendingReply<QMap<QString, QVariant>> reply = call("getAppWidgetConfigs", appwidgetname);
    confs =reply.value();
    return confs;
}

int KAppWidgetManager::registerAppWidget(const QString &appwidgetname,
                                         const QString &username)
{
    qDebug() << "KAppWidgetManager::registerAppWidget";
    if(appwidgetname.isEmpty() || username.isEmpty()) {
        qWarning() << "parameter is empty";
        return -1;
    }
    int id = -1;
    QDBusMessage response = call("registerAppWidget", appwidgetname, username);
    if (response.type() == QDBusMessage::ErrorMessage ) {
        qCritical() <<  "ERROR" << response.errorMessage();
        return -1;
    }
    if (response.type() == QDBusMessage::ReplyMessage) {
       id = response.arguments().takeFirst().toInt();
    }
    return id;
}
bool KAppWidgetManager::unregisterAppWidget(const int &appwidgetid)
{
    qDebug() << "KAppWidgetManager::unregisterAppWidget";
    if(appwidgetid < 0) {
        qWarning() << "parameter is error";
        return false;
    }
    bool unregister = false;
    QDBusMessage response = call("unregisterAppWidget", appwidgetid);
    if (response.type() == QDBusMessage::ErrorMessage ) {
        qCritical() << __FUNCTION__ << __LINE__ << response.errorMessage();
        return false;
    }
    if (response.type() == QDBusMessage::ReplyMessage) {
        unregister = response.arguments().takeFirst().toBool();
    }
    return unregister;
}
QMap<QString, QVariant> KAppWidgetManager::getAppWidgetData(const int &appwidgetid)
{
    qDebug() << "KAppWidgetManager::getAppWidgetData";
    if(appwidgetid < 0) {
        qWarning() << __FUNCTION__ << __LINE__ << "get invalid id :" << appwidgetid;
        return QMap<QString, QVariant>();
    }
    QDBusPendingReply<QMap<QString, QVariant>> reply = asyncCall("getAppWidgetData", appwidgetid);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << __FUNCTION__ << __LINE__ << reply.error().message();
        return QMap<QString, QVariant>();
    }
    QMap<QString, QVariant> map = reply.value();
    return map;
}
void KAppWidgetManager::updateAppWidget(const QString &providername,
                                        const QMap<QString, QVariant> &data)
{
    qDebug() << "KAppWidgetManager::updateAppWidget" << providername << data;
    if(providername.isEmpty()) {
        qWarning() << "parameter is empty";
        return ;
    }
    asyncCall("updateAppWidget", providername, data);
}
QString KAppWidgetManager::enumToSTring(const AppWidgetConfig& config)
{
    QString conf;
    switch (config) {
    case AppWidgetConfig::NAME:
        conf = "name";
        break;
    case AppWidgetConfig::PROVIDER_NAME:
        conf = "providerName";
        break;
    case AppWidgetConfig::PREVIEW_PATH:
        conf = "previewPath";
        break;
    case AppWidgetConfig::ZOOM:
        conf = "zoom";
        break;
    case AppWidgetConfig::RIGHT_BUTTON:
        conf = "rightButton";
        break;
    case AppWidgetConfig::REPEAT_PLACEMENT:
        conf = "repeatPlacement";
        break;
    case AppWidgetConfig::MIN_WIDTH:
        conf = "minWidget";
        break;
    case AppWidgetConfig::MIN_HEIGHT:
        conf = "minHeight";
        break;
    case AppWidgetConfig::TARGET_CELL_WIDTH:
        conf = "targetCellWidth";
        break;
    case AppWidgetConfig::TARGET_CELL_HEIGHT:
        conf = "targetCellHeight";
        break;
    case AppWidgetConfig::UPDATE_PERIODMILLIS:
        conf = "updatePeriodMillis";
        break;
    case AppWidgetConfig::DESCRIBE:
        conf = "describe";
        break;
    default:
        break;
    }
    return conf;
}
QString KAppWidgetManager::enumUserStatusToSTring(const UserStatus& userstatus)
{
    QString status = "Normal";
    switch (userstatus) {
    case UserStatus::NORMAL:
        status = "Normal";
        break;
    case UserStatus::EDITABLE:
        status = "Editable";
        break;
    default:
        break;
    }
    return status;
}
void KAppWidgetManager::updateUserStatus(const QString &username, const UserStatus& userstatus)
{
    if(username.isEmpty()) {
        qWarning() << __FUNCTION__ << __LINE__ << "参数不能为空";
        return;
    }
    QString status = enumUserStatusToSTring(userstatus);
    asyncCall("updateUserStatus", username, status);
}
void KAppWidgetManager::notifyProviderUpdate(int id) {
    asyncCall("notifyProviderUpdate", id);
}

QMap<QString, QString> KAppWidgetManager::getAppWidgetLanguageFiles(const QString &appwidgetname)
{
    if(appwidgetname.isEmpty()) {
        qWarning() << __FILE__ << __FUNCTION__ << "appwidgetname is empty";
        return QMap<QString, QString>();
    }

    QDBusReply<QMap<QString, QString>> reply = call("getAppWidgetLanguageFiles", appwidgetname);
    if (!reply.isValid()) {
        qWarning() << __FUNCTION__ << __LINE__ << reply.error();
        return QMap<QString, QString>();
    }

    return reply.value();
}

}// namespace AppWidget
