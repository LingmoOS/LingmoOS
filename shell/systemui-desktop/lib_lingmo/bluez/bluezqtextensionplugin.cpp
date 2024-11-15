/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "bluezqtextensionplugin.h"
#include "declarativeadapter.h"
// #include "declarativebattery.h"
#include "declarativedevice.h"
#include "declarativedevicesmodel.h"
#include "declarativeinput.h"
#include "declarativemanager.h"
#include "declarativemediaplayer.h"

#include "applet/devicesproxymodel.h"
#include "applet/bluetoothmanager.h"
#include "applet/bluetoothagent.h"

#include <BluezQt/Device>
#include <BluezQt/PendingCall>
#include <BluezQt/Rfkill>
#include <BluezQt/Services>

#include <QtQml> // krazy:exclude=includes

static QObject *manager_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    return new DeclarativeManager;
}

static QJSValue services_singleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)

    QJSValue object = scriptEngine->newObject();
    object.setProperty(QStringLiteral("ServiceDiscoveryServer"), BluezQt::Services::ServiceDiscoveryServer);
    object.setProperty(QStringLiteral("SerialPort"), BluezQt::Services::SerialPort);
    object.setProperty(QStringLiteral("DialupNetworking"), BluezQt::Services::DialupNetworking);
    object.setProperty(QStringLiteral("ObexObjectPush"), BluezQt::Services::ObexObjectPush);
    object.setProperty(QStringLiteral("ObexFileTransfer"), BluezQt::Services::ObexFileTransfer);
    object.setProperty(QStringLiteral("Headset"), BluezQt::Services::Headset);
    object.setProperty(QStringLiteral("AudioSource"), BluezQt::Services::AudioSource);
    object.setProperty(QStringLiteral("AudioVideoRemoteControlTarget"), BluezQt::Services::AudioVideoRemoteControlTarget);
    object.setProperty(QStringLiteral("AdvancedAudioDistribution"), BluezQt::Services::AdvancedAudioDistribution);
    object.setProperty(QStringLiteral("AudioVideoRemoteControl"), BluezQt::Services::AudioVideoRemoteControl);
    object.setProperty(QStringLiteral("HeadsetAudioGateway"), BluezQt::Services::HeadsetAudioGateway);
    object.setProperty(QStringLiteral("Panu"), BluezQt::Services::Panu);
    object.setProperty(QStringLiteral("Nap"), BluezQt::Services::Nap);
    object.setProperty(QStringLiteral("Handsfree"), BluezQt::Services::Handsfree);
    object.setProperty(QStringLiteral("HandsfreeAudioGateway"), BluezQt::Services::HandsfreeAudioGateway);
    object.setProperty(QStringLiteral("HumanInterfaceDevice"), BluezQt::Services::HumanInterfaceDevice);
    object.setProperty(QStringLiteral("SimAccess"), BluezQt::Services::SimAccess);
    object.setProperty(QStringLiteral("PhonebookAccessServer"), BluezQt::Services::PhonebookAccessServer);
    object.setProperty(QStringLiteral("MessageAccessServer"), BluezQt::Services::MessageAccessServer);
    object.setProperty(QStringLiteral("PnpInformation"), BluezQt::Services::PnpInformation);
    return object;
}

void BluezQtExtensionPlugin::registerTypes(const char *uri)
{
    using namespace BluezQt;

    Q_ASSERT(QLatin1String(uri) == QLatin1String("Lingmo.Bluez"));

    qmlRegisterSingletonType<DeclarativeManager>(uri, 1, 0, "Manager", manager_singleton);
    qmlRegisterType<DeclarativeDevicesModel>(uri, 1, 0, "DevicesModelPrivate");
    qmlRegisterType<BluetoothAgent>(uri, 1, 0, "BluetoothAgent");
    qmlRegisterType<BluetoothManager>(uri, 1, 0, "BluetoothManager");

    qmlRegisterUncreatableType<DeclarativeAdapter>(uri, 1, 0, "Adapter", QStringLiteral("Adapter cannot be created"));
    // qmlRegisterUncreatableType<DeclarativeBattery>(uri, 1, 0, "Battery", QStringLiteral("Battery cannot be created"));
    qmlRegisterUncreatableType<DeclarativeDevice>(uri, 1, 0, "Device", QStringLiteral("Device cannot be created"));
    qmlRegisterUncreatableType<DeclarativeInput>(uri, 1, 0, "Input", QStringLiteral("Input cannot be created"));
    qmlRegisterUncreatableType<DeclarativeMediaPlayer>(uri, 1, 0, "MediaPlayer", QStringLiteral("MediaPlayer cannot be created"));
    qmlRegisterUncreatableType<PendingCall>(uri, 1, 0, "PendingCall", QStringLiteral("PendingCall cannot be created"));
    qmlRegisterUncreatableType<Rfkill>(uri, 1, 0, "Rfkill", QStringLiteral("Rfkill cannot be created"));
    qmlRegisterSingletonType(uri, 1, 0, "Services", services_singleton);

    qmlRegisterType<DevicesProxyModel>(uri, 1, 0, "DevicesProxyModel");

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<QAbstractItemModel>();
#else
    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);
#endif
}
