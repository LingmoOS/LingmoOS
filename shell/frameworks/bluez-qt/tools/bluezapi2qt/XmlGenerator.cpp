/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "XmlGenerator.h"

#include <QDebug>
#include <QFile>

#include "BluezApiParser.h"
#include "TypeAnnotation.h"

XmlGenerator::XmlGenerator(const Config &config)
    : m_config(config)
{
}

bool XmlGenerator::generate(const BluezApiParser &parser)
{
    // Iterate interfaces
    for (const auto &interface : parser.interfaces()) {
        // Only consider interfaces with methods
        if (interface.methods().methods().empty()) {
            continue;
        }

        // Create file
        QFile file(interface.name() + QStringLiteral(".xml"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Error opening file for writing:" << file.fileName();
            return false;
        }

        // Write content
        QTextStream stream(&file);
        writeHeader(stream);
        writeInterface(stream, interface.name());

        // Iterate methods
        for (const auto &method : interface.methods().methods()) {
            // Respect config
            if ((method.tags().isDeprecated && !m_config.useDeprecated) || (method.tags().isOptional && !m_config.useOptional)
                || (method.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }

            writeMethod(stream, method);
        }

        closeInterface(stream);
        writeFooter(stream);
        file.close();
    }

    return true;
}

void XmlGenerator::writeHeader(QTextStream &stream)
{
    stream << "<?xml version=\"1.0\"?>\n";
    stream << "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\" "
              "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n";
    stream << "<node>\n";
}

void XmlGenerator::writeFooter(QTextStream &stream)
{
    stream << "</node>\n";
}

void XmlGenerator::writeInterface(QTextStream &stream, const QString &name)
{
    stream << "  <interface name=\"" << name << "\">\n";
}

void XmlGenerator::closeInterface(QTextStream &stream)
{
    stream << "  </interface>\n";
}

bool XmlGenerator::writeMethod(QTextStream &stream, const Method &method)
{
    stream << "    <method name=\"" << method.name() << "\"";

    // Some beautification
    if (method.inParameters().empty() && method.outParameters().empty()) {
        stream << "/>\n";
        return true;
    }

    stream << ">\n";

    for (const auto &param : method.inParameters()) {
        if (!writeArg(stream, param, QStringLiteral("in"))) {
            return false;
        }
    }
    for (const auto &param : method.outParameters()) {
        if (!writeArg(stream, param, QStringLiteral("out"))) {
            return false;
        }
    }
    for (int i = 0; i < method.inParameters().size(); ++i) {
        writeAnnotation(stream, method.inParameters().at(i), QStringLiteral("In"), i);
    }
    for (int i = 0; i < method.outParameters().size(); ++i) {
        writeAnnotation(stream, method.outParameters().at(i), QStringLiteral("Out"), i);
    }

    stream << "    </method>\n";

    return true;
}

bool XmlGenerator::writeArg(QTextStream &stream, const Parameter &param, const QString &dir)
{
    auto dbusType = annotateType(AnnotationType::Bluez, AnnotationType::Dbus, param.type());
    if (dbusType.isEmpty()) {
        return false;
    }
    stream << "      <arg name=\"" << param.name() << "\" type=\"" << dbusType << "\" direction=\"" << dir << "\"/>\n";

    return true;
}

void XmlGenerator::writeAnnotation(QTextStream &stream, const Parameter &param, const QString &dir, int i)
{
    auto qtType = annotateType(AnnotationType::Bluez, AnnotationType::Qt, param.type());
    if (qtType.isEmpty()) {
        return;
    }
    stream << "      <annotation name=\"org.qtproject.QtDBus.QtTypeName." << dir << QString::number(i) << "\" value=\"" << qtType << "\"/>\n";

    return;
}
