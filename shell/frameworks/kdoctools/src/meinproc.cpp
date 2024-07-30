
#include "../config-kdoctools.h"
#include "docbookxslt.h"
#include "docbookxslt_p.h"
#include "loggingcategory.h"
#include "meinproc_common.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QStandardPaths>
#include <QString>

#include <QUrl>

#include <libexslt/exslt.h>
#include <libxml/HTMLtree.h>
#include <libxml/debugXML.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlversion.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltutils.h>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <qplatformdefs.h>
#include <string.h>

using namespace KDocTools;

#ifndef _WIN32
extern "C" int xmlLoadExtDtdDefaultValue;
#endif

#define DIE(x)                                                                                                                                                 \
    do {                                                                                                                                                       \
        qCCritical(KDocToolsLog) << "Error:" << x;                                                                                                             \
        exit(1);                                                                                                                                               \
    } while (0)

int main(int argc, char **argv)
{
    // xsltSetGenericDebugFunc(stderr, NULL);

    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("meinproc"));
    app.setApplicationVersion(QStringLiteral("5.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "KDE Translator for XML"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(
        QCommandLineOption(QStringList() << QStringLiteral("stylesheet"), QCoreApplication::translate("main", "Stylesheet to use"), QStringLiteral("xsl")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("stdout"), QCoreApplication::translate("main", "Output whole document to stdout")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("o") << QStringLiteral("output"),
                                        QCoreApplication::translate("main", "Output whole document to file"),
                                        QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("check"), QCoreApplication::translate("main", "Check the document for validity")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("cache"),
                                        QCoreApplication::translate("main", "Create a cache file for the document"),
                                        QStringLiteral("file")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("srcdir"),
                                        QCoreApplication::translate("main", "Set the srcdir, for kdoctools"),
                                        QStringLiteral("dir")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("param"),
                                        QCoreApplication::translate("main", "Parameters to pass to the stylesheet"),
                                        QStringLiteral("key=value")));
    parser.addPositionalArgument(QStringLiteral("xml"), QCoreApplication::translate("main", "The file to transform"));
    parser.process(app);

    const QStringList filesToRead = parser.positionalArguments();
    if (filesToRead.count() != 1) {
        parser.showHelp();
        return (1);
    }

    exsltRegisterAll();

    // Need to set SRCDIR before calling setupStandardDirs
    QString srcdir;
    if (parser.isSet(QStringLiteral("srcdir"))) {
        srcdir = QDir(parser.value(QStringLiteral("srcdir"))).absolutePath();
    }
    setupStandardDirs(srcdir);

    LIBXML_TEST_VERSION

    const QString checkFilename(filesToRead.first());
    CheckFileResult ckr = checkFile(checkFilename);
    switch (ckr) {
    case CheckFileSuccess:
        break;
    case CheckFileDoesNotExist:
        DIE("File" << checkFilename << "does not exist.");
    case CheckFileIsNotFile:
        DIE(checkFilename << "is not a file.");
    case CheckFileIsNotReadable:
        DIE("File" << checkFilename << "is not readable.");
    }

    if (parser.isSet(QStringLiteral("check"))) {
        QStringList lst = getKDocToolsCatalogs();
        if (lst.isEmpty()) {
            DIE("Could not find kdoctools catalogs");
        }
        QByteArray catalogs = lst.join(" ").toLocal8Bit();
        QString exe;
#if defined(XMLLINT)
        exe = QStringLiteral(XMLLINT);
#endif
        if (!QFileInfo(exe).isExecutable()) {
            exe = QStandardPaths::findExecutable(QStringLiteral("xmllint"));
        }

        CheckResult cr = check(checkFilename, exe, catalogs);
        switch (cr) {
        case CheckSuccess:
            break;
        case CheckNoXmllint:
            DIE("Could not find xmllint");
        case CheckNoOut:
            DIE("`xmllint --noout` outputted text");
        }
    }

    QList<const char *> params;
    {
        const QStringList paramList = parser.values(QStringLiteral("param"));
        for (const QString &tuple : paramList) {
            const int ch = tuple.indexOf(QLatin1Char('='));
            if (ch == -1) {
                DIE("Key-Value tuple" << tuple << "lacks a '='!");
            }
            params.append(qstrdup(tuple.left(ch).toUtf8().constData()));
            params.append(qstrdup(tuple.mid(ch + 1).toUtf8().constData()));
        }
    }
    params.append(nullptr);

    QString tss = parser.value(QStringLiteral("stylesheet"));
    if (tss.isEmpty()) {
        tss = QStringLiteral("customization/kde-chunk.xsl");
    }

    QString tssPath = locateFileInDtdResource(tss);
    if (tssPath.isEmpty()) {
        DIE("Unable to find the stylesheet named" << tss << "in dtd resources");
    }
#ifndef MEINPROC_NO_KARCHIVE
    const QString cache = parser.value(QStringLiteral("cache"));
#else
    if (parser.isSet("cache")) {
        qCWarning(KDocToolsLog) << QCoreApplication::translate(
            "main",
            "The cache option is not available, please re-compile with KArchive support. See MEINPROC_NO_KARCHIVE in KDocTools");
    }
#endif
    const bool usingStdOut = parser.isSet(QStringLiteral("stdout"));
    const bool usingOutput = parser.isSet(QStringLiteral("output"));
    const QString outputOption = parser.value(QStringLiteral("output"));

    QString output = transform(checkFilename, tssPath, params);
    if (output.isEmpty()) {
        DIE("Unable to parse" << checkFilename);
    }

#ifndef MEINPROC_NO_KARCHIVE
    if (!cache.isEmpty()) {
        if (!saveToCache(output, cache)) {
            qCWarning(KDocToolsLog) << QCoreApplication::translate("main", "Could not write to cache file %1.").arg(cache);
        }
        goto end;
    }
#endif

    doOutput(output, usingStdOut, usingOutput, outputOption, true /* replaceCharset */);
#ifndef MEINPROC_NO_KARCHIVE
end:
#endif
    xmlCleanupParser();
    xmlMemoryDump();
    return (0);
}
