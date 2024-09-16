// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAEMONAUDIOMANAGERSERVICE_H
#define DAEMONAUDIOMANAGERSERVICE_H

#include <QDBusObjectPath>
#include <QObject>

class FakeDaemonService : public QObject
{
    Q_OBJECT
public:
    explicit FakeDaemonService(const QString &path = QString(),
                               const QString &interface = QString(),
                               QObject *parent = nullptr);
    virtual ~FakeDaemonService();

    QDBusObjectPath path() const
    {
        return QDBusObjectPath(m_servicePath);
    }

    virtual void init() {}

private:
    void registerService();
    void unregisterService();

    QString m_serviceName;
    QString m_servicePath;
    QString m_serviceInterface;
};

class FakeDaemonAudioDeviceService : public FakeDaemonService
{
    Q_OBJECT

    Q_PROPERTY(bool Mute READ Mute WRITE SetMute NOTIFY MuteChanged)
    Q_PROPERTY(double Fade READ Fade WRITE SetFade NOTIFY FadeChanged)
    Q_PROPERTY(double Volume READ Volume WRITE SetVolume NOTIFY VolumeChanged)
    Q_PROPERTY(double Balance READ Balance WRITE SetBalance NOTIFY BalanceChanged)
    Q_PROPERTY(bool SupportBalance READ SupportBalance NOTIFY SupportBalanceChanged)
    Q_PROPERTY(bool SupportFade READ SupportFade NOTIFY SupportFadeChanged)
    Q_PROPERTY(double BaseVolume READ BaseVolume NOTIFY BaseVolumeChanged)
    Q_PROPERTY(QString Name READ Name NOTIFY NameChanged)
    Q_PROPERTY(QString Description READ Description NOTIFY DescriptionChanged)
    Q_PROPERTY(quint32 Card READ Card NOTIFY CardChanged)

public:
    explicit FakeDaemonAudioDeviceService(const QString &path, const QString &interface, QObject *parent = nullptr);
    bool Mute() const
    {
        return m_Mute;
    }
    double Fade() const
    {
        return m_Fade;
    }

    double Volume() const
    {
        return m_Volume;
    }

    double Balance() const
    {
        return m_Balance;
    }

    bool SupportBalance() const
    {
        return m_SupportBalance;
    }

    bool SupportFade() const
    {
        return m_SupportFade;
    }

    double BaseVolume() const
    {
        return m_BaseVolume;
    }

    QString Name() const
    {
        return m_Name;
    }

    QString Description() const
    {
        return m_Description;
    }

    quint32 Card() const
    {
        return m_Card;
    }

public Q_SLOTS:
    void SetMute(bool Mute)
    {
        if (m_Mute == Mute)
            return;

        m_Mute = Mute;
        emit MuteChanged(m_Mute);
    }
    void SetFade(double Fade)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Fade, Fade))
            return;

        m_Fade = Fade;
        emit FadeChanged(m_Fade);
    }

    void SetVolume(double Volume)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Volume, Volume))
            return;

        m_Volume = Volume;
        emit VolumeChanged(m_Volume);
    }

    void SetBalance(double Balance)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Balance, Balance))
            return;

        m_Balance = Balance;
        emit BalanceChanged(m_Balance);
    }
public:
    void setCard(quint32 cardId)
    {
        m_Card = cardId;
    }

Q_SIGNALS:
    void MuteChanged(bool Mute);
    void FadeChanged(double Fade);
    void VolumeChanged(double Volume);
    void BalanceChanged(double Balance);
    void SupportBalanceChanged(bool SupportBalance);
    void SupportFadeChanged(bool SupportFade);
    void BaseVolumeChanged(double BaseVolume);
    void NameChanged(QString Name);
    void DescriptionChanged(QString Description);
    void CardChanged(quint32 Card);

private:
    bool m_Mute = true;
    double m_Fade = 1.0;
    double m_Volume = 1.0;
    double m_Balance = 1.0;
    bool m_SupportBalance = true;
    bool m_SupportFade = true;
    double m_BaseVolume = 1.0;
    QString m_Name;
    QString m_Description;
    quint32 m_Card = 0;
};

class FakeDaemonAudioSourceService : public FakeDaemonAudioDeviceService
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAudio.Source")
public:
    explicit FakeDaemonAudioSourceService(const QString &path, QObject *parent = nullptr);
    virtual void init() override;
};

