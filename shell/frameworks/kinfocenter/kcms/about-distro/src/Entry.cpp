/*
    SPDX-FileCopyrightText: 2012-2022 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "Entry.h"

Entry::Entry(const KLocalizedString &label_, const QString &value_, Hidden hidden)
    : m_label(label_)
    , m_value(value_)
    , m_hidden(hidden)
{
    Q_ASSERT(m_label.isEmpty() || localizedLabel(Language::English).endsWith(':'));
}

Entry::~Entry() = default;

// When false this entry is garbage (e.g. incomplete data) and shouldn't be rendered.
bool Entry::isValid() const
{
    return !localizedLabel().isEmpty() && !localizedValue().isEmpty();
}

// Returns textual representation of entry.
QString Entry::diagnosticLine(Language language) const
{
    // FIXME: This isn't really working for right-to-left
    // The answer probably is in unicode control characters, but
    // didn't work when tried.
    // Essentially what needs to happen is that the label should be RTL
    // that is to say the colon should be on the left, BUT englishy words
    // within that should be LTR, everything besides the label should be LTR
    // because we do not localize the values I don't think?
    return localizedLabel(language) + ' ' + localizedValue(language) + '\n';
}

QString Entry::localize(const KLocalizedString &string, Language language) const
{
    switch (language) {
    case Language::System:
        return string.toString();
    case Language::English:
        // https://bugs.kde.org/show_bug.cgi?id=416247
        return string.toString(QStringList{QStringLiteral("en_US")});
    }
    Q_UNREACHABLE();
    return QStringLiteral("Unknown Language %1 (bug in KInfocenter!):").arg(QString::number(static_cast<int>(language)));
}

QString Entry::localizedLabel(Language language) const
{
    return localize(m_label, language);
}

QString Entry::localizedValue(Language language) const
{
    Q_UNUSED(language);
    return m_value;
}

bool Entry::isHidden() const
{
    return m_hidden == Hidden::Yes;
}

QLocale Entry::localeForLanguage(Language language) const
{
    switch (language) {
    case Language::System:
        return QLocale::system();
    case Language::English:
        return QLocale(QLocale::English, QLocale::UnitedStates);
    }
    Q_UNREACHABLE();
    return QLocale::system();
}

#include "moc_Entry.cpp"
