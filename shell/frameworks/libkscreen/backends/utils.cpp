/*
 *  SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "utils.h"

#include <QList>

KScreen::Output::Type Utils::guessOutputType(const QString &type, const QString &name)
{
    static const auto embedded = {QLatin1String("LVDS"), QLatin1String("IDP"), QLatin1String("EDP"), QLatin1String("LCD"), QLatin1String("DSI")};

    for (const QLatin1String &pre : embedded) {
        if (name.startsWith(pre, Qt::CaseInsensitive)) {
            return KScreen::Output::Panel;
        }
    }

    if (type.contains(QLatin1String("VGA"))) {
        return KScreen::Output::VGA;
    } else if (type.contains(QLatin1String("DVI"))) {
        return KScreen::Output::DVI;
    } else if (type.contains(QLatin1String("DVI-I"))) {
        return KScreen::Output::DVII;
    } else if (type.contains(QLatin1String("DVI-A"))) {
        return KScreen::Output::DVIA;
    } else if (type.contains(QLatin1String("DVI-D"))) {
        return KScreen::Output::DVID;
    } else if (type.contains(QLatin1String("HDMI"))) {
        return KScreen::Output::HDMI;
    } else if (type.contains(QLatin1String("Panel"))) {
        return KScreen::Output::Panel;
    } else if (type.contains(QLatin1String("TV-Composite"))) {
        return KScreen::Output::TVComposite;
    } else if (type.contains(QLatin1String("TV-SVideo"))) {
        return KScreen::Output::TVSVideo;
    } else if (type.contains(QLatin1String("TV-Component"))) {
        return KScreen::Output::TVComponent;
    } else if (type.contains(QLatin1String("TV-SCART"))) {
        return KScreen::Output::TVSCART;
    } else if (type.contains(QLatin1String("TV-C4"))) {
        return KScreen::Output::TVC4;
    } else if (type.contains(QLatin1String("TV"))) {
        return KScreen::Output::TV;
    } else if (type.contains(QLatin1String("DisplayPort")) || type.startsWith(QLatin1String("DP"))) {
        return KScreen::Output::DisplayPort;
    } else if (type.contains(QLatin1String("unknown"))) {
        return KScreen::Output::Unknown;
    } else {
        return KScreen::Output::Unknown;
    }
}
