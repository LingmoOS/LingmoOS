/*
    SPDX-FileCopyrightText: 2010 Sebastian Sauer <mail@dipe.org>
    SPDX-FileCopyrightText: 2012 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_VIEW_ACCESSIBLE_
#define _KATE_VIEW_ACCESSIBLE_

#ifndef QT_NO_ACCESSIBILITY

#include "katedocument.h"
#include "kateview.h"
#include "kateviewinternal.h"

#include <KLocalizedString>
#include <QAccessible>
#include <QAccessibleWidget>
#include <QTextBoundaryFinder>

/**
 * This class implements a QAccessible-interface for a KateViewInternal.
 *
 * This is the root class for the kateview. The \a KateCursorAccessible class
 * represents the cursor in the kateview and is a child of this class.
 */
class KateViewAccessible : public QAccessibleWidget, public QAccessibleTextInterface, public QAccessibleEditableTextInterface
{
public:
    explicit KateViewAccessible(KateViewInternal *view)
        : QAccessibleWidget(view, QAccessible::EditableText)
        , m_lastPosition(-1)
    {
        // to invalidate positionFromCursor cache when the document is changed
        m_conn = QObject::connect(view->view()->document(), &KTextEditor::Document::textChanged, [this]() {
            m_lastPosition = -1;
        });
    }

    void *interface_cast(QAccessible::InterfaceType t) override
    {
        if (t == QAccessible::EditableTextInterface)
            return static_cast<QAccessibleEditableTextInterface *>(this);
        if (t == QAccessible::TextInterface)
            return static_cast<QAccessibleTextInterface *>(this);
        return nullptr;
    }

    ~KateViewAccessible() override
    {
        QObject::disconnect(m_conn);
    }

    QAccessibleInterface *childAt(int x, int y) const override
    {
        Q_UNUSED(x);
        Q_UNUSED(y);
        return nullptr;
    }

    void setText(QAccessible::Text t, const QString &text) override
    {
        if (t == QAccessible::Value && view()->view()->document()) {
            view()->view()->document()->setText(text);
            m_lastPosition = -1;
        }
    }

    QAccessible::State state() const override
    {
        QAccessible::State s = QAccessibleWidget::state();
        s.focusable = view()->focusPolicy() != Qt::NoFocus;
        s.focused = view()->hasFocus();
        s.editable = true;
        s.multiLine = true;
        s.selectableText = true;
        return s;
    }

    QString text(QAccessible::Text t) const override
    {
        QString s;
        if (view()->view()->document()) {
            if (t == QAccessible::Name) {
                s = view()->view()->document()->documentName();
            }
            if (t == QAccessible::Value) {
                s = view()->view()->document()->text();
            }
        }
        return s;
    }

    int characterCount() const override
    {
        return view()->view()->document()->text().size();
    }

    void addSelection(int startOffset, int endOffset) override
    {
        KTextEditor::Range range;
        range.setRange(cursorFromInt(startOffset), cursorFromInt(endOffset));
        view()->view()->setSelection(range);
        view()->view()->setCursorPosition(cursorFromInt(endOffset));
    }

    QString attributes(int offset, int *startOffset, int *endOffset) const override
    {
        Q_UNUSED(offset);
        *startOffset = 0;
        *endOffset = characterCount();
        return QString();
    }

    QRect characterRect(int offset) const override
    {
        KTextEditor::Cursor c = cursorFromInt(offset);
        if (!c.isValid()) {
            return QRect();
        }
        QPoint p = view()->cursorToCoordinate(c);
        KTextEditor::Cursor endCursor = KTextEditor::Cursor(c.line(), c.column() + 1);
        QPoint size = view()->cursorToCoordinate(endCursor) - p;
        return QRect(view()->mapToGlobal(p), QSize(size.x(), size.y()));
    }

    int cursorPosition() const override
    {
        KTextEditor::Cursor c = view()->cursorPosition();
        return positionFromCursor(view(), c);
    }

