/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "fm-window-factory.h"
#include "fm-window.h"

Peony::FMWindowFactory *global_instance = nullptr;

Peony::FMWindowFactory::FMWindowFactory(QObject *parent) : QObject(parent)
{

}

Peony::FMWindowFactory *Peony::FMWindowFactory::getInstance()
{
    if (!global_instance)
        global_instance = new FMWindowFactory;
    return global_instance;
}

Peony::FMWindowIface *Peony::FMWindowFactory::create(const QString &uri)
{
    return new FMWindow(uri);
}

Peony::FMWindowIface *Peony::FMWindowFactory::create(const QStringList &uris)
{
    if (uris.isEmpty())
        return nullptr;
    auto window = new FMWindow(uris.first());
    QStringList l;
    for (auto uri : uris) {
        if (uris.indexOf(uri) != 0) {
            l<<uri;
        }
    }
    if (!l.isEmpty())
        window->addNewTabs(l);
    return window;
}
