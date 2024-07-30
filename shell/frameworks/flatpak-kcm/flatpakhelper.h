/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QList>
#include <QString>
#include <QUrl>

namespace FlatpakHelper
{

QString userBaseDirectory();
QString systemBaseDirectory();

QString userOverridesDirectory();
QString systemOverridesDirectory();

QString metadataPathForUserInstallation(const QString &flatpakName);
QString metadataPathForSystemInstallation(const QString &flatpakName);

QUrl iconSourceUrl(const QString &displayName, const QString &flatpakName, const QString &appBaseDirectory);

// Port of flatpak_verify_dbus_name static/internal function.
bool verifyDBusName(QStringView name);
}
