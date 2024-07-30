/*
 * SPDX-FileCopyrightText: 2007 Frerich Raabe <raabe@kde.org>
 * SPDX-FileCopyrightText: 2007-2008 Aaron Seigo <aseigo@kde.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <QApplication>
#include <QQmlDebuggingEnabler>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>
#include <QDebug>
#include <QPixmapCache>
#include <QSurfaceFormat>

#include <qcommandlineoption.h>
#include <qcommandlineparser.h>

#include "view.h"

#include <Plasma/Theme>

int main(int argc, char **argv)
{
    QQmlTriviallyDestructibleDebuggingEnabler debugEnabler;

    auto format = QSurfaceFormat::defaultFormat();
    format.setOption(QSurfaceFormat::ResetNotification);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("plasmoidviewer");

    app.setApplicationName(QStringLiteral("plasmoidviewer"));
    app.setApplicationDisplayName(i18n("Plasmoidviewer"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));
    app.setApplicationVersion(PROJECT_VERSION);
    app.setDesktopFileName(QStringLiteral("org.kde.plasmoidviewer"));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("plasma"), app.windowIcon()));

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Run Plasma widgets in their own window"));

    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("c") << QStringLiteral("containment"),
                                        i18n("The name of the containment plugin"),
                                        QStringLiteral("containment"),
                                        QStringLiteral("org.kde.desktopcontainment")));
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("a") << QStringLiteral("applet"), i18n("The name of the applet plugin"), QStringLiteral("applet")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("f") << QStringLiteral("formfactor"),
                                        i18n("The formfactor to use (horizontal, vertical, mediacenter, planar or application)"),
                                        QStringLiteral("formfactor"),
                                        QStringLiteral("planar")));
    parser.addOption(QCommandLineOption(
        QStringList() << QStringLiteral("l") << QStringLiteral("location"),
        i18n("The location constraint to start the Containment with (floating, desktop, fullscreen, topedge, bottomedge, leftedge, rightedge)"),
        QStringLiteral("location"),
        QStringLiteral("floating")));

    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("x") << QStringLiteral("xPosition"),
                                        i18n("Set the x position of the plasmoidviewer on the Plasma desktop"),
                                        QStringLiteral("xPosition")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("y") << QStringLiteral("yPosition"),
                                        i18n("Set the y position of the plasmoidviewer on the Plasma desktop"),
                                        QStringLiteral("yPosition")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("s") << QStringLiteral("size"),
                                        i18n("Set the window size of the plasmoidview"),
                                        QStringLiteral("widthXheight")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("p") << QStringLiteral("pixmapcache"),
                                        i18n("The size in kB to set the pixmap cache to"),
                                        QStringLiteral("size")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("t") << QStringLiteral("theme"),
                                        i18n("The name of the theme which the shell will use"),
                                        QStringLiteral("themeName")));

    parser.addPositionalArgument(QStringLiteral("externalData"), i18n("Data that should be passed to the applet as 'externalData' event"));

    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    KDBusService service(KDBusService::Multiple);

    const QString applet = parser.value("applet");
    if (applet.isEmpty()) {
        qWarning() << "An applet name or path must be specified, e.g. --applet org.kde.plasma.analogclock";
        return 1;
    }

    Plasma::Theme theme;
    if (parser.isSet("theme")) {
        theme.setUseGlobalSettings(false);
        theme.setThemeName(parser.value("theme"));
    }

    View *v = new View(View::createCorona());

    v->addContainment(parser.value("containment"));
    v->addFormFactor(parser.value("formfactor"));
    v->addApplet(applet);
    v->addLocation(parser.value("location"));

    if (parser.isSet("size")) {
        // The size could be 800X640 or 800x640, so always do toLower.
        const QStringList realSize = parser.value("size").toLower().split(QChar('x'));

        // check if the parameter is valid.
        if (!parser.value("size").toLower().contains(QChar('x'))) {
            qWarning() << "The size " + parser.value("size") + " is not valid, the size parameter will be ignored.";
        } else {
            const int realWidth = realSize.at(0).toInt();
            const int realHeight = realSize.at(1).toInt();
            if (realWidth != 0 && realHeight != 0) {
                v->setWidth(realWidth);
                v->setHeight(realHeight);
            }
        }
    }

    if (parser.isSet("xPosition")) {
        v->setX(parser.value("xPosition").toInt());
    }

    if (parser.isSet("yPosition")) {
        v->setY(parser.value("yPosition").toInt());
    }

    if (parser.isSet("pixmapcache")) {
        QPixmapCache::setCacheLimit(parser.value("pixmapcache").toInt());
    }

    // emit externalData event so we you can launch e.g. an icon applet already with a proper URL
    if (parser.positionalArguments().count() == 1) {
        v->emitExternalData(parser.positionalArguments().constFirst());
    }

    v->show();

    return app.exec();
}
