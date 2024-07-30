/*
   SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

   SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <KNotification>

#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    KNotification *notification = new KNotification(QStringLiteral("notification"));
    notification->setComponentName(QStringLiteral("plasma_workspace"));
    notification->setText(QStringLiteral("Hello!"));
    notification->setTitle(QStringLiteral("Yo"));

    KNotificationAction *action = notification->addAction(QStringLiteral("Open it"));
    QObject::connect(action, &KNotificationAction::activated, &app, [notification] {
        qWarning() << "action activated" << notification->xdgActivationToken();
    });

    KNotificationAction *defaultAction = notification->addDefaultAction(QStringLiteral("Aaaa"));
    QObject::connect(defaultAction, &KNotificationAction::activated, &app, [notification] {
        qWarning() << "default action activated" << notification->xdgActivationToken();
    });

    notification->sendEvent();

    return app.exec();
}
