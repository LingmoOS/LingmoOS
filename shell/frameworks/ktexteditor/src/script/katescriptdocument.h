/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_DOCUMENT_H
#define KATE_SCRIPT_DOCUMENT_H

#include <QObject>
#include <QStringList>

#include <ktexteditor_export.h>

#include <QJSValue>

#include <ktexteditor/cursor.h>
#include <ktexteditor/range.h>

namespace KTextEditor
{
class DocumentPrivate;
}

/**
 * Thinish wrapping around KTextEditor::DocumentPrivate, exposing the methods we want exposed
 * and adding some helper methods.
 *
 * setDocument _must_ be called before using any other method. This is not checked
 * for the sake of speed.
 */
class KTEXTEDITOR_EXPORT KateScriptDocument : public QObject
{
    Q_OBJECT
    // Note: we have no Q_PROPERTIES due to consistency: everything is a function.

public:
    explicit KateScriptDocument(QJSEngine *, QObject *parent = nullptr);
    void setDocument(KTextEditor::DocumentPrivate *document);
    KTextEditor::DocumentPrivate *document();

    // BEGIN
    Q_INVOKABLE QString fileName();
    Q_INVOKABLE QString url();
    Q_INVOKABLE QString mimeType();
    Q_INVOKABLE QString encoding();
    Q_INVOKABLE QString highlightingMode();
    Q_INVOKABLE QStringList embeddedHighlightingModes();
    Q_INVOKABLE QString highlightingModeAt(const QJSValue &pos);
    Q_INVOKABLE bool isModified();
    Q_INVOKABLE QString text();
    Q_INVOKABLE QString text(int fromLine, int fromColumn, int toLine, int toColumn);
    Q_INVOKABLE QString text(const QJSValue &jsfrom, const QJSValue &jsto);
    Q_INVOKABLE QString text(const QJSValue &jsrange);
    Q_INVOKABLE QString line(int line);
    Q_INVOKABLE QString wordAt(int line, int column);
    Q_INVOKABLE QString wordAt(const QJSValue &jscursor);
    Q_INVOKABLE QJSValue wordRangeAt(int line, int column);
    Q_INVOKABLE QJSValue wordRangeAt(const QJSValue &jscursor);
    Q_INVOKABLE QString charAt(int line, int column);
    Q_INVOKABLE QString charAt(const QJSValue &jscursor);
    Q_INVOKABLE QString firstChar(int line);
    Q_INVOKABLE QString lastChar(int line);
    Q_INVOKABLE bool isSpace(int line, int column);
    Q_INVOKABLE bool isSpace(const QJSValue &jscursor);
    Q_INVOKABLE bool matchesAt(int line, int column, const QString &s);
    Q_INVOKABLE bool matchesAt(const QJSValue &cursor, const QString &s);
    Q_INVOKABLE bool setText(const QString &s);
    Q_INVOKABLE bool clear();
    Q_INVOKABLE bool truncate(int line, int column);
    Q_INVOKABLE bool truncate(const QJSValue &jscursor);
    Q_INVOKABLE bool insertText(int line, int column, const QString &s);
    Q_INVOKABLE bool insertText(const QJSValue &jscursor, const QString &s);
    Q_INVOKABLE bool removeText(int fromLine, int fromColumn, int toLine, int toColumn);
    Q_INVOKABLE bool removeText(const QJSValue &range);
    Q_INVOKABLE bool removeText(const QJSValue &jsfrom, const QJSValue &jsto);
    Q_INVOKABLE bool insertLine(int line, const QString &s);
    Q_INVOKABLE bool removeLine(int line);
    Q_INVOKABLE bool wrapLine(int line, int column);
    Q_INVOKABLE bool wrapLine(const QJSValue &cursor);
    Q_INVOKABLE void joinLines(int startLine, int endLine);
    Q_INVOKABLE int lines();
    Q_INVOKABLE bool isLineModified(int line);
    Q_INVOKABLE bool isLineSaved(int line);
    Q_INVOKABLE bool isLineTouched(int line);
    Q_INVOKABLE int findTouchedLine(int startLine, bool down);
    Q_INVOKABLE int length();
    Q_INVOKABLE int lineLength(int line);
    Q_INVOKABLE void editBegin();
    Q_INVOKABLE void editEnd();
    Q_INVOKABLE int firstColumn(int line);
    Q_INVOKABLE int lastColumn(int line);
    Q_INVOKABLE int prevNonSpaceColumn(int line, int column);
    Q_INVOKABLE int prevNonSpaceColumn(const QJSValue &jscursor);
    Q_INVOKABLE int nextNonSpaceColumn(int line, int column);
    Q_INVOKABLE int nextNonSpaceColumn(const QJSValue &jscursor);
    Q_INVOKABLE int prevNonEmptyLine(int line);
    Q_INVOKABLE int nextNonEmptyLine(int line);
    Q_INVOKABLE bool isInWord(const QString &character, int attribute);
    Q_INVOKABLE bool canBreakAt(const QString &character, int attribute);
    Q_INVOKABLE bool canComment(int startAttribute, int endAttribute);
    Q_INVOKABLE QString commentMarker(int attribute);
    Q_INVOKABLE QString commentStart(int attribute);
    Q_INVOKABLE QString commentEnd(int attribute);

