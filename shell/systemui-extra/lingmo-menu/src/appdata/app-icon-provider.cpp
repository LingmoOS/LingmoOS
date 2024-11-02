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

#include "app-icon-provider.h"

#include <QDebug>
#include <QIcon>
#include <QFile>
#include <qt5xdg/XdgIcon>

namespace LingmoUIMenu {

QSize AppIconProvider::s_defaultSize = QSize(128, 128);

AppIconProvider::AppIconProvider() : QQuickImageProvider(QQmlImageProviderBase::Pixmap)
{
}

QPixmap AppIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return AppIconProvider::getPixmap(id, size, requestedSize);
}

QPixmap AppIconProvider::getPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QIcon icon;
    loadIcon(id, icon);

    QPixmap pixmap = icon.pixmap(requestedSize.isEmpty() ? s_defaultSize : requestedSize);

    if (size) {
        QSize pixmapSize = pixmap.size();
        size->setWidth(pixmapSize.width());
        size->setHeight(pixmapSize.height());
    }

    return pixmap;
}

void AppIconProvider::loadIcon(const QString &id, QIcon &icon)
{
    if (id.isEmpty()) {
        loadDefault(icon);
        return;
    }

    bool isOk;
    QUrl url(id);
    if (!url.scheme().isEmpty()) {
        isOk = loadIconFromUrl(url, icon);

    } else if (id.startsWith(QLatin1String("/")) || id.startsWith(QLatin1String(":/"))) {
        //qrc path: :/xxx/xxx.png
        isOk = loadIconFromPath(id, icon);

    } else {
        isOk = loadIconFromTheme(id, icon);
        if (!isOk) {
            isOk = loadIconFromXdg(id, icon);
        }
    }

    if (!isOk) {
        loadDefault(icon);
    }
}

// see: https://doc.qt.io/archives/qt-5.12/qurl.html#details
bool AppIconProvider::loadIconFromUrl(const QUrl &url, QIcon &icon)
{
    QString path = url.path();
    if (path.isEmpty()) {
        qWarning() << "Error: loadIconFromUrl, path is empty.";
        return false;
    }

    // qrc example: the Path ":/images/cut.png" or the URL "qrc:///images/cut.png"
    // see: https://doc.qt.io/archives/qt-5.12/resources.html
    if (url.scheme() == QLatin1String("qrc")) {
        path.prepend(QLatin1String(":"));
        return loadIconFromPath(path, icon);
    }

    // net?
    return false;
}

bool AppIconProvider::loadIconFromPath(const QString &path, QIcon &icon)
{
    if (!QFile::exists(path)) {
        qWarning() << "Error: loadIconFromPath, File dose not exists." << path;
        return false;
    }
    QPixmap filePixmap;
    bool isOk = filePixmap.load(path);
    if (!isOk) {
        return false;
    }
    icon.addPixmap(filePixmap);

    return true;
}

bool AppIconProvider::loadIconFromTheme(const QString &name, QIcon &icon)
{
    if (!QIcon::hasThemeIcon(name)) {
        return false;
    }
    icon = QIcon::fromTheme(name);
    return true;
}

void AppIconProvider::loadDefault(QIcon &icon)
{
    if (!loadIconFromTheme("application-x-desktop", icon)) {
        loadIconFromPath(":/res/icon/application-x-desktop.png", icon);
    }
}

bool AppIconProvider::loadIconFromXdg(const QString &name, QIcon &icon)
{
    icon = XdgIcon::fromTheme(name);
    if (icon.isNull()) {
        qWarning() << "Error: loadIconFromXdg, icon dose not exists. name:" << name;
        return false;
    }
    return true;
}

} // LingmoUIMenu