class FakeDaemonAudioSinkService : public FakeDaemonAudioDeviceService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAudio.Sink")
public:
    explicit FakeDaemonAudioSinkService(const QString &path, QObject *parent = nullptr);
    virtual void init() override;
};

class FakeDaemonAudioStreamService : public FakeDaemonService
{
    Q_OBJECT

    Q_PROPERTY(bool Mute READ Mute WRITE SetMute NOTIFY MuteChanged)
    Q_PROPERTY(double Fade READ Fade WRITE SetFade NOTIFY FadeChanged)
    Q_PROPERTY(double Volume READ Volume WRITE SetVolume NOTIFY VolumeChanged)
    Q_PROPERTY(double Balance READ Balance WRITE SetBalance NOTIFY BalanceChanged)
    Q_PROPERTY(bool SupportBalance READ SupportBalance NOTIFY SupportBalanceChanged)
    Q_PROPERTY(bool SupportFade READ SupportFade NOTIFY SupportFadeChanged)
    Q_PROPERTY(double BaseVolume READ BaseVolume NOTIFY BaseVolumeChanged)
    Q_PROPERTY(QString Name READ Name NOTIFY NameChanged)
    Q_PROPERTY(QString Description READ Description NOTIFY DescriptionChanged)

    Q_PROPERTY(quint32 SinkIndex READ SinkIndex NOTIFY SinkIndexChanged)

public:
    explicit FakeDaemonAudioStreamService(const QString &path, const QString &interface, QObject *parent = nullptr);

    bool Mute() const
    {
        return m_Mute;
    }
    double Fade() const
    {
        return m_Fade;
    }

    double Volume() const
    {
        return m_Volume;
    }

    double Balance() const
    {
        return m_Balance;
    }

    bool SupportBalance() const
    {
        return m_SupportBalance;
    }

    bool SupportFade() const
    {
        return m_SupportFade;
    }

    double BaseVolume() const
    {
        return m_BaseVolume;
    }

    QString Name() const
    {
        return m_Name;
    }

    QString Description() const
    {
        return m_Description;
    }

public Q_SLOTS:
    void SetMute(bool Mute)
    {
        if (m_Mute == Mute)
            return;

        m_Mute = Mute;
        emit MuteChanged(m_Mute);
    }
    void SetFade(double Fade)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Fade, Fade))
            return;

        m_Fade = Fade;
        emit FadeChanged(m_Fade);
    }

    void SetVolume(double Volume)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Volume, Volume))
            return;

        m_Volume = Volume;
        emit VolumeChanged(m_Volume);
    }

    void SetBalance(double Balance)
    {
        qWarning("Floating point comparison needs context sanity check");
        if (qFuzzyCompare(m_Balance, Balance))
            return;

        m_Balance = Balance;
        emit BalanceChanged(m_Balance);
    }

    quint32 SinkIndex() const
    {
        return m_SinkIndex;
    }
    void setSink(quint32 index)
    {
        if (index == m_SinkIndex)
            return;

        m_SinkIndex = index;
        Q_EMIT SinkIndexChanged(m_SinkIndex);
    }

Q_SIGNALS:
    void MuteChanged(bool Mute);
    void FadeChanged(double Fade);
    void VolumeChanged(double Volume);
    void BalanceChanged(double Balance);
    void SupportBalanceChanged(bool SupportBalance);
    void SupportFadeChanged(bool SupportFade);
    void BaseVolumeChanged(double BaseVolume);
    void NameChanged(QString Name);
    void DescriptionChanged(QString Description);

    void SinkIndexChanged(quint32 SinkIndex);
private:
    bool m_Mute = true;
    double m_Fade = 1.0;
    double m_Volume = 1.0;
    double m_Balance = 1.0;
    bool m_SupportBalance = true;
    bool m_SupportFade = true;
    double m_BaseVolume = 1.0;
    QString m_Name;
    QString m_Description;

    quint32 m_SinkIndex = 1;
};

class FakeDaemonAudioSinkInputService : public FakeDaemonAudioStreamService
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAudio.SinkInput")

public:
    explicit FakeDaemonAudioSinkInputService(const QString &path, QObject *parent = nullptr);
};

class FakeDaemonAudioSourceOutputService : public FakeDaemonAudioStreamService
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAudio.SourceOutput")
public:
    explicit FakeDaemonAudioSourceOutputService(const QString &path, QObject *parent = nullptr);
};

