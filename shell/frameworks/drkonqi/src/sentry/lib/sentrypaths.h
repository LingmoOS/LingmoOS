// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <optional>

#include <QString>

namespace SentryPaths
{
QString payloadsDir();
QString sentPayloadsDir();
QString payloadPath(const QString &eventId);
QString sentPayloadPath(const QString &eventId);
} // namespace SentryPaths
