/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kaccess.h"

#include <KAboutData>
#include <KLocalizedString>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <QApplication>
#include <QDebug>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#else
#include <QtGui/private/qtx11extras_p.h>
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kaccess"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kaccessrc"));
    migrate.migrate();
#endif

    qunsetenv("SESSION_MANAGER");

    // this application is currently only relevant on X, force to run under X
    // note if someone does port this we still need to run kaccess under X for xwayland apps
    qputenv("QT_QPA_PLATFORM", "xcb");

    // verify the Xlib has matching XKB extension
    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;
    if (!XkbLibraryVersion(&major, &minor)) {
        qWarning() << "Xlib XKB extension does not match";
        return 1;
    }
    qDebug() << "Xlib XKB extension major=" << major << " minor=" << minor;

    // we need an application object for QX11Info
    QApplication app(argc, argv);

    KAboutData about(QStringLiteral("kaccess"), QString(), i18n("Accessibility"), {}, KAboutLicense::GPL_V2, i18n("(c) 2000, Matthias Hoelzer-Kluepfel"));

    about.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), i18n("Author"), QStringLiteral("hoelzer@kde.org"));
    // set data as used for D-Bus by KAccessApp
    KAboutData::setApplicationData(about);

    KAccessApp acc;
    if (acc.isFailed()) {
        return 1;
    }

    // verify the X server has matching XKB extension
    // if yes, the XKB extension is initialized
    int opcode_rtrn;
    int error_rtrn;
    int xkb_opcode;
    if (!XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn, &major, &minor)) {
        qWarning() << "X server has not matching XKB extension" << Qt::endl;
        return 1;
    }
    qDebug() << "X server XKB extension major=" << major << " minor=" << minor;

    app.installNativeEventFilter(&acc);

    // Without that, the application dies when the dialog is closed only once.
    app.setQuitOnLastWindowClosed(false);

    acc.setXkbOpcode(xkb_opcode);
    return app.exec();
}
