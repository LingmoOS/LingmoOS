/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Interface.h"

#include <QRegularExpression>

Interface::Interface()
{
}

bool Interface::parse(const QString &line)
{
    if (line.startsWith(QLatin1String("Service\t"))) {
        m_state = State::Service;
    } else if (line.startsWith(QLatin1String("Interface\t"))) {
        m_state = State::Interface;
    } else if (line.startsWith(QLatin1String("Object path\t"))) {
        m_state = State::ObjectPath;
    } else if (line.startsWith(QLatin1String("Methods\t")) || Methods::isMethod(line)) { // Argh! AgentManager is missing the Methods keyword
        m_state = State::Methods;
    } else if (line.startsWith(QLatin1String("Properties\t"))) {
        m_state = State::Properties;
    } else if (m_state != State::Comment && !line.isEmpty() && !line.startsWith(QLatin1String("\t"))) {
        // If we do not parse comment, but line starts with characters, we are done.
        return false;
    }

    switch (m_state) {
    case State::Comment:
        parseComment(line);
        break;
    case State::Service:
        parseService(line);
        break;
    case State::Interface:
        parseInterface(line);
        break;
    case State::ObjectPath:
        parseObjectPath(line);
        break;
    case State::Methods:
        m_methods.parse(line);
        break;
    case State::Properties:
        m_properties.parse(line);
        break;
    }

    return true;
}

bool Interface::finalize()
{
    bool success = true;

    success &= m_methods.finalize();
    success &= m_properties.finalize();

    return success;
}

QStringList Interface::comment() const
{
    return m_comment;
}
QString Interface::service() const
{
    return m_service;
}

QString Interface::name() const
{
    return m_name;
}

QString Interface::objectPath() const
{
    return m_objectPath;
}

Methods Interface::methods() const
{
    return m_methods;
}

Properties Interface::properties() const
{
    return m_properties;
}

void Interface::parseComment(const QString &line)
{
    if (line.isEmpty()) {
        m_comment.append(QString());
        return;
    } else if (line.startsWith(QLatin1Char(' ')) || line.startsWith(QStringLiteral("\t"))) {
        m_comment.append(QString());
    }

    if (!m_comment.last().isEmpty()) {
        m_comment.last() += QLatin1Char(' ');
    }
    m_comment.last() += line;
}

void Interface::parseService(const QString &line)
{
    const QRegularExpression rx(QStringLiteral("Service\\t+(.+)"));
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        m_service = match.captured(1);
    }
}

void Interface::parseInterface(const QString &line)
{
    const QRegularExpression rx(QStringLiteral("Interface\\t+(.+)"));
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        m_name = match.captured(1);
    }
}

void Interface::parseObjectPath(const QString &line)
{
    const QRegularExpression rx(QStringLiteral("Object path\\t+(.+)"));
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        m_objectPath = match.captured(1);
    }
}
