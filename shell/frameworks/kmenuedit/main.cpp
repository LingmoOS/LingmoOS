/*
 *   SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
 *   SPDX-FileCopyrightText: 2001-2002 Raffaele Sandrini <sandrini@kde.org>
 *   SPDX-FileCopyrightText: 2008 Montel Laurent <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <KAboutData>
#include <KLocalizedString>
#include <KWindowSystem>
#include <kdbusservice.h>

#include "kmenuedit.h"
#include <QApplication>
#include <QCommandLineParser>

class KMenuApplication : public QApplication
{
public:
    KMenuApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
        QCoreApplication::setApplicationName(QStringLiteral("kmenuedit"));
        QCoreApplication::setApplicationVersion(QStringLiteral(PROJECT_VERSION));
        QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
        QApplication::setApplicationDisplayName(i18n("KDE Menu Editor"));
    }
};

int main(int argc, char **argv)
{
    KMenuApplication app(argc, argv);
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kmenuedit"));

    KAboutData aboutData(QStringLiteral("kmenuedit"),
                         i18n("KDE Menu Editor"),
                         QStringLiteral(PROJECT_VERSION),
                         i18n("KDE menu editor"),
                         KAboutLicense::GPL,
                         i18n("(C) 2000-2003, Waldo Bastian, Raffaele Sandrini, Matthias Elter"));
    aboutData.addAuthor(i18n("Waldo Bastian"), i18n("Maintainer"), QStringLiteral("bastian@kde.org"));
    aboutData.addAuthor(i18n("Raffaele Sandrini"), i18n("Previous Maintainer"), QStringLiteral("sandrini@kde.org"));
    aboutData.addAuthor(i18n("Matthias Elter"), i18n("Original Author"), QStringLiteral("elter@kde.org"));
    aboutData.addAuthor(i18n("Montel Laurent"), QString(), QStringLiteral("montel@kde.org"));
    KAboutData::setApplicationData(aboutData);

    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("KDE Menu Editor"));
    parser.addPositionalArgument(QStringLiteral("menu"), i18n("Sub menu to pre-select"), QStringLiteral("[menu]"));
    parser.addPositionalArgument(QStringLiteral("menu-id"), i18n("Menu entry to pre-select"), QStringLiteral("[menu-id]"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kmenuedit")));
    auto *menuEdit = new KMenuEdit();

    auto useArgs = [menuEdit](const QCommandLineParser &parser) {
        const QStringList args = parser.positionalArguments();
        if (!args.isEmpty()) {
            menuEdit->selectMenu(args.at(0));
            if (args.count() > 1) {
                menuEdit->selectMenuEntry(args.at(1));
            }
        }
    };

    useArgs(parser);

    QObject::connect(&service, &KDBusService::activateRequested, menuEdit, [useArgs, &parser, menuEdit](const QStringList &args, const QString & /*workDir*/) {
        parser.parse(args);
        useArgs(parser);

        KWindowSystem::updateStartupId(menuEdit->windowHandle());
        KWindowSystem::activateWindow(menuEdit->windowHandle());
    });

    menuEdit->show();

    return app.exec();
}
