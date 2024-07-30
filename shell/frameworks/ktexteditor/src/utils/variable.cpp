/*
    SPDX-FileCopyrightText: 2019 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variable.h"

namespace KTextEditor
{
Variable::Variable(const QString &name, const QString &description, Variable::ExpandFunction func, bool isPrefixMatch)
    : m_name(name)
    , m_description(description)
    , m_function(std::move(func))
    , m_isPrefixMatch(isPrefixMatch)
{
}

bool Variable::isValid() const
{
    return (!m_name.isEmpty()) && (m_function != nullptr);
}

bool Variable::isPrefixMatch() const
{
    return m_isPrefixMatch;
}

QString Variable::name() const
{
    return m_name;
}

QString Variable::description() const
{
    return m_description;
}

QString Variable::evaluate(const QStringView &prefix, KTextEditor::View *view) const
{
    return isValid() ? m_function(prefix, view) : QString();
}

}
