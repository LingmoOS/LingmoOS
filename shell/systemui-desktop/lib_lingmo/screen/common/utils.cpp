/*
 * Copyright 2013  Daniel Vrátil <dvratil@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "utils.h"

#include <kscreen/output.h>
#include <kscreen/edid.h>

QString Utils::outputName(const KScreen::OutputPtr& output)
{
    return outputName(output.data());
}

QString Utils::outputName(const KScreen::Output *output)
{
    if (output->type() == KScreen::Output::Panel) {
        return QObject::tr("Laptop Screen");
    }

    if (output->edid()) {
        // The name will be "VendorName ModelName (ConnectorName)",
        // but some components may be empty.
        QString name;
        if (!(output->edid()->vendor().isEmpty())) {
            name = output->edid()->vendor() + QLatin1Char(' ');
        }
        if (!output->edid()->name().isEmpty()) {
            name += output->edid()->name() + QLatin1Char(' ');
        }
        if (!name.trimmed().isEmpty()) {
            return name + QLatin1Char('(') + output->name() + QLatin1Char(')');
        }
    }
    return output->name();
}

QString Utils::sizeToString(const QSize &size)
{
    return QStringLiteral("%1x%2").arg(size.width()).arg(size.height());
}

