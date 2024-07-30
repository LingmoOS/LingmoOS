#include "docbookxslt.h"
#include "docbookxslt_p.h"

#ifdef Q_OS_WIN
// one of the xslt/xml headers pulls in windows.h and breaks <limits>
#define NOMINMAX
#include <QHash>
#endif

#include "../config-kdoctools.h"
#include "loggingcategory.h"

#include <libxml/catalog.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlsave.h>
#include <libxslt/transform.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltconfig.h>
#include <libxslt/xsltutils.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QList>
#include <QStandardPaths>
#include <QString>
#include <QUrl>

#if !defined(SIMPLE_XSLT)
extern HelpProtocol *slave;
#define INFO(x)                                                                                                                                                \
    if (slave)                                                                                                                                                 \
        slave->infoMessage(x);
#else
#define INFO(x)
#endif

int writeToQString(void *context, const char *buffer, int len)
{
    QString *t = (QString *)context;
    *t += QString::fromUtf8(buffer, len);
    return len;
}

#if defined(SIMPLE_XSLT) && defined(Q_OS_WIN)

#define MAX_PATHS 64
xmlExternalEntityLoader defaultEntityLoader = NULL;
static xmlChar *paths[MAX_PATHS + 1];
static int nbpaths = 0;
static QHash<QString, QString> replaceURLList;

/*
 * Entity loading control and customization.
 * taken from xsltproc.c
 */
static xmlParserInputPtr xsltprocExternalEntityLoader(const char *_URL, const char *ID, xmlParserCtxtPtr ctxt)
{
    xmlParserInputPtr ret;
    warningSAXFunc warning = NULL;

    // use local available dtd versions instead of fetching it every time from the internet
    QString url = QLatin1String(_URL);
    QHash<QString, QString>::const_iterator i;
    for (i = replaceURLList.constBegin(); i != replaceURLList.constEnd(); i++) {
        if (url.startsWith(i.key())) {
            url.replace(i.key(), i.value());
            qCDebug(KDocToolsLog) << "converted" << _URL << "to" << url;
        }
    }
    char URL[1024];
    strcpy(URL, url.toLatin1().constData());

    const char *lastsegment = URL;
    const char *iter = URL;

    if (nbpaths > 0) {
        while (*iter != 0) {
            if (*iter == '/') {
                lastsegment = iter + 1;
            }
            iter++;
        }
    }

    if ((ctxt != NULL) && (ctxt->sax != NULL)) {
        warning = ctxt->sax->warning;
        ctxt->sax->warning = NULL;
    }

    if (defaultEntityLoader != NULL) {
        ret = defaultEntityLoader(URL, ID, ctxt);
        if (ret != NULL) {
            if (warning != NULL) {
                ctxt->sax->warning = warning;
            }
            qCDebug(KDocToolsLog) << "Loaded URL=\"" << URL << "\" ID=\"" << ID << "\"";
            return (ret);
        }
    }
    for (int i = 0; i < nbpaths; i++) {
        xmlChar *newURL;

        newURL = xmlStrdup((const xmlChar *)paths[i]);
        newURL = xmlStrcat(newURL, (const xmlChar *)"/");
        newURL = xmlStrcat(newURL, (const xmlChar *)lastsegment);
        if (newURL != NULL) {
            ret = defaultEntityLoader((const char *)newURL, ID, ctxt);
            if (ret != NULL) {
                if (warning != NULL) {
                    ctxt->sax->warning = warning;
                }
                qCDebug(KDocToolsLog) << "Loaded URL=\"" << newURL << "\" ID=\"" << ID << "\"";
                xmlFree(newURL);
                return (ret);
            }
            xmlFree(newURL);
        }
    }
    if (warning != NULL) {
        ctxt->sax->warning = warning;
        if (URL != NULL) {
            warning(ctxt, "failed to load external entity \"%s\"\n", URL);
        } else if (ID != NULL) {
            warning(ctxt, "failed to load external entity \"%s\"\n", ID);
        }
    }
    return (NULL);
}
#endif

