/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2024 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 *
 */
#ifndef CALAMARES_COMPAT_XML_H
#define CALAMARES_COMPAT_XML_H

#include <QCheckBox>

namespace Calamares
{

#if QT_VERSION < QT_VERSION_CHECK( 6, 7, 0 )
using checkBoxStateType = int;
const auto checkBoxStateChangedSignal = &QCheckBox::stateChanged;
constexpr checkBoxStateType checkBoxUncheckedValue = 0;
#else
using checkBoxStateType = Qt::CheckState;
const auto checkBoxStateChangedSignal = &QCheckBox::checkStateChanged;
constexpr checkBoxStateType checkBoxUncheckedValue = Qt::Unchecked;
#endif

}  // namespace Calamares

#endif
