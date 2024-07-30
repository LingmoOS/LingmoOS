/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTimer>
#include <knotification.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QGuiApplication>

void notificationDBusCall(const QString &iconName, const QString &title, const QString &body, const QStringList &actions, bool persistent = false)
{
    QDBusMessage dbusNotificationMessage = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.Notifications"),
                                                                          QStringLiteral("/org/freedesktop/Notifications"),
                                                                          QStringLiteral("org.freedesktop.Notifications"),
                                                                          QStringLiteral("Notify"));

    QList<QVariant> args;

    args.append(QString()); // app_name
    args.append((uint)0); // notification to update
    args.append(iconName); // app_icon
    args.append(title); // summary
    args.append(body); // body

    QStringList actionList;
    int actId = 0;
    for (const QString &actionName : actions) {
        actId++;
        actionList.append(QString::number(actId));
        actionList.append(actionName);
    }

    args.append(actionList); // actions

    args.append(QVariantMap()); // hints

    // Persistent     => 0  == infinite timeout
    // CloseOnTimeout => -1 == let the server decide
    int timeout = persistent ? 0 : -1;

    args.append(timeout); // expire timeout

    dbusNotificationMessage.setArguments(args);

    QDBusMessage reply = QDBusConnection::sessionBus().call(dbusNotificationMessage, QDBus::Block, 4000);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "Error sending notification:" << reply.errorMessage();
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    notificationDBusCall(QStringLiteral("amarok"),
                         QStringLiteral("Testing notification #1"),
                         QStringLiteral("Lorem ipsum dolor sit amet, consectetur adipiscing elit. In condimentum"),
                         QStringList() << QStringLiteral("action1") << QStringLiteral("action2"));

    // wait a little before sending another notification
    QEventLoop a;
    QTimer::singleShot(500, &a, &QEventLoop::quit);
    a.exec();

    notificationDBusCall(QStringLiteral("kwalletmanager"),
                         QStringLiteral("Testing notification #2"),
                         QStringLiteral("Praesent odio ipsum, posuere a magna ac, egestas vehicula lectus"),
                         QStringList() << QStringLiteral("action1") << QStringLiteral("action2"));

    QTimer::singleShot(1000, &a, &QEventLoop::quit);
    a.exec();

    notificationDBusCall(QStringLiteral("preferences-desktop-accessibility"),
                         QStringLiteral("Testing notification #3"),
                         QStringLiteral("Fusce hendrerit egestas pellentesque"),
                         QStringList(),
                         true);

    QTimer::singleShot(2000, &app, SLOT(quit()));

    return app.exec();
}
