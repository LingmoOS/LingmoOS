/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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

#include <QGuiApplication>

#include "wm-interface.h"
#include "wm-impl-x11.h"
#include "wm-impl-wayland.h"

namespace LingmoUIQuick {

class WMInterface::Private
{
public:
    QWindow *window {nullptr};
};

WMInterface::WMInterface(QWindow *window) : QObject(window), d(new Private)
{
    d->window = window;
}

WMInterface::~WMInterface()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QWindow *WMInterface::window() const
{
    return d->window;
}

bool WMInterface::isWayland()
{
    static bool isWl = QGuiApplication::platformName().startsWith(QStringLiteral("wayland"));
    return isWl;
}

QPoint WMInterface::position() const
{
    return d->window->position();
}

void WMInterface::setPosition(const QPoint &pos)
{
    d->window->setPosition(pos);
}

WMInterface *WManager::getWM(QWindow *window)
{
    // cache
    static QHash<QWindow*, WMInterface*> globalCache = QHash<QWindow*, WMInterface*>();

    auto wm = globalCache.value(window);
    if (!wm) {
        if (WMInterface::isWayland()) {
            wm = new WMImplWayland(window);
        } else {
            wm = new WMImplX11(window);
        }

        globalCache.insert(window, wm);
        QObject::connect(wm, &QObject::destroyed, wm, [window] {
            globalCache.remove(window);
        });
    }

    return wm;
}

} // LingmoUIQuick
