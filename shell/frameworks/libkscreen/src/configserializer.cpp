/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include "configserializer_p.h"

#include "config.h"
#include "kscreen_debug.h"
#include "mode.h"
#include "output.h"
#include "screen.h"

#include <QDBusArgument>
#include <QFile>
#include <QJsonDocument>
#include <QRect>

#include <cstdint>
#include <optional>

using namespace KScreen;

QJsonObject ConfigSerializer::serializePoint(const QPoint &point)
{
    QJsonObject obj;
    obj[QLatin1String("x")] = point.x();
    obj[QLatin1String("y")] = point.y();
    return obj;
}

QJsonObject ConfigSerializer::serializeSize(const QSize &size)
{
    QJsonObject obj;
    obj[QLatin1String("width")] = size.width();
    obj[QLatin1String("height")] = size.height();
    return obj;
}

QJsonObject ConfigSerializer::serializeConfig(const ConfigPtr &config)
{
    QJsonObject obj;

    if (!config) {
        return obj;
    }

    obj[QLatin1String("features")] = static_cast<int>(config->supportedFeatures());

    QJsonArray outputs;
    for (const OutputPtr &output : config->outputs()) {
        outputs.append(serializeOutput(output));
    }
    obj[QLatin1String("outputs")] = outputs;
    if (config->screen()) {
        obj[QLatin1String("screen")] = serializeScreen(config->screen());
    }

    obj[QLatin1String("tabletModeAvailable")] = config->tabletModeAvailable();
    obj[QLatin1String("tabletModeEngaged")] = config->tabletModeEngaged();

    return obj;
}

QJsonObject ConfigSerializer::serializeOutput(const OutputPtr &output)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = output->id();
    obj[QLatin1String("name")] = output->name();
    obj[QLatin1String("type")] = static_cast<int>(output->type());
    obj[QLatin1String("icon")] = output->icon();
    obj[QLatin1String("pos")] = serializePoint(output->pos());
    obj[QLatin1String("scale")] = output->scale();
    obj[QLatin1String("size")] = serializeSize(output->size());
    obj[QLatin1String("rotation")] = static_cast<int>(output->rotation());
    obj[QLatin1String("currentModeId")] = output->currentModeId();
    obj[QLatin1String("preferredModes")] = serializeList(output->preferredModes());
    obj[QLatin1String("connected")] = output->isConnected();
    obj[QLatin1String("followPreferredMode")] = output->followPreferredMode();
    obj[QLatin1String("enabled")] = output->isEnabled();
    obj[QLatin1String("priority")] = static_cast<int>(output->priority());
    obj[QLatin1String("clones")] = serializeList(output->clones());
    // obj[QLatin1String("edid")] = output->edid()->raw();
    obj[QLatin1String("sizeMM")] = serializeSize(output->sizeMm());
    obj[QLatin1String("replicationSource")] = output->replicationSource();

    QJsonArray modes;
    for (const ModePtr &mode : output->modes()) {
        modes.append(serializeMode(mode));
    }
    obj[QLatin1String("modes")] = modes;

    if (output->capabilities() & Output::Capability::Overscan) {
        obj[QLatin1String("overscan")] = static_cast<int>(output->overscan());
    }

    if (output->capabilities() & Output::Capability::Vrr) {
        obj[QLatin1String("vrrPolicy")] = static_cast<int>(output->vrrPolicy());
    }

    if (output->capabilities() & Output::Capability::RgbRange) {
        obj[QLatin1String("rgbRange")] = static_cast<int>(output->rgbRange());
    }

    if (output->capabilities() & Output::Capability::HighDynamicRange) {
        obj[QLatin1String("hdr")] = output->isHdrEnabled();
    }

    if (output->capabilities() & Output::Capability::HighDynamicRange) {
        obj[QLatin1String("sdr-brightness")] = static_cast<int>(output->sdrBrightness());
    }

    if (output->capabilities() & Output::Capability::WideColorGamut) {
        obj[QLatin1String("wcg")] = output->isWcgEnabled();
    }

    return obj;
}

