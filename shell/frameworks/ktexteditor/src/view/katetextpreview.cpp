/*
    SPDX-FileCopyrightText: 2016 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katetextpreview.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katelayoutcache.h"
#include "katepartdebug.h"
#include "katerenderer.h"
#include "kateview.h"
#include "kateviewinternal.h"

#include <QPainter>

#include <cmath>

KateTextPreview::KateTextPreview(KTextEditor::ViewPrivate *view, QWidget *parent)
    : QFrame(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint)
    , m_view(view)
    , m_line(0)
    , m_showFoldedLines(false)
    , m_center(true)
    , m_scale(1.0)
{
}

KTextEditor::ViewPrivate *KateTextPreview::view() const
{
    return m_view;
}

void KateTextPreview::setLine(qreal line)
{
    if (m_line != line) {
        m_line = qMax(0.0, line);
        update();
    }
}

qreal KateTextPreview::line() const
{
    return m_line;
}

void KateTextPreview::setCenterView(bool center)
{
    if (m_center != center) {
        m_center = center;
        update();
    }
}

bool KateTextPreview::centerView() const
{
    return m_center;
}

void KateTextPreview::setScaleFactor(qreal factor)
{
    if (m_scale <= 0.0) {
        qCWarning(LOG_KTE) << "Negative scale factors are not supported, ignoring.";
        return;
    }

    if (m_scale != factor) {
        m_scale = factor;
        update();
    }
}

qreal KateTextPreview::scaleFactor() const
{
    return m_scale;
}

void KateTextPreview::setShowFoldedLines(bool on)
{
    if (m_showFoldedLines != on) {
        m_showFoldedLines = on;
        update();
    }
}

bool KateTextPreview::showFoldedLines() const
{
    return m_showFoldedLines;
}

void KateTextPreview::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    KateRenderer *const renderer = view()->renderer();
    const int lastLine = showFoldedLines() ? view()->document()->lines() : view()->textFolding().visibleLines();

    const QRectF r = contentsRect(); // already subtracted QFrame's frame width
    const int xStart = 0;
    const int xEnd = r.width() / m_scale;
    const int lineHeight = qMax(1, renderer->lineHeight());
    const int lineCount = ceil(static_cast<qreal>(r.height()) / (lineHeight * m_scale));
    int startLine = qMax(0.0, m_line - (m_center ? (ceil(lineCount / 2.0)) : 0));
    // at the very end of the document, make sure the preview is filled
    if (qMax(0.0, m_line - (m_center ? (ceil(lineCount / 2.0)) : 0)) + lineCount - 1 > lastLine) {
        m_line = qMax(0.0, lastLine - static_cast<qreal>(r.height()) / (lineHeight * m_scale) + floor(lineCount / 2.0) - 1);
        startLine = qMax(0.0, m_line - (m_center ? (ceil(lineCount / 2.0)) : 0) + 1);
    }
    const int endLine = startLine + lineCount;

    QPainter paint(this);
    paint.setClipRect(r);
    paint.fillRect(r, m_view->rendererConfig()->backgroundColor());

    //     renderer->setShowTabs(doc()->config()->showTabs());
    //     renderer->setShowTrailingSpaces(doc()->config()->showSpaces());

    paint.scale(m_scale, m_scale);
    paint.translate(r.topLeft());
    if (m_center && m_line - ceil(lineCount / 2.0) > 0.0) {
        paint.translate(0, -lineHeight * (m_line - static_cast<int>(m_line)));
    }

    for (int line = startLine; line <= endLine; ++line) {
        // get real line, skip if invalid!
        const int realLine = showFoldedLines() ? line : view()->textFolding().visibleLineToLine(line);
        if (realLine < 0 || realLine >= renderer->doc()->lines()) {
            continue;
        }

        // compute layout WITHOUT cache to not poison it + render it
        KateLineLayout lineLayout(*renderer);
        lineLayout.setLine(realLine, -1);
        renderer->layoutLine(&lineLayout, -1 /* no wrap */, false /* no layout cache */);
        renderer->paintTextLine(paint, &lineLayout, xStart, xEnd, QRectF{}, nullptr, KateRenderer::SkipDrawFirstInvisibleLineUnderlined);

        // translate for next line
        paint.translate(0, lineHeight);
    }
}

#include "moc_katetextpreview.cpp"
