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

#ifndef SCREENMAP_H
#define SCREENMAP_H

#include "screenrotation.h"
#include "screenspace.h"
#include "tabletarea.h"

#include <QString>

namespace Wacom
{

class ScreenMapPrivate;

/**
 * @brief Contains device mappings for each screen
 *
 * Can be (de)serialized to be stored in the configuration file
 */
class ScreenMap
{
public:
    explicit ScreenMap(const TabletArea &tabletGeometry = TabletArea());
    explicit ScreenMap(const QString &mapping);
    explicit ScreenMap(const ScreenMap &screenMap);

    virtual ~ScreenMap();

    ScreenMap &operator=(const ScreenMap &screenMap);

    void fromString(const QString &mappings);

    const TabletArea getMapping(const ScreenSpace &screen) const;

    const QString getMappingAsString(const ScreenSpace &screen) const;

    void setMapping(const ScreenSpace &screen, const TabletArea &mapping);

    const QString toString() const;

private:
    Q_DECLARE_PRIVATE(ScreenMap)
    ScreenMapPrivate *const d_ptr = nullptr;

}; // CLASS
} // NAMESPACE
#endif // HEADER PROTECTION
