// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "dbusobjectmanagerserver.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMetaType>

#include "kded_debug.h"

// QtDBus doesn't implement PropertiesChanged for some reason.
// This is meant to be childed' on an object to track that
// objects' property notifications.
class KDBusPropertiesChangedAdaptor : public QObject
{
    Q_OBJECT
public:
    KDBusPropertiesChangedAdaptor(const QString &objectPath, QObject *adaptee)
        : QObject(adaptee)
        , m_objectPath(objectPath)
    {
        auto mo = adaptee->metaObject();
        for (int i = 0; i < mo->propertyCount(); ++i) {
            QMetaProperty property = mo->property(i);
            if (!property.hasNotifySignal()) {
                continue;
            }
            const int fooIndex = metaObject()->indexOfMethod("onPropertyChanged()"); // of adaptor
            Q_ASSERT(fooIndex != -1);
            connect(adaptee, property.notifySignal(), this, metaObject()->method(fooIndex));
        }
    }

private Q_SLOTS:
    void onPropertyChanged()
    {
        if (!sender() || senderSignalIndex() == -1) {
            return;
        }
        auto mo = sender()->metaObject();
        for (int i = 0; i < mo->propertyCount(); ++i) {
            QMetaProperty property = mo->property(i);
            if (!property.hasNotifySignal()) {
                continue;
            }
            if (property.notifySignalIndex() != senderSignalIndex()) {
                continue;
            }
            const int ciid = mo->indexOfClassInfo("D-Bus Interface");
            if (ciid == -1) {
                continue;
            }
            QDBusMessage signal = QDBusMessage::createSignal(m_objectPath, //
                                                             QStringLiteral("org.freedesktop.DBus.Properties"),
                                                             QStringLiteral("PropertiesChanged"));
            signal << QLatin1String(mo->classInfo(ciid).value());
            signal << QVariantMap({{QString::fromLatin1(property.name()), property.read(sender())}}); // changed properties DICT<STRING,VARIANT>
            signal << QStringList(); // invalidated property names no clue what invalidation means
            QDBusConnection::sessionBus().send(signal);
        }
    }

private:
    const QString m_objectPath;
};

KDBusObjectManagerServer::KDBusObjectManagerServer(QObject *parent)
    : QObject(parent)
{
    registerTypes();
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerObject(m_path,
                                   this,
                                   QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties
                                       | QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllContents | QDBusConnection::ExportAdaptors)) {
        qCDebug(KDED) << "failed to register" << m_path;
        return;
    }
}

bool KDBusObjectManagerServer::serve(QObject *object)
{
    m_managedObjects << object;
    Q_EMIT InterfacesAdded(path(object), interfacePropertiesMap(object));
    connect(object, &QObject::destroyed, this, [this](QObject *obj) {
        unserve(obj);
    }); // auto-unserve
    const QString dbusPath = path(object).path();
    new KDBusPropertiesChangedAdaptor(dbusPath, object);
    QDBusConnection connection = QDBusConnection::sessionBus();
    return connection.registerObject(dbusPath,
                                     object,
                                     QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties
                                         | QDBusConnection::ExportAllInvokables | QDBusConnection::ExportAllContents | QDBusConnection::ExportAdaptors);
}

void KDBusObjectManagerServer::unserve(QObject *object)
{
    const QStringList interfaces = metaObjectsFor(object).keys();
    Q_EMIT InterfacesRemoved(path(object), {interfaces});
    QDBusConnection::sessionBus().unregisterObject(path(object).path());
    m_managedObjects.removeAll(object);
}

void KDBusObjectManagerServer::registerTypes()
{
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    qDBusRegisterMetaType<KDBusObjectManagerPropertiesMap>();
    qDBusRegisterMetaType<KDBusObjectManagerInterfacePropertiesMap>();
    qDBusRegisterMetaType<KDBusObjectManagerObjectPathInterfacePropertiesMap>();

    // For some reason we need to manually map the names. No idea why it works
    // for the maps though.
    qRegisterMetaType<KDBusObjectManagerInterfaceList>("KDBusObjectManagerInterfaceList");
    qDBusRegisterMetaType<KDBusObjectManagerInterfaceList>();
}

KDBusObjectManagerObjectPathInterfacePropertiesMap KDBusObjectManagerServer::GetManagedObjects()
{
    QMap<QDBusObjectPath, QMap<QString, QMap<QString, QVariant>>> map;
    for (const auto *object : m_managedObjects) {
        const QDBusObjectPath dbusPath = path(object);
        if (dbusPath.path().isEmpty()) {
            qCDebug(KDED) << "Invalid dbus path for" << object->objectName();
            continue;
        }
        map[dbusPath] = interfacePropertiesMap(object);
    }
    return map;
}

QDBusObjectPath KDBusObjectManagerServer::path(const QObject *object)
{
    const QString path = m_path + QLatin1String("/") + object->objectName();

    qCDebug(KDED) << "path for " << object->objectName() << object->metaObject()->className() << ":" << path;
    return QDBusObjectPath(path);
}

KDBusObjectManagerInterfacePropertiesMap KDBusObjectManagerServer::interfacePropertiesMap(const QObject *child)
{
    // Technically this is duplicating qdbus' internal GetAll implementation, so we could actually
    // shoot dbus queries to ourself and have Qt figure things out. All we need to do is
    // compose it into the dbuspath tree we manage.

    QMap<QString, QVariantMap> interfaceMap;

    const auto metaObjectHash = metaObjectsFor(child);
    for (auto it = metaObjectHash.cbegin(); it != metaObjectHash.cend(); ++it) {
        const QString interface = it.key();
        const QMetaObject *mo = it.value();
        QVariantMap properties;
        for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
            auto property = mo->property(i);
            properties[QLatin1String(property.name())] = property.read(child);
        }
        interfaceMap[interface] = properties;
    }

    return interfaceMap;
}

KDBusObjectManagerServer::InterfaceMetaObjectHash KDBusObjectManagerServer::metaObjectsFor(const QObject *object)
{
    InterfaceMetaObjectHash map;
    for (auto mo = object->metaObject(); mo; mo = mo->superClass()) {
        if (strcmp(mo->className(), "QObject") == 0) {
            continue;
        }

        int ciid = mo->indexOfClassInfo("D-Bus Interface");
        if (ciid == -1) {
            qCWarning(KDED) << mo->className() << "defines no interface";
            continue;
        }

        map[QString::fromLatin1(mo->classInfo(ciid).value())] = mo;
    }
    return map;
}

#include "dbusobjectmanagerserver.moc"

#include "moc_dbusobjectmanagerserver.cpp"
