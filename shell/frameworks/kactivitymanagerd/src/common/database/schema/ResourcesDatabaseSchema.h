/*
 *   SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "../Database.h"
#include <QStringList>

namespace Common
{
namespace ResourcesDatabaseSchema
{
QString version();

QStringList schema();

QString path();
void overridePath(const QString &path);

void initSchema(Database &database);

} // namespace ResourcesDatabase
} // namespace Common
