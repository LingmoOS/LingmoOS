/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "safestring.h"

#include <QStringList>

using namespace KTextTemplate;

SafeString::SafeString()
    : m_nestedString(this)
    , m_safety(IsNotSafe)
    , m_needsescape(false)
{
}

SafeString::SafeString(const KTextTemplate::SafeString &safeString)
    : m_nestedString(safeString.get(), this)
    , m_safety(safeString.m_safety)
    , m_needsescape(safeString.m_needsescape)
{
}

SafeString::SafeString(const QString &str, bool safe)
    : m_nestedString(str, this)
    , m_safety(safe ? IsSafe : IsNotSafe)
    , m_needsescape(false)
{
}

SafeString::SafeString(const QString &str, Safety safety)
    : m_nestedString(str, this)
    , m_safety(safety)
    , m_needsescape(false)
{
}

SafeString::~SafeString() = default;

void SafeString::setNeedsEscape(bool needsEscape)
{
    m_needsescape = needsEscape;
}

bool SafeString::needsEscape() const
{
    return m_needsescape;
}

void SafeString::setSafety(KTextTemplate::SafeString::Safety safety)
{
    m_safety = safety;
}

bool SafeString::isSafe() const
{
    return m_safety == IsSafe;
}

SafeString::NestedString::NestedString(SafeString *safeString)
    : m_safeString(safeString)
{
}

SafeString::NestedString::NestedString(const QString &content, SafeString *safeString)
    : QString(content)
    , m_safeString(safeString)
{
}

SafeString &SafeString::operator=(const SafeString &str)
{
    if (&str == this)
        return *this;
    m_nestedString = str.m_nestedString;
    m_nestedString.m_safeString = this;
    m_safety = str.m_safety;
    m_needsescape = str.m_needsescape;
    return *this;
}

SafeString SafeString::operator+(const QString &str)
{
    return {static_cast<QString>(m_nestedString) + str, IsNotSafe};
}

SafeString SafeString::operator+(const SafeString &str)
{
    if (!str.isSafe())
        return {static_cast<QString>(m_nestedString) + static_cast<QString>(str.get()), IsNotSafe};
    return {static_cast<QString>(m_nestedString) + static_cast<QString>(str.get()), m_safety};
}

SafeString &SafeString::operator+=(const QString &str)
{
    m_safety = IsNotSafe;

    m_nestedString += str;
    return *this;
}

SafeString &SafeString::operator+=(const SafeString &str)
{
    if (!str.isSafe())
        m_safety = IsNotSafe;

    m_nestedString += str.get();
    return *this;
}

bool SafeString::operator==(const KTextTemplate::SafeString &other) const
{
    // Don't compare safety or account for future escaping here.
    // See TestBuiltins testEscaping
    return m_nestedString == other.get();
}

bool SafeString::operator==(const QString &other) const
{
    return m_nestedString == other;
}

SafeString &SafeString::NestedString::append(const SafeString &str)
{
    QString::append(str.get());
    if (!str.isSafe())
        m_safeString->m_safety = IsNotSafe;

    return *m_safeString;
}

