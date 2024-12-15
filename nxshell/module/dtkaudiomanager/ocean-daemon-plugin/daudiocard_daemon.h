// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOCARD_DAEMON_H
#define DAUDIOCARD_DAEMON_H

#include "daudiocard_p.h"

#include <QDBusInterface>
#include <QObject>
#include <DDBusInterface>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;

class DPlatformAudioPort;
class DDaemonAudioCard : public DPlatformAudioCard
{
    Q_OBJECT

public:
    explicit DDaemonAudioCard(QObject *parent = nullptr);
    virtual ~DDaemonAudioCard() override;
};

class DAudioBluetoothCardPrivate;
class DDaemonAudioBluetoothCard : public DDaemonAudioCard
{
    Q_OBJECT

public:
    explicit DDaemonAudioBluetoothCard(QObject *parent = nullptr);
    virtual ~DDaemonAudioBluetoothCard() override;

    QString mode() const;
    QStringList modeOptions() const;

public Q_SLOTS:
    DExpected<void> setMode(const QString &mode);

Q_SIGNALS:
    void modeChanged(const QString &mode);

    void modeOptionsChanged(const QStringList &modeOptions);

    void BluetoothAudioModeChanged(const QString &mode);
    void BluetoothAudioModeOptsChanged(const QStringList &modeOpts);

private:
    QScopedPointer<DDBusInterface>  m_inter;
};
DAUDIOMANAGER_END_NAMESPACE

#endif
