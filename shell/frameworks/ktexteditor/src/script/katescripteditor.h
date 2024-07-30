/*
    SPDX-FileCopyrightText: 2017 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SCRIPT_EDITOR_H
#define KATE_SCRIPT_EDITOR_H

#include <QJSValue>
#include <QObject>

#include <ktexteditor_export.h>

namespace KTextEditor
{
class ViewPrivate;
}
class QJSEngine;

/**
 * This class wraps the global editor instance KateGlobal, exposing some
 * helper methods such as the clipboard history etc.
 */
class KTEXTEDITOR_EXPORT KateScriptEditor : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;

    Q_INVOKABLE QString clipboardText() const;
    Q_INVOKABLE QStringList clipboardHistory() const;
    Q_INVOKABLE void setClipboardText(const QString &text);
};

#endif
