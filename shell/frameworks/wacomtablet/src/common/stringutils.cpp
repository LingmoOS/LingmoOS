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

#include "stringutils.h"

#include <QRect>
#include <QStringList>

using namespace Wacom;

bool StringUtils::asBool(const QString &value)
{
    QString trimmedValue = value.trimmed();

    return (trimmedValue.compare(QLatin1String("1")) == 0 || trimmedValue.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0
            || trimmedValue.compare(QLatin1String("on"), Qt::CaseInsensitive) == 0 || trimmedValue.compare(QLatin1String("yes"), Qt::CaseInsensitive) == 0);
}

const QString StringUtils::fromQRect(const QRect &rect, bool returnCoordinates)
{
    QString area;

    if (returnCoordinates) {
        area = QString::fromLatin1("%1 %2 %3 %4").arg(rect.x()).arg(rect.y()).arg(rect.x() + rect.width()).arg(rect.y() + rect.height());
    } else {
        area = QString::fromLatin1("%1 %2 %3 %4").arg(rect.x()).arg(rect.y()).arg(rect.width()).arg(rect.height());
    }

    return area;
}

const QRect StringUtils::toQRect(const QString &value, bool allowOnlyPositiveValues)
{
    QRect rect;
    QStringList rectValues = value.split(QLatin1String(" "), Qt::SkipEmptyParts);

    if (rectValues.count() != 4) {
        return rect;
    }

    bool xOk, yOk, widthOk, heightOk;
    int x = rectValues.at(0).toInt(&xOk);
    int y = rectValues.at(1).toInt(&yOk);
    int width = rectValues.at(2).toInt(&widthOk);
    int height = rectValues.at(3).toInt(&heightOk);

    if (!xOk || !yOk || !widthOk || !heightOk) {
        return rect;
    }

    if (allowOnlyPositiveValues && (x < 0 || y < 0 || width < 0 || height < 0)) {
        return rect;
    }

    rect.setX(x);
    rect.setY(y);
    rect.setWidth(width);
    rect.setHeight(height);

    return rect;
}

const QRect StringUtils::toQRectByCoordinates(const QString &value, bool allowOnlyPositiveValues)
{
    QRect rect;
    QStringList rectValues = value.split(QLatin1String(" "), Qt::SkipEmptyParts);

    if (rectValues.count() != 4) {
        return rect;
    }

    bool x1Ok, y1Ok, x2Ok, y2Ok;
    int x1 = rectValues.at(0).toInt(&x1Ok);
    int y1 = rectValues.at(1).toInt(&y1Ok);
    int x2 = rectValues.at(2).toInt(&x2Ok);
    int y2 = rectValues.at(3).toInt(&y2Ok);

    if (!x1Ok || !y1Ok || !x2Ok || !y2Ok) {
        return rect;
    }

    if (allowOnlyPositiveValues && (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)) {
        return rect;
    }

    rect.setX(x1);
    rect.setY(y1);
    rect.setWidth(x2 - x1);
    rect.setHeight(y2 - y1);

    return rect;
}
