// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAUDIOMANAGER_DAEMON_H
#define DAUDIOMANAGER_DAEMON_H

#include "daudiomanager_p.h"

#include <QDBusInterface>
#include <QObject>
#include <DDBusInterface>
#include <DExpected>

DAUDIOMANAGER_BEGIN_NAMESPACE
using DTK_CORE_NAMESPACE::DDBusInterface;
using DTK_CORE_NAMESPACE::DExpected;


class LIBDTKAUDIOMANAGERSHARED_EXPORT DDaemonAudioManager : public DAudioManagerPrivate
{
    Q_OBJECT

public:
    DDaemonAudioManager(QObject *parent = nullptr);
    virtual ~DDaemonAudioManager() override;

    virtual void reset() override;
    virtual void setReConnectionEnabled(const bool enable) override;

    virtual bool increaseVolume() const override;
    virtual bool reduceNoise() const override;
    virtual double maxVolume() const override;
public Q_SLOTS:
    virtual DExpected<void> setIncreaseVolume(bool increaseVolume) override;
    virtual DExpected<void> setReduceNoise(bool reduceNoise) override;

Q_SIGNALS:
    void deviceAdded(const QString &name, const bool isInputDevice);
    void deviceRemoved(const QString &name, const bool isInputDevice);

    void IncreaseVolumeChanged(bool increaseVolume);
    void ReduceNoiseChanged(bool reduceNoise);
    void MaxUIVolumeChanged(double maxVolume);

    void DefaultSinkChanged(QDBusObjectPath path);
    void DefaultSourceChanged(QDBusObjectPath path);
    void SinkInputsChanged(const QList<QDBusObjectPath> &paths);
    void CardsChanged(const QString &cards);
    void CardsWithoutUnavailableChanged(const QString &availableCards);
    void SinksChanged(const QList<QDBusObjectPath> &paths);
    void SourcesChanged(const QList<QDBusObjectPath> &paths);

private Q_SLOTS:
    void onPortEnableChanged(quint32 cardIndex, const QString &portName, bool enabled);
private:
    void updateCards();
    void updateAvailableOfCardPort();
    QMap<quint32, QStringList> availableCardAndPorts() const;
    void updateInputDevice();
    void updateOutputDevice();
    void updateOutputStream();
    void updateInputStream();

    bool containInputDevice(const QString &deviceName);
    bool containOutputDevice(const QString &deviceName);

    void compareAndDestroyCards(const QList<quint32> &newCards);
    void compareAndDestroyInputDevices(const QList<QString> &newDevices);
    void compareAndDestroyOutputDevices(const QList<QString> &newDevices);
    void compareAndDestroyInputStreams(const QList<QString> &nowStreams);
    void compareAndDestroyOutputStreams(const QList<QString> &nowStreams);

private:
    QScopedPointer<DDBusInterface> m_inter;

};
DAUDIOMANAGER_END_NAMESPACE

#endif
