// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOCARD_H
#define DAUDIOCARD_H

#include <dtkaudiomanager_global.h>
#include <dexpected.h>

#include <QObject>
#include <QSharedPointer>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DAudioPort;
class DPlatformAudioCard;
using DAudioPortPtr = QSharedPointer<DAudioPort>;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioCard : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged)
public:
    enum Type{
        Normal,
        Bluetooth,
        Other = 64
    };
    Q_ENUM(Type)

    explicit DAudioCard(DPlatformAudioCard *d);
    virtual ~DAudioCard() override;

    QList<DAudioPortPtr> ports() const;
    QString name() const;
    quint32 id() const;
    virtual Type type() const;

    bool enabled() const;

Q_SIGNALS:
    void enabledChanged(bool enabled);

protected:
    friend class DPlatformAudioCard;
    QExplicitlySharedDataPointer<DPlatformAudioCard> d;
};

class DPlatformAudioBluetoothCard;
class LIBDTKAUDIOMANAGERSHARED_EXPORT DAudioBluetoothCard : public DAudioCard
{
    Q_OBJECT

    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(QStringList modeOptions READ modeOptions NOTIFY modeOptionsChanged)
public:
    explicit DAudioBluetoothCard(DPlatformAudioBluetoothCard *d);
    virtual ~DAudioBluetoothCard() override;

    QString mode() const;
    QStringList modeOptions() const;

    virtual DAudioCard::Type type() const override;

public Q_SLOTS:
    DExpected<void> setMode(QString &mode);

Q_SIGNALS:
    void modeChanged(QString mode);

    void modeOptionsChanged(QStringList modeOptions);

private:
    friend class DPlatformAudioBluetoothCard;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
