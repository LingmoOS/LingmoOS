// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDBusObjectPath>
#include "dnotificationtypes.h"

DNOTIFICATIONS_USE_NAMESPACE

class FakeNotificationManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.FakeNotification")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.dde.FakeNotification\">\n"
                "    <method name=\"CloseNotification\">\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "    </method>\n"
                "    <method name=\"GetCapbilities\">\n"
                "      <arg direction=\"out\" type=\"as\"/>\n"
                "    </method>\n"
                "    <method name=\"GetServerInformation\">\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"Notify\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"as\"/>\n"
                "      <arg direction=\"in\" type=\"a{sv}\"/>\n"
                "      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In6\"/>\n"
                "      <arg direction=\"in\" type=\"i\"/>\n"
                "      <arg direction=\"out\" type=\"u\"/>\n"
                "    </method>\n"
                "    <method name=\"GetAllRecords\">\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"GetRecordById\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"GetRecordsFromId\">\n"
                "      <arg direction=\"in\" type=\"i\"/>\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"RemoveRecord\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"ClearRecords\"/>\n"
                "    <method name=\"getAppSetting\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"out\" type=\"s\"/>\n"
                "    </method>\n"
                "    <method name=\"Toggle\"/>\n"
                "    <method name=\"Show\"/>\n"
                "    <method name=\"Hide\"/>\n"
                "    <method name=\"recordCount\">\n"
                "      <arg direction=\"out\" type=\"u\"/>\n"
                "    </method>\n"
                "    <method name=\"GetAppList\">\n"
                "      <arg direction=\"out\" type=\"as\"/>\n"
                "    </method>\n"
                "    <method name=\"GetAppInfo\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "      <arg direction=\"out\" type=\"v\"/>\n"
                "    </method>\n"
                "    <method name=\"GetSystemInfo\">\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "      <arg direction=\"out\" type=\"v\"/>\n"
                "    </method>\n"
                "    <method name=\"SetAppInfo\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "      <arg direction=\"in\" type=\"v\"/>\n"
                "    </method>\n"
                "    <method name=\"SetSystemInfo\">\n"
                "      <arg direction=\"in\" type=\"u\"/>\n"
                "      <arg direction=\"in\" type=\"v\"/>\n"
                "    </method>\n"
                "    <method name=\"setAppSetting\">\n"
                "      <arg direction=\"in\" type=\"s\"/>\n"
                "    </method>\n"
                "    <signal name=\"NotificationClosed\">\n"
                "      <arg type=\"u\"/>\n"
                "      <arg type=\"u\"/>\n"
                "    </signal>\n"
                "    <signal name=\"ActionInvoked\">\n"
                "      <arg type=\"u\"/>\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"RecordAdded\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"AppInfoChanged\">\n"
                "      <arg type=\"s\"/>\n"
                "      <arg type=\"u\"/>\n"
                "      <arg type=\"v\"/>\n"
                "    </signal>\n"
                "    <signal name=\"SystemInfoChanged\">\n"
                "      <arg type=\"u\"/>\n"
                "      <arg type=\"v\"/>\n"
                "    </signal>\n"
                "    <signal name=\"AppAddedSignal\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"AppRemovedSignal\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"appRemoved\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"appAdded\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"appSettingChanged\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <signal name=\"systemSettingChanged\">\n"
                "      <arg type=\"s\"/>\n"
                "    </signal>\n"
                "    <property access=\"readwrite\" type=\"s\" name=\"allSetting\"/>\n"
                "    <property access=\"readwrite\" type=\"s\" name=\"systemSetting\"/>\n"
                "  </interface>\n"
                "")

public:
    explicit FakeNotificationManagerService(QObject *parent = nullptr);
    virtual ~FakeNotificationManagerService();

    bool m_clearRecords{false}, m_closeNotification{false};
    QDBusVariant m_appInfo{QString("app")}, m_sysInfo{QString("sys")};
    QStringList m_appList{"dde-control-center", "dde-dock"};
    QStringList m_capabilities{"action-icons", "actions", "body", "body-hyperlinks", "body-markup"};
    QString m_getAllRecords{"test"}, m_record{"test"}, m_records{"test0, test1"}, m_serverInfo{"deepin"};
    uint m_id{110}, m_recordCount{10};
    bool m_removeRecord{false}, m_toggle{false}, m_show{false}, m_hide{false};
    QString m_appName{"appName"}, m_appIcon{"appIcon"};
    bool m_enableNotification{false}, m_enablePreview{false}, m_enableSound{false};
    bool m_showInNotification{false}, m_lockScreenShowNotification{false};
    QString m_startTime{"10:00"}, m_endTime{"24:00"};
    bool m_enable{false}, m_DNDModeInLockScreenEnabled{false}, m_showInNotificationCenterEnabled{false};
    bool m_openByTimeIntervalEnabled{false}, m_showIconEnabled{false};
