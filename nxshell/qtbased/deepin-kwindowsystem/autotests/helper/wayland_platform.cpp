/*
 *   Copyright 2016 Martin Gräßlin <mgraesslin@kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QGuiApplication>
#include <KWindowSystem>

int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QGuiApplication app(argc, argv);
    if (KWindowSystem::platform() != KWindowSystem::Platform::Wayland) {
        return 1;
    }
    if (!KWindowSystem::isPlatformWayland()) {
        return 1;
    }
    if (KWindowSystem::isPlatformX11()) {
        return 1;
    }
    return 0;
}
