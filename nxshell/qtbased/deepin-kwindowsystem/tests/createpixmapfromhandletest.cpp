/* This file is part of the KDE libraries

    Copyright 2013 Martin Gräßlin <mgraesslin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <kwindowsystem.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QLabel>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addPositionalArgument(QStringLiteral("WId"),
                                 QStringLiteral("window id for the window to take the icon from"),
                                 QStringLiteral("[WId]"));
    parser.addHelpOption();
    parser.process(app);
    QLabel label;
    label.setMinimumSize(250, 250);
    label.show();
    QString wId = parser.positionalArguments().first();
    label.setPixmap(KWindowSystem::icon(wId.toULongLong(nullptr, 0), 250, 250, false, KWindowSystem::WMHints));
    return app.exec();
}
