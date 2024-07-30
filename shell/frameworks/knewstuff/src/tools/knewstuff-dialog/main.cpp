/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "knsrcmodel.h"

#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("knewstuff-dialog"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    KLocalizedString::setApplicationDomain("knewstuff-dialog");

    QCommandLineParser *parser = new QCommandLineParser;
    parser->addHelpOption();
    parser->addPositionalArgument(QStringLiteral("knsrcfile"),
                                  i18n("The KNSRC file you want to show. If none is passed, you will be presented with a dialog which lets you switch between "
                                       "all the config files installed into the systemwide knsrc file location"));
    parser->addOption(
        QCommandLineOption(QStringLiteral("url"),
                           i18n("A kns url to show information from. The format for a kns url is kns://knsrcfile/providerid/entryid\n'knsrcfile'\nis the name "
                                "of a knsrc file as might be passed directly through this tool's knsrcfile argument\n'providerid'\nis the hostname of the "
                                "provider the entry should exist on\n'entryid'\nis the unique ID of an entry found in the provider specified by the knsrc "
                                "file.\n An example of such a url is kns://sddmtheme.knsrc/api.kde-look.org/2059021"),
                           QStringLiteral("knsurl")));
    parser->process(app);

    QQmlApplicationEngine *appengine = new QQmlApplicationEngine();
    qmlRegisterType<KNSRCModel>("org.kde.newstuff.tools.dialog", 1, 0, "KNSRCModel");
    auto *context = new KLocalizedContext(appengine);
    context->setTranslationDomain(QStringLiteral("knewstuff6"));
    appengine->rootContext()->setContextObject(context);

    if (parser->optionNames().contains(QStringLiteral("url"))) {
        const QUrl url(parser->value(QStringLiteral("url")));
        Q_ASSERT(url.isValid());
        Q_ASSERT(url.scheme() == QLatin1String("kns"));

        const QString knsrcfile{url.host()};

        const QStringList pathParts = url.path().split(QLatin1Char('/'), Qt::SkipEmptyParts);
        const QString providerId = pathParts.at(0);
        const QString entryId = pathParts.at(1);
        if (pathParts.size() != 2) {
            qWarning() << "wrong format in the url path" << url << pathParts;
        }
        Q_ASSERT(!providerId.isEmpty() && !entryId.isEmpty());

        appengine->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), knsrcfile);
        appengine->rootContext()->setContextProperty(QStringLiteral("knsProviderId"), providerId);
        appengine->rootContext()->setContextProperty(QStringLiteral("knsEntryId"), entryId);
        appengine->load(QStringLiteral("qrc:/qml/dialog.qml"));
    } else if (!parser->positionalArguments().isEmpty()) {
        appengine->rootContext()->setContextProperty(QStringLiteral("knsrcfile"), parser->positionalArguments().first());
        appengine->load(QStringLiteral("qrc:/qml/dialog.qml"));
    } else {
        appengine->load(QStringLiteral("qrc:/qml/main.qml"));
    }

    return app.exec();
}
