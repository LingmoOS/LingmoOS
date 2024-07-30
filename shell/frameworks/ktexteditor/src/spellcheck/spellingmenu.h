/*
    SPDX-FileCopyrightText: 2008-2009 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SPELLINGMENU_H
#define SPELLINGMENU_H

#include <QObject>

#include <KActionCollection>
#include <KActionMenu>
#include <ktexteditor/movingrange.h>
#include <ktexteditor/movingrangefeedback.h>
#include <ktexteditor/range.h>
#include <ktexteditor/view.h>

namespace KTextEditor
{
class ViewPrivate;
}
class KateOnTheFlyChecker;

class KateSpellingMenu : public QObject
{
    Q_OBJECT
    friend class KateOnTheFlyChecker;

public:
    explicit KateSpellingMenu(KTextEditor::ViewPrivate *view);
    ~KateSpellingMenu() override;

    bool isEnabled() const;
    bool isVisible() const;

    void createActions(KActionCollection *ac);

    /**
     * This method has to be called before the menu is shown in response to a context
     * menu event. Ensure contextMenu is valid pointer!
     **/
    void prepareToBeShown(QMenu *contextMenu);

    /**
     * This method has to be called after a context menu event.
     **/
    void cleanUpAfterShown();

    void setEnabled(bool b);
    void setVisible(bool b);

protected:
    KTextEditor::ViewPrivate *m_view;
    KActionMenu *m_spellingMenuAction;
    QAction *m_ignoreWordAction, *m_addToDictionaryAction;
    QActionGroup *m_dictionaryGroup;
    QList<QAction *> m_menuOnTopSuggestionList;
    QMenu *m_spellingMenu;
    KTextEditor::MovingRange *m_currentMisspelledRange;
    /**
     * Set to true when a word was selected. Needed because in such case we got no "exited-notification"
     * and end up with an always active m_currentMisspelledRange
     **/
    bool m_currentMisspelledRangeNeedCleanUp = false;
    KTextEditor::Range m_selectedRange;
    QString m_currentDictionary;
    QStringList m_currentSuggestions;

    // These methods are called from KateOnTheFlyChecker to inform about events involving
    // moving ranges.
    void rangeDeleted(KTextEditor::MovingRange *range);
    void caretEnteredMisspelledRange(KTextEditor::MovingRange *range);
    void caretExitedMisspelledRange(KTextEditor::MovingRange *range);

protected Q_SLOTS:
    void populateSuggestionsMenu();
    void replaceWordBySuggestion(const QString &suggestion);

    void addCurrentWordToDictionary();
    void ignoreCurrentWord();
};

#endif
