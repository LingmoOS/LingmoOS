// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCQUICKDBUSINTERFACE_P_H
#define DCCQUICKDBUSINTERFACE_P_H
#include "oceanuiquickdbusinterface.h"

#include <QDBusConnection>
#include <QJSValue>
#include <QObject>

namespace oceanuiV25 {
class DBusPropertySignalSpy;

class OceanUIQuickDBusCallback : public QObject
{
    Q_OBJECT
public:
    explicit OceanUIQuickDBusCallback(QJSValue member, QJSValue errorSlot, QObject *parent);
    ~OceanUIQuickDBusCallback() override;

    static QVariant toValue(const QVariant &value);

public Q_SLOTS:
    void returnMethod(const QDBusMessage &msg);
    void errorMethod(const QDBusError &error, const QDBusMessage &msg);

protected:
    QJSValue m_member;
    QJSValue m_errorSlot;
};

class OceanUIDBusSignalCallback : public QObject
{
    Q_OBJECT
public:
    explicit OceanUIDBusSignalCallback(const QMetaMethod &method, QObject *parent);
    ~OceanUIDBusSignalCallback() override;
public Q_SLOTS:
    void returnMethod(const QDBusMessage &msg);

protected:
    QMetaMethod m_method;
    QObject *m_target;
};

class OceanUIQuickDBusInterface::Private : public QObject
{
    Q_OBJECT
public:
    explicit Private(OceanUIQuickDBusInterface *q);
    ~Private() override;

    void getAllPropertys();

private Q_SLOTS:
    void onAllProperties(const QVariantMap &changedProperties);
    void onGetPropertiesErr(const QDBusError &e);
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    QVariant getProperty(const QString &propname);
    void setProperty(const QString &propname, const QVariant &value);
    void onPropertySpy(const char *propname, const QVariant &value);

public:
    OceanUIQuickDBusInterface *q_ptr;
    DBusPropertySignalSpy *m_propertySpy;
    QString m_service;
    QString m_path;
    QString m_interface;
    QMap<QString, QString> m_mapProperties; // 保存属性名对照表，qml要求属性名首字母小写
    QString m_suffix;
    BusType m_connectionType;
    QDBusConnection m_connection;
    QVariantMap m_propertyMap; // DBus属性值

    friend class OceanUIQuickDBusCallback;
};
} // namespace oceanuiV25
#endif // DCCQUICKDBUSINTERFACE_P_H
