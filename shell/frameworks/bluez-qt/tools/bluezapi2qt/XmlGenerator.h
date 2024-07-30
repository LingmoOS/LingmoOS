/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef XMLGENERATOR_H
#define XMLGENERATOR_H

class BluezApiParser;
class Method;
class Parameter;
class QString;
class QTextStream;

class XmlGenerator
{
public:
    struct Config {
        bool useOptional = false;
        bool useDeprecated = false;
        bool useExperimental = false;
    };
    XmlGenerator(const Config &config);

    bool generate(const BluezApiParser &parser);

private:
    static void writeHeader(QTextStream &stream);
    static void writeFooter(QTextStream &stream);
    static void writeInterface(QTextStream &stream, const QString &name);
    static void closeInterface(QTextStream &stream);
    static bool writeMethod(QTextStream &stream, const Method &method);
    static bool writeArg(QTextStream &stream, const Parameter &param, const QString &dir);
    static void writeAnnotation(QTextStream &stream, const Parameter &param, const QString &dir, int i);

    Config m_config;
};

#endif // XMLGENERATOR_H
