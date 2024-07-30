/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "../common/control.h"
#include "../common/globals.h"

#include <kscreen/output.h>
#include <kscreen/types.h>

#include <QOrientationReading>
#include <QVariantMap>

#include <optional>

class Output
{
public:
    static void readInOutputs(KScreen::ConfigPtr config, const QVariantList &outputsInfo);

    static void writeGlobal(const KScreen::OutputPtr &output, bool hasDuplicate);
    static bool writeGlobalPart(const KScreen::OutputPtr &output, QVariantMap &info, const KScreen::OutputPtr &fallback);

    static QString dirPath();

    static bool updateOrientation(KScreen::OutputPtr &output, QOrientationReading::Orientation orientation);

    struct GlobalConfig {
        std::optional<qreal> scale;
        std::optional<QString> modeId;
        std::optional<KScreen::Output::Rotation> rotation;
        std::optional<KScreen::Output::VrrPolicy> vrrPolicy;
        std::optional<uint32_t> overscan;
        std::optional<KScreen::Output::RgbRange> rgbRange;
    };
    static GlobalConfig readGlobal(const KScreen::OutputPtr &output);

private:
    static QVariantMap getGlobalData(KScreen::OutputPtr output);

    static void readIn(KScreen::OutputPtr output, const QVariantMap &info);
    static bool readInGlobal(KScreen::OutputPtr output);
    static void readInGlobalPartFromInfo(KScreen::OutputPtr output, const QVariantMap &info);
    /*
     * When a global output value (scale, rotation) is changed we might
     * need to reposition the outputs when another config is read.
     */
    static void adjustPositions(KScreen::ConfigPtr config, const QVariantList &outputsInfo);

    static QString s_dirName;
};
