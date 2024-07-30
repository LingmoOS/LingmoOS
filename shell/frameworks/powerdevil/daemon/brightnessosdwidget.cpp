/*
 * brightnessosdwidget.cpp
 * adapted from kdemultimedia/kmix/osdwidget.cpp
 * SPDX-FileCopyrightText: 2009 Aurélien Gâteau <agateau@kde.org>
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2009 Christian Esken <christian.esken@arcor.de>
 * SPDX-FileCopyrightText: 2010 Felix Geyer <debfx-kde@fobos.de>
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "brightnessosdwidget.h"

#include <QDBusInterface>
#include <QDBusPendingCall>

namespace BrightnessOSDWidget
{
void show(int percentage, PowerDevil::BrightnessControlType type)
{
    QString method;
    if (type == PowerDevil::BrightnessControlType::Keyboard) {
        method = QLatin1String("keyboardBrightnessChanged");
    } else {
        method = QLatin1String("brightnessChanged");
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                      QStringLiteral("/org/kde/osdService"),
                                                      QStringLiteral("org.kde.osdService"),
                                                      method);

    msg << percentage;

    QDBusConnection::sessionBus().asyncCall(msg);
}

}
