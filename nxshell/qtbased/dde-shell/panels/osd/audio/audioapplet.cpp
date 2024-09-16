// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audioapplet.h"

#include "pluginfactory.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusReply>

#include <DDBusSender>

namespace osd {

static DDBusSender audioInter()
{
    return DDBusSender().service("org.deepin.dde.Audio1")
        .path("/org/deepin/dde/Audio1")
        .interface("org.deepin.dde.Audio1");
}

static DDBusSender audioSinkInter()
{
    QDBusReply<QVariant> defaultSink = audioInter().property("DefaultSink").get();
    if (!defaultSink.isValid()) {
        qWarning() << "Failed to fetch DefaultSink" << defaultSink.error();
        return DDBusSender();
    }
    const auto path = qdbus_cast<QDBusObjectPath>(defaultSink);
    return DDBusSender().service("org.deepin.dde.Audio1")
        .path(path.path())
        .interface("org.deepin.dde.Audio1.Sink");
}

AudioApplet::AudioApplet(QObject *parent)
    : DApplet(parent)
{

}

bool AudioApplet::increaseVolume() const
{
    return m_increaseVolume;
}

void AudioApplet::setIncreaseVolume(bool newIncreaseVolume)
{
    if (m_increaseVolume == newIncreaseVolume)
        return;
    m_increaseVolume = newIncreaseVolume;
    emit increaseVolumeChanged();
}

void AudioApplet::sync()
{
    setVolumeValue(fetchVolume());

    setIncreaseVolume(fetchIncreaseVolume());

    setIconName(fetchIconName());
}

QString AudioApplet::fetchIconName() const
{
    QDBusReply<QVariant> muteReply = audioSinkInter().property("Mute").get();
    if (!muteReply.isValid()) {
        qWarning() << "Failed to fetch Mute" << muteReply.error();
        return QString();
    }
    if (qdbus_cast<bool>(muteReply)) {
        return "osd_volume_mute";
    }

    const double volume = m_volumeValue;

    QString level = "0";
    if (volume > 0 && volume <= 0.33)
        level = "33";
    else if (volume > 0.33 && volume <= 0.66)
        level = "66";
    else if (volume > 0.66 && volume <= 1)
        level = "100";
    else if (volume > 1)
        level = "more";

    return QString("osd_volume_%1").arg(level);
}

double AudioApplet::fetchVolume() const
{
    QDBusReply<QVariant> volumeReply = audioSinkInter().property("Volume").get();
    if (!volumeReply.isValid()) {
        qWarning() << "Failed to Volume Volume" << volumeReply.error();
        return 0.0;
    }

    return qdbus_cast<double>(volumeReply);
}

bool AudioApplet::fetchIncreaseVolume() const
{
    QDBusReply<QVariant> volumeReply = audioInter().property("IncreaseVolume").get();
    if (!volumeReply.isValid()) {
        qWarning() << "Failed to Volume IncreaseVolume" << volumeReply.error();
        return false;
    }

    return qdbus_cast<bool>(volumeReply);
}

QString AudioApplet::iconName() const
{
    return m_iconName;
}

void AudioApplet::setIconName(const QString &newIconName)
{
    if (m_iconName == newIconName)
        return;
    m_iconName = newIconName;
    emit iconNameChanged();
}

double AudioApplet::volumeValue() const
{
    return m_volumeValue;
}

void AudioApplet::setVolumeValue(double newVolumeValue)
{
    if (qFuzzyCompare(m_volumeValue, newVolumeValue))
        return;
    m_volumeValue = newVolumeValue;
    emit volumeValueChanged();
}

D_APPLET_CLASS(AudioApplet)

}

#include "audioapplet.moc"
