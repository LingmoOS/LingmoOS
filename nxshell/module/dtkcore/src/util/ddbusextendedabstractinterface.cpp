// SPDX-FileCopyrightText: 2015 Jolla Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "ddbusextendedpendingcallwatcher_p.h"

#include <DDBusExtendedAbstractInterface>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusPendingCall>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>

#include <QtCore/QDebug>
#include <QtCore/QMetaProperty>

DCORE_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, dBusInterface, ("org.freedesktop.DBus"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, dBusPropertiesInterface, ("org.freedesktop.DBus.Properties"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, dBusPropertiesChangedSignal, ("PropertiesChanged"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, propertyChangedSignature, ("propertyChanged(QString,QVariant)"))
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, propertyInvalidatedSignature, ("propertyInvalidated(QString)"))

DDBusExtendedAbstractInterface::DDBusExtendedAbstractInterface(
    const QString &service, const QString &path, const char *interface, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, interface, connection, parent)
    , m_sync(true)
    , m_useCache(false)
    , m_getAllPendingCallWatcher(0)
    , m_propertiesChangedConnected(false)
{
    const_cast<QDBusConnection &>(connection)
        .connect(QString("org.freedesktop.DBus"),
                 QString("/org/freedesktop/DBus"),
                 QString("org.freedesktop.DBus"),
                 QString("NameOwnerChanged"),
                 this,
                 SLOT(onDBusNameOwnerChanged(QString, QString, QString)));
}

DDBusExtendedAbstractInterface::~DDBusExtendedAbstractInterface() {}

void DDBusExtendedAbstractInterface::setSync(bool sync)
{
    setSync(sync, true);
}

/*
 * @~english
 * @note After sync is set to false, it will always return a empty value
 * if you call the property's get function directly. So you can only get it
 * through the changed signal when you get an property, and it's also a good
 * idea to save a cache yourself.
 */
void DDBusExtendedAbstractInterface::setSync(bool sync, bool autoStart)
{
    m_sync = sync;

    // init all properties
    if (autoStart && !m_sync && !isValid())
        startServiceProcess();
}

void DDBusExtendedAbstractInterface::getAllProperties()
{
    m_lastExtendedError = QDBusError();

    if (!isValid()) {
        QString errorMessage = QStringLiteral("This Extended DBus interface is not valid yet.");
        m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
        qDebug() << Q_FUNC_INFO << errorMessage;
        return;
    }

    if (!m_sync && m_getAllPendingCallWatcher) {
        // Call already in place, not repeating ...
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(), *dBusPropertiesInterface(), QStringLiteral("GetAll"));
    msg << interface();

    if (m_sync) {
        QDBusMessage reply = connection().call(msg);

        if (reply.type() != QDBusMessage::ReplyMessage) {
            m_lastExtendedError = QDBusError(reply);
            qWarning() << Q_FUNC_INFO << m_lastExtendedError.message();
            return;
        }

        if (reply.signature() != QLatin1String("a{sv}")) {
            QString errorMessage = QStringLiteral("Invalid signature \"%1\" in return from call to %2")
                                       .arg(reply.signature(), QString(*dBusPropertiesInterface()));
            qWarning() << Q_FUNC_INFO << errorMessage;
            m_lastExtendedError = QDBusError(QDBusError::InvalidSignature, errorMessage);
            return;
        }

        QVariantMap value = reply.arguments().at(0).toMap();
        onPropertiesChanged(interface(), value, QStringList());
    } else {
        QDBusPendingReply<QVariantMap> async = connection().asyncCall(msg);
        m_getAllPendingCallWatcher = new QDBusPendingCallWatcher(async, this);

        connect(m_getAllPendingCallWatcher,
                SIGNAL(finished(QDBusPendingCallWatcher *)),
                this,
                SLOT(onAsyncGetAllPropertiesFinished(QDBusPendingCallWatcher *)));
        return;
    }
}

void DDBusExtendedAbstractInterface::connectNotify(const QMetaMethod &signal)
{
    if (signal.methodType() == QMetaMethod::Signal && (signal.methodSignature() == *propertyChangedSignature() ||
                                                       signal.methodSignature() == *propertyInvalidatedSignature())) {
        if (!m_propertiesChangedConnected) {
            QStringList argumentMatch;
            argumentMatch << interface();
            connection().connect(service(),
                                 path(),
                                 *dBusPropertiesInterface(),
                                 *dBusPropertiesChangedSignal(),
                                 argumentMatch,
                                 QString(),
                                 this,
                                 SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

            m_propertiesChangedConnected = true;
            return;
        }
    } else {
        QDBusAbstractInterface::connectNotify(signal);
    }
}

void DDBusExtendedAbstractInterface::disconnectNotify(const QMetaMethod &signal)
{
    if (signal.methodType() == QMetaMethod::Signal && (signal.methodSignature() == *propertyChangedSignature() ||
                                                       signal.methodSignature() == *propertyInvalidatedSignature())) {
        if (m_propertiesChangedConnected && 0 == receivers(propertyChangedSignature()->constData()) &&
            0 == receivers(propertyInvalidatedSignature()->constData())) {
            QStringList argumentMatch;
            argumentMatch << interface();
            connection().disconnect(service(),
                                    path(),
                                    *dBusPropertiesInterface(),
                                    *dBusPropertiesChangedSignal(),
                                    argumentMatch,
                                    QString(),
                                    this,
                                    SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

            m_propertiesChangedConnected = false;
            return;
        }
    } else {
        QDBusAbstractInterface::disconnectNotify(signal);
    }
}

QVariant DDBusExtendedAbstractInterface::internalPropGet(const char *propname, void *propertyPtr)
{
    m_lastExtendedError = QDBusError();

    if (m_useCache) {
        int propertyIndex = metaObject()->indexOfProperty(propname);
        QMetaProperty metaProperty = metaObject()->property(propertyIndex);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        return QVariant{metaProperty.metaType(), propertyPtr};
#else
        return QVariant(metaProperty.userType(), propertyPtr);
#endif
    }

    if (m_sync) {
        QVariant ret = property(propname);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        ret.metaType().construct(propertyPtr, ret.constData());
#else
        QMetaType::construct(ret.userType(), propertyPtr, ret.constData());

#endif
        return ret;
    } else {
        if (!isValid()) {
            QString errorMessage = QStringLiteral("This Extended DBus interface is not valid yet.");
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qDebug() << Q_FUNC_INFO << errorMessage;
            return QVariant();
        }

        int propertyIndex = metaObject()->indexOfProperty(propname);

        if (-1 == propertyIndex) {
            QString errorMessage = QStringLiteral("Got unknown property \"%1\" to read").arg(QString::fromLatin1(propname));
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qWarning() << Q_FUNC_INFO << errorMessage;
            return QVariant();
        }

        QMetaProperty metaProperty = metaObject()->property(propertyIndex);

        if (!metaProperty.isReadable()) {
            QString errorMessage = QStringLiteral("Property \"%1\" is NOT readable").arg(QString::fromLatin1(propname));
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qWarning() << Q_FUNC_INFO << errorMessage;
            return QVariant();
        }

        // is this metatype registered?
        const char *expectedSignature = "";
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        if (metaProperty.typeId() != QMetaType::QVariant) {
            expectedSignature = QDBusMetaType::typeToSignature(metaProperty.metaType());
#else
        if (int(metaProperty.type()) != QMetaType::QVariant) {
            expectedSignature = QDBusMetaType::typeToSignature(metaProperty.userType());
#endif
            if (0 == expectedSignature) {
                QString errorMessage = QStringLiteral("Type %1 must be registered with Qt D-Bus "
                                                      "before it can be used to read property "
                                                      "%2.%3")
                                           .arg(metaProperty.typeName(), interface(), propname);
                m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
                qWarning() << Q_FUNC_INFO << errorMessage;
                return QVariant();
            }
        }

        asyncProperty(propname);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        return QVariant{metaProperty.metaType(), propertyPtr};
#else
        return QVariant(metaProperty.userType(), propertyPtr);
#endif
    }
}

void DDBusExtendedAbstractInterface::internalPropSet(const char *propname, const QVariant &value, void *propertyPtr)
{
    m_lastExtendedError = QDBusError();

    if (m_sync) {
        setProperty(propname, value);
    } else {
        if (!isValid()) {
            QString errorMessage = QStringLiteral("This interface is not yet valid");
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qDebug() << Q_FUNC_INFO << errorMessage;
            return;
        }

        int propertyIndex = metaObject()->indexOfProperty(propname);

        if (-1 == propertyIndex) {
            QString errorMessage = QStringLiteral("Got unknown property \"%1\" to write").arg(QString::fromLatin1(propname));
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qWarning() << Q_FUNC_INFO << errorMessage;
            return;
        }

        QMetaProperty metaProperty = metaObject()->property(propertyIndex);

        if (!metaProperty.isWritable()) {
            QString errorMessage = QStringLiteral("Property \"%1\" is NOT writable").arg(QString::fromLatin1(propname));
            m_lastExtendedError = QDBusMessage::createError(QDBusError::Failed, errorMessage);
            qWarning() << Q_FUNC_INFO << errorMessage;
            return;
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QVariant variant = QVariant{metaProperty.metaType(), propertyPtr};
#else
        QVariant variant = QVariant(metaProperty.type(), propertyPtr);
#endif
        variant = value;

        asyncSetProperty(propname, variant);
    }
}

QVariant DDBusExtendedAbstractInterface::asyncProperty(const QString &propertyName)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(), *dBusPropertiesInterface(), QStringLiteral("Get"));
    msg << interface() << propertyName;
    QDBusPendingReply<QVariant> async = connection().asyncCall(msg);
    DDBusExtendedPendingCallWatcher *watcher = new DDBusExtendedPendingCallWatcher(async, propertyName, QVariant(), this);

    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(onAsyncPropertyFinished(QDBusPendingCallWatcher *)));

    return QVariant();
}

void DDBusExtendedAbstractInterface::asyncSetProperty(const QString &propertyName, const QVariant &value)
{
    QDBusMessage msg = QDBusMessage::createMethodCall(service(), path(), *dBusPropertiesInterface(), QStringLiteral("Set"));

    msg << interface() << propertyName << QVariant::fromValue(QDBusVariant(value));
    QDBusPendingReply<> async = connection().asyncCall(msg);
    DDBusExtendedPendingCallWatcher *watcher = new DDBusExtendedPendingCallWatcher(async, propertyName, value, this);

    connect(
        watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, SLOT(onAsyncSetPropertyFinished(QDBusPendingCallWatcher *)));
}

void DDBusExtendedAbstractInterface::startServiceProcess()
{
    const QString &servName = service();

    if (isValid()) {
        qWarning() << "Service" << servName << "is already started.";
        return;
    }

    QDBusMessage msg =
        QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", *dBusInterface(), QStringLiteral("StartServiceByName"));
    msg << servName << quint32(0);
    QDBusPendingReply<quint32> async = connection().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, &DDBusExtendedAbstractInterface::onStartServiceProcessFinished);
}

void DDBusExtendedAbstractInterface::onStartServiceProcessFinished(QDBusPendingCallWatcher *w)
{
    if (w->isError()) {
        m_lastExtendedError = w->error();
    } else {
        m_lastExtendedError = QDBusError();
    }

    QDBusPendingReply<quint32> reply = *w;

    Q_EMIT serviceStartFinished(reply.value());

    w->deleteLater();
}

void DDBusExtendedAbstractInterface::onAsyncPropertyFinished(QDBusPendingCallWatcher *w)
{
    DDBusExtendedPendingCallWatcher *watcher = qobject_cast<DDBusExtendedPendingCallWatcher *>(w);
    Q_ASSERT(watcher);

    QDBusPendingReply<QVariant> reply = *watcher;

    if (reply.isError()) {
        m_lastExtendedError = reply.error();
    } else {
        int propertyIndex = metaObject()->indexOfProperty(watcher->asyncProperty().toLatin1().constData());
        QVariant value = demarshall(interface(), metaObject()->property(propertyIndex), reply.value(), &m_lastExtendedError);

        if (m_lastExtendedError.isValid()) {
            Q_EMIT propertyInvalidated(watcher->asyncProperty());
        } else {
            Q_EMIT propertyChanged(watcher->asyncProperty(), value);
        }
    }

    Q_EMIT asyncPropertyFinished(watcher->asyncProperty());
    watcher->deleteLater();
}

void DDBusExtendedAbstractInterface::onAsyncSetPropertyFinished(QDBusPendingCallWatcher *w)
{
    DDBusExtendedPendingCallWatcher *watcher = qobject_cast<DDBusExtendedPendingCallWatcher *>(w);
    Q_ASSERT(watcher);

    QDBusPendingReply<> reply = *watcher;

    if (reply.isError()) {
        m_lastExtendedError = reply.error();
    } else {
        m_lastExtendedError = QDBusError();
    }

    Q_EMIT asyncSetPropertyFinished(watcher->asyncProperty());

    // Resetting the property to its previous value after sending the
    // finished signal
    if (!reply.isError()) {
        m_lastExtendedError = QDBusError();
        Q_EMIT propertyChanged(watcher->asyncProperty(), watcher->previousValue());
    }

    watcher->deleteLater();
}

void DDBusExtendedAbstractInterface::onAsyncGetAllPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    m_getAllPendingCallWatcher = 0;

    QDBusPendingReply<QVariantMap> reply = *watcher;

    if (reply.isError()) {
        m_lastExtendedError = reply.error();
    } else {
        m_lastExtendedError = QDBusError();
    }

    Q_EMIT asyncGetAllPropertiesFinished();

    if (!reply.isError()) {
        onPropertiesChanged(interface(), reply.value(), QStringList());
    }

    watcher->deleteLater();
}

