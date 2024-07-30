/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include <QRegularExpression>

#include "Method.h"

Method::Method()
{
}

bool Method::finalize()
{
    for (const auto &tag : m_stringTags) {
        m_tags.isOptional |= tag.contains(QLatin1String("optional"), Qt::CaseInsensitive);
        m_tags.isDeprecated |= tag.contains(QLatin1String("deprecated"), Qt::CaseInsensitive);
        m_tags.isExperimental |= tag.contains(QLatin1String("experimental"), Qt::CaseInsensitive);
    }

    bool success = true;
    success &= m_comment.finalize();

    for (const auto &inParam : m_inParameterStrings) {
        m_inParameters.push_back(Parameter::fromString(inParam));
    }

    m_outParameterStrings.removeOne(QStringLiteral("void"));
    if (m_outParameterStrings.isEmpty()) {
        m_outParameter = Parameter::fromString(QStringLiteral("void unnamend"));
        return true;
    }

    // Guess out parameter name from method name
    QString paramName = guessOutParameterName();
    if (!paramName.isEmpty()) {
        m_outParameterStrings.front() += QLatin1Char(' ') + paramName;
        m_outParameter = Parameter::fromString(m_outParameterStrings.front());
    } else {
        for (int i = 0; i < m_outParameterStrings.size(); ++i) {
            m_outParameterStrings[i] += QLatin1String(" value") + QString::number(i);
        }
    }

    for (const auto &outParam : m_outParameterStrings) {
        m_outParameters.push_back(Parameter::fromString(outParam));
    }

    return success;
}

QString Method::name() const
{
    return m_name;
}

QList<Parameter> Method::inParameters() const
{
    return m_inParameters;
}

QList<Parameter> Method::outParameters() const
{
    return m_outParameters;
}

Parameter Method::outParameter() const
{
    return m_outParameter;
}

Method::Tags Method::tags() const
{
    return m_tags;
}

QStringList Method::comment() const
{
    return m_comment;
}

QString Method::guessOutParameterName() const
{
    if (m_outParameterStrings.size() != 1) {
        return QString();
    }

    const QRegularExpression rx(QStringLiteral("([A-Z][a-z0-9]+)+"));
    QRegularExpressionMatch match = rx.match(m_name, 1);
    if (!match.hasMatch()) {
        return QStringLiteral("value");
    }

    return match.captured(1).toLower();
}
