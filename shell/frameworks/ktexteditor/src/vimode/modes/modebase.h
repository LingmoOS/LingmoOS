/*
    SPDX-FileCopyrightText: 2008-2009 Erlend Hamberg <ehamberg@gmail.com>
    SPDX-FileCopyrightText: 2009 Paul Gideon Dann <pdgiddie@gmail.com>
    SPDX-FileCopyrightText: 2011 Svyatoslav Kuzmich <svatoslav1@gmail.com>
    SPDX-FileCopyrightText: 2012-2013 Simon St James <kdedevel@etotheipiplusone.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_MODE_BASE_H
#define KATEVI_MODE_BASE_H

#include <ktexteditor/range.h>

#include "kateview.h"
#include <vimode/definitions.h>
#include <vimode/range.h>

class QKeyEvent;
class QString;
class QRegularExpression;
class KateViewInternal;
namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
class Message;
}

namespace KateVi
{
class InputModeManager;

enum Direction { Up, Down, Left, Right, Next, Prev };

class ModeBase : public QObject
{
public:
    ModeBase() = default;
    ~ModeBase() override = default;

    /**
     * @return normal mode command accumulated so far
     */
    QString getVerbatimKeys() const;
    virtual bool handleKeypress(const QKeyEvent *e) = 0;

    void setCount(unsigned int count)
    {
        m_count = count;
    }
    void setRegister(QChar reg)
    {
        m_register = reg;
    }

    void error(const QString &errorMsg);
    void message(const QString &msg);

    Range motionFindNext();
    Range motionFindPrev();

    virtual void goToPos(const Range &r);
    int getCount() const;

protected:
    // helper methods
    void yankToClipBoard(QChar chosen_register, const QString &text);
    bool deleteRange(Range &r, OperationMode mode = LineWise, bool addToRegister = true);
    const QString getRange(Range &r, OperationMode mode = LineWise) const;
    const QString getLine(int line = -1) const;
    const QChar getCharUnderCursor() const;
    const QString getWordUnderCursor() const;
    const KTextEditor::Range getWordRangeUnderCursor() const;
    KTextEditor::Cursor findNextWordStart(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findNextWORDStart(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findPrevWordStart(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findPrevWORDStart(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findPrevWordEnd(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findPrevWORDEnd(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findWordEnd(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;
    KTextEditor::Cursor findWORDEnd(int fromLine, int fromColumn, bool onlyCurrentLine = false) const;

    Range findSurroundingBrackets(const QChar &c1, const QChar &c2, bool inner, const QChar &nested1, const QChar &nested2) const;

    Range findSurrounding(const QRegularExpression &c1, const QRegularExpression &c2, bool inner = false) const;
    Range findSurroundingQuotes(const QChar &c, bool inner = false) const;

    int findLineStartingWitchChar(const QChar &c, int count, bool forward = true) const;
    void updateCursor(const KTextEditor::Cursor c) const;
    static const QChar getCharAtVirtualColumn(const QString &line, int virtualColumn, int tabWidht);

    void addToNumberUnderCursor(int count);

    Range goLineUp();
    Range goLineDown();
    Range goLineUpDown(int lines);
    Range goVisualLineUpDown(int lines);

    unsigned int linesDisplayed() const;
    void scrollViewLines(int l);

    bool isCounted() const
    {
        return m_iscounted;
    }

    bool startNormalMode();
    bool startInsertMode();
    bool startVisualMode();
    bool startVisualLineMode();
    bool startVisualBlockMode();
    bool startReplaceMode();

    QChar getChosenRegister(const QChar &defaultReg) const;
    QString getRegisterContent(const QChar &reg);
    OperationMode getRegisterFlag(const QChar &reg) const;
    void fillRegister(const QChar &reg, const QString &text, OperationMode flag = CharWise);

    void switchView(Direction direction = Next);

    KTextEditor::Cursor getNextJump(KTextEditor::Cursor) const;
    KTextEditor::Cursor getPrevJump(KTextEditor::Cursor) const;

    inline KTextEditor::DocumentPrivate *doc() const
    {
        return m_view->doc();
    }

protected:
    QChar m_register;

    Range m_commandRange;
    unsigned int m_count = 0;
    int m_oneTimeCountOverride = -1;
    bool m_iscounted = false;

    QString m_extraWordCharacters;
    QString m_keysVerbatim;

    int m_stickyColumn = -1;
    bool m_lastMotionWasVisualLineUpOrDown;
    bool m_currentMotionWasVisualLineUpOrDown;

    KTextEditor::ViewPrivate *m_view;
    KateViewInternal *m_viewInternal;
    InputModeManager *m_viInputModeManager;

    // info message of vi mode
    QPointer<KTextEditor::Message> m_infoMessage;
};
}

#endif