void DDBusExtendedAbstractInterface::onPropertiesChanged(const QString &interfaceName,
                                                         const QVariantMap &changedProperties,
                                                         const QStringList &invalidatedProperties)
{
    if (interfaceName == interface()) {
        QVariantMap::const_iterator i = changedProperties.constBegin();
        while (i != changedProperties.constEnd()) {
            int propertyIndex = metaObject()->indexOfProperty(i.key().toLatin1().constData());

            if (-1 == propertyIndex) {
                qDebug() << Q_FUNC_INFO << "Got unknown changed property" << i.key();
            } else {
                QVariant value = demarshall(interface(), metaObject()->property(propertyIndex), i.value(), &m_lastExtendedError);

                if (m_lastExtendedError.isValid()) {
                    Q_EMIT propertyInvalidated(i.key());
                } else {
                    Q_EMIT propertyChanged(i.key(), value);
                }
            }

            ++i;
        }

        QStringList::const_iterator j = invalidatedProperties.constBegin();
        while (j != invalidatedProperties.constEnd()) {
            if (-1 == metaObject()->indexOfProperty(j->toLatin1().constData())) {
                qDebug() << Q_FUNC_INFO << "Got unknown invalidated property" << *j;
            } else {
                m_lastExtendedError = QDBusError();
                Q_EMIT propertyInvalidated(*j);
            }

            ++j;
        }
    }
}

