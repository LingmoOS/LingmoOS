// SPDX-FileCopyrightText: 2023 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef KLIPPERUTILS_H_
#define KLIPPERUTILS_H_

namespace PlasmaPass
{
namespace KlipperUtils
{

enum class State { Unknown, Missing, Available, SupportsPasswordManagerHint };

State getState();

} // namespace KlipperUtils
} // namespace PlasmaPass

#endif // KLIPPERUTILS_H_