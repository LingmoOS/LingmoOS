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

#include "aboutdata.h"

#include <KLocalizedString>

using namespace Wacom;

AboutData::AboutData(const QString &componentName,
                     const QString &displayName,
                     const QString &version,
                     const QString &shortDescription,
                     const QString &otherText)
    : KAboutData(componentName,
                 displayName,
                 version,
                 shortDescription,
                 KAboutLicense::GPL,
                 i18n("(c), 2009,2010 Jörg Ehrichs"),
                 otherText,
                 QLatin1String("https://kde.org/applications/system/org.kde.wacomtablet"))
{
    addAuthor(i18n("Jörg Ehrichs"), i18n("Maintainer"), QLatin1String("joerg.ehrichs@gmx.de"));
    addAuthor(i18n("Alexander Maret-Huskinson"), i18n("Developer"), QLatin1String("alex@maret.de"));
}
