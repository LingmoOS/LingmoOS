// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "daudiomanager_daemon.h"
#include "daudiocard_daemon.h"
#include "daudioport_daemon.h"
#include "daudiodevice_daemon.h"
#include "daudiocard.h"
#include "daudiodevice.h"
#include "daudiostream_daemon.h"
#include "daemonhelpers.hpp"

#include <QDBusAbstractInterface>
#include <QDBusReply>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

DAUDIOMANAGER_BEGIN_NAMESPACE

DDaemonAudioManager::DDaemonAudioManager(QObject *parent)
    : DAudioManagerPrivate (parent)
    , m_inter(DDaemonInternal::newAudioInterface2(this))
{
    updateCards();

    updateOutputDevice();
    updateInputStream();

    updateInputDevice();
    updateOutputStream();

    connect(this, &DDaemonAudioManager::CardsChanged, this, [this](const QString &cards) {
        Q_UNUSED(cards);
        updateCards();
    });
    connect(this, &DDaemonAudioManager::CardsWithoutUnavailableChanged, this, [this](const QString &availableCards) {
        Q_UNUSED(availableCards);
        updateCards();
    });
    connect(this, &DDaemonAudioManager::SinkInputsChanged, this, [this](const QList<QDBusObjectPath> &paths) {
        Q_UNUSED(paths);
        updateInputStream();
    });
    connect(this, &DDaemonAudioManager::SinksChanged, this, [this](const QList<QDBusObjectPath> &paths) {
        Q_UNUSED(paths);
        updateOutputDevice();
    });
    connect(this, &DDaemonAudioManager::SourcesChanged, this, [this](const QList<QDBusObjectPath> &paths) {
        Q_UNUSED(paths);
        updateInputDevice();
    });

    connect(this, &DDaemonAudioManager::IncreaseVolumeChanged, this, &DAudioManagerPrivate::increaseVolumeChanged);
    connect(this, &DDaemonAudioManager::ReduceNoiseChanged, this, &DAudioManagerPrivate::reduceNoiseChanged);
    connect(this, &DDaemonAudioManager::MaxUIVolumeChanged, this, &DAudioManagerPrivate::maxVolumeChanged);

    connect(this, &DDaemonAudioManager::DefaultSourceChanged, this, [this](const QDBusObjectPath &path) {
        const auto &deviceName = DDaemonInternal::deviceName(path.path());
        qDebug(amLog()) << "Default input device changed :" << deviceName;
        for (auto item : m_inputDevices) {
            const bool isDefault = item->name() == deviceName;
            item->setDefault(isDefault);
        }
        Q_EMIT defaultInputDeviceChanged(deviceName);
    });
    connect(this, &DDaemonAudioManager::DefaultSinkChanged, this, [this](const QDBusObjectPath &path) {
        const auto &deviceName = DDaemonInternal::deviceName(path.path());
        qDebug(amLog()) << "Default output device changed :" << deviceName;
        for (auto item : m_outputDevices) {
            const bool isDefault = item->name() == deviceName;
            item->setDefault(isDefault);
        }
        Q_EMIT defaultOutputDeviceChanged(deviceName);
    });

    QDBusConnection Connection = QDBusConnection::sessionBus();
    Connection.connect(DDaemonInternal::AudioServiceName,
                       DDaemonInternal::AudioPath,
                       DDaemonInternal::AudioServiceInterface,
                       "PortEnableChanged", this,
                       SLOT(onPortEnableChanged(quint32, const QString &, bool)));
}

DDaemonAudioManager::~DDaemonAudioManager()
{

}

void DDaemonAudioManager::reset()
{
    m_inter->call("Reset");
}

void DDaemonAudioManager::setReConnectionEnabled(const bool enable)
{
    if (!enable) {
        m_inter->call("NoRestartPulseAudio");
    } else {
        qWarning() << "Don't support to switch, it's default action.";
    }
}

bool DDaemonAudioManager::increaseVolume() const
{
    return qdbus_cast<bool>(m_inter->property("IncreaseVolume"));
}

bool DDaemonAudioManager::reduceNoise() const
{
    return qdbus_cast<bool>(m_inter->property("ReduceNoise"));
}

double DDaemonAudioManager::maxVolume() const
{
    return qdbus_cast<double>(m_inter->property("MaxUIVolume"));
}

DExpected<void> DDaemonAudioManager::setIncreaseVolume(bool increaseVolume)
{
    m_inter->setProperty("IncreaseVolume", increaseVolume);
    return {};
}

DExpected<void> DDaemonAudioManager::setReduceNoise(bool reduceNoise)
{
    m_inter->setProperty("ReduceNoise", reduceNoise);
    return {};
}

void DDaemonAudioManager::onPortEnableChanged(quint32 cardIndex, const QString &portName, bool enabled)
{
    qDebug(amLog()) << Q_FUNC_INFO;

    if (auto card = cardById(cardIndex)) {
        if (auto port = card->portByName(portName)) {
            port->DPlatformAudioPort::setEnabled(enabled);
        }
    }
}