QString KDocTools::transform(const QString &pat, const QString &tss, const QList<const char *> &params)
{
    QString parsed;

    INFO(i18n("Parsing stylesheet"));
#if defined(SIMPLE_XSLT) && defined(Q_OS_WIN)
    // prepare use of local available dtd versions instead of fetching every time from the internet
    // this approach is url based
    if (!defaultEntityLoader) {
        defaultEntityLoader = xmlGetExternalEntityLoader();
        xmlSetExternalEntityLoader(xsltprocExternalEntityLoader);

        replaceURLList[QLatin1String("http://www.oasis-open.org/docbook/xml/4.5")] = QString("file:///%1").arg(DOCBOOK_XML_CURRDTD);
    }
#endif

    xsltStylesheetPtr style_sheet = xsltParseStylesheetFile((const xmlChar *)QFile::encodeName(tss).constData());

    if (!style_sheet) {
        return parsed;
    }
    if (style_sheet->indent == 1) {
        xmlIndentTreeOutput = 1;
    } else {
        xmlIndentTreeOutput = 0;
    }

    INFO(i18n("Parsing document"));

    xmlParserCtxtPtr pctxt;

    pctxt = xmlNewParserCtxt();
    if (pctxt == nullptr) {
        return parsed;
    }

    xmlDocPtr doc = xmlCtxtReadFile(pctxt, QFile::encodeName(pat).constData(), nullptr, XML_PARSE_NOENT | XML_PARSE_DTDLOAD | XML_PARSE_NONET);
    /* Clean the context pointer, now useless */
    const bool context_valid = (pctxt->valid == 0);
    xmlFreeParserCtxt(pctxt);

    /* Check both the returned doc (for parsing errors) and the context
       (for validation errors) */
    if (doc == nullptr) {
        return parsed;
    } else {
        if (context_valid) {
            xmlFreeDoc(doc);
            return parsed;
        }
    }

    INFO(i18n("Applying stylesheet"));
    QList<const char *> p = params;
    p.append(nullptr);
    xmlDocPtr res = xsltApplyStylesheet(style_sheet, doc, const_cast<const char **>(&p[0]));
    xmlFreeDoc(doc);
    if (res != nullptr) {
        xmlOutputBufferPtr outp = xmlOutputBufferCreateIO(writeToQString, nullptr, &parsed, nullptr);
        outp->written = 0;
        INFO(i18n("Writing document"));
        xsltSaveResultTo(outp, res, style_sheet);
        xmlOutputBufferClose(outp);
        xmlFreeDoc(res);
    }
    xsltFreeStylesheet(style_sheet);

    if (parsed.isEmpty()) {
        parsed = QLatin1Char(' '); // avoid error message
    }
    return parsed;
}

/*
xmlParserInputPtr meinExternalEntityLoader(const char *URL, const char *ID,
                       xmlParserCtxtPtr ctxt) {
    xmlParserInputPtr ret = NULL;

    // fprintf(stderr, "loading %s %s %s\n", URL, ID, ctxt->directory);

    if (URL == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,
                    "failed to load external entity \"%s\"\n", ID);
        return(NULL);
    }
    if (!qstrcmp(ID, "-//OASIS//DTD DocBook XML V4.1.2//EN"))
        URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";
    if (!qstrcmp(ID, "-//OASIS//DTD XML DocBook V4.1.2//EN"))
    URL = "docbook/xml-dtd-4.1.2/docbookx.dtd";

    QString file;
    if (QFile::exists( QDir::currentPath() + "/" + URL ) )
        file = QDir::currentPath() + "/" + URL;
    else
        file = locate("dtd", URL);

    ret = xmlNewInputFromFile(ctxt, file.toLatin1().constData());
    if (ret == NULL) {
        if ((ctxt->sax != NULL) && (ctxt->sax->warning != NULL))
            ctxt->sax->warning(ctxt,

                "failed to load external entity \"%s\"\n", URL);
    }
    return(ret);
}
*/

QString splitOut(const QString &parsed, int index)
{
    int start_index = index + 1;
    while (parsed.at(start_index - 1) != QLatin1Char('>')) {
        start_index++;
    }

    int inside = 0;

    QString filedata;

    while (true) {
        int endindex = parsed.indexOf(QStringLiteral("</FILENAME>"), index);
        int startindex = parsed.indexOf(QStringLiteral("<FILENAME "), index) + 1;

        // qCDebug(KDocToolsLog) << "FILENAME " << startindex << " " << endindex << " " << inside << " " << parsed.mid(startindex + 18, 15)<< " " <<
        // parsed.length();

        if (startindex > 0) {
            if (startindex < endindex) {
                // qCDebug(KDocToolsLog) << "finding another";
                index = startindex + 8;
                inside++;
            } else {
                index = endindex + 8;
                inside--;
            }
        } else {
            inside--;
            index = endindex + 1;
        }

        if (inside == 0) {
            filedata = parsed.mid(start_index, endindex - start_index);
            break;
        }
    }

    index = filedata.indexOf(QStringLiteral("<FILENAME "));

    if (index > 0) {
        int endindex = filedata.lastIndexOf(QStringLiteral("</FILENAME>"));
        while (filedata.at(endindex) != QLatin1Char('>')) {
            endindex++;
        }
        endindex++;
        filedata = filedata.left(index) + filedata.mid(endindex);
    }

    return filedata;
}

QByteArray fromUnicode(const QString &data)
{
    return data.toUtf8();
}

void replaceCharsetHeader(QString &output)
{
    // may be required for all xml output
    if (output.contains("<table-of-contents>"))
        output.replace(QLatin1String("<?xml version=\"1.0\"?>"), QLatin1String("<?xml version=\"1.0\" encoding=\"utf-8\"?>"));
}

