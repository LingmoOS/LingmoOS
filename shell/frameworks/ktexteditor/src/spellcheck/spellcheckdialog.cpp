/*
    SPDX-FileCopyrightText: 2009-2010 Michel Ludwig <michel.ludwig@kdemail.net>
    SPDX-FileCopyrightText: 2008 Mirko Stocker <me@misto.ch>
    SPDX-FileCopyrightText: 2004-2005 Anders Lund <anders@alweb.dk>
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001-2004 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "spellcheckdialog.h"

#include "katedocument.h"
#include "kateglobal.h"
#include "kateview.h"
#include "spellcheck/spellcheck.h"
#include "spellcheck/spellcheckbar.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardActions>

#include <sonnet/backgroundchecker.h>
#include <sonnet/speller.h>

KateSpellCheckDialog::KateSpellCheckDialog(KTextEditor::ViewPrivate *view)
    : QObject(view)
    , m_view(view)
    , m_speller(nullptr)
    , m_backgroundChecker(nullptr)
    , m_sonnetDialog(nullptr)
    , m_globalSpellCheckRange(nullptr)
    , m_spellCheckCancelledByUser(false)
{
}

KateSpellCheckDialog::~KateSpellCheckDialog()
{
    delete m_globalSpellCheckRange;
    delete m_sonnetDialog;
    delete m_backgroundChecker;
    delete m_speller;
}

void KateSpellCheckDialog::createActions(KActionCollection *ac)
{
    ac->addAction(KStandardActions::Spelling, this, qOverload<>(&KateSpellCheckDialog::spellcheck));

    auto *a = new QAction(i18nc("@action", "Spelling (from Cursor)…"), this);
    ac->addAction(QStringLiteral("tools_spelling_from_cursor"), a);
    a->setIcon(QIcon::fromTheme(QStringLiteral("tools-check-spelling")));
    a->setWhatsThis(i18n("Check the document's spelling from the cursor and forward"));
    connect(a, &QAction::triggered, this, &KateSpellCheckDialog::spellcheckFromCursor);
}

void KateSpellCheckDialog::spellcheckFromCursor()
{
    if (m_view->selection()) {
        spellcheckSelection();
    } else {
        spellcheck(m_view->cursorPosition());
    }
}

void KateSpellCheckDialog::spellcheckSelection()
{
    spellcheck(m_view->selectionRange().start(), m_view->selectionRange().end());
}

void KateSpellCheckDialog::spellcheck()
{
    if (m_view->selection()) {
        spellcheckSelection();
    } else {
        spellcheck(KTextEditor::Cursor(0, 0));
    }
}

void KateSpellCheckDialog::spellcheck(const KTextEditor::Cursor from, const KTextEditor::Cursor to)
{
    KTextEditor::Cursor start = from;
    KTextEditor::Cursor end = to;

    if (end.line() == 0 && end.column() == 0) {
        end = m_view->doc()->documentEnd();
    }

    if (!m_speller) {
        m_speller = new Sonnet::Speller();
    }
    m_speller->restore();

    if (!m_backgroundChecker) {
        m_backgroundChecker = new Sonnet::BackgroundChecker(*m_speller);
    }

    if (!m_sonnetDialog) {
        m_sonnetDialog = new SpellCheckBar(m_backgroundChecker, m_view);
        m_sonnetDialog->showProgressDialog(200);
        m_sonnetDialog->showSpellCheckCompletionMessage();
        m_sonnetDialog->setSpellCheckContinuedAfterReplacement(false);

        connect(m_sonnetDialog, &SpellCheckBar::done, this, &KateSpellCheckDialog::installNextSpellCheckRange);

        connect(m_sonnetDialog, &SpellCheckBar::replace, this, &KateSpellCheckDialog::corrected);

        connect(m_sonnetDialog, &SpellCheckBar::misspelling, this, &KateSpellCheckDialog::misspelling);

        connect(m_sonnetDialog, &SpellCheckBar::cancel, this, &KateSpellCheckDialog::cancelClicked);

        connect(m_sonnetDialog, &SpellCheckBar::destroyed, this, &KateSpellCheckDialog::objectDestroyed);

        connect(m_sonnetDialog, &SpellCheckBar::languageChanged, this, &KateSpellCheckDialog::languageChanged);
    }

    m_view->bottomViewBar()->addBarWidget(m_sonnetDialog);

    m_userSpellCheckLanguage.clear();
    m_previousGivenSpellCheckLanguage.clear();
    delete m_globalSpellCheckRange;
    // we expand to handle the situation when the last word in the range is replace by a new one
    m_globalSpellCheckRange =
        m_view->doc()->newMovingRange(KTextEditor::Range(start, end), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
    m_spellCheckCancelledByUser = false;
    performSpellCheck(*m_globalSpellCheckRange);
}

KTextEditor::Cursor KateSpellCheckDialog::locatePosition(int pos)
{
    uint remains;

    while (m_spellLastPos < (uint)pos) {
        remains = pos - m_spellLastPos;
        uint l = m_view->doc()->lineLength(m_spellPosCursor.line()) - m_spellPosCursor.column();
        if (l > remains) {
            m_spellPosCursor.setColumn(m_spellPosCursor.column() + remains);
            m_spellLastPos = pos;
        } else {
            m_spellPosCursor.setLine(m_spellPosCursor.line() + 1);
            m_spellPosCursor.setColumn(0);
            m_spellLastPos += l + 1;
        }
    }

    return m_spellPosCursor;
}

void KateSpellCheckDialog::misspelling(const QString &word, int pos)
{
    KTextEditor::Cursor cursor;
    int length;
    int origPos = m_view->doc()->computePositionWrtOffsets(m_currentDecToEncOffsetList, pos);
    cursor = locatePosition(origPos);
    length = m_view->doc()->computePositionWrtOffsets(m_currentDecToEncOffsetList, pos + word.length()) - origPos;

    m_view->setCursorPositionInternal(cursor, 1);
    m_view->setSelection(KTextEditor::Range(cursor, length));
}

void KateSpellCheckDialog::corrected(const QString &word, int pos, const QString &newWord)
{
    int origPos = m_view->doc()->computePositionWrtOffsets(m_currentDecToEncOffsetList, pos);

    int length = m_view->doc()->computePositionWrtOffsets(m_currentDecToEncOffsetList, pos + word.length()) - origPos;

    KTextEditor::Cursor replacementStartCursor = locatePosition(origPos);
    KTextEditor::Range replacementRange = KTextEditor::Range(replacementStartCursor, length);
    KTextEditor::DocumentPrivate *doc = m_view->doc();
    KTextEditor::EditorPrivate::self()->spellCheckManager()->replaceCharactersEncodedIfNecessary(newWord, doc, replacementRange);

    // we have to be careful here: due to static word wrapping the text might change in addition to simply
    // the misspelled word being replaced, i.e. new line breaks might be inserted as well. As such, the text
    // in the 'Sonnet::Dialog' might be eventually out of sync with the visible text. Therefore, we 'restart'
    // spell checking from the current position.
    performSpellCheck(KTextEditor::Range(replacementStartCursor, m_globalSpellCheckRange->end()));
}

void KateSpellCheckDialog::performSpellCheck(KTextEditor::Range range)
{
    if (range.isEmpty()) {
        spellCheckDone();
        m_sonnetDialog->closed();
        return;
    }
    m_languagesInSpellCheckRange = KTextEditor::EditorPrivate::self()->spellCheckManager()->spellCheckLanguageRanges(m_view->doc(), range);
    m_currentLanguageRangeIterator = m_languagesInSpellCheckRange.begin();
    m_currentSpellCheckRange = KTextEditor::Range::invalid();
    installNextSpellCheckRange();
    // first check if there is really something to spell check
    if (m_currentSpellCheckRange.isValid()) {
        m_view->bottomViewBar()->showBarWidget(m_sonnetDialog);
        m_sonnetDialog->show();
        m_sonnetDialog->setFocus();
    } else {
        m_sonnetDialog->closed();
    }
}

void KateSpellCheckDialog::installNextSpellCheckRange()
{
    if (m_spellCheckCancelledByUser || m_currentLanguageRangeIterator == m_languagesInSpellCheckRange.end()) {
        spellCheckDone();
        return;
    }
    KateSpellCheckManager *spellCheckManager = KTextEditor::EditorPrivate::self()->spellCheckManager();
    KTextEditor::Cursor nextRangeBegin = (m_currentSpellCheckRange.isValid() ? m_currentSpellCheckRange.end() : KTextEditor::Cursor::invalid());
    m_currentSpellCheckRange = KTextEditor::Range::invalid();
    m_currentDecToEncOffsetList.clear();
    QList<QPair<KTextEditor::Range, QString>> rangeDictionaryPairList;
    while (m_currentLanguageRangeIterator != m_languagesInSpellCheckRange.end()) {
        KTextEditor::Range currentLanguageRange = (*m_currentLanguageRangeIterator).first;
        const QString &dictionary = (*m_currentLanguageRangeIterator).second;
        KTextEditor::Range languageSubRange =
            (nextRangeBegin.isValid() ? KTextEditor::Range(nextRangeBegin, currentLanguageRange.end()) : currentLanguageRange);
        rangeDictionaryPairList = spellCheckManager->spellCheckWrtHighlightingRanges(m_view->doc(), languageSubRange, dictionary, false, true);
        Q_ASSERT(rangeDictionaryPairList.size() <= 1);
        if (rangeDictionaryPairList.size() == 0) {
            ++m_currentLanguageRangeIterator;
            if (m_currentLanguageRangeIterator != m_languagesInSpellCheckRange.end()) {
                nextRangeBegin = (*m_currentLanguageRangeIterator).first.start();
            }
        } else {
            m_currentSpellCheckRange = rangeDictionaryPairList.first().first;
            QString dictionary = rangeDictionaryPairList.first().second;
            const bool languageChanged = (dictionary != m_previousGivenSpellCheckLanguage);
            m_previousGivenSpellCheckLanguage = dictionary;

            // if there was no change of dictionary stemming from the document language ranges and
            // the user has set a dictionary in the dialog, we use that one
            if (!languageChanged && !m_userSpellCheckLanguage.isEmpty()) {
                dictionary = m_userSpellCheckLanguage;
            }
            // we only allow the user to override the preset dictionary within a language range
            // given by the document
            else if (languageChanged) {
                m_userSpellCheckLanguage.clear();
            }

            m_spellPosCursor = m_currentSpellCheckRange.start();
            m_spellLastPos = 0;

            m_currentDecToEncOffsetList.clear();
            KTextEditor::DocumentPrivate::OffsetList encToDecOffsetList;
            QString text = m_view->doc()->decodeCharacters(m_currentSpellCheckRange, m_currentDecToEncOffsetList, encToDecOffsetList);
            // ensure that no empty string is passed on to Sonnet as this can lead to a crash
            // (bug 228789)
            if (text.isEmpty()) {
                nextRangeBegin = m_currentSpellCheckRange.end();
                continue;
            }

            if (m_speller->language() != dictionary) {
                m_speller->setLanguage(dictionary);
                m_backgroundChecker->setSpeller(*m_speller);
            }

            m_sonnetDialog->setBuffer(text);
            break;
        }
    }
    if (m_currentLanguageRangeIterator == m_languagesInSpellCheckRange.end()) {
        spellCheckDone();
        return;
    }
}

void KateSpellCheckDialog::cancelClicked()
{
    m_spellCheckCancelledByUser = true;
    spellCheckDone();
}

void KateSpellCheckDialog::spellCheckDone()
{
    m_currentSpellCheckRange = KTextEditor::Range::invalid();
    m_currentDecToEncOffsetList.clear();
    m_view->clearSelection();
}

void KateSpellCheckDialog::objectDestroyed(QObject *object)
{
    Q_UNUSED(object);
    m_sonnetDialog = nullptr;
}

void KateSpellCheckDialog::languageChanged(const QString &language)
{
    m_userSpellCheckLanguage = language;
}

// END

#include "moc_spellcheckdialog.cpp"
