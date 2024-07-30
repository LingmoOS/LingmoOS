/*
 * SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <kcolorschemewatcher.h>

#include <QGuiApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    KColorSchemeWatcher w;

    qDebug() << "System color preference:";

    if (w.systemPreference() == KColorSchemeWatcher::NoPreference) {
        qDebug() << "no preference";
    } else if (w.systemPreference() == KColorSchemeWatcher::PreferDark) {
        qDebug() << "dark";
    }
    if (w.systemPreference() == KColorSchemeWatcher::PreferLight) {
        qDebug() << "light";
    }

    QObject::connect(&w, &KColorSchemeWatcher::systemPreferenceChanged, &app, [&w] {
        qDebug() << "preference changed to:";

        if (w.systemPreference() == KColorSchemeWatcher::NoPreference) {
            qDebug() << "no preference";
        } else if (w.systemPreference() == KColorSchemeWatcher::PreferDark) {
            qDebug() << "dark";
        }
        if (w.systemPreference() == KColorSchemeWatcher::PreferLight) {
            qDebug() << "light";
        }
    });

    return app.exec();
}
