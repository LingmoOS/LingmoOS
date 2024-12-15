// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "appchoosermodel.h"
#include "iteminfo.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QSettings>

AppChooserModel::AppChooserModel(QObject *parent)
    :QAbstractListModel(parent)
{
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<PropMap>();
    QMetaObject::invokeMethod(this, &AppChooserModel::loadApplications, Qt::QueuedConnection);
}

void AppChooserModel::click(const QModelIndex &index)
{
    if (index.row() >= 0 && index.row() <= m_datas.size() -1) {
        beginResetModel();
        m_datas[index.row()].selected = ! m_datas[index.row()].selected;
        endResetModel();
    }
}

void AppChooserModel::click(const QString &appId)
{
    for (auto data : m_datas) {
        if (data.appId == appId) {
            data.selected = true;
            break;
        }
    }
}

QStringList AppChooserModel::choices()
{
    QStringList list;
    for (auto data : m_datas) {
        if (data.selected)
            list.append(data.appId);
    }
    return list;
}

void AppChooserModel::loadApplications()
{
    ItemInfo::registerMetaType();

#ifdef QT_DEBUG
    QDBusPendingCall call = QDBusInterface("org.desktopspec.ApplicationManager1", "/org/desktopspec/ApplicationManager1", "org.desktopspec.DBus.ObjectManager", QDBusConnection::sessionBus()).asyncCall(QString("GetManagedObjects"));
#else
    QDBusPendingCall call = QDBusInterface("org.desktopspec.ApplicationManager1", "/org/desktopspec/ApplicationManager1", "org.desktopspec.DBus.ObjectManager", QDBusConnection::sessionBus()).asyncCall(QString("GetManagedObjects"));
#endif
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher* call){
        QDBusPendingReply<ObjectMap> reply = *call;
        if (!reply.isError()) {
            m_datas.clear();
            const ObjectMap objectMap = reply.value();
            const auto language = getenv("LANGUAGE");
            for (auto obj = objectMap.begin(); obj != objectMap.end(); obj++)
            {
                const ObjectInterfaceMap interfaceMap = obj.value();
                const auto infoMap = interfaceMap.begin().value();
                DesktopInfo info;
                info.appId = infoMap["ID"].toString();
                const auto nameMap = infoMap["DisplayName"].value<QDBusArgument>();
                PropMap propName;
                nameMap >> propName;
                if (propName["Name"].contains(language)) {
                    info.name = propName["Name"][language];
                } else {
                    info.name = propName["Name"]["default"];
                }
                const auto iconMap = infoMap["Icons"].value<QDBusArgument>();
                PropMap propIcon;
                iconMap >> propIcon;
                info.icon = propIcon["default"]["default"];
                info.selected = false;
                beginInsertRows(QModelIndex(), m_datas.size(), m_datas.size());
                m_datas.append(info);
                endInsertRows();
            }
        }
        call->deleteLater();
    });
}

int AppChooserModel::rowCount(const QModelIndex &parent) const
{
    return m_datas.size();
}

QVariant AppChooserModel::data(const QModelIndex &index, int role) const
{
    const DesktopInfo &info = m_datas.at(index.row());
    switch (role) {
    case AppChooserModel::DataRole:
        return info.appId;
    case AppChooserModel::IconRole:
        return info.icon;
    case AppChooserModel::NameRole:
        return info.name;
    case AppChooserModel::SelectRole:
        return info.selected;
    default:
        return QVariant();
    }
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PropMap &dict) {
    argument.beginMap();
    while (!argument.atEnd()) {
        QString arg;
        QMap<QString, QString> argMap;
        argument.beginMapEntry();
        argument >> arg >> argMap;
        argument.endMapEntry();
        dict.insert(arg, argMap);
    }
    argument.endMap();
    return argument;
}

