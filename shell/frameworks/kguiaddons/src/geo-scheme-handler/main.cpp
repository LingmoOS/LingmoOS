/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kgeourihandler_p.h"
#include <kguiaddons_version.h>

#include <QCommandLineParser>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QUrl>

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("kde-geo-uri-handler"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KGUIADDONS_VERSION_STRING));

    QGuiApplication app(argc, argv);

    QCommandLineParser parser;
    QCommandLineOption coordTmplOpt(QStringLiteral("coordinate-template"),
                                    QStringLiteral("URL template for coordinate-based access."),
                                    QStringLiteral("coordinate-template"));
    parser.addOption(coordTmplOpt);
    QCommandLineOption queryTmplOpt(QStringLiteral("query-template"), QStringLiteral("URL template for query-based access."), QStringLiteral("query-template"));
    parser.addOption(queryTmplOpt);
    QCommandLineOption fallbackOpt(QStringLiteral("fallback"), QStringLiteral("URL to use in case of errors."), QStringLiteral("fallback-url"));
    parser.addOption(fallbackOpt);
    parser.addPositionalArgument(QStringLiteral("uri"), QStringLiteral("geo: URI to handle"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    KGeoUriHandler handler;
    handler.setCoordinateTemplate(parser.value(coordTmplOpt));
    handler.setQueryTemplate(parser.value(queryTmplOpt));
    handler.setFallbackUrl(parser.value(fallbackOpt));

    const auto args = parser.positionalArguments();
    for (const auto &arg : args) {
        const auto url = handler.handleUri(QUrl(arg));
        QDesktopServices::openUrl(QUrl(url));
    }

    return 0;
}
