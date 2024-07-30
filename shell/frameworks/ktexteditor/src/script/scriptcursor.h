/*
    SPDX-FileCopyrightText: 2017 Allan Sandfeld Jensen <kde@carewolf.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_SCRIPTCURSOR_H
#define KTEXTEDITOR_SCRIPTCURSOR_H

#include <QJSEngine>
#include <QJSValue>

#include "ktexteditor/cursor.h"

inline QJSValue cursorToScriptValue(QJSEngine *engine, const KTextEditor::Cursor cursor)
{
    const auto result = engine->globalObject().property(QStringLiteral("Cursor")).callAsConstructor(QJSValueList() << cursor.line() << cursor.column());
    Q_ASSERT(!result.isError());
    return result;
}

inline KTextEditor::Cursor cursorFromScriptValue(const QJSValue &obj)
{
    const auto line = obj.property(QStringLiteral("line"));
    const auto column = obj.property(QStringLiteral("column"));
    return KTextEditor::Cursor(line.toInt(), column.toInt());
}

#endif
