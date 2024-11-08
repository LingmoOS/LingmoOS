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

#ifndef LINGMO_MENU_APP_ICON_PROVIDER_H
#define LINGMO_MENU_APP_ICON_PROVIDER_H

#include <QSize>
#include <QQuickImageProvider>
#include <QIcon>

namespace LingmoUIMenu {

// see: https://doc.qt.io/archives/qt-5.12/qquickimageprovider.html#details
class AppIconProvider : public QQuickImageProvider
{
public:
    AppIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    static QPixmap getPixmap(const QString &id, QSize *size, const QSize &requestedSize);

private:
    static void loadIcon(const QString &id, QIcon &icon);
    static void loadDefault(QIcon &icon);
    static bool loadIconFromUrl(const QUrl &url, QIcon &icon);
    static bool loadIconFromPath(const QString &path, QIcon &icon);
    static bool loadIconFromTheme(const QString &name, QIcon &icon);
    static bool loadIconFromXdg(const QString &name, QIcon &icon);

private:
    static QSize s_defaultSize;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_ICON_PROVIDER_H
