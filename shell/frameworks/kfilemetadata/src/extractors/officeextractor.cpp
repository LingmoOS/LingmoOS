/*
    This file is part of a KMetaData File Extractor
    SPDX-FileCopyrightText: 2013 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "officeextractor.h"
#include "kfilemetadata_debug.h"

#include <QRegularExpression>
#include <QStandardPaths>

#include <QProcess>

using namespace KFileMetaData;

OfficeExtractor::OfficeExtractor(QObject* parent)
    : ExtractorPlugin(parent)
{
    // Find the executables of catdoc, catppt and xls2csv. If an executable cannot
    // be found, indexing its corresponding MIME type will be disabled
    findExe(QStringLiteral("application/msword"), QStringLiteral("catdoc"), m_catdoc);
    findExe(QStringLiteral("application/vnd.ms-excel"), QStringLiteral("xls2csv"), m_xls2csv);
    findExe(QStringLiteral("application/vnd.ms-powerpoint"), QStringLiteral("catppt"), m_catppt);
}

void OfficeExtractor::findExe(const QString& mimeType, const QString& name, QString& fullPath)
{
    fullPath = QStandardPaths::findExecutable(name);

    if (!fullPath.isEmpty()) {
        m_available_mime_types << mimeType;
    }
}

QStringList OfficeExtractor::mimetypes() const
{
    return m_available_mime_types;
}


void OfficeExtractor::extract(ExtractionResult* result)
{
    QStringList args;
    QString contents;

    args << QStringLiteral("-s") << QStringLiteral("cp1252"); // FIXME: Store somewhere a map between the user's language and the encoding of the Windows files it may use ?
    args << QStringLiteral("-d") << QStringLiteral("utf8");

    const QString fileUrl = result->inputUrl();
    const QString mimeType = result->inputMimetype();
    if (mimeType == QLatin1String("application/msword")) {
        result->addType(Type::Document);

        args << QStringLiteral("-w");
        contents = textFromFile(fileUrl, m_catdoc, args);

        // Now that we have the plain text content, count words, lines and characters
        // (original code from plaintextextractor.cpp, authored by Vishesh Handa)
        int lines = contents.count(QLatin1Char('\n'));
        int words = contents.count(QRegularExpression(QStringLiteral("\\b\\w+\\b"), QRegularExpression::UseUnicodePropertiesOption));

        result->add(Property::WordCount, words);
        result->add(Property::LineCount, lines);
    } else if (mimeType == QLatin1String("application/vnd.ms-excel")) {
        result->addType(Type::Document);
        result->addType(Type::Spreadsheet);

        args << QStringLiteral("-c") << QStringLiteral(" ");
        args << QStringLiteral("-b") << QStringLiteral(" ");
        args << QStringLiteral("-q") << QStringLiteral("0");
        contents = textFromFile(fileUrl, m_xls2csv, args);
    } else if (mimeType == QLatin1String("application/vnd.ms-powerpoint")) {
        result->addType(Type::Document);
        result->addType(Type::Presentation);

        contents = textFromFile(fileUrl, m_catppt, args);
    }

    if (contents.isEmpty()) {
        return;
    }

    result->append(contents);

    return;
}

QString OfficeExtractor::textFromFile(const QString& fileUrl, const QString& command, QStringList& arguments)
{
    const QString exec = QStandardPaths::findExecutable(command);
    if (exec.isEmpty()) {
        qCDebug(KFILEMETADATA_LOG) << "Could not find executable in PATH:" << command;
        return {};
    }

    arguments << fileUrl;

    // Start a process and read its standard output
    QProcess process;

    process.setReadChannel(QProcess::StandardOutput);
    process.start(exec, arguments, QIODevice::ReadOnly);
    process.waitForFinished();

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return QString();
    } else {
        return QString::fromUtf8(process.readAll());
    }
}

#include "moc_officeextractor.cpp"
