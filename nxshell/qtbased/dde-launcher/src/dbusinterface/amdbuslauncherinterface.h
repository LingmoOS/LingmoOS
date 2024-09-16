// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMDBUSLAUNCHERINTERFACE_H_1456917835
#define AMDBUSLAUNCHERINTERFACE_H_1456917835

#include "categoryinfo.h"
#include "frequencyinfo.h"
#include "iteminfo.h"
#include "installedtimeinfo.h"

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#define INTERFACE_NAME "org.deepin.dde.daemon.Launcher1"
#define SERVICE_PATH "/org/deepin/dde/daemon/Launcher1"

/*
 * Proxy class for interface org.deepin.dde.daemon.Launcher1
 */
class AMDBusLauncherInter: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;

        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != staticInterfaceName())
            return;

        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        foreach(const QString &prop, changedProps.keys()) {
        const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                const QMetaProperty property = self->property(i);
                if (property.name() == prop) {
                    Q_EMIT property.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return INTERFACE_NAME; }

public:
    explicit AMDBusLauncherInter(QObject *parent = Q_NULLPTR);

    ~AMDBusLauncherInter();

    Q_PROPERTY(bool Fullscreen READ fullscreen NOTIFY FullscreenChanged)
    inline bool fullscreen() const
    { return qvariant_cast< bool >(property("Fullscreen")); }

    Q_PROPERTY(int DisplayMode READ displaymode NOTIFY DisplayModeChanged)
    inline int displaymode() const
    { return qvariant_cast< int >(property("DisplayMode")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<CategoryInfoList> GetAllCategoryInfos()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAllCategoryInfos"), argumentList);
    }

    inline QDBusPendingReply<FrequencyInfoList> GetAllFrequency()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAllFrequency"), argumentList);
    }

    inline QDBusPendingReply<ItemInfoList_v2> GetAllItemInfos()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAllItemInfos"), argumentList);
    }

    inline QDBusPendingReply<QStringList> GetAllNewInstalledApps()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAllNewInstalledApps"), argumentList);
    }

    inline QDBusPendingReply<InstalledTimeInfoList> GetAllTimeInstalled()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetAllTimeInstalled"), argumentList);
    }

    inline QDBusPendingReply<CategoryInfo> GetCategoryInfo(qlonglong in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetCategoryInfo"), argumentList);
    }

    inline QDBusPendingReply<ItemInfo_v2> GetItemInfo(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetItemInfo"), argumentList);
    }

    inline QDBusPendingReply<bool> IsItemOnDesktop(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsItemOnDesktop"), argumentList);
    }

    inline QDBusPendingReply<> MarkLaunched(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("MarkLaunched"), argumentList);
    }

    inline QDBusPendingReply<> RecordFrequency(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RecordFrequency"), argumentList);
    }

    inline QDBusPendingReply<> RecordRate(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RecordRate"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestRemoveFromDesktop(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RequestRemoveFromDesktop"), argumentList);
    }

    inline QDBusPendingReply<bool> RequestSendToDesktop(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RequestSendToDesktop"), argumentList);
    }

    inline QDBusPendingReply<> RequestUninstall(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("RequestUninstall"), argumentList);
    }

    inline QDBusPendingReply<> Search(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("Search"), argumentList);
    }

    inline QDBusPendingReply<> SetUseProxy(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetUseProxy"), argumentList);
    }

    inline QDBusPendingReply<bool> GetUseProxy(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetUseProxy"), argumentList);
    }

    inline QDBusPendingReply<> SetDisableScaling(const QString &in0, bool in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("SetDisableScaling"), argumentList);
    }

    inline QDBusPendingReply<bool> GetDisableScaling(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("GetDisableScaling"), argumentList);
    }

   Q_SIGNALS: // SIGNALS
       void ItemChanged(const QString &in0, ItemInfo_v2 in1, qlonglong in2);
       void NewAppLaunched(const QString &in0);
       void NewAppMarkedAsLaunched(const QString &in0);
       void RemoveFromDesktopFailed(const QString &in0, const QString &in1);
       void RemoveFromDesktopSuccess(const QString &in0);
       void SearchDone(const QStringList &in0);
       void SendToDesktopFailed(const QString &in0, const QString &in1);
       void SendToDesktopSuccess(const QString &in0);
       void UninstallFailed(const QString &in0, const QString &in1);
       void UninstallSuccess(const QString &in0);

       void FullscreenChanged();
       void DisplayModeChanged();
};

#endif
