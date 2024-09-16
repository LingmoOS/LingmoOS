// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dccdbusinterface.h"
#include "dccdbusinterface_p.h"

#include <QMetaObject>
#include <qmetaobject.h>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QDebug>

const QString DBusService = QStringLiteral("org.freedesktop.DBus");
const QString DBusPath = QStringLiteral("/org/freedesktop/DBus");
const QString DBusInterface = QStringLiteral("org.freedesktop.DBus");
const QString NameOwnerChanged = QStringLiteral("NameOwnerChanged");

const QString PropertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");
const QString PropertiesChanged = QStringLiteral("PropertiesChanged");
const static char *PropertyName = "propname";

using namespace dde::network;

DCCDBusInterfacePrivate::DCCDBusInterfacePrivate(DCCDBusInterface *interface, QObject *parent)
    : QObject(interface)
    , m_parent(parent)
    , q_ptr(interface)
{
    QDBusMessage message = QDBusMessage::createMethodCall(DBusService, DBusPath, DBusInterface, "NameHasOwner");
    message << interface->service();
    interface->connection().callWithCallback(message, this, SLOT(onDBusNameHasOwner(bool)));

    QStringList argumentMatch;
    argumentMatch << interface->interface();
    interface->connection().connect(interface->service(), interface->path(), PropertiesInterface, PropertiesChanged, argumentMatch, QString(), this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

QVariant DCCDBusInterfacePrivate::demarshall(const QMetaProperty &metaProperty, const QVariant &value)
{
    if (value.userType() == metaProperty.userType())
        return value;

    QVariant result = QVariant(metaProperty.userType(), nullptr);
    if (value.userType() == qMetaTypeId<QDBusArgument>()) {
        QDBusArgument dbusArg = value.value<QDBusArgument>();
        QDBusMetaType::demarshall(dbusArg, metaProperty.userType(), result.data());
    }

    return result;
}

QVariant DCCDBusInterfacePrivate::updateProp(const char *propname, const QVariant &value)
{
    QVariant result;
    const QMetaObject *metaObj = m_parent->metaObject();
    int i = metaObj->indexOfProperty(propname);
    if (i != -1) {
        QMetaProperty metaProperty = metaObj->property(i);
        result = demarshall(metaProperty, value);
        m_propertyMap.insert(propname, result);
        QMetaObject::invokeMethod(m_parent, propname + QString("Changed").toLatin1(), Qt::DirectConnection, QGenericArgument(result.typeName(), result.data()));
    } else
        qInfo() << "invalid property changed:" << propname << value;

    return result;
}

void DCCDBusInterfacePrivate::initDBusConnection()
{
    Q_Q(DCCDBusInterface);
    QDBusConnection connection = q->connection();
    QStringList signalList;
    QDBusInterface inter(q->service(), q->path(), q->interface(), connection);
    const QMetaObject *meta = inter.metaObject();
    for (int i = meta->methodOffset(); i < meta->methodCount(); ++i) {
        const QMetaMethod &method = meta->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            signalList << method.methodSignature();
        }
    }
    const QMetaObject *parentMeta = m_parent->metaObject();
    for (const QString &signal : signalList) {
        int i = parentMeta->indexOfSignal(QMetaObject::normalizedSignature(signal.toLatin1()));
        if (i != -1) {
            const QMetaMethod &parentMethod = parentMeta->method(i);
            connection.connect(q->service(), q->path(), q->interface(), parentMethod.name(), m_parent, QT_STRINGIFY(QSIGNAL_CODE) + parentMethod.methodSignature());
        }
    }
}

void DCCDBusInterfacePrivate::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName)
    Q_UNUSED(invalidatedProperties)
    for (QVariantMap::const_iterator it = changedProperties.cbegin(); it != changedProperties.cend(); ++it)
        updateProp((it.key() + m_suffix).toLatin1(), it.value());
}

void DCCDBusInterfacePrivate::onAsyncPropertyFinished(QDBusPendingCallWatcher *w)
{
    QDBusPendingReply<QVariant> reply = *w;
    if (!reply.isError()) {
        updateProp(w->property(PropertyName).toString().toLatin1(), reply.value());
    }
    w->deleteLater();
}

void DCCDBusInterfacePrivate::onDBusNameHasOwner(bool valid)
{
    Q_Q(DCCDBusInterface);
    if (valid)
        initDBusConnection();
    else
        q->connection().connect(DBusService, DBusPath, DBusInterface, NameOwnerChanged, this, SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
}

void DCCDBusInterfacePrivate::onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOWner)
{
    Q_Q(DCCDBusInterface);
    if (name == q->service() && oldOwner.isEmpty()) {
        initDBusConnection();
        q->connection().disconnect(DBusService, DBusPath, DBusInterface, NameOwnerChanged, this, SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
    }
}
//////////////////////////////////////////////////////////
DCCDBusInterface::DCCDBusInterface(const QString &service, const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, interface.toLatin1(), connection, parent)
    , d_ptr(new DCCDBusInterfacePrivate(this, parent))
{
}

DCCDBusInterface::~DCCDBusInterface()
{
}

QString DCCDBusInterface::suffix() const
{
    Q_D(const DCCDBusInterface);
    return d->m_suffix;
}

void DCCDBusInterface::setSuffix(const QString &suffix)
{
    Q_D(DCCDBusInterface);
    d->m_suffix = suffix;
}

inline QString originalPropname(const char *propname, QString suffix)
{
    QString propStr(propname);
    return propStr.left(propStr.length() - suffix.length());
}

QVariant DCCDBusInterface::property(const char *propname)
{
    Q_D(DCCDBusInterface);
    if (d->m_propertyMap.contains(propname))
        return d->m_propertyMap.value(propname);

    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(), PropertiesInterface, QStringLiteral("Get"));
    msg << interface() << originalPropname(propname, d->m_suffix);
    QDBusPendingReply<QVariant> prop = connection().asyncCall(msg);
    if (prop.value().isValid())
        return d->updateProp(propname, prop.value());

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(prop, this);
    watcher->setProperty(PropertyName, propname);
    connect(watcher, &QDBusPendingCallWatcher::finished, d, &DCCDBusInterfacePrivate::onAsyncPropertyFinished);
    if (d->m_propertyMap.contains(propname))
        return d->m_propertyMap.value(propname);

    return QVariant();
}

void DCCDBusInterface::setProperty(const char *propname, const QVariant &value)
{
    Q_D(const DCCDBusInterface);
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(), PropertiesInterface, QStringLiteral("Set"));
    msg << interface() << originalPropname(propname, d->m_suffix) << value;
    connection().asyncCall(msg);
}
