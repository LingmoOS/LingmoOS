/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "devicemanager.h"
#include "adapterinterface.h"
#include "deviceinterface.h"
#include "gattserviceinterface.h"
#include "gattcharacteristicinterface.h"
#include "gattdescriptorinterface.h"
#include "gattmanagerinterface.h"
#include "leadvertisingmanagerinterface.h"
#include "mediainterface.h"
#include "mediatransportinterface.h"
#include "objectmanager.h"

DeviceManager::DeviceManager(ObjectManager *parent)
    : QObject(parent)
    , m_objectManager(parent)
{
}

void DeviceManager::runAction(const QString &actionName, const QVariantMap &properties)
{
    if (actionName == QLatin1String("create-adapter")) {
        runCreateAdapterAction(properties);
    } else if (actionName == QLatin1String("create-device")) {
        runCreateDeviceAction(properties);
    } else if (actionName == QLatin1String("create-gatt-service")) {
        runCreateGattServiceAction(properties);
    } else if (actionName == QLatin1String("create-gatt-characteristic")) {
        runCreateGattCharacteristicAction(properties);
    } else if (actionName == QLatin1String("create-gatt-descriptor")) {
        runCreateGattDescriptorAction(properties);
    } else if (actionName == QLatin1String("remove-adapter")) {
        runRemoveAdapterAction(properties);
    } else if (actionName == QLatin1String("remove-device")) {
        runRemoveDeviceAction(properties);
    } else if (actionName == QLatin1String("remove-gatt-service")) {
        runRemoveGattServiceAction(properties);
    } else if (actionName == QLatin1String("remove-gatt-characteristic")) {
        runRemoveGattCharacteristicAction(properties);
    } else if (actionName == QLatin1String("remove-gatt-descriptor")) {
        runRemoveGattDescriptorAction(properties);
    } else if (actionName == QLatin1String("change-adapter-property")) {
        runChangeAdapterProperty(properties);
    } else if (actionName == QLatin1String("change-device-property")) {
        runChangeDeviceProperty(properties);
    } else if (actionName.startsWith(QLatin1String("adapter-media:"))) {
        runAdapterMediaAction(actionName.mid(14), properties);
    } else if (actionName.startsWith(QLatin1String("adapter-leadvertisingmanager:"))) {
        runAdapterLeAdvertisingManagerAction(actionName.mid(29), properties);
    } else if (actionName.startsWith(QLatin1String("adapter-gattmanager:"))) {
        runAdapterGattManagerAction(actionName.mid(20), properties);
    } else if (actionName == QLatin1String("bug377405")) {
        runBug377405();
    } else if (actionName == QLatin1String("bug403289")) {
        runBug403289(properties);
    }
}

void DeviceManager::runCreateAdapterAction(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    QVariantMap props = properties;
    props.remove(QStringLiteral("Path"));

    AdapterObject *adapterObj = new AdapterObject(path);
    AdapterInterface *adapter = new AdapterInterface(path, props, adapterObj);
    m_objectManager->addObject(adapter);
    m_objectManager->addAutoDeleteObject(adapterObj);
}

void DeviceManager::runCreateDeviceAction(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    QVariantMap props = properties;
    props.remove(QStringLiteral("Path"));

    DeviceObject *deviceObj = new DeviceObject(path);
    DeviceInterface *device = new DeviceInterface(path, props, deviceObj);
    m_objectManager->addObject(device);
    m_objectManager->addAutoDeleteObject(deviceObj);
}

void DeviceManager::runCreateGattServiceAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    QVariantMap props = properties;
    props.remove(QStringLiteral("Path"));

    GattServiceObject *serviceObj = new GattServiceObject(path);
    GattServiceInterface *service = new GattServiceInterface(path, props, serviceObj);
    m_objectManager->addObject(service);
    m_objectManager->addAutoDeleteObject(serviceObj);
}

void DeviceManager::runCreateGattCharacteristicAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    QVariantMap props = properties;
    props.remove(QStringLiteral("Path"));

    GattCharacteristicObject *characteristicObj = new GattCharacteristicObject(path);
    GattCharacteristicInterface *characteristic = new GattCharacteristicInterface(path, props, characteristicObj);
    m_objectManager->addObject(characteristic);
    m_objectManager->addAutoDeleteObject(characteristicObj);
}

