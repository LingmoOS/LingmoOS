/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "desktopiconprovider.h"
#include <QIcon>

DesktopIconProvider::DesktopIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{

}

QPixmap DesktopIconProvider::requestPixmap(const QString &id, QSize *realSize,
                                           const QSize &requestedSize)
{
    QSize size(requestedSize);
    if (size.width() < 1)
        size.setWidth(1);
    if (size.height() < 1)
        size.setHeight(1);

    if (realSize)
        *realSize = size;

    if (id.startsWith(QLatin1Char('/')))
        return QPixmap(id).scaled(size);

    QIcon icon = QIcon::fromTheme(id);
    if (icon.isNull())
        icon = QIcon::fromTheme(QLatin1String("application-x-desktop"));

    return icon.pixmap(size);
}
