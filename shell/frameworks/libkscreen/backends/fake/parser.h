/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QByteArray>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariant>

#include "types.h"

class Parser
{
public:
    static KScreen::ConfigPtr fromJson(const QByteArray &data);
    static KScreen::ConfigPtr fromJson(const QString &path);
    static bool validate(const QByteArray &data);
    static bool validate(const QString &data);

private:
    static void qvariant2qobject(const QVariantMap &variant, QObject *object);
    static KScreen::ScreenPtr screenFromJson(const QMap<QString, QVariant> &data);
    static KScreen::OutputPtr outputFromJson(QMap<QString, QVariant> data /* sic */);
    static KScreen::ModePtr modeFromJson(const QVariant &data);
    static QSize sizeFromJson(const QVariant &data);
    static QRect rectFromJson(const QVariant &data);
    static QPoint pointFromJson(const QVariant &data);
};
