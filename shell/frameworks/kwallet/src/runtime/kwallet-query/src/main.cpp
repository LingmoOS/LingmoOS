/*
    This file is part of the KDE
    SPDX-FileCopyrightText: 2015 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QStringList>
#include <iostream>

#include <KAboutData>
#include <KLocalizedString>

#include "querydriver.h"

int main(int argc, char *argv[])
{
    KLocalizedString::setApplicationDomain("kwallet6-query");

    QueryDriver app(argc, argv);

    KAboutData aboutData(QStringLiteral("kwallet-query"),
                         i18n("KWallet query interface"),
                         QStringLiteral("0.1"),
                         i18n("KWallet query interface"),
                         KAboutLicense::GPL,
                         i18n("(c) 2015, The KDE Developers"));

    QCommandLineParser cmdParser;
    // clang-format off
    QCommandLineOption verboseOption(QStringList() << QStringLiteral("v") << QStringLiteral("verbose"), i18n("verbose output"));
    QCommandLineOption listOption(QStringList() << QStringLiteral("l") << QStringLiteral("list-entries"), i18n("list password entries"));
    QCommandLineOption readOption(QStringList() << QStringLiteral("r") << QStringLiteral("read-password"), i18n("reads the secrets from the given <entry>"), i18n("Entry"));
    QCommandLineOption writeOption(QStringList() << QStringLiteral("w") << QStringLiteral("write-password"), i18n("write secrets to the given <entry>. The values are read from the standard input. IMPORTANT: previous wallet entry value will be overwritten!"), i18n("Entry"));
    QCommandLineOption folderOption(QStringList() << QStringLiteral("f") << QStringLiteral("folder"), i18n("specify the folder in the wallet <folder>"), i18n("Folder"));
    // clang-format on

    cmdParser.addHelpOption();
    cmdParser.addPositionalArgument("wallet", i18n("The wallet to query"));
    cmdParser.addOption(listOption);
    cmdParser.addOption(readOption);
    cmdParser.addOption(writeOption);
    cmdParser.addOption(folderOption);
    cmdParser.addOption(verboseOption);
    cmdParser.process(app);

    const QStringList args = cmdParser.positionalArguments();
    if (args.empty()) {
        std::cout << i18n("Missing argument").toStdString() << std::endl;
        return 1;
    }
    if (args.count() > 1) {
        std::cout << i18n("Too many arguments given").toStdString() << std::endl;
        return 1;
    }
    app.setWalletName(args.first());
    if (cmdParser.isSet(listOption) && cmdParser.isSet(readOption) && cmdParser.isSet(writeOption)) {
        std::cout << i18n("Only one mode (list, read or write) can be set. Aborting").toStdString() << std::endl;
        return 1;
    }
    if (!cmdParser.isSet(listOption) && !cmdParser.isSet(readOption) && !cmdParser.isSet(writeOption)) {
        std::cout << i18n("Please specify the mode (list or read).").toStdString() << std::endl;
        return 1;
    }
    if (cmdParser.isSet(listOption)) {
        app.setMode(QueryDriver::List);
    }
    if (cmdParser.isSet(readOption)) {
        app.setEntryName(cmdParser.value(readOption));
        app.setMode(QueryDriver::Read);
    }
    if (cmdParser.isSet(writeOption)) {
        app.setEntryName(cmdParser.value(writeOption));
        app.setMode(QueryDriver::Write);
    }
    if (cmdParser.isSet(folderOption)) {
        app.setEntryFolder(cmdParser.value(folderOption));
    }
    if (cmdParser.isSet(verboseOption)) {
        app.setVerbose();
    }

    return app.exec();
}
