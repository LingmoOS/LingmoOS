/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
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

#include "desktop.h"
#include <QQmlContext>
#include <QQmlEngine>

#include <QGuiApplication>
#include <QDBusServiceWatcher>
#include <QTranslator>

Desktop::Desktop(int& argc, char** argv)
    : QApplication(argc, argv)
{
    for (QScreen *screen : QGuiApplication::screens()) {
        screenAdded(screen);
    }
    setOrganizationName("lingmoos");
    setApplicationName("lingmo-desktop");

    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-filemanager/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(this);
        if (translator->load(qmFilePath)) {
            installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    connect(qApp, &QGuiApplication::screenAdded, this, &Desktop::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved, this, &Desktop::screenRemoved);
}

int Desktop::run()
{
    return QApplication::exec();
}

void Desktop::screenAdded(QScreen *screen)
{
    if (!m_list.contains(screen)) {
        DesktopView *view = new DesktopView(screen);
        view->show();
        m_list.insert(screen, view);
    }
}

void Desktop::screenRemoved(QScreen *screen)
{
    if (m_list.contains(screen)) {
        DesktopView *view = m_list.find(screen).value();
        view->setVisible(false);
        view->deleteLater();
        m_list.remove(screen);
    }
}