public slots:
    Q_SCRIPTABLE void ClearRecords()
    {
        m_clearRecords = true;
    }

    Q_SCRIPTABLE void CloseNotification(uint in0)
    {
        Q_UNUSED(in0)
        m_closeNotification = true;
    }

    Q_SCRIPTABLE QString GetAllRecords()
    {
        return m_getAllRecords;
    }

    Q_SCRIPTABLE QDBusVariant GetAppInfo(const QString &in0, uint in1)
    {
        Q_UNUSED(in0)
        if (in1 == AppName) {
            return QDBusVariant{m_appName};
        } else if (in1 == AppIcon) {
            return QDBusVariant{m_appIcon};
        } else if (in1 == EnableNotification) {
            return QDBusVariant{m_enableNotification};
        } else if (in1 == EnablePreview) {
            return QDBusVariant{m_enablePreview};
        } else if (in1 == EnableSound) {
            return QDBusVariant{m_enableSound};
        } else if (in1 == ShowInNotificationCenter) {
            return QDBusVariant{m_showInNotification};
        } else if (in1 == LockScreenShowNofitication) {
            return QDBusVariant{m_lockScreenShowNotification};
        }
        return m_appInfo;
    }

    Q_SCRIPTABLE void SetAppInfo(const QString &, uint in1, const QDBusVariant & value)
    {
        if (in1 == AppName) {
            m_appName = value.variant().toString();
        } else if (in1 == AppIcon) {
            m_appIcon = value.variant().toString();
        } else if (in1 == EnableNotification) {
            m_enableNotification = value.variant().toBool();
        } else if (in1 == EnablePreview) {
            m_enablePreview = value.variant().toBool();
        } else if (in1 == EnableSound) {
            m_enableSound = value.variant().toBool();
        } else if (in1 == ShowInNotificationCenter) {
            m_showInNotification = value.variant().toBool();
        } else if (in1 == LockScreenShowNofitication) {
            m_lockScreenShowNotification = value.variant().toBool();
        }
        m_appInfo = QDBusVariant{"appInfoChanged"};
    }

    Q_SCRIPTABLE QStringList GetAppList()
    {
        return m_appList;
    }

    Q_SCRIPTABLE QStringList GetCapbilities()
    {
        return m_capabilities;
    }

    Q_SCRIPTABLE QString GetRecordById(const QString &in0)
    {
        Q_UNUSED(in0)
        return m_record;
    }

    Q_SCRIPTABLE QString GetRecordsFromId(int in0, const QString &in1)
    {
        Q_UNUSED(in0)
        Q_UNUSED(in1)
        return m_records;
    }

    Q_SCRIPTABLE QString GetServerInformation(QString &out1, QString &out2, QString &out3)
    {
        Q_UNUSED(out1)
        Q_UNUSED(out2)
        Q_UNUSED(out3)
        return m_serverInfo;
    }

    Q_SCRIPTABLE QDBusVariant GetSystemInfo(uint in0)
    {
        if (in0 == DNDMode) {
            return QDBusVariant{m_enable};
        } else if (in0 == LockScreenOpenDNDMode) {
            return QDBusVariant{m_DNDModeInLockScreenEnabled};
        } else if (in0 == OpenByTimeInterval) {
            return QDBusVariant{m_openByTimeIntervalEnabled};
        } else if (in0 == StartTime) {
            return QDBusVariant{m_startTime};
        } else if (in0 == EndTime) {
            return QDBusVariant{m_endTime};
        } else if (in0 == ShowIcon) {
            return QDBusVariant{m_showIconEnabled};
        }
        return m_sysInfo;
    }

    Q_SCRIPTABLE void SetSystemInfo(uint in0, const QDBusVariant &in1)
    {
        if (in0 == DNDMode) {
            m_enable = in1.variant().toBool();
        } else if (in0 == LockScreenOpenDNDMode) {
            m_DNDModeInLockScreenEnabled = in1.variant().toBool();
        } else if (in0 == OpenByTimeInterval) {
            m_openByTimeIntervalEnabled = in1.variant().toBool();
        } else if (in0 == StartTime) {
            m_startTime = in1.variant().toString();
        } else if (in0 == EndTime) {
            m_endTime = in1.variant().toString();
        } else if (in0 == ShowIcon) {
            m_showIconEnabled = in1.variant().toBool();
        }
        m_sysInfo = QDBusVariant{"sysInfoChanged"};
    }

    Q_SCRIPTABLE uint Notify(const QString &, uint, const QString &, const QString &, const QString &, const QStringList &, const QVariantMap &, int)
    {
        return m_id;
    }

    Q_SCRIPTABLE void RemoveRecord(const QString &in0)
    {
        Q_UNUSED(in0)
        m_removeRecord = true;
    }

    Q_SCRIPTABLE void Toggle()
    {
        m_toggle = true;
    }

    Q_SCRIPTABLE void Show()
    {
        m_show = true;
    }

    Q_SCRIPTABLE void Hide()
    {
        m_hide = true;
    }

    Q_SCRIPTABLE uint recordCount()
    {
        return m_recordCount;
    }

private:
    void registerService();
    void unregisterService();
};
