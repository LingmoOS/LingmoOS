// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDISKDEVICE_H
#define DDISKDEVICE_H

#include <QObject>
#include <QVariantMap>
#include <QDBusError>

class DDiskDevicePrivate;
class DDiskDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DDiskDevice)

    Q_PROPERTY(QString path READ path CONSTANT FINAL)
    Q_PROPERTY(bool canPowerOff READ canPowerOff CONSTANT FINAL)
    Q_PROPERTY(QVariantMap configuration READ configuration CONSTANT FINAL)
    Q_PROPERTY(QString connectionBus READ connectionBus CONSTANT FINAL)
    Q_PROPERTY(bool ejectable READ ejectable CONSTANT FINAL)
    Q_PROPERTY(QString id READ id CONSTANT FINAL)
    Q_PROPERTY(QString media READ media CONSTANT FINAL)
    Q_PROPERTY(bool mediaAvailable READ mediaAvailable CONSTANT FINAL)
    Q_PROPERTY(bool mediaChangeDetected READ mediaChangeDetected CONSTANT FINAL)
    Q_PROPERTY(QStringList mediaCompatibility READ mediaCompatibility CONSTANT FINAL)
    Q_PROPERTY(bool mediaRemovable READ mediaRemovable CONSTANT FINAL)
    Q_PROPERTY(QString model READ model CONSTANT FINAL)
    Q_PROPERTY(bool optical READ optical CONSTANT FINAL)
    Q_PROPERTY(bool opticalBlank READ opticalBlank CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumAudioTracks READ opticalNumAudioTracks CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumDataTracks READ opticalNumDataTracks CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumSessions READ opticalNumSessions CONSTANT FINAL)
    Q_PROPERTY(uint opticalNumTracks READ opticalNumTracks CONSTANT FINAL)
    Q_PROPERTY(bool removable READ removable CONSTANT FINAL)
    Q_PROPERTY(QString revision READ revision CONSTANT FINAL)
    Q_PROPERTY(int rotationRate READ rotationRate CONSTANT FINAL)
    Q_PROPERTY(QString seat READ seat CONSTANT FINAL)
    Q_PROPERTY(QString serial READ serial CONSTANT FINAL)
    Q_PROPERTY(QString siblingId READ siblingId CONSTANT FINAL)
    Q_PROPERTY(qulonglong size READ size CONSTANT FINAL)
    Q_PROPERTY(QString sortKey READ sortKey CONSTANT FINAL)
    Q_PROPERTY(qulonglong timeDetected READ timeDetected CONSTANT FINAL)
    Q_PROPERTY(qulonglong timeMediaDetected READ timeMediaDetected CONSTANT FINAL)
    Q_PROPERTY(QString vendor READ vendor CONSTANT FINAL)
    Q_PROPERTY(QString WWN READ WWN CONSTANT FINAL)

public:
    ~DDiskDevice();
    QString path() const;
    bool canPowerOff() const;
    QVariantMap configuration() const;
    QString connectionBus() const;
    bool ejectable() const;
    QString id() const;
    QString media() const;
    bool mediaAvailable() const;
    bool mediaChangeDetected() const;
    QStringList mediaCompatibility() const;
    bool mediaRemovable() const;
    QString model() const;
    bool optical() const;
    bool opticalBlank() const;
    uint opticalNumAudioTracks() const;
    uint opticalNumDataTracks() const;
    uint opticalNumSessions() const;
    uint opticalNumTracks() const;
    bool removable() const;
    QString revision() const;
    int rotationRate() const;
    QString seat() const;
    QString serial() const;
    QString siblingId() const;
    qulonglong size() const;
    QString sortKey() const;
    qulonglong timeDetected() const;
    qulonglong timeMediaDetected() const;
    QString vendor() const;
    QString WWN() const;

    QDBusError lastError() const;

public Q_SLOTS: // METHODS
    void eject(const QVariantMap &options);
    void powerOff(const QVariantMap &options);
    void setConfiguration(const QVariantMap &value, const QVariantMap &options);

private:
    explicit DDiskDevice(const QString &path, QObject *parent = nullptr);

    QScopedPointer<DDiskDevicePrivate> d_ptr;

    friend class DDiskManager;
};

#endif // DDISKDEVICE_H
