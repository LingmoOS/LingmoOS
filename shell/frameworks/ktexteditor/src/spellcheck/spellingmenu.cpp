/*
    SPDX-FileCopyrightText: 2009-2010 Michel Ludwig <michel.ludwig@kdemail.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spellingmenu.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"
#include "ontheflycheck.h"
#include "spellcheck/spellcheck.h"

#include "katepartdebug.h"

#include <QActionGroup>
#include <QMenu>

#include <KLocalizedString>
#include <KTextEditor/Range>

KateSpellingMenu::KateSpellingMenu(KTextEditor::ViewPrivate *view)
    : QObject(view)
    , m_view(view)
    , m_spellingMenuAction(nullptr)
    , m_ignoreWordAction(nullptr)
    , m_addToDictionaryAction(nullptr)
    , m_spellingMenu(nullptr)
    , m_currentMisspelledRange(nullptr)
{
}

KateSpellingMenu::~KateSpellingMenu()
{
    m_currentMisspelledRange = nullptr; // it shouldn't be accessed anymore as it could
}

bool KateSpellingMenu::isEnabled() const
{
    if (!m_spellingMenuAction) {
        return false;
    }
    return m_spellingMenuAction->isEnabled();
}

bool KateSpellingMenu::isVisible() const
{
    if (!m_spellingMenuAction) {
        return false;
    }
    return m_spellingMenuAction->isVisible();
}

void KateSpellingMenu::setEnabled(bool b)
{
    if (m_spellingMenuAction) {
        m_spellingMenuAction->setEnabled(b);
    }
}

void KateSpellingMenu::setVisible(bool b)
{
    if (m_spellingMenuAction) {
        m_spellingMenuAction->setVisible(b);
    }
}

void KateSpellingMenu::createActions(KActionCollection *ac)
{
    m_spellingMenuAction = new KActionMenu(i18n("Spelling"), this);
    ac->addAction(QStringLiteral("spelling_suggestions"), m_spellingMenuAction);
    m_spellingMenu = m_spellingMenuAction->menu();
    connect(m_spellingMenu, &QMenu::aboutToShow, this, &KateSpellingMenu::populateSuggestionsMenu);

    m_ignoreWordAction = new QAction(i18n("Ignore Word"), this);
    connect(m_ignoreWordAction, &QAction::triggered, this, &KateSpellingMenu::ignoreCurrentWord);

    m_addToDictionaryAction = new QAction(i18n("Add to Dictionary"), this);
    connect(m_addToDictionaryAction, &QAction::triggered, this, &KateSpellingMenu::addCurrentWordToDictionary);

    m_dictionaryGroup = new QActionGroup(this);
    QMapIterator<QString, QString> i(Sonnet::Speller().preferredDictionaries());
    while (i.hasNext()) {
        i.next();
        QAction *action = m_dictionaryGroup->addAction(i.key());
        action->setData(i.value());
    }
    connect(m_dictionaryGroup, &QActionGroup::triggered, [this](QAction *action) {
        if (m_selectedRange.isValid() && !m_selectedRange.isEmpty()) {
            const bool blockmode = m_view->blockSelection();
            m_view->doc()->setDictionary(action->data().toString(), m_selectedRange, blockmode);
        }
    });

    setVisible(false);
}

void KateSpellingMenu::caretEnteredMisspelledRange(KTextEditor::MovingRange *range)
{
    if (m_currentMisspelledRange == range) {
        return;
    }
    m_currentMisspelledRange = range;
}

void KateSpellingMenu::caretExitedMisspelledRange(KTextEditor::MovingRange *range)
{
    if (range != m_currentMisspelledRange) {
        // The order of 'exited' and 'entered' signals was wrong
        return;
    }
    m_currentMisspelledRange = nullptr;
}

void KateSpellingMenu::rangeDeleted(KTextEditor::MovingRange *range)
{
    if (m_currentMisspelledRange == range) {
        m_currentMisspelledRange = nullptr;
    }
}

void KateSpellingMenu::cleanUpAfterShown()
{
    // Ugly hack to avoid segfaults.
    // cleanUpAfterShown/ViewPrivate::aboutToHideContextMenu is called before
    // some action slot is processed.
    QTimer::singleShot(0, [this]() {
        if (m_currentMisspelledRangeNeedCleanUp) {
            m_currentMisspelledRange = nullptr;
            m_currentMisspelledRangeNeedCleanUp = false;
        }

        // We need to remove our list or they will accumulated on next show event
        for (auto act : m_menuOnTopSuggestionList) {
            qobject_cast<QWidget *>(act->parent())->removeAction(act);
            delete act;
        }
        m_menuOnTopSuggestionList.clear();
    });
}

void KateSpellingMenu::prepareToBeShown(QMenu *contextMenu)
{
    Q_ASSERT(contextMenu);

    if (!m_view->doc()->onTheFlySpellChecker()) {
        // Nothing todo!
        return;
    }

    m_selectedRange = m_view->selectionRange();
    if (m_selectedRange.isValid() && !m_selectedRange.isEmpty()) {
        // Selected words need a special handling to work properly
        auto imv = m_view->doc()->onTheFlySpellChecker()->installedMovingRanges(m_selectedRange);
        for (int i = 0; i < imv.size(); ++i) {
            if (imv.at(i)->toRange() == m_selectedRange) {
                m_currentMisspelledRange = imv.at(i);
                m_currentMisspelledRangeNeedCleanUp = true;
                break;
            }
        }
    }

    if (m_currentMisspelledRange != nullptr) {
        setVisible(true);
        m_selectedRange = m_currentMisspelledRange->toRange(); // Support actions of m_dictionaryGroup
        const QString &misspelledWord = m_view->doc()->text(*m_currentMisspelledRange);
        m_spellingMenuAction->setText(i18n("Spelling '%1'", misspelledWord));
        // Add suggestions on top of menu
        m_currentDictionary = m_view->doc()->dictionaryForMisspelledRange(*m_currentMisspelledRange);
        m_currentSuggestions = KTextEditor::EditorPrivate::self()->spellCheckManager()->suggestions(misspelledWord, m_currentDictionary);
        int counter = 5;
        QFont boldFont; // Emphasize on-top suggestions, so does Falkon
        boldFont.setBold(true);
        for (QStringList::const_iterator i = m_currentSuggestions.cbegin(); i != m_currentSuggestions.cend() && counter > 0; ++i) {
            const QString &suggestion = *i;
            QAction *action = new QAction(suggestion, contextMenu);
            action->setFont(boldFont);
            m_menuOnTopSuggestionList.append(action);
            connect(action, &QAction::triggered, this, [suggestion, this]() {
                replaceWordBySuggestion(suggestion);
            });
            m_spellingMenu->addAction(action);
            --counter;
        }
        contextMenu->insertActions(m_spellingMenuAction, m_menuOnTopSuggestionList);

    } else if (m_selectedRange.isValid() && !m_selectedRange.isEmpty()) {
        setVisible(true);
        m_spellingMenuAction->setText(i18n("Spelling"));
    } else {
        setVisible(false);
    }
}

void KateSpellingMenu::populateSuggestionsMenu()
{
    m_spellingMenu->clear();

    if (m_currentMisspelledRange) {
        m_spellingMenu->addAction(m_ignoreWordAction);
        m_spellingMenu->addAction(m_addToDictionaryAction);

        m_spellingMenu->addSeparator();
        bool dictFound = false;
        for (auto action : m_dictionaryGroup->actions()) {
            action->setCheckable(true);
            if (action->data().toString() == m_currentDictionary) {
                dictFound = true;
                action->setChecked(true);
            }
            m_spellingMenu->addAction(action);
        }
        if (!dictFound && !m_currentDictionary.isEmpty()) {
            const QString dictName = Sonnet::Speller().availableDictionaries().key(m_currentDictionary);
            QAction *action = m_dictionaryGroup->addAction(dictName);
            action->setData(m_currentDictionary);
            action->setCheckable(true);
            action->setChecked(true);
            m_spellingMenu->addAction(action);
        }

        m_spellingMenu->addSeparator();
        int counter = 10;
        for (QStringList::const_iterator i = m_currentSuggestions.cbegin(); i != m_currentSuggestions.cend() && counter > 0; ++i) {
            const QString &suggestion = *i;
            QAction *action = new QAction(suggestion, m_spellingMenu);
            connect(action, &QAction::triggered, this, [suggestion, this]() {
                replaceWordBySuggestion(suggestion);
            });
            m_spellingMenu->addAction(action);
            --counter;
        }

    } else if (m_selectedRange.isValid() && !m_selectedRange.isEmpty()) {
        for (auto action : m_dictionaryGroup->actions()) {
            action->setCheckable(false);
            m_spellingMenu->addAction(action);
        }
    }
}

void KateSpellingMenu::replaceWordBySuggestion(const QString &suggestion)
{
    if (!m_currentMisspelledRange) {
        return;
    }
    // Ensure we keep some special dictionary setting...
    const QString dictionary = m_view->doc()->dictionaryForMisspelledRange(*m_currentMisspelledRange);
    KTextEditor::Range newRange = m_currentMisspelledRange->toRange();
    newRange.setEnd(KTextEditor::Cursor(newRange.start().line(), newRange.start().column() + suggestion.size()));

    KTextEditor::DocumentPrivate *doc = m_view->doc();
    KTextEditor::EditorPrivate::self()->spellCheckManager()->replaceCharactersEncodedIfNecessary(suggestion, doc, *m_currentMisspelledRange);

    // ...on the replaced word
    m_view->doc()->setDictionary(dictionary, newRange);
    m_view->clearSelection(); // Ensure cursor move and next right click works properly if there was a selection
}

void KateSpellingMenu::addCurrentWordToDictionary()
{
    if (!m_currentMisspelledRange) {
        return;
    }
    const QString &misspelledWord = m_view->doc()->text(*m_currentMisspelledRange);
    const QString dictionary = m_view->doc()->dictionaryForMisspelledRange(*m_currentMisspelledRange);
    KTextEditor::EditorPrivate::self()->spellCheckManager()->addToDictionary(misspelledWord, dictionary);
    m_view->doc()->clearMisspellingForWord(misspelledWord); // WARNING: 'm_currentMisspelledRange' is deleted here!
    m_view->clearSelection();
}

void KateSpellingMenu::ignoreCurrentWord()
{
    if (!m_currentMisspelledRange) {
        return;
    }
    const QString &misspelledWord = m_view->doc()->text(*m_currentMisspelledRange);
    const QString dictionary = m_view->doc()->dictionaryForMisspelledRange(*m_currentMisspelledRange);
    KTextEditor::EditorPrivate::self()->spellCheckManager()->ignoreWord(misspelledWord, dictionary);
    m_view->doc()->clearMisspellingForWord(misspelledWord); // WARNING: 'm_currentMisspelledRange' is deleted here!
    m_view->clearSelection();
}

#include "moc_spellingmenu.cpp"
