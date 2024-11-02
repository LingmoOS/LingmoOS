/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "main-window-factory.h"
#include "main-window.h"
#include "plugin-manager.h"
#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif
#include "global-settings.h"

#include <QDir>
#include <QtConcurrent>

StableMainWindowFactory *global_instance = nullptr;

Peony::MainWindowFactoryIface *StableMainWindowFactory::getInstance()
{
    if (!global_instance)
        global_instance = new StableMainWindowFactory;
    return global_instance;
}

Peony::FMWindowIface *StableMainWindowFactory::create(const QString &uri)
{
    auto window = new MainWindow(uri);
    return window;
}

Peony::FMWindowIface *StableMainWindowFactory::create(const QStringList &uris)
{
    if (uris.isEmpty())
        return new MainWindow;
    auto uri = uris.first();
    auto l = uris;
    l.removeAt(0);
    auto window = new MainWindow(uri);
    window->addNewTabs(l);
    return window;
}

QWidget *StableMainWindowFactory::createWindow(const QString &uri)
{
    auto window = new MainWindow(uri);
    return window;
}

QWidget *StableMainWindowFactory::createWindow(const QStringList &uris)
{
    if (uris.isEmpty())
        return new MainWindow;
    auto uri = uris.first();
    auto l = uris;
    l.removeAt(0);
    auto window = new MainWindow(uri);
    window->addNewTabs(l);
    return window;
}

QWidget *StableMainWindowFactory::createWindow(const QString &uri, QStringList selectUris)
{
    auto window = new MainWindow(uri);
    connect(window, &MainWindow::locationChangeEnd, [=]() {
        Q_EMIT window->setSelection(selectUris);
    });
    return window;
}

StableMainWindowFactory::StableMainWindowFactory(QObject *parent) : Peony::MainWindowFactoryIface()
{

}
