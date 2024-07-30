/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#pragma once

#include <QDBusArgument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

#include "kscreen_export.h"
#include "types.h"

namespace KScreen
{
namespace ConfigSerializer
{
KSCREEN_EXPORT QJsonObject serializePoint(const QPoint &point);
KSCREEN_EXPORT QJsonObject serializeSize(const QSize &size);
template<typename T> KSCREEN_EXPORT QJsonArray serializeList(const QList<T> &list)
{
    QJsonArray arr;
    for (const T &t : list) {
        arr.append(t);
    }
    return arr;
}

KSCREEN_EXPORT QJsonObject serializeConfig(const KScreen::ConfigPtr &config);
KSCREEN_EXPORT QJsonObject serializeOutput(const KScreen::OutputPtr &output);
KSCREEN_EXPORT QJsonObject serializeMode(const KScreen::ModePtr &mode);
KSCREEN_EXPORT QJsonObject serializeScreen(const KScreen::ScreenPtr &screen);

KSCREEN_EXPORT QPoint deserializePoint(const QDBusArgument &map);
KSCREEN_EXPORT QSize deserializeSize(const QDBusArgument &map);
template<typename T> KSCREEN_EXPORT QList<T> deserializeList(const QDBusArgument &arg)
{
    QList<T> list;
    arg.beginArray();
    while (!arg.atEnd()) {
        QVariant v;
        arg >> v;
        list.append(v.value<T>());
    }
    arg.endArray();
    return list;
}
KSCREEN_EXPORT KScreen::ConfigPtr deserializeConfig(const QVariantMap &map);
KSCREEN_EXPORT KScreen::OutputPtr deserializeOutput(const QDBusArgument &output);
KSCREEN_EXPORT KScreen::ModePtr deserializeMode(const QDBusArgument &mode);
KSCREEN_EXPORT KScreen::ScreenPtr deserializeScreen(const QDBusArgument &screen);

}

}
