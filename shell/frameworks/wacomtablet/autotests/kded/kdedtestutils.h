/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KDEDTESTUTILS_H
#define KDEDTESTUTILS_H

#include "tabletinformation.h"

#include <QString>

namespace Wacom
{
class KdedTestUtils
{
public:
    static void assertTabletInformation(const TabletInformation &expectedInformation, const TabletInformation &actualInformation);

    static const QString getAbsoluteDir(const QString &fileName);

    static const QString getAbsolutePath(const QString &fileName);

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
