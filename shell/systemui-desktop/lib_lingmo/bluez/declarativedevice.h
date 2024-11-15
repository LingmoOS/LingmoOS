/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef DECLARATIVEDEVICE_H
#define DECLARATIVEDEVICE_H

#include "declarativeinput.h"
#include "declarativemediaplayer.h"

#include <BluezQt/Device>

class DeclarativeAdapter;
// class DeclarativeBattery;

class DeclarativeDevice : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString ubi READ ubi CONSTANT)
    Q_PROPERTY(QString address READ address CONSTANT)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString friendlyName READ friendlyName NOTIFY friendlyNameChanged)
    Q_PROPERTY(QString remoteName READ remoteName NOTIFY remoteNameChanged)
    Q_PROPERTY(quint32 deviceClass READ deviceClass NOTIFY deviceClassChanged)
    Q_PROPERTY(BluezQt::Device::Type type READ type NOTIFY typeChanged)
    Q_PROPERTY(quint16 appearance READ appearance NOTIFY appearanceChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(bool paired READ isPaired NOTIFY pairedChanged)
    Q_PROPERTY(bool trusted READ isTrusted WRITE setTrusted NOTIFY trustedChanged)
    Q_PROPERTY(bool blocked READ isBlocked WRITE setBlocked NOTIFY blockedChanged)
    Q_PROPERTY(bool legacyPairing READ hasLegacyPairing NOTIFY legacyPairingChanged)
    Q_PROPERTY(qint16 rssi READ rssi NOTIFY rssiChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(QStringList uuids READ uuids NOTIFY uuidsChanged)
    Q_PROPERTY(QString modalias READ modalias NOTIFY modaliasChanged)
    Q_PROPERTY(DeclarativeInput *input READ input NOTIFY inputChanged)
    Q_PROPERTY(DeclarativeMediaPlayer *mediaPlayer READ mediaPlayer NOTIFY mediaPlayerChanged)
    Q_PROPERTY(DeclarativeAdapter *adapter READ adapter CONSTANT)

public:
    explicit DeclarativeDevice(BluezQt::DevicePtr device, DeclarativeAdapter *adapter);

    QString ubi() const;

    QString address() const;

    QString name() const;
    void setName(const QString &name);

    QString friendlyName() const;

    QString remoteName() const;

    quint32 deviceClass() const;

    BluezQt::Device::Type type() const;

    quint16 appearance() const;

    QString icon() const;

    bool isPaired() const;

    bool isTrusted() const;
    void setTrusted(bool trusted);

    bool isBlocked() const;
    void setBlocked(bool blocked);

    bool hasLegacyPairing() const;

    qint16 rssi() const;

    bool isConnected() const;

    QStringList uuids() const;

    QString modalias() const;

    // DeclarativeBattery *battery() const;

    DeclarativeInput *input() const;

    DeclarativeMediaPlayer *mediaPlayer() const;

    DeclarativeAdapter *adapter() const;

public Q_SLOTS:
    BluezQt::PendingCall *connectToDevice();
    BluezQt::PendingCall *disconnectFromDevice();
    BluezQt::PendingCall *connectProfile(const QString &uuid);
    BluezQt::PendingCall *disconnectProfile(const QString &uuid);
    BluezQt::PendingCall *pair();
    BluezQt::PendingCall *cancelPairing();

Q_SIGNALS:
    void deviceRemoved(DeclarativeDevice *device);
    void deviceChanged(DeclarativeDevice *device);
    void nameChanged(const QString &name);
    void friendlyNameChanged(const QString &friendlyName);
    void remoteNameChanged(const QString &remoteName);
    void deviceClassChanged(quint32 deviceClass);
    void typeChanged(BluezQt::Device::Type type);
    void appearanceChanged(quint16 appearance);
    void iconChanged(const QString &icon);
    void pairedChanged(bool paired);
    void trustedChanged(bool trusted);
    void blockedChanged(bool blocked);
    void legacyPairingChanged(bool legacyPairing);
    void rssiChanged(qint16 rssi);
    void connectedChanged(bool connected);
    void uuidsChanged(const QStringList &uuids);
    void modaliasChanged(const QString &modalias);
    // void batteryChanged(DeclarativeBattery *battery);
    void inputChanged(DeclarativeInput *input);
    void mediaPlayerChanged(DeclarativeMediaPlayer *mediaPlayer);

private:
    void updateBattery();
    void updateInput();
    void updateMediaPlayer();

    BluezQt::DevicePtr m_device;
    DeclarativeAdapter *m_adapter;
    // DeclarativeBattery *m_battery;
    DeclarativeInput *m_input;
    DeclarativeMediaPlayer *m_mediaPlayer;
};

#endif // DECLARATIVEDEVICE_H
