/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QDebug>
#include <QFile>
#include <QRegularExpression>

#include "CppGenerator.h"

#include "BluezApiParser.h"
#include "TypeAnnotation.h"

CppGenerator::CppGenerator(const Config &config)
    : m_config(config)
{
}

bool CppGenerator::generate(const BluezApiParser &parser)
{
    writeAdaptorHeader(parser);
    writeAdaptorSource(parser);

    return true;
}

void CppGenerator::writeAdaptorHeader(const BluezApiParser &parser)
{
    // Iterate interfaces
    for (const auto &interface : parser.interfaces()) {
        auto className = interfaceToClassName(interface.name());
        const QString includeGuard = QLatin1String("BLUEZQT_") + className.toUpper() + QLatin1String("ADAPTOR_H");

        // Create file
        QFile file(className.toLower() + QStringLiteral("adaptor.h"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Error opening file for writing:" << file.fileName();
            return;
        }

        // Write content
        QTextStream stream(&file);
        writeCopyrightHeader(stream);
        stream << "#ifndef " << includeGuard << "\n";
        stream << "#define " << includeGuard << "\n\n";
        stream << "#include <QDBusAbstractAdaptor>\n\n";
        stream << "class QDBusObjectPath;\n\n";
        stream << "namespace BluezQt\n{\n\n";
        stream << "class " << className << ";\n\n";
        stream << "class " << className << "Adaptor : public QDBusAbstractAdaptor\n{\n";
        stream << "    Q_OBJECT \n";
        stream << "    Q_CLASSINFO(\"D-Bus Interface\", \"" << interface.name() << "\")\n";

        // Write properties
        for (const auto &property : interface.properties().properties()) {
            // Respect config
            if ((property.tags().isOptional && !m_config.useOptional) || (property.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }
            stream << "    Q_PROPERTY(" << bluezToQt(property.type()) << " " << property.name() << " READ " << lowerFirstChars(property.name());
            if (!property.tags().isReadOnly) {
                stream << " WRITE set" << property.name();
            }
            stream << ")\n";
        }

        stream << "\npublic:\n";
        stream << "    explicit " << className << "Adaptor(" << className << "* parent);\n\n";

        // Write property accessors
        for (const auto &property : interface.properties().properties()) {
            // Respect config
            if ((property.tags().isOptional && !m_config.useOptional) || (property.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }
            stream << "    " << bluezToQt(property.type()) << " " << lowerFirstChars(property.name()) << "() const;\n";
            if (!property.tags().isReadOnly) {
                stream << "    void set" << property.name() << "(const " << bluezToQt(property.type()) << " &" << lowerFirstChars(property.name()) << ");\n";
            }
            stream << "\n";
        }

        stream << "public Q_SLOTS:\n";

        // write Methods
        for (const auto &method : interface.methods().methods()) {
            // Respect config
            if ((method.tags().isOptional && !m_config.useOptional) || (method.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }
            stream << "    " << bluezToQt(method.outParameter().type()) << " " << method.name() << "(";
            for (auto it = method.inParameters().begin(); it != method.inParameters().end(); ++it) {
                stream << "const " << bluezToQt(it->type()) << " &" << it->name();
                if (it != std::prev(method.inParameters().end())) {
                    stream << ", ";
                }
            }
            stream << ");\n";
        }

        // write private members
        stream << "\nprivate:\n";
        stream << "    " << className << " *m_" << lowerFirstChars(className) << ";\n";
        stream << "};\n\n} // namespace BluezQt\n\n";

        // include guard
        stream << "#endif\n";

        file.close();
    }
}

void CppGenerator::writeAdaptorSource(const BluezApiParser &parser)
{
    // Iterate interfaces
    for (const auto &interface : parser.interfaces()) {
        auto className = interfaceToClassName(interface.name());

        // Create file
        QFile file(className.toLower() + QStringLiteral("adaptor.cpp"));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Error opening file for writing:" << file.fileName();
            return;
        }

        // Write content
        QTextStream stream(&file);
        writeCopyrightHeader(stream);
        stream << "#include \"" << className << "Adaptor.h\"\n\n";
        stream << "#include \"" << className << ".h\"\n\n";
        stream << "namespace BluezQt\n{\n\n";
        stream << className << "Adaptor::" << className << "Adaptor(" << className << " *parent)\n";
        stream << "    : QDBusAbstractAdaptor(parent)\n";
        stream << "    , m_" << lowerFirstChars(className) << "(parent)\n";
        stream << "{\n}\n\n";

        // Write property accessors
        for (const auto &property : interface.properties().properties()) {
            // Respect config
            if ((property.tags().isOptional && !m_config.useOptional) || (property.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }
            stream << bluezToQt(property.type()) << " " << className << "Adaptor::" << lowerFirstChars(property.name()) << "() const\n";
            stream << "{\n";
            stream << "    return m_" << lowerFirstChars(className) << "->" << lowerFirstChars(property.name()) << "();\n";
            stream << "}\n\n";
            if (!property.tags().isReadOnly) {
                stream << "void " << className << "Adaptor::set" << property.name() << "(const " << bluezToQt(property.type()) << " &"
                       << lowerFirstChars(property.name()) << ");\n";
                stream << "{\n";
                stream << "    m_" << lowerFirstChars(className) << "->set" << property.name() << "(" << lowerFirstChars(property.name()) << ");\n";
                stream << "}\n\n";
            }
        }

        // write Methods
        for (const auto &method : interface.methods().methods()) {
            // Respect config
            if ((method.tags().isOptional && !m_config.useOptional) || (method.tags().isExperimental && !m_config.useExperimental)) {
                continue;
            }
            stream << bluezToQt(method.outParameter().type()) << " " << className << "Adaptor::" << method.name() << "(";
            for (auto it = method.inParameters().begin(); it != method.inParameters().end(); ++it) {
                stream << "const " << bluezToQt(it->type()) << " &" << it->name();
                if (it != std::prev(method.inParameters().end())) {
                    stream << ", ";
                }
            }
            stream << ")\n{\n";
            stream << "    return m_" << lowerFirstChars(className) << "->" << lowerFirstChars(method.name()) << "(";
            for (auto it = method.inParameters().begin(); it != method.inParameters().end(); ++it) {
                stream << it->name();
                if (it != std::prev(method.inParameters().end())) {
                    stream << ", ";
                }
            }
            stream << ");\n}\n\n";
        }

        stream << "} // namespace BluezQt\n";

        file.close();
    }
}

QString CppGenerator::interfaceToClassName(const QString &interface)
{
    const int index = interface.lastIndexOf(QRegularExpression(QStringLiteral("\\.[A-Z]\\w+"))) + 1;
    auto className = interface.mid(index);
    while (className.back() > QLatin1Char('0') && className.back() <= QLatin1Char('9')) {
        className.remove(className.size() - 1, 1);
    }

    return className;
}

QString CppGenerator::lowerFirstChars(const QString &string)
{
    QString str(string);
    // str.replace(0, 1, string.at(0).toLower());

    const QRegularExpression rx(QStringLiteral("^([A-Z]+)"));
    QRegularExpressionMatch match = rx.match(string);
    if (match.hasMatch()) {
        QString matchedStr = match.captured(1);
        for (int i = 0; i < matchedStr.size() - 1; ++i) {
            str.replace(i, 1, str.at(i).toLower());
        }
    }
    str.replace(0, 1, string.at(0).toLower());
    str.replace(string.size() - 1, 1, string.at(string.size() - 1).toLower());

    return str;
}

void CppGenerator::writeCopyrightHeader(QTextStream &stream)
{
    stream << "/*\n";
    stream << " * BluezQt - Asynchronous Bluez wrapper library\n";
    stream << " *\n";
    stream << " * Copyright (C) " << m_config.year << " " << m_config.author << "\n";
    stream << " *\n";
    stream << " * This library is free software; you can redistribute it and/or\n";
    stream << " * modify it under the terms of the GNU Lesser General Public\n";
    stream << " * License as published by the Free Software Foundation; either\n";
    stream << " * version 2.1 of the License, or (at your option) version 3, or any\n";
    stream << " * later version accepted by the membership of KDE e.V. (or its\n";
    stream << " * successor approved by the membership of KDE e.V.), which shall\n";
    stream << " * act as a proxy defined in Section 6 of version 3 of the license.\n";
    stream << " *\n";
    stream << " * This library is distributed in the hope that it will be useful,\n";
    stream << " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n";
    stream << " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n";
    stream << " * Lesser General Public License for more details.\n";
    stream << " *\n";
    stream << " * You should have received a copy of the GNU Lesser General Public\n";
    stream << " * License along with this library. If not, see <http://www.gnu.org/licenses/>.\n";
    stream << " */\n\n";
}
