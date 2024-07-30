/*
 *  SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QQmlEngine>

#include "config.h"

/**
 * Tiny wrapper to make the generated Config class available from QML.
 */
struct QmlConfig {
    Q_GADGET
    QML_FOREIGN(Config)
    QML_SINGLETON
    QML_NAMED_ELEMENT(Config)
};