    int offsetAtPoint(const QPoint &point) const override
    {
        if (view()) {
            KTextEditor::Cursor c = view()->coordinatesToCursor(point);
            return positionFromCursor(view(), c);
        }
        return 0;
    }

    void removeSelection(int selectionIndex) override
    {
        if (selectionIndex != 0) {
            return;
        }
        view()->view()->clearSelection();
    }

    void scrollToSubstring(int startIndex, int /*endIndex*/) override
    {
        auto c = cursorFromInt(startIndex);
        if (!c.isValid()) {
            return;
        }
        view()->view()->setScrollPosition(c);
    }

    void selection(int selectionIndex, int *startOffset, int *endOffset) const override
    {
        if (selectionIndex != 0 || !view()->view()->selection()) {
            *startOffset = 0;
            *endOffset = 0;
            return;
        }
        KTextEditor::Range range = view()->view()->selectionRange();
        *startOffset = positionFromCursor(view(), range.start());
        *endOffset = positionFromCursor(view(), range.end());
    }

    int selectionCount() const override
    {
        return view()->view()->selection() ? 1 : 0;
    }

    void setCursorPosition(int position) override
    {
        view()->view()->setCursorPosition(cursorFromInt(position));
    }

    void setSelection(int selectionIndex, int startOffset, int endOffset) override
    {
        if (selectionIndex != 0) {
            return;
        }
        KTextEditor::Range range = KTextEditor::Range(cursorFromInt(startOffset), cursorFromInt(endOffset));
        view()->view()->setSelection(range);
    }

    QString text(int startOffset, int endOffset) const override
    {
        if (startOffset > endOffset) {
            return QString();
        }
        return view()->view()->document()->text().mid(startOffset, endOffset - startOffset);
    }

    QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const override
    {
        *startOffset = -1;
        *endOffset = -1;
        if (!view() || !startOffset || !endOffset) {
            return {};
        }
        if (offset == -1) {
            offset = positionFromCursor(view(), view()->view()->doc()->documentEnd());
        }
        KTextEditor::Cursor c = cursorFromInt(offset);
        if (!c.isValid()) {
            return {};
        }
        auto doc = view()->view()->doc();

        switch (boundaryType) {
        case QAccessible::TextBoundaryType::CharBoundary: {
            QString t = doc->characterAt(c);
            *startOffset = offset;
            *endOffset = *startOffset + 1;
            return t;
        } break;
        case QAccessible::TextBoundaryType::WordBoundary: {
            QString t = doc->wordAt(c);
            *startOffset = offset;
            *endOffset = offset + t.size();
            return t;
        } break;
        case QAccessible::TextBoundaryType::LineBoundary:
        case QAccessible::TextBoundaryType::ParagraphBoundary: {
            const QString line = doc->line(c.line());
            if (line.isEmpty()) {
                *startOffset = offset;
                *endOffset = offset;
                return {};
            }
            const int start = positionFromCursor(view(), KTextEditor::Cursor(c.line(), 0));
            *startOffset = start;
            *endOffset = offset + line.size();
            return line;
        } break;
        case QAccessible::TextBoundaryType::SentenceBoundary: {
            const QString line = doc->line(c.line());
            if (line.isEmpty()) {
                *startOffset = offset;
                *endOffset = offset;
                return {};
            }
            QTextBoundaryFinder bf(QTextBoundaryFinder::BoundaryType::Sentence, line);
            int e = bf.toNextBoundary();
            if (e != -1) {
                int start = positionFromCursor(view(), KTextEditor::Cursor(c.line(), 0));
                *startOffset = start;
                *endOffset = start + bf.position();
                return line.mid(0, e);
            }
        } break;
        case QAccessible::TextBoundaryType::NoBoundary: {
            const QString text = doc->text();
            *startOffset = 0;
            *endOffset = text.size();
            return text;
        } break;
        }

        return {};
    }

