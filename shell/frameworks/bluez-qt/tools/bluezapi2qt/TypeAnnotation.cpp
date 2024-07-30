/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "TypeAnnotation.h"

#include <vector>

QString annotateType(AnnotationType from, AnnotationType to, const QString &type)
{
    // clang-format off
    static const std::vector<std::vector<std::string>> table = {
        {"boolean",        "b",         ""},
        //{{"fd"},             "",       ""},
        {"object",         "o",         ""},
        {"string",         "s",         ""},
        //{{"uint16"},         "",       ""},
        //{{"uint32"},         "",       ""},
        {"dict",           "a{sv}",     "QVariantMap"},
        {"array{byte}",    "ay",        ""},
        {"array{dict}",    "aa{sv}",    "QVariantMapList"},
        {"array{string}",  "as",        ""},
    };
    // clang-format on

    for (const auto &entry : table) {
        if (entry.at(static_cast<size_t>(from)) == type.toStdString()) {
            return QString::fromStdString(entry.at(static_cast<size_t>(to)));
        }
    }

    return QString();
}

QString bluezToQt(const QString &type)
{
    // clang-format off
    static const std::vector<std::vector<std::string>> table = {
        {"boolean",         "bool"},
        //{{"fd"},           ""},
        {"object",          "QDBusObjectPath"},
        {"string",          "QString"},
        {"dict",            "QVariantMap"},
        {"array{byte}",     "QByteArray"},
        {"array{dict}",     "QVariantMapList"},
        {"array{string}",   "QStringList"},
    };
    // clang-format on

    for (const auto &entry : table) {
        if (entry.front() == type.toStdString()) {
            return QString::fromStdString(entry.back());
        }
    }

    return type;
}
