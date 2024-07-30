/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_ABSTRACT_INPUT_MODE_H
#define KATE_ABSTRACT_INPUT_MODE_H

class KateLayoutCache;
class KateViewInternal;

#include "ktexteditor/view.h"

#include <QKeyEvent>

class KateViewInternal;
class KConfigGroup;
class QKeyEvent;
namespace KTextEditor
{
class ViewPrivate;
enum class caretStyles;
class Cursor;
}

class KateAbstractInputMode
{
protected:
    KateAbstractInputMode(KateViewInternal *);

public:
    virtual ~KateAbstractInputMode() = default;

    virtual KTextEditor::View::ViewMode viewMode() const = 0;
    virtual QString viewModeHuman() const = 0;
    virtual KTextEditor::View::InputMode viewInputMode() const = 0;
    virtual QString viewInputModeHuman() const = 0;

    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void reset() = 0;

    virtual bool overwrite() const = 0;
    virtual void overwrittenChar(const QChar &) = 0;
    virtual void clearSelection() = 0;
    virtual bool stealKey(QKeyEvent *k) = 0;

    virtual void gotFocus() = 0;
    virtual void lostFocus() = 0;

    virtual void readSessionConfig(const KConfigGroup &config) = 0;
    virtual void writeSessionConfig(KConfigGroup &config) = 0;
    virtual void updateRendererConfig() = 0;
    virtual void updateConfig() = 0;
    virtual void readWriteChanged(bool rw) = 0;

    virtual void find() = 0;
    virtual void findSelectedForwards() = 0;
    virtual void findSelectedBackwards() = 0;
    virtual void findReplace() = 0;
    virtual void findNext() = 0;
    virtual void findPrevious() = 0;

    virtual void activateCommandLine() = 0;

    virtual bool keyPress(QKeyEvent *) = 0;
    virtual bool blinkCaret() const = 0;
    virtual KTextEditor::caretStyles caretStyle() const = 0;

    virtual void toggleInsert() = 0;
    virtual void launchInteractiveCommand(const QString &command) = 0;

    virtual QString bookmarkLabel(int line) const = 0;

    /* functions that are currently view private, but vi-mode needs to access them */
public:
    void updateCursor(const KTextEditor::Cursor newCursor);
    KateLayoutCache *layoutCache() const;
    int linesDisplayed() const;
    void scrollViewLines(int offset);

protected:
    KateViewInternal *viewInternal() const
    {
        return m_viewInternal;
    }
    KTextEditor::ViewPrivate *view() const
    {
        return m_view;
    }

private:
    KateViewInternal *m_viewInternal;
    KTextEditor::ViewPrivate *m_view;
};

#endif
