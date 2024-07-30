/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SYNDICATION_ATOM_CONSTANTS_H
#define SYNDICATION_ATOM_CONSTANTS_H

#include "../syndication_export.h"

class QString;

namespace Syndication
{
namespace Atom
{
/**
 * namespace used by Atom 1.0 elements
 */
SYNDICATION_EXPORT
QString atom1Namespace();

/**
 * namespace used by Atom 0.3 elements
 */
SYNDICATION_EXPORT
QString atom0_3Namespace();

} // namespace Atom
} // namespace Syndication

#endif // SYNDICATION_ATOM_CONSTANTS_H