    Q_INVOKABLE QJSValue documentRange();
    Q_INVOKABLE QJSValue documentEnd();
    Q_INVOKABLE bool isValidTextPosition(int line, int column);
    Q_INVOKABLE bool isValidTextPosition(const QJSValue &cursor);

    /**
     * Get the syntax highlighting attribute at a given position in the document.
     */
    Q_INVOKABLE int attribute(int line, int column);
    Q_INVOKABLE int attribute(const QJSValue &jscursor);

    /**
     * Return true if the highlight attribute equals @p attr.
     */
    Q_INVOKABLE bool isAttribute(int line, int column, int attr);
    Q_INVOKABLE bool isAttribute(const QJSValue &jscursor, int attr);

    /**
     * Get the name of the syntax highlighting attribute at the given position.
     */
    Q_INVOKABLE QString attributeName(int line, int column);
    Q_INVOKABLE QString attributeName(const QJSValue &jscursor);

    /**
     * Return true is the name of the syntax attribute equals @p name.
     */
    Q_INVOKABLE bool isAttributeName(int line, int column, const QString &name);
    Q_INVOKABLE bool isAttributeName(const QJSValue &cursor, const QString &name);

    Q_INVOKABLE QString variable(const QString &s);
    Q_INVOKABLE void setVariable(const QString &s, const QString &v);
    // END

    Q_INVOKABLE int firstVirtualColumn(int line);
    Q_INVOKABLE int lastVirtualColumn(int line);
    Q_INVOKABLE int toVirtualColumn(int line, int column);
    Q_INVOKABLE int toVirtualColumn(const QJSValue &cursor);
    Q_INVOKABLE QJSValue toVirtualCursor(int line, int column);
    Q_INVOKABLE QJSValue toVirtualCursor(const QJSValue &jscursor);
    Q_INVOKABLE int fromVirtualColumn(int line, int virtualColumn);
    Q_INVOKABLE int fromVirtualColumn(const QJSValue &jscursor);
    Q_INVOKABLE QJSValue fromVirtualCursor(int line, int column);
    Q_INVOKABLE QJSValue fromVirtualCursor(const QJSValue &jscursor);

    KTextEditor::Cursor anchorInternal(int line, int column, QChar character);
    KTextEditor::Cursor anchor(KTextEditor::Cursor cursor, QChar character);
    Q_INVOKABLE QJSValue anchor(int line, int column, QChar character);
    Q_INVOKABLE QJSValue anchor(const QJSValue &cursor, QChar character);
    KTextEditor::Cursor rfindInternal(int line, int column, const QString &text, int attribute = -1);
    KTextEditor::Cursor rfind(KTextEditor::Cursor cursor, const QString &text, int attribute = -1);
    Q_INVOKABLE QJSValue rfind(int line, int column, const QString &text, int attribute = -1);
    Q_INVOKABLE QJSValue rfind(const QJSValue &cursor, const QString &text, int attribute = -1);

    Q_INVOKABLE int defStyleNum(int line, int column);
    Q_INVOKABLE int defStyleNum(const QJSValue &cursor);
    Q_INVOKABLE bool isCode(int line, int column);
    Q_INVOKABLE bool isCode(const QJSValue &cursor);
    Q_INVOKABLE bool isComment(int line, int column);
    Q_INVOKABLE bool isComment(const QJSValue &cursor);
    Q_INVOKABLE bool isString(int line, int column);
    Q_INVOKABLE bool isString(const QJSValue &cursor);
    Q_INVOKABLE bool isRegionMarker(int line, int column);
    Q_INVOKABLE bool isRegionMarker(const QJSValue &cursor);
    Q_INVOKABLE bool isChar(int line, int column);
    Q_INVOKABLE bool isChar(const QJSValue &cursor);
    Q_INVOKABLE bool isOthers(int line, int column);
    Q_INVOKABLE bool isOthers(const QJSValue &cursor);

    Q_INVOKABLE bool startsWith(int line, const QString &pattern, bool skipWhiteSpaces);
    Q_INVOKABLE bool endsWith(int line, const QString &pattern, bool skipWhiteSpaces);

    Q_INVOKABLE void indent(const QJSValue &jsrange, int change);

private:
    KTEXTEDITOR_NO_EXPORT
    static bool _isCode(int defaultStyle);

    KTextEditor::DocumentPrivate *m_document;
    QJSEngine *m_engine;
};

#endif
