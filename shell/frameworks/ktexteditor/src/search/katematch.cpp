/*
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katematch.h"

#include "katedocument.h"
#include "kateregexpsearch.h"

KateMatch::KateMatch(KTextEditor::DocumentPrivate *document, KTextEditor::SearchOptions options)
    : m_document(document)
    , m_options(options)
{
    m_resultRanges.append(KTextEditor::Range::invalid());
}

KTextEditor::Range KateMatch::searchText(KTextEditor::Range range, const QString &pattern)
{
    m_resultRanges = m_document->searchText(range, pattern, m_options);

    return m_resultRanges[0];
}

KTextEditor::Range KateMatch::replace(const QString &replacement, bool blockMode, int replacementCounter)
{
    // Placeholders depending on search mode
    // skip place-holder stuff if we have no \ at all inside the replacement, the buildReplacement is expensive
    const bool usePlaceholders =
        (m_options.testFlag(KTextEditor::Regex) || m_options.testFlag(KTextEditor::EscapeSequences)) && replacement.contains(QLatin1Char('\\'));

    const QString finalReplacement = usePlaceholders ? buildReplacement(replacement, blockMode, replacementCounter) : replacement;

    // Track replacement operation, reuse range if already there
    if (m_afterReplaceRange) {
        m_afterReplaceRange->setRange(range());
    } else {
        m_afterReplaceRange.reset(m_document->newMovingRange(range(), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight));
    }

    // replace and return results range
    m_document->replaceText(range(), finalReplacement, blockMode && !range().onSingleLine());
    return m_afterReplaceRange->toRange();
}

KTextEditor::Range KateMatch::range() const
{
    if (!m_resultRanges.isEmpty()) {
        return m_resultRanges[0];
    }

    return KTextEditor::Range::invalid();
}

bool KateMatch::isEmpty() const
{
    return range().isEmpty();
}

bool KateMatch::isValid() const
{
    return range().isValid();
}

QString KateMatch::buildReplacement(const QString &replacement, bool blockMode, int replacementCounter) const
{
    QStringList capturedTexts;
    capturedTexts.reserve(m_resultRanges.size());
    for (KTextEditor::Range captureRange : std::as_const(m_resultRanges)) {
        // Copy capture content
        capturedTexts << m_document->text(captureRange, blockMode);
    }

    return KateRegExpSearch::buildReplacement(replacement, capturedTexts, replacementCounter);
}