class FakeDaemonAudioManagerService : public FakeDaemonService
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakeAudio")

    Q_PROPERTY(QString Cards READ Cards NOTIFY CardsChanged)
    Q_PROPERTY(QList<QDBusObjectPath> Sinks READ Sinks NOTIFY SinksChanged)
    Q_PROPERTY(QString CardsWithoutUnavailable READ cardsWithoutUnavailable NOTIFY CardsWithoutUnavailableChanged)
    Q_PROPERTY(QDBusObjectPath DefaultSink READ defaultSink NOTIFY DefaultSinkChanged)
    Q_PROPERTY(QDBusObjectPath DefaultSource READ defaultSource NOTIFY DefaultSourceChanged)
    Q_PROPERTY(QList<QDBusObjectPath> SinkInputs READ sinkInputs NOTIFY SinkInputsChanged)
    Q_PROPERTY(QList<QDBusObjectPath> Sources READ sources NOTIFY SourcesChanged)

    Q_PROPERTY(bool IncreaseVolume READ IncreaseVolume WRITE SetIncreaseVolume NOTIFY IncreaseVolumeChanged)
    Q_PROPERTY(bool ReduceNoise READ ReduceNoise WRITE SetReduceNoise NOTIFY ReduceNoiseChanged)
    Q_PROPERTY(double MaxUIVolume READ MaxUIVolume NOTIFY MaxUIVolumeChanged)

public:
    explicit FakeDaemonAudioManagerService(QObject *parent = nullptr);
    virtual ~FakeDaemonAudioManagerService() override;
    virtual void init() override;

    QList<QDBusObjectPath> Sinks() const
    {
        return m_Sinks;
    }

    QString Cards() const
    {
        return m_Cards;
    }

    bool IncreaseVolume() const
    {
        return m_IncreaseVolume;
    }

    bool ReduceNoise() const
    {
        return m_ReduceNoise;
    }

    double MaxUIVolume() const
    {
        return m_MaxUIVolume;
    }

    QString cardsWithoutUnavailable() const
    {
        return m_CardsWithoutUnavailable;
    }

    QDBusObjectPath defaultSink() const
    {
        return m_DefaultSink;
    }

    QDBusObjectPath defaultSource() const
    {
        return m_DefaultSource;
    }

    QList<QDBusObjectPath> sinkInputs() const
    {
        return m_SinkInputs;
    }

    QList<QDBusObjectPath> sources() const
    {
        return m_Sources;
    }

    void setCard(const QString &card)
    {
        if (card == m_Cards)
            return;

        m_Cards = card;
        Q_EMIT CardsChanged(m_Cards);
    }

public Q_SLOTS:

    void SetIncreaseVolume(bool IncreaseVolume)
    {
        if (m_IncreaseVolume == IncreaseVolume)
            return;

        m_IncreaseVolume = IncreaseVolume;
        emit IncreaseVolumeChanged(m_IncreaseVolume);
    }

    void SetReduceNoise(bool ReduceNoise)
    {
        if (m_ReduceNoise == ReduceNoise)
            return;

        m_ReduceNoise = ReduceNoise;
        emit ReduceNoiseChanged(m_ReduceNoise);
    }

Q_SIGNALS:
    void SinksChanged(QList<QDBusObjectPath> Sinks);

    void CardsChanged(QString Cards);

    void IncreaseVolumeChanged(bool IncreaseVolume);

    void ReduceNoiseChanged(bool ReduceNoise);

    void MaxUIVolumeChanged(double MaxUIVolume);

    void CardsWithoutUnavailableChanged(QString CardsWithoutUnavailable);

    void DefaultSinkChanged(QDBusObjectPath DefaultSink);

    void DefaultSourceChanged(QDBusObjectPath DefaultSource);

    void SinkInputsChanged(QList<QDBusObjectPath> SinkInputs);

    void SourcesChanged(QList<QDBusObjectPath> Sources);

private:
    QList<QDBusObjectPath> m_Sinks;
    QString m_Cards;
    bool m_IncreaseVolume = false;
    bool m_ReduceNoise = false;
    double m_MaxUIVolume = 1.0;
    QString m_CardsWithoutUnavailable;
    QDBusObjectPath m_DefaultSink;
    QDBusObjectPath m_DefaultSource;
    QList<QDBusObjectPath> m_SinkInputs;
    QList<QDBusObjectPath> m_Sources;
};

#endif
