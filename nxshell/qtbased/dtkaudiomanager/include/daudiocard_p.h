// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOCARD_P_H
#define DAUDIOCARD_P_H

#include "daudioport.h"
#include "daudiocard.h"
#include "daudioport_p.h"
#include "dtkcore_global.h"

#include <QObject>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
DCORE_USE_NAMESPACE

class DPlatformAudioPort;
class DAudioPort;
class DAudioCard;
class DPlatformAudioCard : public QObject, public QSharedData
{
    Q_OBJECT

public:
    explicit DPlatformAudioCard(QObject *parent = nullptr);
    virtual ~DPlatformAudioCard();

    quint32 id() const;
    void setId(const quint32 id);
    QString name() const;
    void setName(const QString &name);

    bool enabled() const;
    void setEnabled(const bool enabled);

    void addPort(DPlatformAudioPort *port);
    DPlatformAudioPort *portByName(const QString &portName) const;
    QList<DPlatformAudioPort *> ports() const;
    virtual DAudioCard *create()
    {
        return new DAudioCard(this);
    }

Q_SIGNALS:
    void enabledChanged(bool enabled);

public:
    QList<QExplicitlySharedDataPointer<DPlatformAudioPort>> m_ports;
    QString m_name;
    quint32 m_id;
    bool m_enabled;
};

class DPlatformAudioBluetoothCard : public DPlatformAudioCard
{
    Q_OBJECT

public:
    virtual DAudioCard *create() override
    {
        return new DAudioBluetoothCard(this);
    }

    virtual QString mode() const = 0;
    virtual QStringList modeOptions() const = 0;

public Q_SLOTS:
    virtual DExpected<void> setMode(const QString &mode) = 0;

Q_SIGNALS:
    void modeChanged(const QString &mode);

    void modeOptionsChanged(const QStringList &modeOptions);

};
DAUDIOMANAGER_END_NAMESPACE

#endif
