// SPDX-FileCopyrightText: 2014 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONS_DBUS_ADAPTOR_H_1413533171
#define NOTIFICATIONS_DBUS_ADAPTOR_H_1413533171

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*!
 * \~chinese \class NotificationsDBusAdaptor
 * \~chinese \brief 继承于QDBusAbstractAdaptor,用于给外部提供通知中心的dbus接口
 */
class NotificationsDBusAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.Notifications")
public:
    explicit NotificationsDBusAdaptor(QObject *parent);
    virtual ~NotificationsDBusAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    /*!
     * \~chinese \name CloseNotification
     * \~chinese \brief 给外部提供一个根据通知ID关闭通知的接口
     * \~chinese \param in0:需要关闭的通知的ID号
     */
    void CloseNotification(uint in0);
    QStringList GetCapabilities();
    /*!
     * \~chinese \name GetServerInformation
     * \~chinese \brief 给外部提供一个获取服务信息的接口
     */
    QString GetServerInformation(QString &out1, QString &out2, QString &out3);
    /*!
     * \~chinese \name Notify
     * \~chinese \brief 给外部提供一个接口返回一个根据通知内容生成的通知ID
     * \~chinese \param in0:App名称; in1:ID; in2:App图标名称; in3:通知信息概要; in4:通知信息主体
     * \~chinese \param in5:行为信息; in6:提示信息 in7:多长时间超时过期,值为-1时不会超时
     */
    uint Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7);
Q_SIGNALS:
    void ActionInvoked(uint in0, const QString &in1);
    void NotificationClosed(uint in0, uint in1);
};

/*
 * Adaptor class for interface org.deepin.dde.Notification1
 */
class DDENotifyDBus: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.Notification1")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.deepin.dde.Notification1\">\n"
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
"    <method name=\"ReplaceBubble\">\n"
"      <arg direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"HandleBubbleEnd\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In2\"/>\n"
"      <arg direction=\"in\" type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In3\"/>\n"
"    </method>\n"
"    <method name=\"SetSystemInfo\">\n"
"      <arg direction=\"in\" type=\"u\"/>\n"
"      <arg direction=\"in\" type=\"v\"/>\n"
"    </method>\n"
"    <method name=\"setAppSetting\">\n"
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <signal name=\"ShowBubble\">\n"
"      <arg type=\"s\"/>\n"
"      <arg type=\"u\"/>\n"
"      <arg type=\"s\"/>\n"
"      <arg type=\"s\"/>\n"
"      <arg type=\"s\"/>\n"
"      <arg type=\"as\"/>\n"
"      <arg type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In6\"/>\n"
"      <arg type=\"i\"/>\n"
"      <arg type=\"a{sv}\"/>\n"
"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName.In8\"/>\n"
"    </signal>\n"
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
"    <signal name=\"recordCountChanged\">\n"
"      <arg type=\"u\"/>\n"
"    </signal>\n"
"    <property access=\"readwrite\" type=\"s\" name=\"allSetting\"/>\n"
"    <property access=\"readwrite\" type=\"s\" name=\"systemSetting\"/>\n"
"    <property access=\"read\" type=\"u\" name=\"recordCount\"/>\n"
"  </interface>\n"
        "")
public:
    explicit DDENotifyDBus(QObject *parent);
    virtual ~DDENotifyDBus();

public: // PROPERTIES
    Q_PROPERTY(QString allSetting READ allSetting WRITE setAllSetting)
    QString allSetting() const;
    void setAllSetting(const QString &value);

    Q_PROPERTY(QString systemSetting READ systemSetting WRITE setSystemSetting)
    QString systemSetting() const;
    void setSystemSetting(const QString &value);

    Q_PROPERTY(uint recordCount READ recordCount NOTIFY recordCountChanged)

public Q_SLOTS: // METHODS
    void ClearRecords();
    void CloseNotification(uint in0);
    QString GetAllRecords();
    QDBusVariant GetAppInfo(const QString &in0, uint in1);
    QStringList GetAppList();
    QStringList GetCapbilities();
    QString GetRecordById(const QString &in0);
    QString GetRecordsFromId(int in0, const QString &in1);
    QString GetServerInformation(QString &out1, QString &out2, QString &out3);
    QDBusVariant GetSystemInfo(uint in0);
    uint Notify(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7);
    void RemoveRecord(const QString &in0);
    void SetAppInfo(const QString &in0, uint in1, const QDBusVariant &in2);
    void SetSystemInfo(uint in0, const QDBusVariant &in1);
    void Toggle();
    void Show();
    void Hide();
    QString getAppSetting(const QString &in0);
    uint recordCount();
    void setAppSetting(const QString &in0);
    void HandleBubbleEnd(uint in0, uint in1, const QVariantMap &in2, const QVariantMap &in3);
    void ReplaceBubble(bool in0);
Q_SIGNALS: // SIGNALS
    void ActionInvoked(uint in0, const QString &in1);
    void AppAddedSignal(const QString &in0);
    void AppRemovedSignal(const QString &in0);
    void AppInfoChanged(const QString &in0, uint in1, const QDBusVariant &in2);
    void NotificationClosed(uint in0, uint in1);
    void RecordAdded(const QString &in0);
    void SystemInfoChanged(uint in0, const QDBusVariant &in2);
    void appAdded(const QString &in0);
    void appRemoved(const QString &in0);
    void appSettingChanged(const QString &in0);
    void systemSettingChanged(const QString &in0);
    void recordCountChanged(uint count);
    void ShowBubble(const QString &in0, uint in1, const QString &in2, const QString &in3, const QString &in4, const QStringList &in5, const QVariantMap &in6, int in7, const QVariantMap &in8);
};

#endif
