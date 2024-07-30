/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "Properties.h"

#include <QRegularExpression>
#include <QStringList>

Properties::Properties()
{
}

void Properties::parse(const QString &line)
{
    const QRegularExpression rx(
        QStringLiteral("(?:Properties|^)" // Properties keyword or start of line
                       "\\t{1,2}" // preceding tabs (max 2)
                       "([a-z1-6{}_]+)" // type name
                       " " // space
                       "([A-Z]\\w+)" // method name
                       "(?: \\[(.*)\\])?" // tags
                       "(?: \\((.*)\\))?" // limitations
                       ));

    QRegularExpressionMatch match = rx.match(line);
    // Check if we match a property
    if (match.hasMatch()) {
        m_properties.emplace_back(Property());
        m_currentProperty = &m_properties.back();
        m_currentProperty->m_type = match.captured(1).toLower();
        m_currentProperty->m_name = match.captured(2);
        m_currentProperty->m_stringTags = match.captured(3).toLower().split(QStringLiteral(", "), Qt::SkipEmptyParts);
        m_currentProperty->m_limitation = match.captured(4).toLower();
    } else if (m_currentProperty) {
        // Skip first empty line
        if (line.isEmpty() && m_currentProperty->m_comment.isEmpty()) {
            return;
        }
        m_currentProperty->m_comment.append(line);
    }
}

bool Properties::finalize()
{
    bool success = true;

    for (auto &property : m_properties) {
        success &= property.finalize();
    }

    return success;
}

std::list<Property> Properties::properties() const
{
    return m_properties;
}
