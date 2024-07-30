// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#pragma once

#include <QStringList>

class SMARTData;

// Stringifies smartctl exit codes (= failures) into pretty strings fit for
// display to user. Also filters uninteresting failures.
namespace Instabilities
{
QStringList from(const SMARTData &data);
};
