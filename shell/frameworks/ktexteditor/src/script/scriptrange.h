/*
    SPDX-FileCopyrightText: 2017 Allan Sandfeld Jensen <kde@carewolf.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_SCRIPTRANGE_H
#define KTEXTEDITOR_SCRIPTRANGE_H

#include <QJSEngine>
#include <QJSValue>

#include "ktexteditor/range.h"
#include "scriptcursor.h"

inline QJSValue rangeToScriptValue(QJSEngine *engine, KTextEditor::Range range)
{
    const auto result = engine->globalObject()
                            .property(QStringLiteral("Range"))
                            .callAsConstructor(QJSValueList() << range.start().line() << range.start().column() << range.end().line() << range.end().column());
    Q_ASSERT(!result.isError());
    return result;
}

inline KTextEditor::Range rangeFromScriptValue(const QJSValue &obj)
{
    const auto start = cursorFromScriptValue(obj.property(QStringLiteral("start")));
    const auto end = cursorFromScriptValue(obj.property(QStringLiteral("end")));
    return KTextEditor::Range(start, end);
}

#endif
