/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "icon-helper.h"

#include <QFile>
#include <QSize>
#include <qt5xdg/XdgIcon>
#include <QDebug>

namespace LingmoUIQuick {

QIcon IconHelper::getDefaultIcon()
{
    QIcon icon;
    loadDefaultIcon(icon);
    return icon;
}

QIcon IconHelper::loadIcon(const QString &id)
{
    QIcon icon;
    if (id.isEmpty()) {
        loadDefaultIcon(icon);
        return icon;
    }

    bool isOk = false;
    QString path = toLocalPath(id);
    if (!path.isEmpty()) {
        QPixmap pixmap;
        isOk = loadPixmap(path, pixmap);
        if (isOk) {
            icon.addPixmap(pixmap);
        }

    } else {
        isOk = loadThemeIcon(id, icon);
        if (!isOk) {
            isOk = loadXdgIcon(id, icon);
        }
    }

    if (!isOk) {
        loadDefaultIcon(icon);
    }

    return icon;
}

bool IconHelper::loadPixmap(const QString &path, QPixmap &pixmap)
{
    if (!QFile::exists(path)) {
        qWarning() << "Error: loadPixmap, File dose not exists." << path;
        return false;
    }

    return pixmap.load(path);
}

bool IconHelper::loadThemeIcon(const QString &name, QIcon &icon)
{
    if (!QIcon::hasThemeIcon(name)) {
        return false;
    }

    icon = QIcon::fromTheme(name);
    return true;
}

void IconHelper::loadDefaultIcon(QIcon &icon)
{
    if (!loadThemeIcon("application-x-desktop", icon)) {
        QPixmap pixmap;
        if (loadPixmap(":/res/icon/application-x-desktop.png", pixmap)) {
            icon.addPixmap(pixmap);
        }
    }
}

bool IconHelper::loadXdgIcon(const QString &name, QIcon &icon)
{
    icon = XdgIcon::fromTheme(name);
    if (icon.isNull()) {
        qWarning() << "Error: loadXdgIcon, icon dose not exists. name:" << name;
        return false;
    }
    return true;
}

// see: https://doc.qt.io/archives/qt-5.12/qurl.html#details
QString IconHelper::toLocalPath(const QUrl &url)
{
    if (url.isEmpty()) {
        return {};
    }

    // file:
    if (url.isLocalFile()) {
        return url.path();
    }

    QString schema = url.scheme();
    if (schema.isEmpty()) {
        QString path = url.path();
        if (path.startsWith("/") || path.startsWith(":")) {
            return path;
        }

    } else {
        // qrc example: the Path ":/images/cut.png" or the URL "qrc:///images/cut.png"
        // see: https://doc.qt.io/archives/qt-5.12/resources.html
        if (schema == "qrc") {
            //qrc path: :/xxx/xxx.png
            return ":" + url.path();
        }
    }

    return {};
}

bool IconHelper::isRemoteServerFile(const QUrl &url)
{
    if (url.isEmpty() || url.scheme().isEmpty()) {
        return false;
    }

    return url.scheme() == "http" || url.scheme() == "https";
}

bool IconHelper::isThemeIcon(const QString &name)
{
    return QIcon::hasThemeIcon(name) || !XdgIcon::fromTheme(name).isNull();
}

bool IconHelper::isLocalFile(const QUrl &url)
{
    return !toLocalPath(url).isEmpty();
}

} // LingmoUIQuick
