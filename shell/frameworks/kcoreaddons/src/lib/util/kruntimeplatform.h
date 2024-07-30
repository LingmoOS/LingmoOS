// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QStringList>

#include "kcoreaddons_export.h"

/**
 * @brief Utility functions around the runtime platform.
 */
namespace KRuntimePlatform
{
/**
 * Returns the runtime platform, e.g.\ "desktop" or "tablet, touch". The first entry/ies in the list relate to the platform
 * formfactor and the last is the input method specialization.
 * If the string is empty, there is no specified runtime platform and a traditional desktop environment may be assumed.
 *
 * The value is read using the PLASMA_PLATFORM env variable
 * @since 5.97
 */
KCOREADDONS_EXPORT QStringList runtimePlatform();
}