SafeString &SafeString::NestedString::append(const QString &str)
{
    QString::append(str);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::append(const QLatin1String &str)
{
    QString::append(str);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::append(const QChar ch)
{
    QString::append(ch);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

void SafeString::NestedString::chop(int n)
{
    QString::chop(n);
    m_safeString->m_safety = IsNotSafe;
}

SafeString &SafeString::NestedString::fill(QChar ch, int size)
{
    QString::fill(ch, size);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::insert(int position, const KTextTemplate::SafeString &str)
{
    QString::insert(position, str.get());
    if (!str.isSafe())
        m_safeString->m_safety = IsNotSafe;

    return *m_safeString;
}

SafeString &SafeString::NestedString::insert(int position, const QString &str)
{
    QString::insert(position, str);
    return *m_safeString;
}

SafeString &SafeString::NestedString::insert(int position, const QLatin1String &str)
{
    QString::insert(position, str);
    return *m_safeString;
}

SafeString &SafeString::NestedString::insert(int position, const QChar *unicode, int size)
{
    QString::insert(position, unicode, size);
    return *m_safeString;
}

SafeString &SafeString::NestedString::insert(int position, QChar ch)
{
    QString::insert(position, ch);
    return *m_safeString;
}

SafeString SafeString::NestedString::left(int n) const
{
    return {QString::left(n), m_safeString->m_safety};
}

SafeString SafeString::NestedString::leftJustified(int width, QChar fill, bool truncate) const
{
    return {QString::leftJustified(width, fill, truncate), m_safeString->m_safety};
}

SafeString SafeString::NestedString::mid(int position, int n) const
{
    return {QString::mid(position, n), m_safeString->m_safety};
}

SafeString SafeString::NestedString::normalized(QString::NormalizationForm mode) const
{
    return {QString::normalized(mode), m_safeString->m_safety};
}

SafeString SafeString::NestedString::normalized(QString::NormalizationForm mode, QChar::UnicodeVersion version) const
{
    return {QString::normalized(mode, version), m_safeString->m_safety};
}

SafeString &SafeString::NestedString::prepend(const KTextTemplate::SafeString &str)
{
    QString::prepend(str.get());
    if (!str.isSafe())
        m_safeString->m_safety = IsNotSafe;

    return *m_safeString;
}

SafeString &SafeString::NestedString::prepend(const QString &str)
{
    QString::prepend(str);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::prepend(const QLatin1String &str)
{
    QString::prepend(str);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::prepend(QChar ch)
{
    QString::prepend(ch);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

void SafeString::NestedString::push_back(const KTextTemplate::SafeString &other)
{
    QString::push_back(other.get());
    m_safeString->m_safety = other.m_safety;
}

void SafeString::NestedString::push_front(const KTextTemplate::SafeString &other)
{
    QString::push_front(other.get());
    m_safeString->m_safety = other.m_safety;
}

SafeString &SafeString::NestedString::remove(int position, int n)
{
    QString::remove(position, n);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::remove(QChar ch, Qt::CaseSensitivity cs)
{
    QString::remove(ch, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::remove(const KTextTemplate::SafeString &str, Qt::CaseSensitivity cs)
{
    QString::remove(str, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::remove(const QString &str, Qt::CaseSensitivity cs)
{
    QString::remove(str, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::remove(const QRegularExpression &rx)
{
    QString::remove(rx);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString SafeString::NestedString::repeated(int times) const
{
    return {QString::repeated(times), m_safeString->m_safety};
}

SafeString &SafeString::NestedString::replace(int position, int n, const KTextTemplate::SafeString &after)
{
    QString::replace(position, n, after.get());
    m_safeString->m_safety = after.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(int position, int n, const QString &after)
{
    QString::replace(position, n, after);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(int position, int n, const QChar *unicode, int size)
{
    QString::replace(position, n, unicode, size);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(int position, int n, QChar after)
{
    QString::replace(position, n, after);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const KTextTemplate::SafeString &before, const KTextTemplate::SafeString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before.get(), after.get(), cs);
    m_safeString->m_safety = after.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QString &before, const KTextTemplate::SafeString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after.get(), cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const KTextTemplate::SafeString &before, const QString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before.get(), after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QChar *before, int blen, const QChar *after, int alen, Qt::CaseSensitivity cs)
{
    QString::replace(before, blen, after, alen, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(QChar ch, const SafeString &after, Qt::CaseSensitivity cs)
{
    QString::replace(ch, after.get(), cs);
    m_safeString->m_safety = after.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(QChar ch, const QString &after, Qt::CaseSensitivity cs)
{
    QString::replace(ch, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QLatin1String &before, const QLatin1String &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QLatin1String &before, const KTextTemplate::SafeString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after.get(), cs);
    m_safeString->m_safety = after.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QLatin1String &before, const QString &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const KTextTemplate::SafeString &before, const QLatin1String &after, Qt::CaseSensitivity cs)
{
    QString::replace(before.get(), after, cs);
    m_safeString->m_safety = before.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QString &before, const QLatin1String &after, Qt::CaseSensitivity cs)
{
    QString::replace(before, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(QChar c, const QLatin1String &after, Qt::CaseSensitivity cs)
{
    QString::replace(c, after, cs);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QRegularExpression &rx, const KTextTemplate::SafeString &after)
{
    QString::replace(rx, after.get());
    m_safeString->m_safety = after.m_safety;
    return *m_safeString;
}

SafeString &SafeString::NestedString::replace(const QRegularExpression &rx, const QString &after)
{
    QString::replace(rx, after);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString SafeString::NestedString::right(int n) const
{
    return {QString::right(n), m_safeString->m_safety};
}

SafeString SafeString::NestedString::rightJustified(int width, QChar fill, bool truncate) const
{
    return {QString::rightJustified(width, fill, truncate), m_safeString->m_safety};
}

SafeString SafeString::NestedString::section(const QRegularExpression &reg, int start, int end, QString::SectionFlags flags) const
{
    return {QString::section(reg, start, end, flags), m_safeString->m_safety};
}

SafeString SafeString::NestedString::section(QChar sep, int start, int end, QString::SectionFlags flags) const
{
    return {QString::section(sep, start, end, flags), m_safeString->m_safety};
}

SafeString SafeString::NestedString::section(const KTextTemplate::SafeString &sep, int start, int end, QString::SectionFlags flags) const
{
    return {QString::section(sep, start, end, flags), m_safeString->m_safety};
}

SafeString SafeString::NestedString::section(const QString &sep, int start, int end, QString::SectionFlags flags) const
{
    return {QString::section(sep, start, end, flags), m_safeString->m_safety};
}

SafeString &SafeString::NestedString::setNum(int n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(uint n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(long int n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(ulong n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(qlonglong n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(qulonglong n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(short int n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(ushort n, int base)
{
    QString::setNum(n, base);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(double n, char format, int precision)
{
    QString::setNum(n, format, precision);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setNum(float n, char format, int precision)
{
    QString::setNum(n, format, precision);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setUnicode(const QChar *unicode, int size)
{
    QString::setUnicode(unicode, size);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString &SafeString::NestedString::setUtf16(const ushort *unicode, int size)
{
    QString::setUtf16(unicode, size);
    m_safeString->m_safety = IsNotSafe;
    return *m_safeString;
}

SafeString SafeString::NestedString::simplified() const
{
    return {QString::simplified(), m_safeString->m_safety};
}

QStringList SafeString::NestedString::split(const KTextTemplate::SafeString &sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return QString::split(sep.get(), behavior, cs);
}

QStringList SafeString::NestedString::split(const QString &sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return QString::split(sep, behavior, cs);
}

QStringList SafeString::NestedString::split(const QChar &sep, Qt::SplitBehavior behavior, Qt::CaseSensitivity cs) const
{
    return QString::split(sep, behavior, cs);
}

QStringList SafeString::NestedString::split(const QRegularExpression &rx, Qt::SplitBehavior behavior) const
{
    return QString::split(rx, behavior);
}

SafeString SafeString::NestedString::toLower() const
{
    return {QString::toLower(), IsNotSafe};
}

SafeString SafeString::NestedString::toUpper() const
{
    return {QString::toUpper(), IsNotSafe};
}

SafeString SafeString::NestedString::trimmed() const
{
    return {QString::trimmed(), m_safeString->m_safety};
}
