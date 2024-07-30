/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@gmail.com>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "audioshortcutsservice.h"

#include "audioicon.h"

#include <QAction>
#include <QStringBuilder>

#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

#include <PulseAudioQt/Device>
#include <PulseAudioQt/Server>
#include <PulseAudioQt/Sink>
#include <PulseAudioQt/Source>

#include "preferreddevice.h"

K_PLUGIN_CLASS_WITH_JSON(AudioShortcutsService, "audioshortcutsservice.json")

constexpr QLatin1String OSD_DBUS_SERVICE = "org.kde.plasmashell"_L1;
constexpr QLatin1String OSD_DBUS_PATH = "/org/kde/osdService"_L1;

AudioShortcutsService::AudioShortcutsService(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , m_sinkModel(new PulseAudioQt::SinkModel(this))
    , m_sourceModel(new PulseAudioQt::SourceModel(this))
    , m_cardModel(new PulseAudioQt::CardModel(this))
    , m_globalConfig(new GlobalConfig(this))
    , m_osdDBusInterface(new OsdServiceInterface(OSD_DBUS_SERVICE, OSD_DBUS_PATH, QDBusConnection::sessionBus(), this))
    , m_feedback(new VolumeFeedback(this))
{
    connect(PulseAudioQt::Context::instance()->server(), &PulseAudioQt::Server::defaultSinkChanged, this, &AudioShortcutsService::handleDefaultSinkChange);
    connect(&m_preferredDevice, &PreferredDevice::sinkChanged, this, [this]() {
        auto sink = m_preferredDevice.sink();
        if (!sink) {
            return;
        }
        connect(sink, &PulseAudioQt::Sink::volumeChanged, this, [this]() {
            auto sink = m_preferredDevice.sink();
            if (!sink) {
                return;
            }
            showVolume(volumePercent(sink->volume()));
        });
    });
    connect(m_sinkModel, &PulseAudioQt::SinkModel::rowsInserted, this, &AudioShortcutsService::handleNewSink);

    QList<QAction *> actions;

    QAction *volumeUpAction = new QAction(this);
    actions.append(volumeUpAction);
    volumeUpAction->setObjectName(u"increase_volume"_s);
    volumeUpAction->setText(i18n("Increase Volume"));
    volumeUpAction->setShortcut(Qt::Key_VolumeUp);
    connect(volumeUpAction, &QAction::triggered, this, [this]() {
        auto sink = m_preferredDevice.sink();
        if (!sink) {
            return;
        }
        int percent = changeVolumePercent(sink, m_globalConfig->volumeStep());
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeDownAction = new QAction(this);
    actions.append(volumeDownAction);
    volumeDownAction->setObjectName(u"decrease_volume"_s);
    volumeDownAction->setText(i18n("Decrease Volume"));
    volumeDownAction->setShortcut(Qt::Key_VolumeDown);
    connect(volumeDownAction, &QAction::triggered, this, [this]() {
        auto sink = m_preferredDevice.sink();
        if (!sink) {
            return;
        }
        int percent = changeVolumePercent(sink, -m_globalConfig->volumeStep());
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeUpSmallAction = new QAction(this);
    actions.append(volumeUpSmallAction);
    volumeUpSmallAction->setObjectName(u"increase_volume_small"_s);
    volumeUpSmallAction->setText(i18n("Increase Volume by 1%"));
    volumeUpSmallAction->setShortcut(Qt::ShiftModifier | Qt::Key_VolumeUp);
    connect(volumeUpSmallAction, &QAction::triggered, this, [this]() {
        auto sink = m_preferredDevice.sink();
        if (!sink) {
            return;
        }
        int percent = changeVolumePercent(sink, 1);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeDownSmallAction = new QAction(this);
    actions.append(volumeDownSmallAction);
    volumeDownSmallAction->setObjectName(u"decrease_volume_small"_s);
    volumeDownSmallAction->setText(i18n("Decrease Volume by 1%"));
    volumeDownSmallAction->setShortcut(Qt::ShiftModifier | Qt::Key_VolumeDown);
    connect(volumeDownSmallAction, &QAction::triggered, this, [this]() {
        auto sink = m_preferredDevice.sink();
        if (!sink) {
            return;
        }
        int percent = changeVolumePercent(sink, -1);
        showVolume(percent);
        playFeedback(-1);
    });

    QAction *volumeUpMicAction = new QAction(this);
    actions.append(volumeUpMicAction);
    volumeUpMicAction->setObjectName(u"increase_microphone_volume"_s);
    volumeUpMicAction->setText(i18n("Increase Microphone Volume"));
    volumeUpMicAction->setShortcut(Qt::Key_MicVolumeUp);
    connect(volumeUpMicAction, &QAction::triggered, this, [this]() {
        auto defaultSource = PulseAudioQt::Context::instance()->server()->defaultSource();
        if (!defaultSource) {
            return;
        }
        int percent = changeVolumePercent(defaultSource, m_globalConfig->volumeStep());
        showMicVolume(percent);
    });

    QAction *volumeDownMicAction = new QAction(this);
    actions.append(volumeDownMicAction);
    volumeDownMicAction->setObjectName(u"decrease_microphone_volume"_s);
    volumeDownMicAction->setText(i18n("Decrease Microphone Volume"));
    volumeDownMicAction->setShortcut(Qt::Key_MicVolumeDown);
    connect(volumeDownMicAction, &QAction::triggered, this, [this]() {
        auto defaultSource = PulseAudioQt::Context::instance()->server()->defaultSource();
        if (!defaultSource) {
            return;
        }
        int percent = changeVolumePercent(defaultSource, -m_globalConfig->volumeStep());
        showMicVolume(percent);
    });

    QAction *muteAction = new QAction(this);
    actions.append(muteAction);
    muteAction->setObjectName(u"mute"_s);
    muteAction->setText(i18n("Mute"));
    muteAction->setShortcut(Qt::Key_VolumeMute);
    connect(muteAction, &QAction::triggered, this, [this]() {
        if (m_globalConfig->globalMute()) {
            disableGlobalMute();
        } else {
            enableGlobalMute();
        }
    });

    QAction *muteMicAction = new QAction(this);
    actions.append(muteMicAction);
    muteMicAction->setObjectName(u"mic_mute"_s);
    muteMicAction->setText(i18n("Mute Microphone"));
    muteMicAction->setShortcuts({Qt::Key_MicMute, Qt::MetaModifier | Qt::Key_VolumeMute});
    connect(muteMicAction, &QAction::triggered, this, [this]() {
        auto defaultSource = PulseAudioQt::Context::instance()->server()->defaultSource();
        if (!defaultSource) {
            return;
        }
        const bool toMute = !defaultSource->isMuted();
        defaultSource->setMuted(toMute);
        showMicMute(toMute ? 0 : volumePercent(defaultSource->volume()));
    });

    for (const auto action : actions) {
        action->setProperty("componentName", u"kmix"_s);
        action->setProperty("componentDisplayName", i18n("Audio Volume"));
        KGlobalAccel::setGlobalShortcut(action, action->shortcuts());
    }
}

qint64 AudioShortcutsService::boundVolume(qint64 volume, int maxVolume)
{
    return qMax(PulseAudioQt::minimumVolume(), qMin(volume, maxVolume));
}

int AudioShortcutsService::volumePercent(qint64 volume)
{
    return std::round((double)volume / PulseAudioQt::normalVolume() * 100.0);
}

QString AudioShortcutsService::nameForDevice(const PulseAudioQt::Device *device)
{
    if (!device) {
        return i18n("No such device");
    }

    const QString nodeNick = device->pulseProperties()["node.nick"].toString();
    if (!nodeNick.isEmpty()) {
        return nodeNick;
    }

    if (!device->description().isEmpty()) {
        return device->description();
    }

    if (!device->name().isEmpty()) {
        return device->name();
    }

    return i18n("Device name not found");
}

int AudioShortcutsService::changeVolumePercent(PulseAudioQt::Device *device, int deltaPercent)
{
    const qint64 oldVolume = device->volume();
    const int oldPercent = volumePercent(oldVolume);
    const int targetPercent = oldPercent + deltaPercent;
    const int maxVolume = PulseAudioQt::normalVolume() * (m_globalConfig->raiseMaximumVolume() ? 150 : 100) / 100.0;
    const qint64 newVolume = boundVolume(std::round(PulseAudioQt::normalVolume() * (targetPercent / 100.f)), maxVolume);
    const int newPercent = volumePercent(newVolume);
    device->setMuted(newPercent == 0);
    device->setVolume(newVolume);
    return newPercent;
}

void AudioShortcutsService::handleDefaultSinkChange()
{
    const PulseAudioQt::Sink *defaultSink = PulseAudioQt::Context::instance()->server()->defaultSink();
    // we don't want to show the OSD on startup
    if (!m_initialDefaultSinkSet) {
        m_initialDefaultSinkSet = true;
        return;
    }
    if (!m_globalConfig->defaultOutputDeviceOsd()) {
        return;
    }
    if (!defaultSink) {
        return;
    }
    QString description = nameForDevice(defaultSink);
    if (defaultSink->name() == DUMMY_OUTPUT_NAME) {
        description = i18n("No output device");
    } else {
        auto cardIdx = m_cardModel->index(-1, 0);
        for (int i = 0; i < m_cardModel->rowCount(); i++) {
            const auto idx = m_cardModel->index(i, 0);
            if (m_cardModel->data(idx, m_cardModel->role("Index"_ba)) == defaultSink->cardIndex()) {
                cardIdx = idx;
            }
        }
        if (cardIdx.isValid()) {
            const QVariantMap cardProperties = m_cardModel->data(cardIdx, m_cardModel->role("Properties"_ba)).toMap();
            bool convOk = false;
            const int cardBluetoothBattery = cardProperties[u"bluetooth.battery"_s].toString().remove('%').toInt(&convOk);
            if (convOk) {
                description = i18nc("Device name (Battery percent)", "%1 (%2% Battery)", description, cardBluetoothBattery);
            }
        }
        QString icon = AudioIcon::forFormFactor(defaultSink->formFactor());
        if (icon.isEmpty()) {
            if (defaultSink->name() == DUMMY_OUTPUT_NAME) {
                icon = u"audio-volume-muted"_s;
            } else {
                icon = AudioIcon::forVolume(volumePercent(defaultSink->volume()), defaultSink->isMuted(), QString());
            }
        }
        m_osdDBusInterface->showText(icon, description);
    }
}

void AudioShortcutsService::handleNewSink()
{
    if (m_globalConfig->globalMute()) {
        for (int i = 0; i < m_sinkModel->rowCount(); i++) {
            m_sinkModel->setData(m_sinkModel->index(i, 0), true, m_sinkModel->role("Muted"_ba));
        }
    }
}

void AudioShortcutsService::muteVolume()
{
    auto sink = m_preferredDevice.sink();
    if (!sink || sink->name() == DUMMY_OUTPUT_NAME) {
        return;
    }
    if (!sink->isMuted()) {
        enableGlobalMute();
        showMute(0);
    } else {
        if (m_globalConfig->globalMute()) {
            disableGlobalMute();
        }
        sink->setMuted(false);
        showMute(sink->volume());
        playFeedback(-1);
    }
}

void AudioShortcutsService::enableGlobalMute()
{
    QStringList alreadyMutedDevices;
    for (int i = 0; i < m_sinkModel->rowCount(); i++) {
        const auto idx = m_sinkModel->index(i, 0);
        const bool isAlreadyMuted = m_sinkModel->data(idx, m_sinkModel->role("Muted"_ba)).toBool();
        const QString name = m_sinkModel->data(idx, m_sinkModel->role("Name"_ba)).toString();
        const QString activePortIdx = QString::number(m_sinkModel->data(idx, m_sinkModel->role("ActivePortIndex"_ba)).toUInt());
        if (isAlreadyMuted) {
            alreadyMutedDevices.append(name % "." % activePortIdx);
        } else {
            m_sinkModel->setData(idx, true, m_sinkModel->role("Muted"_ba));
        }
    }
    if (alreadyMutedDevices.length() == m_sinkModel->rowCount()) {
        alreadyMutedDevices.clear();
    }
    m_globalConfig->setGlobalMuteDevices(alreadyMutedDevices);
    m_globalConfig->setGlobalMute(true);
    m_globalConfig->save();
    showMute(0);
}

void AudioShortcutsService::disableGlobalMute()
{
    QStringList keepMutedDevices = m_globalConfig->globalMuteDevices();
    for (int i = 0; i < m_sinkModel->rowCount(); i++) {
        const auto idx = m_sinkModel->index(i, 0);
        const QString name = m_sinkModel->data(idx, m_sinkModel->role("Name"_ba)).toString();
        const QString activePortIdx = QString::number(m_sinkModel->data(idx, m_sinkModel->role("ActivePortIndex"_ba)).toUInt());
        const QString deviceId = name % "." % activePortIdx;
        if (!keepMutedDevices.contains(deviceId)) {
            m_sinkModel->setData(idx, false, m_sinkModel->role("Muted"_ba));
        }
    }
    m_globalConfig->setGlobalMute(false);
    m_globalConfig->setGlobalMuteDevices({});
    m_globalConfig->save();
    auto sink = m_preferredDevice.sink();
    if (sink) {
        showMute(volumePercent(sink->volume()));
        playFeedback(-1);
    }
}

void AudioShortcutsService::playFeedback(int sinkIdx)
{
    if (!m_globalConfig->audioFeedback()) {
        return;
    }
    auto sink = m_preferredDevice.sink();
    if (sinkIdx == -1 && sink) {
        sinkIdx = sink->index();
    }
    m_feedback->play(sinkIdx);
}

void AudioShortcutsService::showMute(int percent)
{
    if (!m_globalConfig->muteOsd()) {
        return;
    }
    m_osdDBusInterface->volumeChanged(percent, m_globalConfig->raiseMaximumVolume() ? 150 : 100);
}

void AudioShortcutsService::showVolume(int percent)
{
    if (!m_globalConfig->volumeOsd()) {
        return;
    }
    m_osdDBusInterface->volumeChanged(percent, m_globalConfig->raiseMaximumVolume() ? 150 : 100);
}

void AudioShortcutsService::showMicVolume(int percent)
{
    if (!m_globalConfig->microphoneSensitivityOsd()) {
        return;
    }
    m_osdDBusInterface->microphoneVolumeChanged(percent);
}

void AudioShortcutsService::showMicMute(int percent)
{
    if (!m_globalConfig->muteOsd()) {
        return;
    }
    m_osdDBusInterface->microphoneVolumeChanged(percent);
}

#include "audioshortcutsservice.moc"
