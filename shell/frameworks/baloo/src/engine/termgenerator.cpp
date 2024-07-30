/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "termgenerator.h"

#include <QTextBoundaryFinder>

using namespace Baloo;

namespace {

QString normalizeTerm(const QString &str)
{
    // Remove all accents. It is important to call toLower after normalization,
    // since some exotic unicode symbols can remain uppercase
    const QString denormalized = str.normalized(QString::NormalizationForm_KD).toLower();

    QString cleanString;
    cleanString.reserve(denormalized.size());
    for (const auto& c : denormalized) {
        if (!c.isMark()) {
            cleanString.append(c);
        }
    }

    return cleanString.normalized(QString::NormalizationForm_KC);
}

void appendTerm(QByteArrayList &list, const QString &term)
{
    if (!term.isEmpty()) {
        // Truncate the string to avoid arbitrarily long terms
        list << QStringView(term).left(TermGenerator::maxTermSize).toUtf8();
    }
}

}

TermGenerator::TermGenerator(Document& doc)
    : m_doc(doc)
    , m_position(1)
{
}

void TermGenerator::indexText(const QString& text)
{
    indexText(text, QByteArray());
}

QByteArrayList TermGenerator::termList(const QString& text_)
{
    QString text(text_);
    text.replace(QLatin1Char('_'), QLatin1Char(' '));

    int start = 0;

    auto isSkipChar = [] (const QChar& c) {
        return c.isPunct() || c.isMark() || c.isSpace() || (!c.isPrint() && !c.isSurrogate());
    };

    QByteArrayList list;
    QTextBoundaryFinder bf(QTextBoundaryFinder::Word, text);
    for (; bf.position() != -1; bf.toNextBoundary()) {
        int end = bf.position();
        while (start < end && isSkipChar(text[start])) {
            start++;
        }
        if (end == start) {
            continue;
        }

        // Typically we commit a term when we have an EndOfItem, starting
        // from the last StartOfItem, everything between last EndOfItem and
        // StartOfItem is just whitespace and punctuation. Unfortunately,
        // most CJK characters do not trigger a StartOfItem and thus no
        // EndOfItem, so everything in front of a StartOfItem has to be
        // committed as well
        bool commit = bf.boundaryReasons() & (QTextBoundaryFinder::EndOfItem | QTextBoundaryFinder::StartOfItem);

        // Also commit term if end-of-text is reached or when we find
        // any punctuation
        if (!commit & (end == text.size() || isSkipChar(text[end]))) {
            commit = true;
        }

        if (commit) {
            const QString term = normalizeTerm(text.mid(start, end - start));
            appendTerm(list, term);
            start = end;
        }
    }
    return list;
}

void TermGenerator::indexText(const QString& text, const QByteArray& prefix)
{
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addPositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

void TermGenerator::indexFileNameText(const QString& text)
{
    const QByteArray prefix = QByteArrayLiteral("F");
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addFileNameTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addFileNamePositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

void TermGenerator::indexXattrText(const QString& text, const QByteArray& prefix)
{
    const QByteArrayList terms = termList(text);
    if (terms.size() == 1) {
        QByteArray finalArr = prefix + terms[0];
        m_doc.addXattrTerm(finalArr);
        return;
    }
    for (const QByteArray& term : terms) {
        QByteArray finalArr = prefix + term;

        m_doc.addXattrPositionTerm(finalArr, m_position);
        m_position++;
    }
    m_position++;
}

int TermGenerator::position() const
{
    return m_position;
}

void TermGenerator::setPosition(int position)
{
    m_position = position;
}
