/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef LINGMO_QUICK_ICON_PROVIDER_H
#define LINGMO_QUICK_ICON_PROVIDER_H

#include <QIcon>
#include <QPixmap>
#include <QQuickImageProvider>

class QSize;

namespace LingmoUIQuick {

/**
 * 为Image提供图标
 * 注册的schema为：theme
 * see: https://doc.qt.io/archives/qt-5.12/qquickimageprovider.html#details
 */
class IconProvider : public QQuickImageProvider
{
    friend class Icon;
public:
    IconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

} // LingmoUIQuick

#endif // LINGMO_QUICK_ICON_PROVIDER_H
