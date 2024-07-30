/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CPPGENERATOR_H
#define CPPGENERATOR_H

#include <QTextStream>

class BluezApiParser;
class Method;

class CppGenerator
{
public:
    struct Config {
        bool useOptional = false;
        bool useDeprecated = false;
        bool useExperimental = false;
        bool useLowercaseFileNames = false;
        QString author = QStringLiteral("John Doe <info@mail.com>");
        QString year = QStringLiteral("2019");
    };
    CppGenerator(const Config &config);

    bool generate(const BluezApiParser &parser);

protected:
    void writeAdaptorHeader(const BluezApiParser &parser);
    void writeAdaptorSource(const BluezApiParser &parser);

    static QString interfaceToClassName(const QString &interface);
    static QString lowerFirstChars(const QString &string);
    static void writeFooter(QTextStream &stream);
    static void writeInterface(QTextStream &stream, const QString &name);
    static void closeInterface(QTextStream &stream);
    static bool writeMethod(QTextStream &stream, const Method &method);
    static bool writeArg(QTextStream &stream, const QString &param, const QString &dir);
    static void writeAnnotation(QTextStream &stream, const QString &param, const QString &dir, int i);

    void writeCopyrightHeader(QTextStream &stream);

private:
    Config m_config;
};

#endif // CPPGENERATOR_H
