// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDISKDRIVE_H
#define DDISKDRIVE_H

#include <DtkMountGlobal>

#include <QObject>
#include <QVariantMap>

#include <DExpected>

DCORE_USE_NAMESPACE
DMOUNT_BEGIN_NAMESPACE

class DDiskDrive;
namespace DDeviceManager {
DExpected<DDiskDrive *> createDiskDrive(const QString &path, QObject *parent);
}   // namespace DDeviceManager

class DDiskDrivePrivate;
class DDiskDrive : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DDiskDrive)

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
    Q_PROPERTY(quint64 size READ size CONSTANT FINAL)
    Q_PROPERTY(QString sortKey READ sortKey CONSTANT FINAL)
    Q_PROPERTY(quint64 timeDetected READ timeDetected CONSTANT FINAL)
    Q_PROPERTY(quint64 timeMediaDetected READ timeMediaDetected CONSTANT FINAL)
    Q_PROPERTY(QString vendor READ vendor CONSTANT FINAL)
    Q_PROPERTY(QString WWN READ WWN CONSTANT FINAL)

public:
    ~DDiskDrive();
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
    quint64 size() const;
    QString sortKey() const;
    quint64 timeDetected() const;
    quint64 timeMediaDetected() const;
    QString vendor() const;
    QString WWN() const;

    // TODO(zhangs): lastError

public Q_SLOTS:
    void eject(const QVariantMap &options);
    void powerOff(const QVariantMap &options);
    void setConfiguration(const QVariantMap &value, const QVariantMap &options);

protected:
    explicit DDiskDrive(const QString &path, QObject *parent = nullptr);

private:
    QScopedPointer<DDiskDrivePrivate> d_ptr;

    friend DExpected<DDiskDrive *> DDeviceManager::createDiskDrive(const QString &path, QObject *parent);
};

DMOUNT_END_NAMESPACE

#endif   // DDISKDRIVE_H
