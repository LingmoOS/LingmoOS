/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kded_bolt.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(KDEDBolt, "kded_bolt.json")

#include "main.moc"