void DDBusExtendedAbstractInterface::onDBusNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    if (name == service() && oldOwner.isEmpty()) {
        m_dbusOwner = newOwner;
        Q_EMIT serviceValidChanged(true);
    } else if (name == m_dbusOwner && newOwner.isEmpty()) {
        m_dbusOwner.clear();
        Q_EMIT serviceValidChanged(false);
    }
}

QVariant DDBusExtendedAbstractInterface::demarshall(const QString &interface,
                                                    const QMetaProperty &metaProperty,
                                                    const QVariant &value,
                                                    QDBusError *error)
{
    Q_ASSERT(metaProperty.isValid());
    Q_ASSERT(error != 0);

    if (value.userType() == metaProperty.userType()) {
        // No need demarshalling. Passing back straight away ...
        *error = QDBusError();
        return value;
    }

    QString errorMessage;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QVariant result = QVariant{metaProperty.metaType()};
    const char *expectedSignature = QDBusMetaType::typeToSignature(metaProperty.metaType());

#else
    QVariant result = QVariant(metaProperty.userType(), (void *)0);
    const char *expectedSignature = QDBusMetaType::typeToSignature(metaProperty.userType());
#endif
    if (value.userType() == qMetaTypeId<QDBusArgument>()) {
        // demarshalling a DBus argument ...
        QDBusArgument dbusArg = value.value<QDBusArgument>();

        if (expectedSignature == dbusArg.currentSignature().toLatin1()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            QDBusMetaType::demarshall(dbusArg, metaProperty.metaType(), result.data());
#else
            QDBusMetaType::demarshall(dbusArg, metaProperty.userType(), result.data());
#endif
            if (!result.isValid()) {
                errorMessage = QStringLiteral("Unexpected failure demarshalling "
                                              "upon PropertiesChanged signal arrival "
                                              "for property `%3.%4' (expected type `%5' (%6))")
                                   .arg(interface,
                                        QString::fromLatin1(metaProperty.name()),
                                        QString::fromLatin1(metaProperty.typeName()),
                                        expectedSignature);
            }
        } else {
            errorMessage = QStringLiteral("Unexpected `user type' (%2) "
                                          "upon PropertiesChanged signal arrival "
                                          "for property `%3.%4' (expected type `%5' (%6))")
                               .arg(dbusArg.currentSignature(),
                                    interface,
                                    QString::fromLatin1(metaProperty.name()),
                                    QString::fromLatin1(metaProperty.typeName()),
                                    QString::fromLatin1(expectedSignature));
        }
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const char *actualSignature = QDBusMetaType::typeToSignature(value.metaType());
#else
        const char *actualSignature = QDBusMetaType::typeToSignature(value.userType());
#endif

        errorMessage = QStringLiteral("Unexpected `%1' (%2) "
                                      "upon PropertiesChanged signal arrival "
                                      "for property `%3.%4' (expected type `%5' (%6))")
                           .arg(QString::fromLatin1(value.typeName()),
                                QString::fromLatin1(actualSignature),
                                interface,
                                QString::fromLatin1(metaProperty.name()),
                                QString::fromLatin1(metaProperty.typeName()),
                                QString::fromLatin1(expectedSignature));
    }

    if (errorMessage.isEmpty()) {
        *error = QDBusError();
    } else {
        *error = QDBusMessage::createError(QDBusError::InvalidSignature, errorMessage);
        qDebug() << Q_FUNC_INFO << errorMessage;
    }

    return result;
}
DCORE_END_NAMESPACE