void DDaemonAudioManager::updateCards()
{
    qDebug(amLog()) << Q_FUNC_INFO;

    const QString &replyValue = qdbus_cast<QString>(m_inter->property("Cards"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return;
    }

    QList<quint32> newCards;
    QJsonDocument doc = QJsonDocument::fromJson(replyValue.toUtf8());
    QJsonArray jCards = doc.array();
    for (QJsonValue cV : jCards) {
        QJsonObject jCard = cV.toObject();
        const uint cardId = static_cast<uint>(jCard["Id"].toInt());
        const QString cardName = jCard["Name"].toString();
        QJsonArray jPorts = jCard["Ports"].toArray();

        newCards << cardId;
        if (cardById(cardId))
            continue;

        bool isBluetooth = false;
        for (QJsonValue pV : jPorts) {
            QJsonObject jPort = pV.toObject();
            isBluetooth |= jPort["Bluetooth"].toBool();
        }
        DPlatformAudioCard *card = nullptr;
        if (isBluetooth) {
            card = new DDaemonAudioBluetoothCard();
        } else {
            card = new DDaemonAudioCard();
        }
        card->setName(cardName);
        card->setId(cardId);

        for (QJsonValue pV : jPorts) {
            QJsonObject jPort = pV.toObject();
            const QString portName = jPort["Name"].toString();
            const QString portDescription = jPort["Description"].toString();
            const bool isEnabled = jPort["Enabled"].toBool();
            const int direction = jPort["Direction"].toInt();

            auto port = new DDaemonAudioPort(card);
            port->DPlatformAudioPort::setEnabled(isEnabled);
            port->setDirection(direction);
            port->setName(portName);
            port->setDescription(portDescription);
        }
        m_cards.append(QExplicitlySharedDataPointer(card));
    }
    compareAndDestroyCards(newCards);
    updateAvailableOfCardPort();

    Q_EMIT cardsChanged();
}

void DDaemonAudioManager::updateAvailableOfCardPort()
{
    const auto &cardPorts = availableCardAndPorts();
    for (auto card : m_cards) {
        const auto cardId = card->id();
        bool existAvailableCard = cardPorts.contains(cardId);
        for (auto port : card->ports()) {
            const auto avaiable = existAvailableCard && cardPorts[cardId].contains(port->name());
            port->DPlatformAudioPort::setAvailable(avaiable);
        }
    }
}

QMap<quint32, QStringList> DDaemonAudioManager::availableCardAndPorts() const
{
    QMap<quint32, QStringList> result;
    const QString &replyValue = qdbus_cast<QString>(m_inter->property("CardsWithoutUnavailable"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return result;
    }

    QJsonDocument doc = QJsonDocument::fromJson(replyValue.toUtf8());
    QJsonArray jCards = doc.array();
    for (QJsonValue cV : jCards) {
        QJsonObject jCard = cV.toObject();
        const uint cardId = static_cast<uint>(jCard["Id"].toInt());
        QStringList &ports = result[cardId];

        QJsonArray jPorts = jCard["Ports"].toArray();

        for (QJsonValue pV : jPorts) {
            QJsonObject jPort = pV.toObject();
            const QString portName = jPort["Name"].toString();
            ports << portName;
        }
    }
    return result;
}

void DDaemonAudioManager::updateInputDevice()
{
    qDebug(amLog()) << Q_FUNC_INFO;

    auto sinkPaths = qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("Sources"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return;
    }

    QList<QString> newDevices;
    for (auto item : sinkPaths) {
        const auto &path = item.path();
        const auto &deviceName = DDaemonInternal::deviceName(path);
        if (containInputDevice(deviceName))
            continue;

        newDevices << deviceName;
        DPlatformAudioCard *card = nullptr;
        auto inter = DDaemonInternal::audioInterface(path);
        const auto cardId = qdbus_cast<quint32>(inter.property("Card"));
        for (auto item : m_cards) {
            if (item->id() == cardId) {
                card = item.data();
                break;
            }
        }
        if (!card) {
            qWarning() << "Don't find card for the inputDevice" << path;
            return;
        }
        auto device = new DDaemonAudioInputDevice(path, card);
        addInputDevice(device);
    }

    compareAndDestroyInputDevices(newDevices);
}

void DDaemonAudioManager::updateOutputDevice()
{
    qDebug(amLog()) << Q_FUNC_INFO;

    auto sinkPaths = qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("Sinks"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return;
    }

    QList<QString> newDevices;
    for (auto item : sinkPaths) {
        const auto &path = item.path();
        const auto &deviceName = DDaemonInternal::deviceName(path);
        if (containOutputDevice(deviceName))
            continue;

        newDevices << deviceName;
        DPlatformAudioCard *card = nullptr;
        auto inter = DDaemonInternal::audioInterface(path);
        const auto cardId = qdbus_cast<quint32>(inter.property("Card"));
        for (auto item : m_cards) {
            if (item->id() == cardId) {
                card = item.data();
                break;
            }
        }
        auto device = new DDaemonAudioOutputDevice(path, card);
        addOutputDevice(device);
    }
    compareAndDestroyOutputDevices(newDevices);
}

void DDaemonAudioManager::updateOutputStream()
{
    qDebug(amLog()) << Q_FUNC_INFO;

    auto sourceOutputPaths = qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("SourceOutputs"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return;
    }

    QList<QString> nowStreams;
    for (auto item : sourceOutputPaths) {
        const auto &path = item.path();
        const auto streamName = DDaemonInternal::streamName(path);

        nowStreams << streamName;
        DPlatformAudioInputDevice *device = nullptr;
        auto inter = DDaemonInternal::audioInterface(path);
        const auto sinkIndex = qdbus_cast<quint32>(inter.property("SinkIndex"));
        for (auto item : m_inputDevices) {
            if (item->name() == QString::number(sinkIndex)) {
                device = item.data();
                break;
            }
        }
        if (!device) {
            qWarning() << "Don't find inputDevice for the outputStream" << path;
            return;
        }
        if (device->stream(streamName))
            continue;

        auto stream = new DDaemonOutputStream(path, device);
        device->addStream(stream);
    }
    compareAndDestroyInputStreams(nowStreams);
}

void DDaemonAudioManager::updateInputStream()
{
    qDebug(amLog()) << Q_FUNC_INFO;

    auto sinkInputPaths = qdbus_cast<QList<QDBusObjectPath>>(m_inter->property("SinkInputs"));
    if (m_inter->lastError().isValid()) {
        qWarning() << Q_FUNC_INFO << m_inter->lastError();
        return;
    }

    QList<QString> nowStreams;
    for (auto item : sinkInputPaths) {
        const auto &path = item.path();
        const auto streamName = DDaemonInternal::streamName(path);

        nowStreams << streamName;
        DPlatformAudioOutputDevice *device = nullptr;
        auto inter = DDaemonInternal::audioInterface(path, DDaemonInternal::AudioServiceSinkInputInterface);
        const auto sinkIndex = qdbus_cast<quint32>(inter.property("SinkIndex"));
        for (auto item : m_outputDevices) {
            if (item->name() == QString::number(sinkIndex)) {
                device = item.data();
                break;
            }
        }
        if (!device) {
            qWarning() << "Don't find outputDevice for the inputStream" << path;
            return;
        }
        auto stream = new DDaemonInputStream(path, device);
        device->addStream(stream);
    }
    compareAndDestroyOutputStreams(nowStreams);
}

bool DDaemonAudioManager::containInputDevice(const QString &deviceName)
{
    for (auto item : m_inputDevices) {
        if (item->name() == deviceName)
            return true;
    }
    return false;
}

bool DDaemonAudioManager::containOutputDevice(const QString &deviceName)
{
    for (auto item : m_outputDevices) {
        if (item->name() == deviceName)
            return true;
    }
    return false;
}

void DDaemonAudioManager::compareAndDestroyCards(const QList<quint32> &newCards)
{
    QList<DPlatformAudioCard *> removedCards;
    for (auto item : m_cards) {
        if (!newCards.contains(item->id())) {
            removedCards << item.data();
        }
    }
    for (auto card : removedCards) {
        Q_ASSERT(card);
        compareAndDestroyInputDevices(QList<QString>());
        compareAndDestroyOutputDevices(QList<QString>());

        removeCard(card->id());
    }
}

void DDaemonAudioManager::compareAndDestroyInputDevices(const QList<QString> &newDevices)
{
    QList<DPlatformAudioDevice *> removedDevices;
    for (auto item : m_inputDevices) {
        if (!newDevices.contains(item->name())) {
            removedDevices << item.data();
        }
    }
    if (!removedDevices.isEmpty()) {
        for (auto device : removedDevices) {
            Q_ASSERT(device);
            removeInputDevice(device->name());
        }
    }
}

void DDaemonAudioManager::compareAndDestroyOutputDevices(const QList<QString> &newDevices)
{
    QList<DPlatformAudioDevice *> removedDevices;
    for (auto item : m_outputDevices) {
        if (!newDevices.contains(item->name())) {
            removedDevices << item.data();
        }
    }
    if (!removedDevices.isEmpty()) {
        for (auto device : removedDevices) {
            Q_ASSERT(device);
            removeOutputDevice(device->name());
        }
    }
}

void DDaemonAudioManager::compareAndDestroyInputStreams(const QList<QString> &nowStreams)
{
    for (auto item : m_inputDevices) {
        if (auto device = qobject_cast<DDaemonAudioInputDevice *>(item.data())) {
            device->compareAndDestroyStreams(nowStreams);
        }
    }
}

void DDaemonAudioManager::compareAndDestroyOutputStreams(const QList<QString> &nowStreams)
{
    for (auto item : m_outputDevices) {
        if (auto device = qobject_cast<DDaemonAudioOutputDevice *>(item.data())) {
            device->compareAndDestroyStreams(nowStreams);
        }
    }
}

DAUDIOMANAGER_END_NAMESPACE
