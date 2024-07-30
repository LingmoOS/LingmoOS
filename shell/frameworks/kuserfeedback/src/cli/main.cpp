/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <kuserfeedback_version.h>

#include <jobs/handshakejob.h>
#include <jobs/productexportjob.h>
#include <jobs/productimportjob.h>
#include <jobs/securityscanjob.h>
#include <rest/restapi.h>
#include <rest/restclient.h>
#include <rest/serverinfo.h>
#include <core/product.h>

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QNetworkReply>

#include <iostream>

using namespace KUserFeedback::Console;

static int jobCount = 0;

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("userfeedbackctl"));
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationVersion(QStringLiteral(KUSERFEEDBACK_VERSION_STRING));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("UserFeedback management tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption serverOpt({ QStringLiteral("server"), QStringLiteral("s") }, QStringLiteral("Server Name"), QStringLiteral("name"));
    parser.addOption(serverOpt);
    QCommandLineOption outputOpt({ QStringLiteral("output"), QStringLiteral("o") }, QStringLiteral("Output path"), QStringLiteral("path"));
    parser.addOption(outputOpt);
    QCommandLineOption forceOpt({ QStringLiteral("force"), QStringLiteral("f") }, QStringLiteral("Force destructive operations"));
    parser.addOption(forceOpt);
    QCommandLineOption urlOpt({ QStringLiteral("url"), QStringLiteral("u") }, QStringLiteral("Server URL"), QStringLiteral("url"));
    parser.addOption(urlOpt);
    QCommandLineOption userOpt({ QStringLiteral("user") }, QStringLiteral("User name"), QStringLiteral("name"));
    parser.addOption(userOpt);
    QCommandLineOption passOpt({ QStringLiteral("password") }, QStringLiteral("Password"), QStringLiteral("pass"));
    parser.addOption(passOpt);
    parser.addPositionalArgument(QStringLiteral("command"), QStringLiteral("Command: add-server, delete-product, delete-server, export-all, export-product, import-product, list-products, list-servers, scan-server"));

    parser.process(app);

    if (parser.positionalArguments().isEmpty())
        parser.showHelp(1);
    const auto cmd = parser.positionalArguments().at(0);
    if (cmd == QLatin1String("list-servers")) {
        const auto servers = ServerInfo::allServerInfoNames();
        for (const auto &n : servers)
            std::cout << qPrintable(n) << std::endl;
        return 0;
    }
    if (!parser.isSet(serverOpt))
        parser.showHelp(1);

    if (cmd == QLatin1String("add-server")) {
        if (!parser.isSet(urlOpt))
            parser.showHelp(1);
        ServerInfo s;
        s.setName(parser.value(serverOpt));
        s.setUrl(QUrl::fromUserInput(parser.value(urlOpt)));
        s.setUserName(parser.value(userOpt));
        s.setPassword(parser.value(passOpt));
        s.save();
        return 0;
    } else if (cmd == QLatin1String("delete-server")) {
        ServerInfo::remove(parser.value(serverOpt));
        return 0;
    }

    const auto server = ServerInfo::load(parser.value(serverOpt));
    if (!server.isValid()) {
        std::cerr << "Invalid server information." << std::endl;
        return 1;
    }

    QFileInfo fi(parser.value(outputOpt));
    if (parser.isSet(outputOpt) && (!fi.isDir() || !fi.isWritable())) {
        std::cerr << "Output path does exist or is not accessible." << std::endl;
        return 1;
    }

    RESTClient restClient;
    QObject::connect(&restClient, &RESTClient::errorMessage, [](const auto &msg) {
        std::cerr << qPrintable(msg) << std::endl;
        QCoreApplication::exit(1);
    });
    restClient.setServerInfo(server);

    if (cmd == QLatin1String("delete-product")) {
        jobCount = parser.positionalArguments().size() - 1;
        if (jobCount <= 0 || !parser.isSet(forceOpt))
            parser.showHelp(1);
        QObject::connect(&restClient, &RESTClient::clientConnected, [&parser, &restClient]() {
            for (int i = 0; i < jobCount; ++i) {
                Product p;
                p.setName(parser.positionalArguments().at(i+1));
                auto reply = RESTApi::deleteProduct(&restClient, p);
                QObject::connect(reply, &QNetworkReply::finished, [reply]() {
                    if (reply->error() != QNetworkReply::NoError)
                        return;
                    --jobCount;
                    if (jobCount == 0)
                        qApp->quit();
                });
            }
        });
    } else if (cmd == QLatin1String("export-all")) {
        if (parser.positionalArguments().size() != 1)
            parser.showHelp(1);
        QObject::connect(&restClient, &RESTClient::clientConnected, [&]() {
            auto reply = RESTApi::listProducts(&restClient);
            QObject::connect(reply, &QNetworkReply::finished, [reply, &parser, &outputOpt, &restClient]() {
                if (reply->error() != QNetworkReply::NoError)
                    return;
                const auto products = Product::fromJson(reply->readAll());
                for (const auto &p : products) {
                    ++jobCount;
                    auto job = new ProductExportJob(p, parser.value(outputOpt), &restClient);
                    QObject::connect(job, &Job::destroyed, []() {
                        --jobCount;
                        if (jobCount == 0)
                            qApp->quit();
                    });
                    QObject::connect(job, &Job::error, [](const auto &msg) {
                        std::cerr << qPrintable(msg) << std::endl;
                    });
                }
            });
        });
    } else if (cmd == QLatin1String("export-product")) {
        if (parser.positionalArguments().size() != 2)
            parser.showHelp(1);
        QObject::connect(&restClient, &RESTClient::clientConnected, [&parser, &outputOpt, &restClient]() {
            auto job = new ProductExportJob(parser.positionalArguments().at(1), parser.value(outputOpt), &restClient);
            QObject::connect(job, &Job::destroyed, qApp, &QCoreApplication::quit);
            QObject::connect(job, &Job::error, [](const auto &msg) {
                std::cerr << qPrintable(msg) << std::endl;
            });
        });
    } else if (cmd == QLatin1String("import-product")) {
        if (parser.positionalArguments().size() != 2)
            parser.showHelp(1);
        QObject::connect(&restClient, &RESTClient::clientConnected, [&parser, &restClient]() {
            auto job = new ProductImportJob(parser.positionalArguments().at(1), &restClient);
            QObject::connect(job, &Job::destroyed, qApp, &QCoreApplication::quit);
            QObject::connect(job, &Job::error, [](const auto &msg) {
                std::cerr << qPrintable(msg) << std::endl;
            });
        });
    } else if (cmd == QLatin1String("list-products")) {
        if (parser.positionalArguments().size() != 1)
            parser.showHelp(1);
        QObject::connect(&restClient, &RESTClient::clientConnected, [&restClient]() {
            auto reply = RESTApi::listProducts(&restClient);
            QObject::connect(reply, &QNetworkReply::finished, [reply]() {
                if (reply->error() != QNetworkReply::NoError)
                    return;
                const auto products = Product::fromJson(reply->readAll());
                for (const auto &p : products)
                    std::cout << qPrintable(p.name()) << std::endl;
                qApp->quit();
            });
        });
    } else if (cmd == QLatin1String("scan-server")) {
        QObject::connect(&restClient, &RESTClient::clientConnected, [&restClient]() {
            auto job = new SecurityScanJob(&restClient);
            QObject::connect(job, &Job::destroyed, qApp, &QCoreApplication::quit);
            QObject::connect(job, &Job::error, [](const auto &msg) {
                std::cerr << qPrintable(msg) << std::endl;
            });
            QObject::connect(job, &Job::info, [](const auto &msg) {
                std::cout << qPrintable(msg) << std::endl;
            });
        });
    } else {
        parser.showHelp(1);
    }

    auto job = new HandshakeJob(&restClient);
    QObject::connect(job, &Job::error, [](const QString &msg) {
        std::cerr << qPrintable(msg) << std::endl;
        QCoreApplication::exit(1);
    });

    return app.exec();
}
