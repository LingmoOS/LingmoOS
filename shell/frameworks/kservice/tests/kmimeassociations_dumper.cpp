/*
    SPDX-FileCopyrightText: 2016 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <kmimeassociations_p.h>
#include <kservice.h>
#include <ksycoca_p.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Parses mimeapps.list files and reports results for a mimetype"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("mime"), QStringLiteral("mimetype name"));
    parser.process(app);

    if (parser.positionalArguments().count() != 1) {
        QTextStream(stderr) << "Exactly one mimetype required\n";
        parser.showHelp(1);
    }

    const QString mime = parser.positionalArguments().at(0);

    // To see which files are being parsed, run this command:
    //    strace -e file ./kmimeassociations_dumper inode/directory |& grep mimeapps

    // It should be the same as the output of these commands (assuming $XDG_CURRENT_DESKTOP=KDE)
    // qtpaths --locate-files GenericConfigLocation kde-mimeapps.list
    // qtpaths --locate-files GenericConfigLocation mimeapps.list
    // qtpaths --locate-files ApplicationsLocation kde-mimeapps.list
    // qtpaths --locate-files ApplicationsLocation mimeapps.list

    KOfferHash offers;
    KMimeAssociations mimeAppsParser(offers, KSycocaPrivate::self()->serviceFactory());
    mimeAppsParser.parseAllMimeAppsList();

    QTextStream output(stdout);
    const auto list = offers.offersFor(mime);
    for (const KServiceOffer &offer : list) {
        output << offer.service()->desktopEntryName() << " " << offer.service()->entryPath() << "\n";
    }

    return 0;
}
