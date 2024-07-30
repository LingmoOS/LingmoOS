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

#include <QString>

#include "devicetype.h"

using namespace Wacom;

/*
 * Instantiate static instances-container of the DeviceType template specialization.
 * This has to be done here obviously before any instance is created.
 */
template<>
DeviceTypeTemplateSpecialization::Container DeviceTypeTemplateSpecialization::instances = DeviceTypeTemplateSpecialization::Container();

bool DeviceType::operator<(const DeviceType &other) const
{
    return (key().compare(other.key(), Qt::CaseInsensitive) < 0);
}

/*
 * Instantiate Device Types.
 */
const DeviceType DeviceType::Cursor(QLatin1String("cursor"));
const DeviceType DeviceType::Eraser(QLatin1String("eraser"));
const DeviceType DeviceType::Pad(QLatin1String("pad"));
const DeviceType DeviceType::Stylus(QLatin1String("stylus"));
const DeviceType DeviceType::Touch(QLatin1String("touch"));
const DeviceType DeviceType::Unknown(QLatin1String("unknown"));
