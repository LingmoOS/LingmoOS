/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <KWindowSystem>
#include <QGuiApplication>

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