QJsonObject ConfigSerializer::serializeMode(const ModePtr &mode)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = mode->id();
    obj[QLatin1String("name")] = mode->name();
    obj[QLatin1String("size")] = serializeSize(mode->size());
    obj[QLatin1String("refreshRate")] = mode->refreshRate();

    return obj;
}

QJsonObject ConfigSerializer::serializeScreen(const ScreenPtr &screen)
{
    QJsonObject obj;

    obj[QLatin1String("id")] = screen->id();
    obj[QLatin1String("currentSize")] = serializeSize(screen->currentSize());
    obj[QLatin1String("maxSize")] = serializeSize(screen->maxSize());
    obj[QLatin1String("minSize")] = serializeSize(screen->minSize());
    obj[QLatin1String("maxActiveOutputsCount")] = screen->maxActiveOutputsCount();

    return obj;
}

QPoint ConfigSerializer::deserializePoint(const QDBusArgument &arg)
{
    int x = 0, y = 0;
    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1Char('x')) {
            x = value.toInt();
        } else if (key == QLatin1Char('y')) {
            y = value.toInt();
        } else {
            qCWarning(KSCREEN) << "Invalid key in Point map: " << key;
            return QPoint();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return QPoint(x, y);
}

QSize ConfigSerializer::deserializeSize(const QDBusArgument &arg)
{
    int w = 0, h = 0;
    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("width")) {
            w = value.toInt();
        } else if (key == QLatin1String("height")) {
            h = value.toInt();
        } else {
            qCWarning(KSCREEN) << "Invalid key in size struct: " << key;
            return QSize();
        }
        arg.endMapEntry();
    }
    arg.endMap();

    return QSize(w, h);
}

ConfigPtr ConfigSerializer::deserializeConfig(const QVariantMap &map)
{
    ConfigPtr config(new Config);

    if (map.contains(QLatin1String("features"))) {
        config->setSupportedFeatures(static_cast<Config::Features>(map[QStringLiteral("features")].toInt()));
    }

    if (map.contains(QLatin1String("tabletModeAvailable"))) {
        config->setTabletModeAvailable(map[QStringLiteral("tabletModeAvailable")].toBool());
    }
    if (map.contains(QLatin1String("tabletModeEngaged"))) {
        config->setTabletModeEngaged(map[QStringLiteral("tabletModeEngaged")].toBool());
    }

    if (map.contains(QLatin1String("outputs"))) {
        const QDBusArgument &outputsArg = map[QStringLiteral("outputs")].value<QDBusArgument>();
        outputsArg.beginArray();
        OutputList outputs;
        while (!outputsArg.atEnd()) {
            QVariant value;
            outputsArg >> value;
            const KScreen::OutputPtr output = deserializeOutput(value.value<QDBusArgument>());
            if (!output) {
                return ConfigPtr();
            }
            outputs.insert(output->id(), output);
        }
        outputsArg.endArray();
        config->setOutputs(outputs);
    }

    if (map.contains(QLatin1String("screen"))) {
        const QDBusArgument &screenArg = map[QStringLiteral("screen")].value<QDBusArgument>();
        const KScreen::ScreenPtr screen = deserializeScreen(screenArg);
        if (!screen) {
            return ConfigPtr();
        }
        config->setScreen(screen);
    }

    return config;
}

