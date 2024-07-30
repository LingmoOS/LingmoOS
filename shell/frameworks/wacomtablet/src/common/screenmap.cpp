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

#include "screenmap.h"

#include "stringutils.h"

#include <QHash>
#include <QStringList>

using namespace Wacom;

namespace Wacom
{
class ScreenMapPrivate
{
public:
    static const QString SCREENAREA_SEPERATOR;
    static const QString SCREEN_SEPERATOR;

    TabletArea tabletGeometry;
    QHash<QString, TabletArea> mappings;
};

const QString ScreenMapPrivate::SCREENAREA_SEPERATOR = QLatin1String(":");
const QString ScreenMapPrivate::SCREEN_SEPERATOR = QLatin1String("|");
}

ScreenMap::ScreenMap(const TabletArea &tabletGeometry)
    : d_ptr(new ScreenMapPrivate)
{
    Q_D(ScreenMap);

    d->tabletGeometry = tabletGeometry;
}

ScreenMap::ScreenMap(const QString &mapping)
    : d_ptr(new ScreenMapPrivate)
{
    fromString(mapping);
}

ScreenMap::ScreenMap(const ScreenMap &screenMap)
    : d_ptr(new ScreenMapPrivate)
{
    operator=(screenMap);
}

ScreenMap::~ScreenMap()
{
    delete this->d_ptr;
}

ScreenMap &ScreenMap::operator=(const ScreenMap &screenMap)
{
    *(this->d_ptr) = *(screenMap.d_ptr);

    return (*this);
}

void ScreenMap::fromString(const QString &mappings)
{
    Q_D(ScreenMap);

    QStringList screenMappings = mappings.split(QLatin1String("|"), Qt::SkipEmptyParts);
    QString separator(QLatin1String(":"));
    QStringList mapping;
    ScreenSpace screen;
    TabletArea tabletArea;

    d->mappings.clear();

    foreach (QString screenMapping, screenMappings) {
        mapping = screenMapping.split(separator, Qt::SkipEmptyParts);

        if (mapping.count() != 2) {
            continue;
        }

        screen = ScreenSpace(mapping.at(0).trimmed());
        tabletArea = TabletArea(mapping.at(1).trimmed());

        setMapping(screen, tabletArea);
    }
}

const TabletArea ScreenMap::getMapping(const ScreenSpace &screen) const
{
    Q_D(const ScreenMap);

    // try to find selection for the current screen
    auto citer = d->mappings.constFind(screen.toString());

    TabletArea area;

    if (citer != d->mappings.constEnd()) {
        area = citer.value();
    } else {
        area = d->tabletGeometry;
    }

    return area;
}

const QString ScreenMap::getMappingAsString(const ScreenSpace &screen) const
{
    return getMapping(screen).toString();
}

void ScreenMap::setMapping(const ScreenSpace &screen, const TabletArea &mapping)
{
    Q_D(ScreenMap);

    if (mapping.isEmpty()) {
        d->mappings.insert(screen.toString(), d->tabletGeometry);
    } else {
        d->mappings.insert(screen.toString(), mapping);
    }
}

const QString ScreenMap::toString() const
{
    Q_D(const ScreenMap);

    // create mapping string
    auto mapping = d->mappings.constBegin();

    QString mappings;
    TabletArea area;

    for (; mapping != d->mappings.constEnd(); ++mapping) {
        area = mapping.value();

        if (!mappings.isEmpty()) {
            mappings.append(ScreenMapPrivate::SCREEN_SEPERATOR);
        }

        mappings.append(QString::fromLatin1("%1%2%3").arg(mapping.key()).arg(ScreenMapPrivate::SCREENAREA_SEPERATOR).arg(area.toString()));
    }

    return mappings;
}
