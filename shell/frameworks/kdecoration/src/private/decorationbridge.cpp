/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decorationbridge.h"

Q_DECLARE_METATYPE(Qt::MouseButton)

namespace KDecoration2
{
DecorationBridge::DecorationBridge(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<Qt::MouseButton>();
}

DecorationBridge::~DecorationBridge() = default;

}

#include "moc_decorationbridge.cpp"
