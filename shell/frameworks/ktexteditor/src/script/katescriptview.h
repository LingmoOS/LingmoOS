/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2008 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_VIEW_H
#define KATE_SCRIPT_VIEW_H

#include <QJSValue>
#include <QObject>

#include <ktexteditor_export.h>

#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

namespace KTextEditor
{
class ViewPrivate;
}
class QJSEngine;
/**
 * Thinish wrapping around KTextEditor::ViewPrivate, exposing the methods we want exposed
 * and adding some helper methods.
 *
 * setView _must_ be called before using any other method. This is not checked
 * for the sake of speed.
 */
class KTEXTEDITOR_EXPORT KateScriptView : public QObject
{
    /// Properties are accessible with a nicer syntax from JavaScript
    Q_OBJECT

public:
    explicit KateScriptView(QJSEngine *, QObject *parent = nullptr);
    void setView(KTextEditor::ViewPrivate *view);
    KTextEditor::ViewPrivate *view();

    Q_INVOKABLE void copy();
    Q_INVOKABLE void cut();
    Q_INVOKABLE void paste();

    Q_INVOKABLE QJSValue cursorPosition();
    Q_INVOKABLE QJSValue cursorPositions();

    /**
     * Set the cursor position in the view.
     * @since 4.4
     */
    Q_INVOKABLE void setCursorPosition(int line, int column);
    Q_INVOKABLE void setCursorPosition(const QJSValue &cursor);
    Q_INVOKABLE void setCursorPositions(const QJSValue &cursors);

    Q_INVOKABLE QJSValue virtualCursorPosition();
    Q_INVOKABLE void setVirtualCursorPosition(int line, int column);
    Q_INVOKABLE void setVirtualCursorPosition(const QJSValue &cursor);

    Q_INVOKABLE QString selectedText();
    Q_INVOKABLE bool hasSelection();
    Q_INVOKABLE QJSValue selection();
    Q_INVOKABLE QJSValue selections();
    Q_INVOKABLE void setSelection(const QJSValue &range);
    Q_INVOKABLE void setSelections(const QJSValue &ranges);
    Q_INVOKABLE void removeSelectedText();
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();

    Q_INVOKABLE void setBlockSelection(bool on);
    Q_INVOKABLE bool blockSelection();

    Q_INVOKABLE void align(const QJSValue &range);
    Q_INVOKABLE void alignOn(const QJSValue &jsrange, const QJSValue &pattern = QJSValue(QStringLiteral("")));

    Q_INVOKABLE QJSValue searchText(const QJSValue &range, const QString &pattern, bool backwards = false);

    Q_INVOKABLE QJSValue executeCommand(const QString &command, const QString &args = QString(), const QJSValue &jsrange = QJSValue());

private:
    KTextEditor::ViewPrivate *m_view;
    QJSEngine *m_engine;
};

#endif
