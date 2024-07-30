/*
    SPDX-FileCopyrightText: 2002-2005 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2003 Anakim Border <aborder@sources.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katelinelayout.h"
#include "katetextfolding.h"
#include "katetextlayout.h"

#include <QTextLine>

#include "katepartdebug.h"

#include "katedocument.h"
#include "katerenderer.h"

KateLineLayout::KateLineLayout(KateRenderer &renderer)
    : m_renderer(renderer)
    , m_line(-1)
    , m_virtualLine(-1)
    , m_layout(nullptr)
{
}

void KateLineLayout::clear()
{
    m_textLine.reset();
    m_line = -1;
    m_virtualLine = -1;
    shiftX = 0;
    // not touching dirty
    m_layout.reset();
    // not touching layout dirty
}

bool KateLineLayout::includesCursor(const KTextEditor::Cursor realCursor) const
{
    return realCursor.line() == line();
}

const Kate::TextLine &KateLineLayout::textLine(bool reloadForce) const
{
    if (reloadForce || !m_textLine) {
        m_textLine.reset();
        if (m_line >= 0 && m_line < m_renderer.doc()->lines()) {
            m_textLine = usePlainTextLine ? m_renderer.doc()->plainKateTextLine(m_line) : m_renderer.doc()->kateTextLine(m_line);
        }
    }

    Q_ASSERT(m_textLine);

    return *m_textLine;
}

int KateLineLayout::line() const
{
    return m_line;
}

void KateLineLayout::setLine(int line, int virtualLine)
{
    m_line = line;
    m_virtualLine = (virtualLine == -1) ? m_renderer.folding().lineToVisibleLine(line) : virtualLine;
    m_textLine.reset();
}

int KateLineLayout::virtualLine() const
{
    return m_virtualLine;
}

void KateLineLayout::setVirtualLine(int virtualLine)
{
    m_virtualLine = virtualLine;
}

bool KateLineLayout::startsInvisibleBlock() const
{
    if (!isValid()) {
        return false;
    }

    return (virtualLine() + 1) != m_renderer.folding().lineToVisibleLine(line() + 1);
}

bool KateLineLayout::isValid() const
{
    return line() != -1 && layout() && (textLine(), m_textLine);
}

QTextLayout *KateLineLayout::layout() const
{
    return m_layout.get();
}

void KateLineLayout::setLayout(QTextLayout *layout)
{
    if (m_layout.get() != layout) {
        m_layout.reset(layout);
    }

    layoutDirty = !m_layout;
    m_dirtyList.clear();
    if (m_layout) {
        for (int i = 0; i < qMax(1, m_layout->lineCount()); ++i) {
            m_dirtyList.append(true);
        }
    }
}

void KateLineLayout::invalidateLayout()
{
    setLayout(nullptr);
}

bool KateLineLayout::isDirty(int viewLine) const
{
    Q_ASSERT(isValid() && viewLine >= 0 && viewLine < viewLineCount());
    return m_dirtyList[viewLine];
}

bool KateLineLayout::setDirty(int viewLine, bool dirty)
{
    Q_ASSERT(isValid() && viewLine >= 0 && viewLine < viewLineCount());
    m_dirtyList[viewLine] = dirty;
    return dirty;
}

KTextEditor::Cursor KateLineLayout::start() const
{
    return KTextEditor::Cursor(line(), 0);
}

int KateLineLayout::length() const
{
    return textLine().length();
}

int KateLineLayout::viewLineCount() const
{
    return m_layout->lineCount();
}

KateTextLayout KateLineLayout::viewLine(int viewLine)
{
    if (viewLine < 0) {
        viewLine += viewLineCount();
    }
    Q_ASSERT(isValid());
    Q_ASSERT(viewLine >= 0 && viewLine < viewLineCount());
    return KateTextLayout(this, viewLine);
}

int KateLineLayout::width() const
{
    int width = 0;

    for (int i = 0; i < m_layout->lineCount(); ++i) {
        width = qMax((int)m_layout->lineAt(i).naturalTextWidth(), width);
    }

    return width;
}

int KateLineLayout::widthOfLastLine()
{
    const KateTextLayout &lastLine = viewLine(viewLineCount() - 1);
    return lastLine.width() + lastLine.xOffset();
}

bool KateLineLayout::isOutsideDocument() const
{
    return line() < 0 || line() >= m_renderer.doc()->lines();
}

void KateLineLayout::debugOutput() const
{
    qCDebug(LOG_KTE) << "KateLineLayout: " << this << " valid " << isValid() << " line " << line() << " length " << length() << " width " << width()
                     << " viewLineCount " << viewLineCount();
}

int KateLineLayout::viewLineForColumn(int column) const
{
    int len = 0;
    int i = 0;
    for (; i < m_layout->lineCount() - 1; ++i) {
        len += m_layout->lineAt(i).textLength();
        if (column < len) {
            return i;
        }
    }
    return i;
}

bool KateLineLayout::isRightToLeft() const
{
    if (!m_layout) {
        return false;
    }

    return m_layout->textOption().textDirection() == Qt::RightToLeft;
}