void DeviceManager::runCreateGattDescriptorAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    QVariantMap props = properties;
    props.remove(QStringLiteral("Path"));

    GattDescriptorObject *descriptorObj = new GattDescriptorObject(path);
    GattDescriptorInterface *descriptor = new GattDescriptorInterface(path, props, descriptorObj);
    m_objectManager->addObject(descriptor);
    m_objectManager->addAutoDeleteObject(descriptorObj);
}

void DeviceManager::runRemoveAdapterAction(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    m_objectManager->removeObject(m_objectManager->objectByPath(path));
}

void DeviceManager::runRemoveDeviceAction(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    m_objectManager->removeObject(m_objectManager->objectByPath(path));
}

void DeviceManager::runRemoveGattServiceAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    m_objectManager->removeObject(m_objectManager->objectByPath(path));
}

void DeviceManager::runRemoveGattCharacteristicAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    m_objectManager->removeObject(m_objectManager->objectByPath(path));
}

void DeviceManager::runRemoveGattDescriptorAction(const QVariantMap& properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    m_objectManager->removeObject(m_objectManager->objectByPath(path));
}

void DeviceManager::runChangeAdapterProperty(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    Object *adapter = m_objectManager->objectByPath(path);
    if (!adapter || adapter->name() != QLatin1String("org.bluez.Adapter1")) {
        return;
    }

    adapter->changeProperty(properties.value(QStringLiteral("Name")).toString(), properties.value(QStringLiteral("Value")));
}

void DeviceManager::runChangeDeviceProperty(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("Path")).value<QDBusObjectPath>();
    Object *device = m_objectManager->objectByPath(path);
    if (!device || device->name() != QLatin1String("org.bluez.Device1")) {
        return;
    }

    device->changeProperty(properties.value(QStringLiteral("Name")).toString(), properties.value(QStringLiteral("Value")));
}

void DeviceManager::runAdapterMediaAction(const QString action, const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("AdapterPath")).value<QDBusObjectPath>();
    AdapterInterface *adapter = dynamic_cast<AdapterInterface *>(m_objectManager->objectByPath(path));
    if (!adapter) {
        return;
    }
    adapter->media()->runAction(action, properties);
}

void DeviceManager::runAdapterLeAdvertisingManagerAction(const QString action, const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("AdapterPath")).value<QDBusObjectPath>();
    AdapterInterface *adapter = dynamic_cast<AdapterInterface *>(m_objectManager->objectByPath(path));
    if (!adapter) {
        return;
    }
    adapter->leAdvertisingManager()->runAction(action, properties);
}

void DeviceManager::runAdapterGattManagerAction(const QString action, const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("AdapterPath")).value<QDBusObjectPath>();
    AdapterInterface *adapter = dynamic_cast<AdapterInterface *>(m_objectManager->objectByPath(path));
    if (!adapter) {
        return;
    }
    adapter->gattManager()->runAction(action, properties);
}

void DeviceManager::runBug377405()
{
    QDBusObjectPath adapter1path = QDBusObjectPath(QStringLiteral("/org/bluez/hci0"));
    QVariantMap adapterProps;
    adapterProps[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    adapterProps[QStringLiteral("Powered")] = false;

    runCreateAdapterAction(adapterProps);

    QVariantMap properties;
    properties[QStringLiteral("Path")] = QVariant::fromValue(adapter1path);
    properties[QStringLiteral("Name")] = QStringLiteral("Powered");
    properties[QStringLiteral("Value")] = true;

    runChangeAdapterProperty(properties);
}

void DeviceManager::runBug403289(const QVariantMap &properties)
{
    const QDBusObjectPath &path = properties.value(QStringLiteral("DevicePath")).value<QDBusObjectPath>();
    DeviceInterface *device = dynamic_cast<DeviceInterface *>(m_objectManager->objectByPath(path));
    if (!device) {
        return;
    }
    device->connectMediaPlayer();
    Q_EMIT m_objectManager->InterfacesRemoved(path, QStringList(QStringLiteral("org.bluez.MediaPlayer1")));
}

#include "moc_devicemanager.cpp"
