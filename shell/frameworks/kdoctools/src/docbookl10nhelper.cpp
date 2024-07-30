/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2010 Luigi Toscano <luigi.toscano@tiscali.it>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "loggingcategory.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QList>
#include <QPair>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

class LangListType : public QList<QPair<QString, QString>>
{
public:
    int searchLang(const QString &el)
    {
        for (int i = 0; i < size(); ++i) {
            if (at(i).first == el) {
                return i;
            }
        }
        return -1;
    }
};

int writeLangFile(const QString &fname, const QString &dtdPath, const LangListType &langMap)
{
    QFile outFile(fname);
    if (!outFile.open(QIODevice::WriteOnly)) {
        qCCritical(KDocToolsLog) << QStringLiteral("Could not write %1").arg(outFile.fileName());
        return (1);
    }

    QTextStream outStream(&outFile);
    outStream << "<?xml version='1.0'?>\n";
    outStream << QStringLiteral("<!DOCTYPE l:i18n SYSTEM \"%1\">").arg(dtdPath) << QLatin1Char('\n');

    if (!langMap.isEmpty()) {
        outStream << "<l:i18n xmlns:l=\"http://docbook.sourceforge.net/xmlns/l10n/1.0\">" << QLatin1Char('\n');
        LangListType::const_iterator i = langMap.constBegin();
        while (i != langMap.constEnd()) {
            outStream << QStringLiteral("<l:l10n language=\"%1\" href=\"%2\"/>").arg((*i).first).arg((*i).second) << QLatin1Char('\n');
            ++i;
        }
        outStream << "</l:i18n>\n";
    }

    outFile.close();

    return (0);
}

inline const QString addTrailingSlash(const QString &p)
{
    return p.endsWith(QStringLiteral("/")) ? p : p + QStringLiteral("/");
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    const QStringList arguments = app.arguments();
    if (arguments.count() != 4) {
        qCCritical(KDocToolsLog) << "wrong argument count";
        return (1);
    }

    const QString l10nDir = addTrailingSlash(arguments[1]);
    const QString l10nCustomDir = addTrailingSlash(arguments[2]);
    const QString destDir = addTrailingSlash(arguments[3]);

    QFile i18nFile(l10nDir + QStringLiteral("common/l10n.xml"));

    if (!i18nFile.open(QIODevice::ReadOnly)) {
        qCCritical(KDocToolsLog) << i18nFile.fileName() << " not found";
        return (1);
    }

    /*
     * for each language defined in the original l10n.xml, copy
     * it into all-l10n.xml and store it in a list;
     **/
    const QRegularExpression rxDocType(QStringLiteral("^\\s*<!DOCTYPE\\s+l:i18n\\s+SYSTEM\\s+\"l10n\\.dtd\"\\s*>$"));
    const QRegularExpression rxEntity(QStringLiteral("^\\s*<l:l10n language=\"([^\\s]+)\"\\s+href=\"([^\\s]+)\"/>\\s*$"));
    QTextStream inStream(&i18nFile);
    int parsingState = 0;

    LangListType allLangs;

    while (!inStream.atEnd()) {
        QString line = inStream.readLine();

        switch (parsingState) {
        case 0:
            if (rxDocType.match(line).hasMatch()) {
                parsingState = 1;
                // qCDebug(KDocToolsLog) << "DTD found";
            }
            break;
        case 1:
            const QRegularExpressionMatch match = rxEntity.match(line);
            if (match.hasMatch()) {
                const QString langCode = match.captured(1);
                const QString langFile = l10nDir + QStringLiteral("common/") + match.captured(2);
                allLangs += qMakePair(langCode, langFile);
                // qCDebug(KDocToolsLog) << langCode << " - " << langFile;
            }
            break;
        }
    }
    i18nFile.close();

    /* read the list of locally-available custom languages */
    QDir outDir(l10nCustomDir);

    QStringList dirFileFilters;
    dirFileFilters << QStringLiteral("*.xml");
    QStringList customLangFiles = outDir.entryList(dirFileFilters, QDir::Files, QDir::Name);
    /* the following two calls to removeOne should not be needed, as
     * the customization directory from the sources should not contain
     * those files
     */
    customLangFiles.removeOne(QStringLiteral("all-l10n.xml"));
    customLangFiles.removeOne(QStringLiteral("kde-custom-l10n.xml"));
    // qCDebug(KDocToolsLog) << "customLangFiles:" << customLangFiles;

    /*
     * for each custom language (from directory listing), if it is not
     * in the list of upstream languages, add it to all-l10n.xml,
     * otherwise add it to kde-custom-l10n.xml
     */
    QStringList::const_iterator i = customLangFiles.constBegin();
    while (i != customLangFiles.constEnd()) {
        QString langFile = (*i);
        /* remove trailing .xml */
        QString langCode = langFile.left(langFile.length() - 4);

        QPair<QString, QString> cl = qMakePair(langCode, langFile);
        if ((allLangs.searchLang(langCode)) < 0) {
            /* custom language not found in upstream list */
            allLangs += cl;
        }
        ++i;
    }

    const QString all10nFName = destDir + QStringLiteral("all-l10n.xml");
    return writeLangFile(all10nFName, l10nDir + QStringLiteral("common/l10n.dtd"), allLangs);
}
