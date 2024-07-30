/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Methods.h"

#include <QRegularExpression>
#include <QStringList>

static const QRegularExpression rx(QStringLiteral("\\t+" // preceding tabs
                                                  "(?:(.+) )?" // return types - Argh! LE Advertising Manager does not specify return type
                                                  "([A-Z]\\w+)" // method name
                                                  "\\(([^\\)]*)\\)" // parameters
                                                  "(?: \\[(.*)\\])?" // tags
                                                  "(?: \\((.*)\\))?" // limitations
                                                  ),
                                   QRegularExpression::CaseInsensitiveOption);

Methods::Methods()
{
}

bool Methods::isMethod(const QString &line)
{
    // Check if we match a method
    return (rx.match(line).hasMatch());
}

void Methods::parse(const QString &line)
{
    // Check if we match a method
    QRegularExpressionMatch match = rx.match(line);
    if (match.hasMatch()) {
        m_methods.emplace_back(Method());
        m_currentMethod = &m_methods.back();
        m_currentMethod->m_outParameterStrings = match.captured(1).toLower().split(QStringLiteral(", "), Qt::SkipEmptyParts);
        m_currentMethod->m_name = match.captured(2);
        m_currentMethod->m_inParameterStrings = match.captured(3).split(QStringLiteral(", "), Qt::SkipEmptyParts);
        m_currentMethod->m_stringTags = match.captured(4).toLower().split(QStringLiteral(", "), Qt::SkipEmptyParts);
        m_currentMethod->m_limitation = match.captured(5).toLower();
    } else if (m_currentMethod) {
        // Skip first empty line
        if (line.isEmpty() && m_currentMethod->m_comment.isEmpty()) {
            return;
        }
        m_currentMethod->m_comment.append(line);
    }
}

bool Methods::finalize()
{
    bool success = true;

    for (auto &method : m_methods) {
        success &= method.finalize();
    }

    return success;
}

std::list<Method> Methods::methods() const
{
    return m_methods;
}
