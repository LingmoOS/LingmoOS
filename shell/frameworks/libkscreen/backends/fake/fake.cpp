/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "fake.h"
#include "parser.h"

#include "edid.h"
#include <output.h>

#include <stdlib.h>

#include <QFile>
#include <QTimer>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDBusConnection>

#include "fakebackendadaptor.h"

using namespace KScreen;

Q_LOGGING_CATEGORY(KSCREEN_FAKE, "kscreen.fake")

Fake::Fake()
    : KScreen::AbstractBackend()
{
    QLoggingCategory::setFilterRules(QStringLiteral("kscreen.fake.debug = true"));

    if (qgetenv("KSCREEN_BACKEND_INPROCESS") != QByteArray("1")) {
        QTimer::singleShot(0, this, &Fake::delayedInit);
    }
}

void Fake::init(const QVariantMap &arguments)
{
    if (!mConfig.isNull()) {
        mConfig.clear();
    }

    mConfigFile = arguments[QStringLiteral("TEST_DATA")].toString();

    if (arguments.contains(QStringLiteral("SUPPORTED_FEATURES"))) {
        bool ok = false;
        const int features = arguments[QStringLiteral("SUPPORTED_FEATURES")].toInt(&ok);
        if (ok) {
            mSupportedFeatures = static_cast<KScreen::Config::Features>(features);
        }
    }

    qCDebug(KSCREEN_FAKE) << "Fake profile file:" << mConfigFile << "features" << mSupportedFeatures;
}

void Fake::delayedInit()
{
    new FakeBackendAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/fake"), this);
}

Fake::~Fake()
{
}

QString Fake::name() const
{
    return QStringLiteral("Fake");
}

QString Fake::serviceName() const
{
    return QStringLiteral("org.kde.KScreen.Backend.Fake");
}

ConfigPtr Fake::config() const
{
    if (mConfig.isNull()) {
        mConfig = Parser::fromJson(mConfigFile);
        if (mConfig) {
            mConfig->setSupportedFeatures(mSupportedFeatures);
        }
    }

    return mConfig;
}

void Fake::setConfig(const ConfigPtr &config)
{
    qCDebug(KSCREEN_FAKE) << "set config" << config->outputs();
    mConfig = config->clone();
    Q_EMIT configChanged(mConfig);
}

bool Fake::isValid() const
{
    return true;
}

QByteArray Fake::edid(int outputId) const
{
    Q_UNUSED(outputId);
    QFile file(mConfigFile);
    file.open(QIODevice::ReadOnly);

    const QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject json = jsonDoc.object();

    const QJsonArray outputs = json[QStringLiteral("outputs")].toArray();
    for (const QJsonValue &value : outputs) {
        const QVariantMap output = value.toObject().toVariantMap();
        if (output[QStringLiteral("id")].toInt() != outputId) {
            continue;
        }

        return QByteArray::fromBase64(output[QStringLiteral("edid")].toByteArray());
    }
    return QByteArray();
}

void Fake::setConnected(int outputId, bool connected)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->isConnected() == connected) {
        return;
    }

    output->setConnected(connected);
    qCDebug(KSCREEN_FAKE) << "emitting configChanged in Fake";
    Q_EMIT configChanged(mConfig);
}

void Fake::setEnabled(int outputId, bool enabled)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->isEnabled() == enabled) {
        return;
    }

    output->setEnabled(enabled);
    Q_EMIT configChanged(mConfig);
}

void Fake::setPrimary(int outputId, bool primary)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (!output || output->isPrimary() == primary) {
        return;
    }

    mConfig->setPrimaryOutput(output);
    Q_EMIT configChanged(mConfig);
}

void Fake::setCurrentModeId(int outputId, const QString &modeId)
{
    KScreen::OutputPtr output = config()->output(outputId);
    if (output->currentModeId() == modeId) {
        return;
    }

    output->setCurrentModeId(modeId);
    Q_EMIT configChanged(mConfig);
}

void Fake::setRotation(int outputId, int rotation)
{
    KScreen::OutputPtr output = config()->output(outputId);
    const KScreen::Output::Rotation rot = static_cast<KScreen::Output::Rotation>(rotation);
    if (output->rotation() == rot) {
        return;
    }

    output->setRotation(rot);
    Q_EMIT configChanged(mConfig);
}

void Fake::addOutput(int outputId, const QString &name)
{
    KScreen::OutputPtr output(new KScreen::Output);
    output->setId(outputId);
    output->setName(name);
    mConfig->addOutput(output);
    Q_EMIT configChanged(mConfig);
}

void Fake::removeOutput(int outputId)
{
    mConfig->removeOutput(outputId);
    Q_EMIT configChanged(mConfig);
}

#include "moc_fake.cpp"
