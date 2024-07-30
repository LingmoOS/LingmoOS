/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <vimode/command.h>
#include <vimode/keyparser.h>

using namespace KateVi;

Command::Command(const QString &pattern, bool (NormalViMode::*commandMethod)(), unsigned int flags)
    : m_pattern(KeyParser::self()->encodeKeySequence(pattern))
    , m_flags(flags)
    , m_ptr2commandMethod(commandMethod)
{
}

Command::~Command() = default;

bool Command::execute(NormalViMode *mode) const
{
    return (mode->*m_ptr2commandMethod)();
}

bool Command::matches(const QString &pattern) const
{
    if (!(m_flags & REGEX_PATTERN)) {
        return m_pattern.startsWith(pattern);
    } else {
        // compile once, void costly isValid check, good enough to have set the pattern.
        if (m_patternRegex.pattern().isEmpty()) {
            m_patternRegex = QRegularExpression(m_pattern, QRegularExpression::UseUnicodePropertiesOption);
        }
        const auto match = m_patternRegex.match(pattern, 0, QRegularExpression::PartialPreferFirstMatch);
        // Partial matching could lead to a complete match, in that case hasPartialMatch() will return false, and hasMatch() will return true
        return match.hasPartialMatch() || match.hasMatch();
    }
}

bool Command::matchesExact(const QString &pattern) const
{
    if (!(m_flags & REGEX_PATTERN)) {
        return (m_pattern == pattern);
    } else {
        // compile once, void costly isValid check, good enough to have set the pattern.
        if (m_patternAnchoredRegex.pattern().isEmpty()) {
            m_patternAnchoredRegex = QRegularExpression(QRegularExpression::anchoredPattern(m_pattern), QRegularExpression::UseUnicodePropertiesOption);
        }
        return m_patternAnchoredRegex.match(pattern).hasMatch();
    }
}
