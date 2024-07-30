/*
    SPDX-FileCopyrightText: 2010 George Kiagiadakis <kiagiadakis.george@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "../../parser/backtraceparser.h"
#include "fakebacktracegenerator.h"
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QMetaEnum>
#include <QSharedPointer>
#include <QTextStream>

#include <iostream>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("backtraceparsertest_manual"), i18n("backtraceparsertest_manual"), QStringLiteral("1.0"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addOption(
        QCommandLineOption(QStringLiteral("debugger"), i18n("The debugger name passed to the parser factory"), QStringLiteral("name"), QStringLiteral("gdb")));
    parser.addPositionalArgument(QStringLiteral("file"), i18n("A file containing the backtrace."), QStringLiteral("[file]"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QString debugger = parser.value(QStringLiteral("debugger"));
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp(1);
        return 1;
    }
    const QString file = parser.positionalArguments().constFirst();

    if (!QFile::exists(file)) {
        std::cerr << "The specified file does not exist" << std::endl;
        return 1;
    }

    FakeBacktraceGenerator generator;
    QSharedPointer<BacktraceParser> btparser(BacktraceParser::newParser(debugger));
    btparser->connectToGenerator(&generator);
    generator.sendData(file);

    QMetaEnum metaUsefulness = BacktraceParser::staticMetaObject.enumerator(BacktraceParser::staticMetaObject.indexOfEnumerator("Usefulness"));
    std::cout << "Usefulness: " << qPrintable(metaUsefulness.valueToKey(btparser->backtraceUsefulness())) << std::endl;
    std::cout << "Simplified backtrace:\n" << qPrintable(btparser->simplifiedBacktrace()) << std::endl;
    const QStringList l = static_cast<QStringList>(btparser->librariesWithMissingDebugSymbols());
    std::cout << "Missing dbgsym libs: " << qPrintable(l.join(QLatin1Char(' '))) << std::endl;

    return 0;
}
