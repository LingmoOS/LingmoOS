/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_SERVICES_H
#define BLUEZQT_SERVICES_H

#include <QString>

namespace BluezQt
{
/** Service UUIDs. */
namespace Services
{
// Note: If you add/remove/modify any UUIDs in this namespace, make sure to
// mirror the changes to src/imports/bluezqtextensionplugin.cpp as well
const QString ServiceDiscoveryServer = QLatin1String("00001000-0000-1000-8000-00805F9B34FB");
const QString SerialPort = QLatin1String("00001101-0000-1000-8000-00805F9B34FB");
const QString DialupNetworking = QLatin1String("00001103-0000-1000-8000-00805F9B34FB");
const QString ObexObjectPush = QLatin1String("00001105-0000-1000-8000-00805F9B34FB");
const QString ObexFileTransfer = QLatin1String("00001106-0000-1000-8000-00805F9B34FB");
const QString Headset = QLatin1String("00001108-0000-1000-8000-00805F9B34FB");
const QString AudioSource = QLatin1String("0000110A-0000-1000-8000-00805F9B34FB");
const QString AudioSink = QLatin1String("0000110B-0000-1000-8000-00805F9B34FB");
const QString AudioVideoRemoteControlTarget = QLatin1String("0000110C-0000-1000-8000-00805F9B34FB");
const QString AdvancedAudioDistribution = QLatin1String("0000110D-0000-1000-8000-00805F9B34FB");
const QString AudioVideoRemoteControl = QLatin1String("0000110E-0000-1000-8000-00805F9B34FB");
const QString HeadsetAudioGateway = QLatin1String("00001112-0000-1000-8000-00805F9B34FB");
const QString Panu = QLatin1String("00001115-0000-1000-8000-00805F9B34FB");
const QString Nap = QLatin1String("00001116-0000-1000-8000-00805F9B34FB");
const QString Handsfree = QLatin1String("0000111E-0000-1000-8000-00805F9B34FB");
const QString HandsfreeAudioGateway = QLatin1String("0000111F-0000-1000-8000-00805F9B34FB");
const QString HumanInterfaceDevice = QLatin1String("00001124-0000-1000-8000-00805F9B34FB");
const QString SimAccess = QLatin1String("0000112D-0000-1000-8000-00805F9B34FB");
const QString PhonebookAccessServer = QLatin1String("0000112F-0000-1000-8000-00805F9B34FB");
const QString MessageAccessServer = QLatin1String("00001132-0000-1000-8000-00805F9B34FB");
const QString PnpInformation = QLatin1String("00001200-0000-1000-8000-00805F9B34FB");

// Bluetooth Low Energy
const QString GenericAccess = QLatin1String("00001800-0000-1000-8000-00805f9b34fb");
const QString GenericAcces = GenericAccess; // TODO KF7: Remove the one with typo
const QString GenericAttribute = QLatin1String("00001801-0000-1000-8000-00805f9b34fb");
const QString ImmediateAlert = QLatin1String("00001802-0000-1000-8000-00805f9b34fb");
const QString LinkLoss = QLatin1String("00001803-0000-1000-8000-00805f9b34fb");
const QString TxPower = QLatin1String("00001804-0000-1000-8000-00805f9b34fb");
const QString HeartRate = QLatin1String("0000180d-0000-1000-8000-00805f9b34fb");
const QString CoordinatedSetIdentification = QLatin1String("00001846-0000-1000-8000-00805f9b34fb");
const QString MicrophoneControl = QLatin1String("0000184D-0000-1000-8000-00805f9b34fb");
const QString BroadcastAudioScan = QLatin1String("0000184F-0000-1000-8000-00805f9b34fb");
const QString PublishedAudioCapabilities = QLatin1String("00001851-0000-1000-8000-00805f9b34fb");
const QString CommonAudio = QLatin1String("00001853-0000-1000-8000-00805f9b34fb");
}

} // namespace BluezQt

#endif // BLUEZQT_SERVICES_H
