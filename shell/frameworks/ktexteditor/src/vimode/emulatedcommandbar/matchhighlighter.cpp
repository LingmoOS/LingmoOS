/*
    SPDX-FileCopyrightText: 2013-2016 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "matchhighlighter.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "katerenderer.h"
#include "kateview.h"

using namespace KateVi;

MatchHighlighter::MatchHighlighter(KTextEditor::ViewPrivate *view)
    : m_view(view)
{
    updateMatchHighlightAttrib();
    m_highlightedMatch = m_view->doc()->newMovingRange(KTextEditor::Range::invalid(), Kate::TextRange::DoNotExpand);
    m_highlightedMatch->setView(m_view); // Show only in this view.
    m_highlightedMatch->setAttributeOnlyForViews(true);
    // Use z depth defined in moving ranges interface.
    m_highlightedMatch->setZDepth(-10000.0);
    m_highlightedMatch->setAttribute(m_highlightMatchAttribute);
    connect(m_view, &KTextEditor::View::configChanged, this, &MatchHighlighter::updateMatchHighlightAttrib);
}

MatchHighlighter::~MatchHighlighter()
{
    delete m_highlightedMatch;
}

void MatchHighlighter::updateMatchHighlight(KTextEditor::Range matchRange)
{
    // Note that if matchRange is invalid, the highlight will not be shown, so we
    // don't need to check for that explicitly.
    m_highlightedMatch->setRange(matchRange);
}

void MatchHighlighter::updateMatchHighlightAttrib()
{
    const QColor &matchColour = m_view->rendererConfig()->searchHighlightColor();
    if (!m_highlightMatchAttribute) {
        m_highlightMatchAttribute = new KTextEditor::Attribute;
    }
    m_highlightMatchAttribute->setBackground(matchColour);
    KTextEditor::Attribute::Ptr mouseInAttribute(new KTextEditor::Attribute());
    m_highlightMatchAttribute->setDynamicAttribute(KTextEditor::Attribute::ActivateMouseIn, mouseInAttribute);
    m_highlightMatchAttribute->dynamicAttribute(KTextEditor::Attribute::ActivateMouseIn)->setBackground(matchColour);
}
