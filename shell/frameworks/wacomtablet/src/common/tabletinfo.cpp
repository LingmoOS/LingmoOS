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

#include "tabletinfo.h"

using namespace Wacom;

/*
 * Instantiate static instances-container of the TabletInfo template specialization.
 * This has to be done here obviously before any instance is created.
 */
template<>
TabletInfoTemplateSpecialization::Container TabletInfoTemplateSpecialization::instances = TabletInfoTemplateSpecialization::Container();

/*
 * Instantiate Properties.
 */
const TabletInfo TabletInfo::ButtonLayout(QLatin1String("ButtonLayout"));
const TabletInfo TabletInfo::CompanyId(QLatin1String("CompanyId"));
const TabletInfo TabletInfo::CompanyName(QLatin1String("CompanyName"));
const TabletInfo TabletInfo::HasLeftTouchStrip(QLatin1String("HasLeftTouchStrip"));
const TabletInfo TabletInfo::HasRightTouchStrip(QLatin1String("HasRightTouchStrip"));
const TabletInfo TabletInfo::HasTouchRing(QLatin1String("HasTouchRing"));
const TabletInfo TabletInfo::HasWheel(QLatin1String("HasWheel"));
const TabletInfo TabletInfo::NumPadButtons(QLatin1String("NumPadButtons"));
const TabletInfo TabletInfo::StatusLEDs(QLatin1String("StatusLEDs"));
const TabletInfo TabletInfo::TabletId(QLatin1String("TabletId"));
const TabletInfo TabletInfo::TabletModel(QLatin1String("TabletModel"));
const TabletInfo TabletInfo::TabletName(QLatin1String("TabletName"));
const TabletInfo TabletInfo::TabletSerial(QLatin1String("TabletSerial"));
const TabletInfo TabletInfo::TouchSensorId(QLatin1String("TouchSensorId"));
const TabletInfo TabletInfo::IsTouchSensor(QLatin1String("IsTouchSensor"));