OutputPtr ConfigSerializer::deserializeOutput(const QDBusArgument &arg)
{
    OutputPtr output(new Output);
    std::optional<bool> primary = std::nullopt;
    std::optional<uint32_t> priority = std::nullopt;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("id")) {
            output->setId(value.toInt());
        } else if (key == QLatin1String("name")) {
            output->setName(value.toString());
        } else if (key == QLatin1String("type")) {
            output->setType(static_cast<Output::Type>(value.toInt()));
        } else if (key == QLatin1String("icon")) {
            output->setIcon(value.toString());
        } else if (key == QLatin1String("pos")) {
            output->setPos(deserializePoint(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("scale")) {
            output->setScale(value.toDouble());
        } else if (key == QLatin1String("size")) {
            output->setSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("rotation")) {
            output->setRotation(static_cast<Output::Rotation>(value.toInt()));
        } else if (key == QLatin1String("currentModeId")) {
            output->setCurrentModeId(value.toString());
        } else if (key == QLatin1String("preferredModes")) {
            output->setPreferredModes(deserializeList<QString>(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("connected")) {
            output->setConnected(value.toBool());
        } else if (key == QLatin1String("followPreferredMode")) {
            output->setFollowPreferredMode(value.toBool());
        } else if (key == QLatin1String("enabled")) {
            output->setEnabled(value.toBool());
        } else if (key == QLatin1String("primary")) {
            // primary is deprecated, but if it appears in config for compatibility reason.
            primary = value.toBool();
        } else if (key == QLatin1String("priority")) {
            // "priority" takes precedence over "primary", but we need to
            //  check it after the loop, otherwise it may come before the
            //  primary and get overridden.
            priority = value.toUInt();
        } else if (key == QLatin1String("clones")) {
            output->setClones(deserializeList<int>(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("replicationSource")) {
            output->setReplicationSource(value.toInt());
        } else if (key == QLatin1String("sizeMM")) {
            output->setSizeMm(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("modes")) {
            const QDBusArgument arg = value.value<QDBusArgument>();
            ModeList modes;
            arg.beginArray();
            while (!arg.atEnd()) {
                QVariant value;
                arg >> value;
                const KScreen::ModePtr mode = deserializeMode(value.value<QDBusArgument>());
                if (!mode) {
                    return OutputPtr();
                }
                modes.insert(mode->id(), mode);
            }
            arg.endArray();
            output->setModes(modes);
        } else if (key == QLatin1String("overscan")) {
            output->setOverscan(value.toUInt());
        } else if (key == QLatin1String("vrrPolicy")) {
            output->setVrrPolicy(static_cast<Output::VrrPolicy>(value.toInt()));
        } else if (key == QLatin1String("rgbRange")) {
            output->setRgbRange(static_cast<Output::RgbRange>(value.toInt()));
        } else {
            qCWarning(KSCREEN) << "Invalid key in Output map: " << key;
            return OutputPtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    if (primary.has_value()) {
        output->setPriority(output->isEnabled() ? (primary.value() ? 1 : 2) : 0);
    }
    if (priority.has_value()) {
        output->setPriority(priority.value());
    }
    return output;
}

ModePtr ConfigSerializer::deserializeMode(const QDBusArgument &arg)
{
    ModePtr mode(new Mode);

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant value;
        arg.beginMapEntry();
        arg >> key >> value;

        if (key == QLatin1String("id")) {
            mode->setId(value.toString());
        } else if (key == QLatin1String("name")) {
            mode->setName(value.toString());
        } else if (key == QLatin1String("size")) {
            mode->setSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("refreshRate")) {
            mode->setRefreshRate(value.toFloat());
        } else {
            qCWarning(KSCREEN) << "Invalid key in Mode map: " << key;
            return ModePtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return mode;
}

ScreenPtr ConfigSerializer::deserializeScreen(const QDBusArgument &arg)
{
    ScreenPtr screen(new Screen);

    arg.beginMap();
    QString key;
    QVariant value;
    while (!arg.atEnd()) {
        arg.beginMapEntry();
        arg >> key >> value;
        if (key == QLatin1String("id")) {
            screen->setId(value.toInt());
        } else if (key == QLatin1String("maxActiveOutputsCount")) {
            screen->setMaxActiveOutputsCount(value.toInt());
        } else if (key == QLatin1String("currentSize")) {
            screen->setCurrentSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("maxSize")) {
            screen->setMaxSize(deserializeSize(value.value<QDBusArgument>()));
        } else if (key == QLatin1String("minSize")) {
            screen->setMinSize(deserializeSize(value.value<QDBusArgument>()));
        } else {
            qCWarning(KSCREEN) << "Invalid key in Screen map:" << key;
            return ScreenPtr();
        }
        arg.endMapEntry();
    }
    arg.endMap();
    return screen;
}
