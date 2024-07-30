/*
    SPDX-FileCopyrightText: 2008 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_INPUT_MODE_MANAGER_H
#define KATEVI_INPUT_MODE_MANAGER_H

#include <QKeyEvent>
#include <QStack>
#include <ktexteditor/cursor.h>
#include <ktexteditor/view.h>
#include <ktexteditor_export.h>

#include <vimode/completion.h>
#include <vimode/definitions.h>

class KConfigGroup;
class KateViewInternal;
class KateViInputMode;
class QString;

namespace KTextEditor
{
class ViewPrivate;
class DocumentPrivate;
class MovingCursor;
class Mark;
class MarkInterface;
}

namespace KateVi
{
class GlobalState;
class Searcher;
class CompletionRecorder;
class CompletionReplayer;
class Marks;
class Jumps;
class MacroRecorder;
class LastChangeRecorder;
class ModeBase;
class NormalViMode;
class InsertViMode;
class VisualViMode;
class ReplaceViMode;
class KeyParser;
class KeyMapper;

class InputModeManager
{
    friend KateViInputMode;

public:
    InputModeManager(KateViInputMode *inputAdapter, KTextEditor::ViewPrivate *view, KateViewInternal *viewInternal);
    ~InputModeManager();
    InputModeManager(const InputModeManager &) = delete;
    InputModeManager &operator=(const InputModeManager &) = delete;

    /**
     * feed key the given key press to the command parser
     * @return true if keypress was is [part of a] command, false otherwise
     */
    bool handleKeypress(const QKeyEvent *e);

    /**
     * feed key the given list of key presses to the key handling code, one by one
     */
    void feedKeyPresses(const QString &keyPresses) const;

    /**
     * Determines whether we are currently processing a Vi keypress
     * @return true if we are still in a call to handleKeypress, false otherwise
     */
    bool isHandlingKeypress() const;

    /**
     * @return The current vi mode
     */
    KTEXTEDITOR_EXPORT ViMode getCurrentViMode() const;

    /**
     * @return The current vi mode string representation
     */
    KTextEditor::View::ViewMode getCurrentViewMode() const;

    /**
     * @return the previous vi mode
     */
    ViMode getPreviousViMode() const;

    /**
     * @return true if and only if the current mode is one of VisualMode, VisualBlockMode or VisualLineMode.
     */
    bool isAnyVisualMode() const;

    /**
     * @return one of getViNormalMode(), getViVisualMode(), etc, depending on getCurrentViMode().
     */
    ModeBase *getCurrentViModeHandler() const;

    const QString getVerbatimKeys() const;

    /**
     * changes the current vi mode to the given mode
     */
    void changeViMode(ViMode newMode);

    /**
     * set normal mode to be the active vi mode and perform the needed setup work
     */
    KTEXTEDITOR_EXPORT void viEnterNormalMode();

    /**
     * set insert mode to be the active vi mode and perform the needed setup work
     */
    void viEnterInsertMode();

    /**
     * set visual mode to be the active vi mode and make the needed setup work
     */
    void viEnterVisualMode(ViMode visualMode = ViMode::VisualMode);

    /**
     * set replace mode to be the active vi mode and make the needed setup work
     */
    void viEnterReplaceMode();

    /**
     * @return the NormalMode instance
     */
    NormalViMode *getViNormalMode();

    /**
     * @return the InsertMode instance
     */
    InsertViMode *getViInsertMode();

    /**
     * @return the VisualMode instance
     */
    VisualViMode *getViVisualMode();

    /**
     * @return the ReplaceMode instance
     */
    ReplaceViMode *getViReplaceMode();

    /**
     * clear the key event log
     */
    void clearCurrentChangeLog();

    /**
     * copy the contents of the key events log to m_lastChange so that it can be repeated
     */
    void storeLastChangeCommand();

    /**
     * repeat last change by feeding the contents of m_lastChange to feedKeys()
     */
    void repeatLastChange();

    void doNotLogCurrentKeypress();

    bool getTemporaryNormalMode()
    {
        return m_temporaryNormalMode;
    }

    void setTemporaryNormalMode(bool b)
    {
        m_temporaryNormalMode = b;
    }

    void reset();

    inline Marks *marks()
    {
        return m_marks;
    }
    inline Jumps *jumps()
    {
        return m_jumps;
    }

    inline Searcher *searcher()
    {
        return m_searcher;
    }

    CompletionRecorder *completionRecorder()
    {
        return m_completionRecorder;
    }
    CompletionReplayer *completionReplayer()
    {
        return m_completionReplayer;
    }

    MacroRecorder *macroRecorder()
    {
        return m_macroRecorder;
    }

    LastChangeRecorder *lastChangeRecorder()
    {
        return m_lastChangeRecorder;
    }

    // session stuff
    void readSessionConfig(const KConfigGroup &config);
    void writeSessionConfig(KConfigGroup &config);

    KTEXTEDITOR_EXPORT KeyMapper *keyMapper();
    GlobalState *globalState() const;
    KTextEditor::ViewPrivate *view() const;

    KateViInputMode *inputAdapter()
    {
        return m_inputAdapter;
    }

    void updateCursor(const KTextEditor::Cursor c);

    void pushKeyMapper(std::shared_ptr<KeyMapper> mapper);
    void popKeyMapper();

private:
    NormalViMode *m_viNormalMode;
    InsertViMode *m_viInsertMode;
    VisualViMode *m_viVisualMode;
    ReplaceViMode *m_viReplaceMode;

    ViMode m_currentViMode;
    ViMode m_previousViMode;

    KateViInputMode *m_inputAdapter;
    KTextEditor::ViewPrivate *m_view;
    KateViewInternal *m_viewInternal;
    KeyParser *m_keyParser;

    // Create a new keymapper for each macro event, to simplify expansion of mappings in macros
    // where the macro itself was triggered by expanding a mapping!
    QStack<std::shared_ptr<KeyMapper>> m_keyMapperStack;

    int m_insideHandlingKeyPressCount;

    /**
     * a list of the (encoded) key events that was part of the last change.
     */
    QString m_lastChange;

    CompletionList m_lastChangeCompletionsLog;

    /**
     * true when normal mode was started by Ctrl-O command in insert mode.
     */
    bool m_temporaryNormalMode;

    Marks *m_marks;
    Jumps *m_jumps;

    Searcher *m_searcher;
    CompletionRecorder *m_completionRecorder;
    CompletionReplayer *m_completionReplayer;

    MacroRecorder *m_macroRecorder;

    LastChangeRecorder *m_lastChangeRecorder;
};

}

#endif
