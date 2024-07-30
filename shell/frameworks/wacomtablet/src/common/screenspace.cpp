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

#include "screenspace.h"

#include "logging.h"
#include "screensinfo.h"

using namespace Wacom;

namespace Wacom
{
static const QString DESKTOP_STRING = QLatin1String("desktop");
static const QString AREA_STRING = QLatin1String("area");
static const QString SPEED_STRING = QLatin1String("speed");
}

ScreenSpace::ScreenSpace()
{
}

ScreenSpace::ScreenSpace(const QString &screenSpaceString)
{
    if (screenSpaceString == DESKTOP_STRING) {
        _type = ScreenSpaceType::Desktop;
        return;
    }

    QStringList tokens = screenSpaceString.split(QChar::fromLatin1('x'));

    if (tokens.size() == 5 && tokens.at(0) == AREA_STRING) {
        _type = ScreenSpaceType::Area;
        _area = QRect(tokens.at(1).toInt(), tokens.at(2).toInt(), tokens.at(3).toInt(), tokens.at(4).toInt());
        return;
    }

    if (tokens.size() == 3 && tokens.at(0) == SPEED_STRING) {
        _type = ScreenSpaceType::ArbitraryTranslationMatrix;
        _speed = QPointF(tokens.at(1).toDouble(), tokens.at(2).toDouble());
        return;
    }

    _type = ScreenSpaceType::Output;
    _output = screenSpaceString;
}

ScreenSpace::~ScreenSpace()
{
}

bool ScreenSpace::operator==(const ScreenSpace &screenSpace) const
{
    if (getType() != screenSpace.getType())
        return false;

    switch (getType()) {
    case Wacom::ScreenSpace::ScreenSpaceType::Output:
        return _output == screenSpace._output;
    case Wacom::ScreenSpace::ScreenSpaceType::Area:
        return _area == screenSpace._area;
    case Wacom::ScreenSpace::ScreenSpaceType::ArbitraryTranslationMatrix:
        return _speed == screenSpace._speed;
    case Wacom::ScreenSpace::ScreenSpaceType::Desktop:
    default:
        return true;
    }
}

const ScreenSpace ScreenSpace::desktop()
{
    return ScreenSpace(DESKTOP_STRING);
}

bool ScreenSpace::isDesktop() const
{
    return _type == ScreenSpaceType::Desktop;
}

bool ScreenSpace::isMonitor() const
{
    return _type == ScreenSpaceType::Output;
}

const ScreenSpace ScreenSpace::monitor(QString output)
{
    return ScreenSpace(output);
}

const ScreenSpace ScreenSpace::matrix(qreal x, qreal y)
{
    return ScreenSpace(QString::fromLatin1("%1x%2x%3").arg(SPEED_STRING).arg(x).arg(y));
}

const ScreenSpace ScreenSpace::area(QRect area)
{
    return ScreenSpace(QString::fromLatin1("%1x%2x%3x%4x%5").arg(AREA_STRING).arg(area.left()).arg(area.top()).arg(area.width()).arg(area.height()));
}

const QString ScreenSpace::toString() const
{
    switch (getType()) {
    case ScreenSpaceType::Desktop:
        return DESKTOP_STRING;
    case ScreenSpaceType::Output:
        return _output;
    case ScreenSpaceType::Area:
        return QString::fromLatin1("%1x%2x%3x%4x%5").arg(AREA_STRING).arg(_area.left()).arg(_area.top()).arg(_area.width()).arg(_area.height());
    case ScreenSpaceType::ArbitraryTranslationMatrix:
        return QString::fromLatin1("%1x%2x%3").arg(SPEED_STRING).arg(_speed.x()).arg(_speed.y());
    default:
        qCDebug(COMMON) << "Broken ScreenSpace serialized";
        return DESKTOP_STRING;
    }
}

QRect ScreenSpace::toScreenGeometry() const
{
    switch (getType()) {
    case ScreenSpaceType::Output: {
        const auto screenList = ScreensInfo::getScreenGeometries();
        return screenList.contains(toString()) ? screenList.value(toString()) : QRect();
    }
    case ScreenSpaceType::Area:
        return getArea();
    case ScreenSpaceType::Desktop:
        return ScreensInfo::getUnifiedDisplayGeometry();
    case ScreenSpaceType::ArbitraryTranslationMatrix:
    default:
        return QRect();
    }
}

ScreenSpace ScreenSpace::next() const
{
    ScreenSpace nextScreen = ScreenSpace::desktop();

    if (getType() != ScreenSpaceType::Output) {
        nextScreen = ScreenSpace::monitor(ScreensInfo::getPrimaryScreenName());
    } else {
        auto nextScreenName = ScreensInfo::getNextScreenName(toString());
        if (nextScreenName == ScreensInfo::getPrimaryScreenName()) {
            nextScreen = ScreenSpace::desktop();
        } else {
            nextScreen = ScreenSpace::monitor(nextScreenName);
        }
    }

    return nextScreen;
}

ScreenSpace::ScreenSpaceType ScreenSpace::getType() const
{
    return _type;
}

QPointF ScreenSpace::getSpeed() const
{
    return _speed;
}

QRect ScreenSpace::getArea() const
{
    return _area;
}
