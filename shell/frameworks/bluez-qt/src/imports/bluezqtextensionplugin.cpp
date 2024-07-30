/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "bluezqtextensionplugin.h"
#include "services.h"

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
    object.setProperty(QStringLiteral("AudioSink"), BluezQt::Services::AudioSink);
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

    // Bluetooth Low Energy
    object.setProperty(QStringLiteral("GenericAccess"), BluezQt::Services::GenericAccess);
    object.setProperty(QStringLiteral("GenericAcces"), BluezQt::Services::GenericAccess); // TODO KF7: Remove the one with typo
    object.setProperty(QStringLiteral("GenericAttribute"), BluezQt::Services::GenericAttribute);
    object.setProperty(QStringLiteral("ImmediateAlert"), BluezQt::Services::ImmediateAlert);
    object.setProperty(QStringLiteral("LinkLoss"), BluezQt::Services::LinkLoss);
    object.setProperty(QStringLiteral("TxPower"), BluezQt::Services::TxPower);
    object.setProperty(QStringLiteral("HeartRate"), BluezQt::Services::HeartRate);
    object.setProperty(QStringLiteral("CoordinatedSetIdentification"), BluezQt::Services::CoordinatedSetIdentification);
    object.setProperty(QStringLiteral("MicrophoneControl"), BluezQt::Services::MicrophoneControl);
    object.setProperty(QStringLiteral("BroadcastAudioScan"), BluezQt::Services::BroadcastAudioScan);
    object.setProperty(QStringLiteral("PublishedAudioCapabilities"), BluezQt::Services::PublishedAudioCapabilities);
    object.setProperty(QStringLiteral("CommonAudio"), BluezQt::Services::CommonAudio);

    return object;
}

void BluezQtExtensionPlugin::registerTypes(const char *uri)
{
    using namespace BluezQt;

    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.bluezqt"));

    qmlRegisterSingletonType(uri, 1, 0, "Services", services_singleton);
}

#include "moc_bluezqtextensionplugin.cpp"
