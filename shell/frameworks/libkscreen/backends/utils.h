/*
 *  SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "output.h"

namespace Utils
{
KScreen::Output::Type guessOutputType(const QString &type, const QString &name);

}