    QString textBeforeOffset(int /*offset*/, QAccessible::TextBoundaryType /*boundaryType*/, int *startOffset, int *endOffset) const override
    {
        // FIXME
        *startOffset = -1;
        *endOffset = -1;
        return {};
    }
    QString textAfterOffset(int /*offset*/, QAccessible::TextBoundaryType /*boundaryType*/, int *startOffset, int *endOffset) const override
    {
        // FIXME
        *startOffset = -1;
        *endOffset = -1;
        return {};
    }

    void deleteText(int startOffset, int endOffset) override
    {
        KTextEditor::Document *document = view()->view()->document();
        KTextEditor::Range range(document->offsetToCursor(startOffset), document->offsetToCursor(endOffset));
        document->removeText(range);
    }

    void insertText(int offset, const QString &text) override
    {
        KTextEditor::Document *document = view()->view()->document();
        KTextEditor::Cursor cursor = document->offsetToCursor(offset);
        document->insertText(cursor, text);
    }

    void replaceText(int startOffset, int endOffset, const QString &text) override
    {
        KTextEditor::Document *document = view()->view()->document();
        KTextEditor::Range range(document->offsetToCursor(startOffset), document->offsetToCursor(endOffset));
        document->replaceText(range, text);
    }

    /**
     * When possible, using the last returned value m_lastPosition do the count
     * from the last cursor position m_lastCursor.
     * @return the number of chars (including one character for new lines)
     *         from the beginning of the file.
     */
    int positionFromCursor(KateViewInternal *view, KTextEditor::Cursor cursor) const
    {
        int pos = m_lastPosition;
        const KTextEditor::DocumentPrivate *doc = view->view()->doc();

        // m_lastPosition < 0 is invalid, calculate from the beginning of the document
        if (m_lastPosition < 0 || view != m_lastView) {
            pos = doc->cursorToOffset(cursor) - cursor.column();
        } else {
            // if the lines are the same, just add the cursor.column(), otherwise
            if (cursor.line() != m_lastCursor.line()) {
                // If the cursor is after the previous cursor
                if (m_lastCursor.line() < cursor.line()) {
                    for (int line = m_lastCursor.line(); line < cursor.line(); ++line) {
                        pos += doc->lineLength(line);
                    }
                    // add new line character for each line
                    pos += cursor.line() - m_lastCursor.line();
                } else {
                    for (int line = cursor.line(); line < m_lastCursor.line(); ++line) {
                        pos -= doc->lineLength(line);
                    }
                    // remove new line character for each line
                    pos -= m_lastCursor.line() - cursor.line();
                }
            }
        }
        m_lastCursor = cursor;
        m_lastPosition = pos;

        return pos + cursor.column();
    }

private:
    inline KateViewInternal *view() const
    {
        return static_cast<KateViewInternal *>(object());
    }

    KTextEditor::Cursor cursorFromInt(int position) const
    {
        return view()->view()->doc()->offsetToCursor(position);
    }

    QString textLine(int shiftLines, int offset, int *startOffset, int *endOffset) const
    {
        KTextEditor::Cursor pos = cursorFromInt(offset);
        pos.setColumn(0);
        if (shiftLines) {
            pos.setLine(pos.line() + shiftLines);
        }
        *startOffset = positionFromCursor(view(), pos);
        QString line = view()->view()->document()->line(pos.line()) + QLatin1Char('\n');
        *endOffset = *startOffset + line.length();
        return line;
    }

private:
    // Cache data for positionFromCursor
    mutable KateViewInternal *m_lastView;
    mutable KTextEditor::Cursor m_lastCursor;
    // m_lastPosition stores the positionFromCursor, with the cursor always in column 0
    mutable int m_lastPosition;
    // to disconnect the signal
    QMetaObject::Connection m_conn;
};

/**
 * Factory-function used to create \a KateViewAccessible instances for KateViewInternal
 * to make the KateViewInternal accessible.
 */
QAccessibleInterface *accessibleInterfaceFactory(const QString &key, QObject *object)
{
    Q_UNUSED(key)
    // if (key == QLatin1String("KateViewInternal"))
    if (KateViewInternal *view = qobject_cast<KateViewInternal *>(object)) {
        return new KateViewAccessible(view);
    }
    return nullptr;
}

#endif
#endif
