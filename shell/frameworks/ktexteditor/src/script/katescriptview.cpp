/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2008 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katescriptview.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"
#include "scriptcursor.h"
#include "scriptrange.h"

#include <KLocalizedString>
#include <KTextEditor/Command>

#include <QJSEngine>

KateScriptView::KateScriptView(QJSEngine *engine, QObject *parent)
    : QObject(parent)
    , m_view(nullptr)
    , m_engine(engine)
{
}

void KateScriptView::setView(KTextEditor::ViewPrivate *view)
{
    m_view = view;
}

KTextEditor::ViewPrivate *KateScriptView::view()
{
    return m_view;
}

void KateScriptView::copy()
{
    m_view->copy();
}

void KateScriptView::cut()
{
    m_view->cut();
}

void KateScriptView::paste()
{
    m_view->paste();
}

QJSValue KateScriptView::cursorPosition()
{
    return cursorToScriptValue(m_engine, m_view->cursorPosition());
}

QJSValue KateScriptView::cursorPositions()
{
    // pack our stuff into js array
    const auto cursors = m_view->cursorPositions();
    QJSValue jsArray = m_engine->newArray(cursors.size());
    for (int i = 0; i < cursors.size(); ++i) {
        jsArray.setProperty(i, cursorToScriptValue(m_engine, cursors[i]));
    }
    return jsArray;
}

void KateScriptView::setCursorPosition(int line, int column)
{
    const KTextEditor::Cursor cursor(line, column);
    m_view->setCursorPosition(cursor);
}

void KateScriptView::setCursorPosition(const QJSValue &jscursor)
{
    const auto cursor = cursorFromScriptValue(jscursor);
    m_view->setCursorPosition(cursor);
}

void KateScriptView::setCursorPositions(const QJSValue &cursors)
{
    // unpack the array of cursors
    QList<KTextEditor::Cursor> unboxedCursors;
    const int length = cursors.property(QStringLiteral("length")).toInt();
    for (int i = 0; i < length; ++i) {
        unboxedCursors.append(cursorFromScriptValue(cursors.property(i)));
    }
    m_view->setCursorPositions(unboxedCursors);
}

QJSValue KateScriptView::virtualCursorPosition()
{
    return cursorToScriptValue(m_engine, m_view->cursorPositionVirtual());
}

void KateScriptView::setVirtualCursorPosition(int line, int column)
{
    const KTextEditor::Cursor cursor(line, column);
    m_view->setCursorPositionVisual(cursor);
}

void KateScriptView::setVirtualCursorPosition(const QJSValue &jscursor)
{
    const auto cursor = cursorFromScriptValue(jscursor);
    setVirtualCursorPosition(cursor.line(), cursor.column());
}

QString KateScriptView::selectedText()
{
    return m_view->selectionText();
}

bool KateScriptView::hasSelection()
{
    return m_view->selection();
}

QJSValue KateScriptView::selection()
{
    return rangeToScriptValue(m_engine, m_view->selectionRange());
}

QJSValue KateScriptView::selections()
{
    // pack our stuff into js array
    const auto ranges = m_view->selectionRanges();
    QJSValue jsArray = m_engine->newArray(ranges.size());
    for (int i = 0; i < ranges.size(); ++i) {
        jsArray.setProperty(i, rangeToScriptValue(m_engine, ranges[i]));
    }
    return jsArray;
}

void KateScriptView::setSelection(const QJSValue &jsrange)
{
    m_view->setSelection(rangeFromScriptValue(jsrange));
}

void KateScriptView::setSelections(const QJSValue &ranges)
{
    // unpack the array of ranges
    QList<KTextEditor::Range> unboxedRanges;
    const int length = ranges.property(QStringLiteral("length")).toInt();
    for (int i = 0; i < length; ++i) {
        unboxedRanges.append(rangeFromScriptValue(ranges.property(i)));
    }
    m_view->setSelections(unboxedRanges);
}

void KateScriptView::removeSelectedText()
{
    m_view->removeSelectedText();
}

void KateScriptView::selectAll()
{
    m_view->selectAll();
}

void KateScriptView::clearSelection()
{
    m_view->clearSelection();
}

void KateScriptView::setBlockSelection(bool on)
{
    m_view->setBlockSelection(on);
}

bool KateScriptView::blockSelection()
{
    return m_view->blockSelection();
}

void KateScriptView::align(const QJSValue &jsrange)
{
    const auto range = rangeFromScriptValue(jsrange);
    m_view->doc()->align(m_view, range);
}

void KateScriptView::alignOn(const QJSValue &jsrange, const QJSValue &pattern)
{
    if (!pattern.isString()) {
        return;
    }
    const auto range = rangeFromScriptValue(jsrange);
    m_view->doc()->alignOn(range, pattern.toString(), m_view->blockSelection());
}

QJSValue KateScriptView::searchText(const QJSValue &range, const QString &pattern, bool backwards)
{
    KTextEditor::SearchOptions option = backwards ? KTextEditor::Backwards : KTextEditor::Default;
    KTextEditor::Range match = m_view->doc()->searchText(rangeFromScriptValue(range), pattern, option).first();
    return rangeToScriptValue(m_engine, match);
}

QJSValue KateScriptView::executeCommand(const QString &command, const QString &args, const QJSValue &jsrange)
{
    QString message;
    bool ok = false;

    const auto range = rangeFromScriptValue(jsrange);
    const auto cmd = KTextEditor::EditorPrivate::self()->queryCommand(command);
    if (!cmd) {
        ok = false;
        message = i18n("Command not found: %1", command);
    } else {
        const auto cmdLine = args.isEmpty() ? (command) : (command + QLatin1Char(' ') + args);
        ok = cmd->exec(m_view, cmdLine, message, range);
    }

    QJSValue object = m_engine->newObject();
    object.setProperty(QStringLiteral("ok"), ok);
    object.setProperty(QStringLiteral("status"), message);
    return object;
}

#include "moc_katescriptview.cpp"