QByteArray KDocTools::extractFileToBuffer(const QString &content, const QString &filename)
{
    const int index = content.indexOf(QLatin1String("<FILENAME filename=\"%1\"").arg(filename));
    if (index == -1) {
        if (filename == QLatin1String("index.html")) {
            return fromUnicode(content);
        } else {
            return QByteArray(); // null value, not just empty
        }
    }
    QString data_file = splitOut(content, index);
    replaceCharsetHeader(data_file);
    return fromUnicode(data_file);
}

class DtdStandardDirs
{
public:
    QString srcdir;
};

Q_GLOBAL_STATIC(DtdStandardDirs, s_dtdDirs)

void KDocTools::setupStandardDirs(const QString &srcdir)
{
    QByteArray catalogs;

    if (srcdir.isEmpty()) {
        catalogs += getKDocToolsCatalogs().join(" ").toLocal8Bit();
    } else {
        catalogs += QUrl::fromLocalFile(srcdir + QStringLiteral("/customization/catalog.xml")).toEncoded();
        s_dtdDirs()->srcdir = srcdir;
    }
    // qCDebug(KDocToolsLog) << "XML_CATALOG_FILES: " << catalogs;
    qputenv("XML_CATALOG_FILES", catalogs);
    xmlInitializeCatalog();
#if defined(_MSC_VER)
    /* Workaround: apparently setting XML_CATALOG_FILES set here
       has no effect on the libxml2 functions.
       This code path could be used in all cases instead of setting the
       variable, but this requires more investigation on the reason of
       the issue. */
    xmlLoadCatalogs(catalogs.constData());
#endif
}

QString KDocTools::locateFileInDtdResource(const QString &file, const QStandardPaths::LocateOptions option)
{
    const QStringList lst = locateFilesInDtdResource(file, option);
    return lst.isEmpty() ? QString() : lst.first();
}

QStringList locateFilesInDtdResource(const QString &file, const QStandardPaths::LocateOptions option)
{
    QFileInfo info(file);
    if (info.exists() && info.isAbsolute()) {
        return QStringList() << file;
    }

    const QString srcdir = s_dtdDirs()->srcdir;
    if (!srcdir.isEmpty()) {
        const QString test = srcdir + QLatin1Char('/') + file;
        if (QFile::exists(test)) {
            return QStringList() << test;
        }
        qCDebug(KDocToolsLog) << "Could not locate file" << file << "in" << srcdir;
        return QStringList();
    }
    // Using locateAll() is necessary to be able to find all catalogs when
    // running in environments where every repository is installed in its own
    // prefix.
    // This is the case on build.kde.org where kdelibs4support installs catalogs
    // in a different prefix than kdoctools.
    const QString fileName = QStringLiteral("kf6/kdoctools/") + file;
    QStringList result = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, fileName, option);

    // fallback to stuff installed with KDocTools
    const QFileInfo fileInInstallDataDir(QStringLiteral(KDOCTOOLS_INSTALL_DATADIR_KF) + QStringLiteral("/kdoctools/") + file);
    if (fileInInstallDataDir.exists()) {
        if ((option == QStandardPaths::LocateFile) && fileInInstallDataDir.isFile()) {
            result.append(fileInInstallDataDir.absoluteFilePath());
        }
        if ((option == QStandardPaths::LocateDirectory) && fileInInstallDataDir.isDir()) {
            result.append(fileInInstallDataDir.absoluteFilePath());
        }
    }

    if (result.isEmpty()) {
        qCDebug(KDocToolsLog) << "Could not locate file" << fileName << "in" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    }
    return result;
}

QStringList getKDocToolsCatalogs()
{
    // Find all catalogs as catalog*.xml, and add them to the list, starting
    // from catalog.xml (the main one).
    const QStringList dirNames = locateFilesInDtdResource(QStringLiteral("customization"), QStandardPaths::LocateDirectory);
    if (dirNames.isEmpty()) {
        return QStringList();
    }
    QStringList catalogFiles;
    for (const QString &customizationDirName : dirNames) {
        QDir customizationDir = QDir(customizationDirName);
        const QStringList catalogFileFilters(QStringLiteral("catalog*.xml"));
        const QFileInfoList catalogInfoFiles = customizationDir.entryInfoList(catalogFileFilters, QDir::Files, QDir::Name);
        for (const QFileInfo &fileInfo : catalogInfoFiles) {
            const QString fullFileName = QUrl::fromLocalFile(fileInfo.absoluteFilePath()).toEncoded();
            if (fileInfo.fileName() == QStringLiteral("catalog.xml")) {
                catalogFiles.prepend(fullFileName);
            } else {
                catalogFiles.append(fullFileName);
            }
        }
    }

    return catalogFiles;
}

QStringList KDocTools::documentationDirs()
{
    /* List of paths containing documentation */
    return QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("doc/HTML"), QStandardPaths::LocateDirectory);
}
