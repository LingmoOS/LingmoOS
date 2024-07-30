/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007 Thomas Friedrichsmeier <thomas.friedrichsmeier@ruhr-uni-bochum.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katesearchbar.h"

#include "kateconfig.h"
#include "katedocument.h"
#include "kateglobal.h"
#include "katematch.h"
#include "kateundomanager.h"
#include "kateview.h"

#include <KTextEditor/DocumentCursor>
#include <KTextEditor/Message>
#include <KTextEditor/MovingRange>

#include "ui_searchbarincremental.h"
#include "ui_searchbarpower.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageBox>

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QElapsedTimer>
#include <QKeyEvent>
#include <QMenu>
#include <QRegularExpression>
#include <QShortcut>
#include <QStringListModel>
#include <QVBoxLayout>

#include <vector>

// Turn debug messages on/off here
// #define FAST_DEBUG_ENABLE

#ifdef FAST_DEBUG_ENABLE
#define FAST_DEBUG(x) qCDebug(LOG_KTE) << x
#else
#define FAST_DEBUG(x)
#endif

using namespace KTextEditor;

namespace
{
class AddMenuManager
{
private:
    QList<QString> m_insertBefore;
    QList<QString> m_insertAfter;
    QSet<QAction *> m_actionPointers;
    uint m_indexWalker;
    QMenu *m_menu;

public:
    AddMenuManager(QMenu *parent, int expectedItemCount)
        : m_insertBefore(QList<QString>(expectedItemCount))
        , m_insertAfter(QList<QString>(expectedItemCount))
        , m_indexWalker(0)
        , m_menu(nullptr)
    {
        Q_ASSERT(parent != nullptr);
        m_menu = parent->addMenu(i18n("Add..."));
        if (m_menu == nullptr) {
            return;
        }
        m_menu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    }

    void enableMenu(bool enabled)
    {
        if (m_menu == nullptr) {
            return;
        }
        m_menu->setEnabled(enabled);
    }

    void addEntry(const QString &before,
                  const QString &after,
                  const QString &description,
                  const QString &realBefore = QString(),
                  const QString &realAfter = QString())
    {
        if (m_menu == nullptr) {
            return;
        }
        QAction *const action = m_menu->addAction(before + after + QLatin1Char('\t') + description);
        m_insertBefore[m_indexWalker] = QString(realBefore.isEmpty() ? before : realBefore);
        m_insertAfter[m_indexWalker] = QString(realAfter.isEmpty() ? after : realAfter);
        action->setData(QVariant(m_indexWalker++));
        m_actionPointers.insert(action);
    }

    void addSeparator()
    {
        if (m_menu == nullptr) {
            return;
        }
        m_menu->addSeparator();
    }

    void handle(QAction *action, QLineEdit *lineEdit)
    {
        if (!m_actionPointers.contains(action)) {
            return;
        }

        const int cursorPos = lineEdit->cursorPosition();
        const int index = action->data().toUInt();
        const QString &before = m_insertBefore[index];
        const QString &after = m_insertAfter[index];
        lineEdit->insert(before + after);
        lineEdit->setCursorPosition(cursorPos + before.size());
        lineEdit->setFocus();
    }
};

} // anon namespace

KateSearchBar::KateSearchBar(bool initAsPower, KTextEditor::ViewPrivate *view, KateViewConfig *config)
    : KateViewBarWidget(true, view)
    , m_view(view)
    , m_config(config)
    , m_layout(new QVBoxLayout())
    , m_widget(nullptr)
    , m_incUi(nullptr)
    , m_incInitCursor(view->cursorPosition())
    , m_powerUi(nullptr)
    , highlightMatchAttribute(new Attribute())
    , highlightReplacementAttribute(new Attribute())
    , m_incHighlightAll(false)
    , m_incFromCursor(true)
    , m_incMatchCase(false)
    , m_powerMatchCase(true)
    , m_powerFromCursor(false)
    , m_powerHighlightAll(false)
    , m_powerMode(0)
{
    connect(view, &KTextEditor::View::cursorPositionChanged, this, &KateSearchBar::updateIncInitCursor);
    connect(view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);
    connect(this, &KateSearchBar::findOrReplaceAllFinished, this, &KateSearchBar::endFindOrReplaceAll);

    auto setSelectionChangedByUndoRedo = [this]() {
        m_selectionChangedByUndoRedo = true;
    };
    auto unsetSelectionChangedByUndoRedo = [this]() {
        m_selectionChangedByUndoRedo = false;
    };
    KateUndoManager *docUndoManager = view->doc()->undoManager();
    connect(docUndoManager, &KateUndoManager::undoStart, this, setSelectionChangedByUndoRedo);
    connect(docUndoManager, &KateUndoManager::undoEnd, this, unsetSelectionChangedByUndoRedo);
    connect(docUndoManager, &KateUndoManager::redoStart, this, setSelectionChangedByUndoRedo);
    connect(docUndoManager, &KateUndoManager::redoEnd, this, unsetSelectionChangedByUndoRedo);

    // When document is reloaded, disable selection-only search so that the search won't be stuck after the first search
    connect(view->doc(), &KTextEditor::Document::reloaded, this, [this]() {
        setSelectionOnly(false);
    });

    // init match attribute
    Attribute::Ptr mouseInAttribute(new Attribute());
    mouseInAttribute->setFontBold(true);
    highlightMatchAttribute->setDynamicAttribute(Attribute::ActivateMouseIn, mouseInAttribute);

    Attribute::Ptr caretInAttribute(new Attribute());
    caretInAttribute->setFontItalic(true);
    highlightMatchAttribute->setDynamicAttribute(Attribute::ActivateCaretIn, caretInAttribute);

    updateHighlightColors();

    // Modify parent
    QWidget *const widget = centralWidget();
    widget->setLayout(m_layout);
    m_layout->setContentsMargins(0, 0, 0, 0);

    // allow to have small size, for e.g. Kile
    setMinimumWidth(100);

    // Copy global to local config backup
    const auto searchFlags = m_config->searchFlags();
    m_incHighlightAll = (searchFlags & KateViewConfig::IncHighlightAll) != 0;
    m_incFromCursor = (searchFlags & KateViewConfig::IncFromCursor) != 0;
    m_incMatchCase = (searchFlags & KateViewConfig::IncMatchCase) != 0;
    m_powerMatchCase = (searchFlags & KateViewConfig::PowerMatchCase) != 0;
    m_powerFromCursor = (searchFlags & KateViewConfig::PowerFromCursor) != 0;
    m_powerHighlightAll = (searchFlags & KateViewConfig::PowerHighlightAll) != 0;
    m_powerMode = ((searchFlags & KateViewConfig::PowerModeRegularExpression) != 0)
        ? MODE_REGEX
        : (((searchFlags & KateViewConfig::PowerModeEscapeSequences) != 0)
               ? MODE_ESCAPE_SEQUENCES
               : (((searchFlags & KateViewConfig::PowerModeWholeWords) != 0) ? MODE_WHOLE_WORDS : MODE_PLAIN_TEXT));

    // Load one of either dialogs
    if (initAsPower) {
        enterPowerMode();
    } else {
        enterIncrementalMode();
    }

    updateSelectionOnly();
}

KateSearchBar::~KateSearchBar()
{
    if (!m_cancelFindOrReplace) {
        // Finish/Cancel the still running job to avoid a crash
        endFindOrReplaceAll();
    }

    clearHighlights();
    delete m_layout;
    delete m_widget;

    delete m_incUi;
    delete m_powerUi;
    if (m_workingRange) {
        delete m_workingRange;
    }
}

void KateSearchBar::closed()
{
    // remove search from the view bar, because it vertically bloats up the
    // stacked layout in KateViewBar.
    if (viewBar()) {
        viewBar()->removeBarWidget(this);
    }

    clearHighlights();
    m_replacement.clear();
    m_unfinishedSearchText.clear();
}

void KateSearchBar::setReplacementPattern(const QString &replacementPattern)
{
    Q_ASSERT(isPower());

    if (this->replacementPattern() == replacementPattern) {
        return;
    }

    m_powerUi->replacement->setEditText(replacementPattern);
}

QString KateSearchBar::replacementPattern() const
{
    Q_ASSERT(isPower());

    return m_powerUi->replacement->currentText();
}

void KateSearchBar::setSearchMode(KateSearchBar::SearchMode mode)
{
    Q_ASSERT(isPower());

    m_powerUi->searchMode->setCurrentIndex(mode);
}

void KateSearchBar::findNext()
{
    const bool found = find();

    if (found) {
        QComboBox *combo = m_powerUi != nullptr ? m_powerUi->pattern : m_incUi->pattern;

        // Add to search history
        addCurrentTextToHistory(combo);
    }
}

void KateSearchBar::findPrevious()
{
    const bool found = find(SearchBackward);

    if (found) {
        QComboBox *combo = m_powerUi != nullptr ? m_powerUi->pattern : m_incUi->pattern;

        // Add to search history
        addCurrentTextToHistory(combo);
    }
}

void KateSearchBar::showResultMessage()
{
    QString text;

    if (m_replaceMode) {
        text = i18ncp("short translation", "1 replacement made", "%1 replacements made", m_matchCounter);
    } else {
        text = i18ncp("short translation", "1 match found", "%1 matches found", m_matchCounter);
    }

    if (m_infoMessage) {
        m_infoMessage->setText(text);
    } else {
        m_infoMessage = new KTextEditor::Message(text, KTextEditor::Message::Positive);
        m_infoMessage->setPosition(KTextEditor::Message::BottomInView);
        m_infoMessage->setAutoHide(3000); // 3 seconds
        m_infoMessage->setView(m_view);
        m_view->doc()->postMessage(m_infoMessage);
    }
}

void KateSearchBar::highlightMatch(Range range)
{
    KTextEditor::MovingRange *const highlight = m_view->doc()->newMovingRange(range, Kate::TextRange::DoNotExpand);
    highlight->setView(m_view); // show only in this view
    highlight->setAttributeOnlyForViews(true);
    // use z depth defined in moving ranges interface
    highlight->setZDepth(-10000.0);
    highlight->setAttribute(highlightMatchAttribute);
    m_hlRanges.append(highlight);
}

void KateSearchBar::highlightReplacement(Range range)
{
    KTextEditor::MovingRange *const highlight = m_view->doc()->newMovingRange(range, Kate::TextRange::DoNotExpand);
    highlight->setView(m_view); // show only in this view
    highlight->setAttributeOnlyForViews(true);
    // use z depth defined in moving ranges interface
    highlight->setZDepth(-10000.0);
    highlight->setAttribute(highlightReplacementAttribute);
    m_hlRanges.append(highlight);
}

void KateSearchBar::indicateMatch(MatchResult matchResult)
{
    QLineEdit *const lineEdit = isPower() ? m_powerUi->pattern->lineEdit() : m_incUi->pattern->lineEdit();
    QPalette background(lineEdit->palette());

    switch (matchResult) {
    case MatchFound: // FALLTHROUGH
    case MatchWrappedForward:
    case MatchWrappedBackward:
        // Green background for line edit
        KColorScheme::adjustBackground(background, KColorScheme::PositiveBackground);
        break;
    case MatchMismatch:
        // Red background for line edit
        KColorScheme::adjustBackground(background, KColorScheme::NegativeBackground);
        break;
    case MatchNothing:
        // Reset background of line edit
        background = QPalette();
        break;
    case MatchNeutral:
        KColorScheme::adjustBackground(background, KColorScheme::NeutralBackground);
        break;
    }

    // Update status label
    if (m_incUi != nullptr) {
        QPalette foreground(m_incUi->status->palette());
        switch (matchResult) {
        case MatchFound: // FALLTHROUGH
        case MatchNothing:
            KColorScheme::adjustForeground(foreground, KColorScheme::NormalText, QPalette::WindowText, KColorScheme::Window);
            m_incUi->status->clear();
            break;
        case MatchWrappedForward:
        case MatchWrappedBackward:
            KColorScheme::adjustForeground(foreground, KColorScheme::NormalText, QPalette::WindowText, KColorScheme::Window);
            if (matchResult == MatchWrappedBackward) {
                m_incUi->status->setText(i18n("Reached top, continued from bottom"));
            } else {
                m_incUi->status->setText(i18n("Reached bottom, continued from top"));
            }
            break;
        case MatchMismatch:
            KColorScheme::adjustForeground(foreground, KColorScheme::NegativeText, QPalette::WindowText, KColorScheme::Window);
            m_incUi->status->setText(i18n("Not found"));
            break;
        case MatchNeutral:
            /* do nothing */
            break;
        }
        m_incUi->status->setPalette(foreground);
    }

    lineEdit->setPalette(background);
}

/*static*/ void KateSearchBar::selectRange(KTextEditor::ViewPrivate *view, KTextEditor::Range range)
{
    view->setCursorPositionInternal(range.end());
    view->setSelection(range);
}

void KateSearchBar::selectRange2(KTextEditor::Range range)
{
    disconnect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);
    selectRange(m_view, range);
    connect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);
}

void KateSearchBar::onIncPatternChanged(const QString &pattern)
{
    if (!m_incUi) {
        return;
    }

    // clear prior highlightings (deletes info message if present)
    clearHighlights();

    m_incUi->next->setDisabled(pattern.isEmpty());
    m_incUi->prev->setDisabled(pattern.isEmpty());

    KateMatch match(m_view->doc(), searchOptions());

    if (!pattern.isEmpty()) {
        // Find, first try
        const Range inputRange = KTextEditor::Range(m_incInitCursor, m_view->document()->documentEnd());
        match.searchText(inputRange, pattern);
    }

    const bool wrap = !match.isValid() && !pattern.isEmpty();

    if (wrap) {
        // Find, second try
        const KTextEditor::Range inputRange = m_view->document()->documentRange();
        match.searchText(inputRange, pattern);
    }

    const MatchResult matchResult = match.isValid() ? (wrap ? MatchWrappedForward : MatchFound) : pattern.isEmpty() ? MatchNothing : MatchMismatch;

    const Range selectionRange = pattern.isEmpty() ? Range(m_incInitCursor, m_incInitCursor) : match.isValid() ? match.range() : Range::invalid();

    // don't update m_incInitCursor when we move the cursor
    disconnect(m_view, &KTextEditor::View::cursorPositionChanged, this, &KateSearchBar::updateIncInitCursor);
    selectRange2(selectionRange);
    connect(m_view, &KTextEditor::View::cursorPositionChanged, this, &KateSearchBar::updateIncInitCursor);

    indicateMatch(matchResult);
}

void KateSearchBar::setMatchCase(bool matchCase)
{
    if (this->matchCase() == matchCase) {
        return;
    }

    if (isPower()) {
        m_powerUi->matchCase->setChecked(matchCase);
    } else {
        m_incUi->matchCase->setChecked(matchCase);
    }
}

void KateSearchBar::onMatchCaseToggled(bool /*matchCase*/)
{
    sendConfig();

    if (m_incUi != nullptr) {
        // Re-search with new settings
        const QString pattern = m_incUi->pattern->currentText();
        onIncPatternChanged(pattern);
    } else {
        indicateMatch(MatchNothing);
    }
}

bool KateSearchBar::matchCase() const
{
    return isPower() ? m_powerUi->matchCase->isChecked() : m_incUi->matchCase->isChecked();
}

void KateSearchBar::onReturnPressed()
{
    const Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    const bool shiftDown = (modifiers & Qt::ShiftModifier) != 0;
    const bool controlDown = (modifiers & Qt::ControlModifier) != 0;

    if (shiftDown) {
        // Shift down, search backwards
        findPrevious();
    } else {
        // Shift up, search forwards
        findNext();
    }

    if (controlDown) {
        Q_EMIT hideMe();
    }
}

bool KateSearchBar::findOrReplace(SearchDirection searchDirection, const QString *replacement)
{
    // What to find?
    if (searchPattern().isEmpty()) {
        return false; // == Pattern error
    }

    // don't let selectionChanged signal mess around in this routine
    disconnect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);

    // clear previous highlights if there are any
    clearHighlights();

    const SearchOptions enabledOptions = searchOptions(searchDirection);

    // Where to find?
    Range inputRange;
    const Range selection = m_view->selection() ? m_view->selectionRange() : Range::invalid();
    if (selection.isValid()) {
        if (selectionOnly()) {
            if (m_workingRange == nullptr) {
                m_workingRange =
                    m_view->doc()->newMovingRange(KTextEditor::Range::invalid(), KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
            }
            if (!m_workingRange->toRange().isValid()) {
                // First match in selection
                inputRange = selection;
                // Remember selection for succeeding selection-only searches
                // Elsewhere, make sure m_workingRange is invalidated when selection/search range changes
                m_workingRange->setRange(selection);
            } else {
                // The selection wasn't changed/updated by user, so we use the previous selection
                // We use the selection's start/end so that the search can move forward/backward
                if (searchDirection == SearchBackward) {
                    inputRange.setRange(m_workingRange->start(), selection.end());
                } else {
                    inputRange.setRange(selection.start(), m_workingRange->end());
                }
            }
        } else {
            // Next match after/before selection if a match was selected before
            if (searchDirection == SearchForward) {
                inputRange.setRange(selection.start(), m_view->document()->documentEnd());
            } else {
                inputRange.setRange(Cursor(0, 0), selection.end());
            }

            // Discard selection/search range previously remembered
            if (m_workingRange) {
                delete m_workingRange;
                m_workingRange = nullptr;
            }
        }
    } else {
        // No selection
        setSelectionOnly(false);
        const Cursor cursorPos = m_view->cursorPosition();
        if (searchDirection == SearchForward) {
            inputRange.setRange(cursorPos, m_view->document()->documentEnd());
        } else {
            inputRange.setRange(Cursor(0, 0), cursorPos);
        }
    }
    FAST_DEBUG("Search range is" << inputRange);

    KateMatch match(m_view->doc(), enabledOptions);
    Range afterReplace = Range::invalid();

    // Find, first try
    match.searchText(inputRange, searchPattern());
    if (match.isValid()) {
        if (match.range() == selection) {
            // Same match again
            if (replacement != nullptr) {
                // Selection is match -> replace
                KTextEditor::MovingRange *smartInputRange =
                    m_view->doc()->newMovingRange(inputRange, KTextEditor::MovingRange::ExpandLeft | KTextEditor::MovingRange::ExpandRight);
                afterReplace = match.replace(*replacement, m_view->blockSelection());
                inputRange = *smartInputRange;
                delete smartInputRange;
            }

            // Find, second try after old selection
            if (searchDirection == SearchForward) {
                const Cursor start = (replacement != nullptr) ? afterReplace.end() : selection.end();
                inputRange.setRange(start, inputRange.end());
            } else {
                const Cursor end = (replacement != nullptr) ? afterReplace.start() : selection.start();
                inputRange.setRange(inputRange.start(), end);
            }

            match.searchText(inputRange, searchPattern());

        } else if (match.isEmpty() && match.range().end() == m_view->cursorPosition()) {
            // valid zero-length match, e.g.: '^', '$', '\b'
            // advance the range to avoid looping
            KTextEditor::DocumentCursor zeroLenMatch(m_view->doc(), match.range().end());

            if (searchDirection == SearchForward) {
                zeroLenMatch.move(1);
                inputRange.setRange(zeroLenMatch.toCursor(), inputRange.end());
            } else { // SearchBackward
                zeroLenMatch.move(-1);
                inputRange.setRange(inputRange.start(), zeroLenMatch.toCursor());
            }

            match.searchText(inputRange, searchPattern());
        }
    }

    bool askWrap = !match.isValid() && (!afterReplace.isValid() || !selectionOnly());
    bool wrap = false;
    if (askWrap) {
        askWrap = false;
        wrap = true;
    }

    if (askWrap) {
        QString question =
            searchDirection == SearchForward ? i18n("Bottom of file reached. Continue from top?") : i18n("Top of file reached. Continue from bottom?");
        wrap = (KMessageBox::questionTwoActions(nullptr,
                                                question,
                                                i18n("Continue search?"),
                                                KStandardGuiItem::cont(),
                                                KStandardGuiItem::cancel(),
                                                QStringLiteral("DoNotShowAgainContinueSearchDialog"))
                == KMessageBox::PrimaryAction);
    }
    if (wrap) {
        m_view->showSearchWrappedHint(searchDirection == SearchBackward);
        if (selectionOnly() && m_workingRange != nullptr && m_workingRange->toRange().isValid()) {
            inputRange = m_workingRange->toRange();
        } else {
            inputRange = m_view->document()->documentRange();
        }
        match.searchText(inputRange, searchPattern());
    }

    if (match.isValid()) {
        selectRange2(match.range());
    }

    const MatchResult matchResult = !match.isValid() ? MatchMismatch
        : !wrap                                      ? MatchFound
        : searchDirection == SearchForward           ? MatchWrappedForward
                                                     : MatchWrappedBackward;
    indicateMatch(matchResult);

    // highlight replacements if applicable
    if (afterReplace.isValid()) {
        highlightReplacement(afterReplace);
    }

    // restore connection
    connect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);

    return true; // == No pattern error
}

void KateSearchBar::findAll()
{
    // clear highlightings of prior search&replace action
    clearHighlights();

    Range inputRange = (m_view->selection() && selectionOnly()) ? m_view->selectionRange() : m_view->document()->documentRange();

    beginFindAll(inputRange);
}

void KateSearchBar::onPowerPatternChanged(const QString & /*pattern*/)
{
    givePatternFeedback();
    indicateMatch(MatchNothing);
}

bool KateSearchBar::isPatternValid() const
{
    if (searchPattern().isEmpty()) {
        return false;
    }

    return searchOptions().testFlag(WholeWords) ? searchPattern().trimmed() == searchPattern()
        : searchOptions().testFlag(Regex)       ? QRegularExpression(searchPattern(), QRegularExpression::UseUnicodePropertiesOption).isValid()
                                                : true;
}

void KateSearchBar::givePatternFeedback()
{
    // Enable/disable next/prev and replace next/all
    m_powerUi->findNext->setEnabled(isPatternValid());
    m_powerUi->findPrev->setEnabled(isPatternValid());
    m_powerUi->replaceNext->setEnabled(isPatternValid());
    m_powerUi->replaceAll->setEnabled(isPatternValid());
    m_powerUi->findAll->setEnabled(isPatternValid());
}

void KateSearchBar::addCurrentTextToHistory(QComboBox *combo)
{
    const QString text = combo->currentText();
    const int index = combo->findText(text);

    if (index > 0) {
        combo->removeItem(index);
    }
    if (index != 0) {
        combo->insertItem(0, text);
        combo->setCurrentIndex(0);
    }

    // sync to application config
    KTextEditor::EditorPrivate::self()->saveSearchReplaceHistoryModels();
}

void KateSearchBar::backupConfig(bool ofPower)
{
    if (ofPower) {
        m_powerMatchCase = m_powerUi->matchCase->isChecked();
        m_powerMode = m_powerUi->searchMode->currentIndex();
    } else {
        m_incMatchCase = m_incUi->matchCase->isChecked();
    }
}

void KateSearchBar::sendConfig()
{
    const auto pastFlags = m_config->searchFlags();
    auto futureFlags = pastFlags;

    if (m_powerUi != nullptr) {
        const bool OF_POWER = true;
        backupConfig(OF_POWER);

        // Update power search flags only
        const auto incFlagsOnly = pastFlags & (KateViewConfig::IncHighlightAll | KateViewConfig::IncFromCursor | KateViewConfig::IncMatchCase);

        futureFlags = incFlagsOnly | (m_powerMatchCase ? KateViewConfig::PowerMatchCase : 0) | (m_powerFromCursor ? KateViewConfig::PowerFromCursor : 0)
            | (m_powerHighlightAll ? KateViewConfig::PowerHighlightAll : 0)
            | ((m_powerMode == MODE_REGEX)
                   ? KateViewConfig::PowerModeRegularExpression
                   : ((m_powerMode == MODE_ESCAPE_SEQUENCES)
                          ? KateViewConfig::PowerModeEscapeSequences
                          : ((m_powerMode == MODE_WHOLE_WORDS) ? KateViewConfig::PowerModeWholeWords : KateViewConfig::PowerModePlainText)));

    } else if (m_incUi != nullptr) {
        const bool OF_INCREMENTAL = false;
        backupConfig(OF_INCREMENTAL);

        // Update incremental search flags only
        const auto powerFlagsOnly = pastFlags
            & (KateViewConfig::PowerMatchCase | KateViewConfig::PowerFromCursor | KateViewConfig::PowerHighlightAll | KateViewConfig::PowerModeRegularExpression
               | KateViewConfig::PowerModeEscapeSequences | KateViewConfig::PowerModeWholeWords | KateViewConfig::PowerModePlainText);

        futureFlags = powerFlagsOnly | (m_incHighlightAll ? KateViewConfig::IncHighlightAll : 0) | (m_incFromCursor ? KateViewConfig::IncFromCursor : 0)
            | (m_incMatchCase ? KateViewConfig::IncMatchCase : 0);
    }

    // Adjust global config
    m_config->setSearchFlags(futureFlags);
}

void KateSearchBar::replaceNext()
{
    const QString replacement = m_powerUi->replacement->currentText();

    if (findOrReplace(SearchForward, &replacement)) {
        // Never merge replace actions with other replace actions/user actions
        m_view->doc()->undoManager()->undoSafePoint();

        // Add to search history
        addCurrentTextToHistory(m_powerUi->pattern);

        // Add to replace history
        addCurrentTextToHistory(m_powerUi->replacement);
    }
}

// replacement == NULL --> Only highlight all matches
// replacement != NULL --> Replace and highlight all matches
void KateSearchBar::beginFindOrReplaceAll(Range inputRange, const QString &replacement, bool replaceMode /* = true*/)
{
    // don't let selectionChanged signal mess around in this routine
    disconnect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);
    // Cancel job when user close the document to avoid crash
    connect(m_view->doc(), &KTextEditor::Document::aboutToClose, this, &KateSearchBar::endFindOrReplaceAll);

    if (m_powerUi) {
        // Offer Cancel button and disable not useful buttons
        m_powerUi->searchCancelStacked->setCurrentIndex(m_powerUi->searchCancelStacked->indexOf(m_powerUi->cancelPage));
        m_powerUi->findNext->setEnabled(false);
        m_powerUi->findPrev->setEnabled(false);
        m_powerUi->replaceNext->setEnabled(false);
    }

    m_highlightRanges.clear();
    m_inputRange = inputRange;
    m_workingRange = m_view->doc()->newMovingRange(m_inputRange);
    m_replacement = replacement;
    m_replaceMode = replaceMode;
    m_matchCounter = 0;
    m_cancelFindOrReplace = false; // Ensure we have a GO!

    findOrReplaceAll();
}

void KateSearchBar::findOrReplaceAll()
{
    const SearchOptions enabledOptions = searchOptions(SearchForward);

    // we highlight all ranges of a replace, up to some hard limit
    // e.g. if you replace 100000 things, rendering will break down otherwise ;=)
    const int maxHighlightings = 65536;

    // reuse match object to avoid massive moving range creation
    KateMatch match(m_view->doc(), enabledOptions);

    // Ignore block mode if selectionOnly option is disabled (see bug 456367)
    bool block = m_view->selection() && m_view->blockSelection() && selectionOnly();

    int line = m_inputRange.start().line();

    bool timeOut = false;
    bool done = false;

    // This variable holds the number of lines that we have searched
    // When it reaches 50K, we break the loop to allow event processing
    int numLinesSearched = 0;
    // Use a simple range in the loop to avoid needless work
    KTextEditor::Range workingRangeCopy = m_workingRange->toRange();
    do {
        if (block) {
            delete m_workingRange; // Never forget that!
            m_workingRange = m_view->doc()->newMovingRange(m_view->doc()->rangeOnLine(m_inputRange, line));
            workingRangeCopy = m_workingRange->toRange();
        }

        do {
            int currentSearchLine = workingRangeCopy.start().line();
            match.searchText(workingRangeCopy, searchPattern());
            if (!match.isValid()) {
                done = true;
                break;
            }
            bool const originalMatchEmpty = match.isEmpty();

            // Work with the match
            Range lastRange;
            if (m_replaceMode) {
                if (m_matchCounter == 0) {
                    static_cast<KTextEditor::DocumentPrivate *>(m_view->document())->editStart();
                }

                // Replace
                lastRange = match.replace(m_replacement, false, ++m_matchCounter);
                // update working range as text must have changed now
                workingRangeCopy = m_workingRange->toRange();
            } else {
                lastRange = match.range();
                ++m_matchCounter;
            }

            // remember ranges if limit not reached
            if (m_matchCounter < maxHighlightings) {
                m_highlightRanges.push_back(lastRange);
            } else {
                m_highlightRanges.clear();
                // TODO Info user that highlighting is disabled
            }

            // Continue after match
            if (lastRange.end() >= workingRangeCopy.end()) {
                done = true;
                break;
            }

            KTextEditor::DocumentCursor workingStart(m_view->doc(), lastRange.end());

            if (originalMatchEmpty) {
                // Can happen for regex patterns with zero-length matches, e.g. ^, $, \b
                // If we don't advance here we will loop forever...
                workingStart.move(1);
            }
            workingRangeCopy.setRange(workingStart.toCursor(), workingRangeCopy.end());

            // Are we done?
            if (!workingRangeCopy.isValid() || workingStart.atEndOfDocument()) {
                done = true;
                break;
            }

            // Check if we have searched through 50K lines and time out.
            // We do this to allow the search operation to be cancelled
            numLinesSearched += workingRangeCopy.start().line() - currentSearchLine;
            timeOut = numLinesSearched >= 50000;

        } while (!m_cancelFindOrReplace && !timeOut);

    } while (!m_cancelFindOrReplace && !timeOut && block && ++line <= m_inputRange.end().line());

    // update m_workingRange
    m_workingRange->setRange(workingRangeCopy);

    if (done || m_cancelFindOrReplace) {
        Q_EMIT findOrReplaceAllFinished();
    } else if (timeOut) {
        QTimer::singleShot(0, this, &KateSearchBar::findOrReplaceAll);
    }

    showResultMessage();
}

void KateSearchBar::endFindOrReplaceAll()
{
    // Don't forget to remove our "crash protector"
    disconnect(m_view->doc(), &KTextEditor::Document::aboutToClose, this, &KateSearchBar::endFindOrReplaceAll);

    // After last match
    if (m_matchCounter > 0) {
        if (m_replaceMode) {
            static_cast<KTextEditor::DocumentPrivate *>(m_view->document())->editEnd();
        }
    }

    // Add ScrollBarMarks
    if (!m_highlightRanges.empty()) {
        m_view->document()->setMarkDescription(KTextEditor::Document::SearchMatch, i18n("SearchHighLight"));
        m_view->document()->setMarkIcon(KTextEditor::Document::SearchMatch, QIcon());
        for (const Range &r : m_highlightRanges) {
            m_view->document()->addMark(r.start().line(), KTextEditor::Document::SearchMatch);
        }
    }

    // Add highlights
    if (m_replaceMode) {
        for (const Range &r : std::as_const(m_highlightRanges)) {
            highlightReplacement(r);
        }
        // Never merge replace actions with other replace actions/user actions
        m_view->doc()->undoManager()->undoSafePoint();

    } else {
        for (const Range &r : std::as_const(m_highlightRanges)) {
            highlightMatch(r);
        }
        //         indicateMatch(m_matchCounter > 0 ? MatchFound : MatchMismatch); TODO
    }

    // Clean-Up the still hold MovingRange
    delete m_workingRange;
    m_workingRange = nullptr; // m_workingRange is also used elsewhere so we signify that it is now "unused"

    // restore connection
    connect(m_view, &KTextEditor::View::selectionChanged, this, &KateSearchBar::updateSelectionOnly);

    if (m_powerUi) {
        // Offer Find and Replace buttons and enable again useful buttons
        m_powerUi->searchCancelStacked->setCurrentIndex(m_powerUi->searchCancelStacked->indexOf(m_powerUi->searchPage));
        m_powerUi->findNext->setEnabled(true);
        m_powerUi->findPrev->setEnabled(true);
        m_powerUi->replaceNext->setEnabled(true);

        // Add to search history
        addCurrentTextToHistory(m_powerUi->pattern);

        // Add to replace history
        addCurrentTextToHistory(m_powerUi->replacement);
    }

    m_cancelFindOrReplace = true; // Indicate we are not running
}

void KateSearchBar::replaceAll()
{
    // clear prior highlightings (deletes info message if present)
    clearHighlights();

    // What to find/replace?
    const QString replacement = m_powerUi->replacement->currentText();

    // Where to replace?
    const bool selected = m_view->selection();
    Range inputRange = (selected && selectionOnly()) ? m_view->selectionRange() : m_view->document()->documentRange();

    beginFindOrReplaceAll(inputRange, replacement);
}

void KateSearchBar::setSearchPattern(const QString &searchPattern)
{
    if (searchPattern == this->searchPattern()) {
        return;
    }

    if (isPower()) {
        m_powerUi->pattern->setEditText(searchPattern);
    } else {
        m_incUi->pattern->setEditText(searchPattern);
    }
}

QString KateSearchBar::searchPattern() const
{
    return (m_powerUi != nullptr) ? m_powerUi->pattern->currentText() : m_incUi->pattern->currentText();
}

void KateSearchBar::setSelectionOnly(bool selectionOnly)
{
    if (this->selectionOnly() == selectionOnly) {
        return;
    }

    if (isPower()) {
        m_powerUi->selectionOnly->setChecked(selectionOnly);
    }
}

bool KateSearchBar::selectionOnly() const
{
    return isPower() ? m_powerUi->selectionOnly->isChecked() : false;
}

KTextEditor::SearchOptions KateSearchBar::searchOptions(SearchDirection searchDirection) const
{
    SearchOptions enabledOptions = KTextEditor::Default;

    if (!matchCase()) {
        enabledOptions |= CaseInsensitive;
    }

    if (searchDirection == SearchBackward) {
        enabledOptions |= Backwards;
    }

    if (m_powerUi != nullptr) {
        switch (m_powerUi->searchMode->currentIndex()) {
        case MODE_WHOLE_WORDS:
            enabledOptions |= WholeWords;
            break;

        case MODE_ESCAPE_SEQUENCES:
            enabledOptions |= EscapeSequences;
            break;

        case MODE_REGEX:
            enabledOptions |= Regex;
            break;

        case MODE_PLAIN_TEXT: // FALLTHROUGH
        default:
            break;
        }
    }

    return enabledOptions;
}

struct ParInfo {
    int openIndex;
    bool capturing;
    int captureNumber; // 1..9
};

QList<QString> KateSearchBar::getCapturePatterns(const QString &pattern) const
{
    QList<QString> capturePatterns;
    capturePatterns.reserve(9);
    QStack<ParInfo> parInfos;

    const int inputLen = pattern.length();
    int input = 0; // walker index
    bool insideClass = false;
    int captureCount = 0;

    while (input < inputLen) {
        if (insideClass) {
            // Wait for closing, unescaped ']'
            if (pattern[input].unicode() == L']') {
                insideClass = false;
            }
            input++;
        } else {
            switch (pattern[input].unicode()) {
            case L'\\':
                // Skip this and any next character
                input += 2;
                break;

            case L'(':
                ParInfo curInfo;
                curInfo.openIndex = input;
                curInfo.capturing = (input + 1 >= inputLen) || (pattern[input + 1].unicode() != '?');
                if (curInfo.capturing) {
                    captureCount++;
                }
                curInfo.captureNumber = captureCount;
                parInfos.push(curInfo);

                input++;
                break;

            case L')':
                if (!parInfos.empty()) {
                    ParInfo &top = parInfos.top();
                    if (top.capturing && (top.captureNumber <= 9)) {
                        const int start = top.openIndex + 1;
                        const int len = input - start;
                        if (capturePatterns.size() < top.captureNumber) {
                            capturePatterns.resize(top.captureNumber);
                        }
                        capturePatterns[top.captureNumber - 1] = pattern.mid(start, len);
                    }
                    parInfos.pop();
                }

                input++;
                break;

            case L'[':
                input++;
                insideClass = true;
                break;

            default:
                input++;
                break;
            }
        }
    }

    return capturePatterns;
}

void KateSearchBar::showExtendedContextMenu(bool forPattern, const QPoint &pos)
{
    // Make original menu
    QComboBox *comboBox = forPattern ? m_powerUi->pattern : m_powerUi->replacement;
    QMenu *const contextMenu = comboBox->lineEdit()->createStandardContextMenu();

    if (contextMenu == nullptr) {
        return;
    }

    bool extendMenu = false;
    bool regexMode = false;
    switch (m_powerUi->searchMode->currentIndex()) {
    case MODE_REGEX:
        regexMode = true;
        // FALLTHROUGH

    case MODE_ESCAPE_SEQUENCES:
        extendMenu = true;
        break;

    default:
        break;
    }

    AddMenuManager addMenuManager(contextMenu, 37);
    if (!extendMenu) {
        addMenuManager.enableMenu(extendMenu);
    } else {
        // Build menu
        if (forPattern) {
            if (regexMode) {
                addMenuManager.addEntry(QStringLiteral("^"), QString(), i18n("Beginning of line"));
                addMenuManager.addEntry(QStringLiteral("$"), QString(), i18n("End of line"));
                addMenuManager.addSeparator();
                addMenuManager.addEntry(QStringLiteral("."), QString(), i18n("Match any character excluding new line (by default)"));
                addMenuManager.addEntry(QStringLiteral("+"), QString(), i18n("One or more occurrences"));
                addMenuManager.addEntry(QStringLiteral("*"), QString(), i18n("Zero or more occurrences"));
                addMenuManager.addEntry(QStringLiteral("?"), QString(), i18n("Zero or one occurrences"));
                addMenuManager.addEntry(QStringLiteral("{a"),
                                        QStringLiteral(",b}"),
                                        i18n("<a> through <b> occurrences"),
                                        QStringLiteral("{"),
                                        QStringLiteral(",}"));

                addMenuManager.addSeparator();
                addMenuManager.addSeparator();
                addMenuManager.addEntry(QStringLiteral("("), QStringLiteral(")"), i18n("Group, capturing"));
                addMenuManager.addEntry(QStringLiteral("|"), QString(), i18n("Or"));
                addMenuManager.addEntry(QStringLiteral("["), QStringLiteral("]"), i18n("Set of characters"));
                addMenuManager.addEntry(QStringLiteral("[^"), QStringLiteral("]"), i18n("Negative set of characters"));
                addMenuManager.addSeparator();
            }
        } else {
            addMenuManager.addEntry(QStringLiteral("\\0"), QString(), i18n("Whole match reference"));
            addMenuManager.addSeparator();
            if (regexMode) {
                const QString pattern = m_powerUi->pattern->currentText();
                const QList<QString> capturePatterns = getCapturePatterns(pattern);

                const int captureCount = capturePatterns.count();
                for (int i = 1; i <= 9; i++) {
                    const QString number = QString::number(i);
                    const QString &captureDetails =
                        (i <= captureCount) ? QLatin1String(" = (") + QStringView(capturePatterns[i - 1]).left(30) + QLatin1Char(')') : QString();
                    addMenuManager.addEntry(QLatin1String("\\") + number, QString(), i18n("Reference") + QLatin1Char(' ') + number + captureDetails);
                }

                addMenuManager.addSeparator();
            }
        }

        addMenuManager.addEntry(QStringLiteral("\\n"), QString(), i18n("Line break"));
        addMenuManager.addEntry(QStringLiteral("\\t"), QString(), i18n("Tab"));

        if (forPattern && regexMode) {
            addMenuManager.addEntry(QStringLiteral("\\b"), QString(), i18n("Word boundary"));
            addMenuManager.addEntry(QStringLiteral("\\B"), QString(), i18n("Not word boundary"));
            addMenuManager.addEntry(QStringLiteral("\\d"), QString(), i18n("Digit"));
            addMenuManager.addEntry(QStringLiteral("\\D"), QString(), i18n("Non-digit"));
            addMenuManager.addEntry(QStringLiteral("\\s"), QString(), i18n("Whitespace (excluding line breaks)"));
            addMenuManager.addEntry(QStringLiteral("\\S"), QString(), i18n("Non-whitespace"));
            addMenuManager.addEntry(QStringLiteral("\\w"), QString(), i18n("Word character (alphanumerics plus '_')"));
            addMenuManager.addEntry(QStringLiteral("\\W"), QString(), i18n("Non-word character"));
        }

        addMenuManager.addEntry(QStringLiteral("\\0???"), QString(), i18n("Octal character 000 to 377 (2^8-1)"), QStringLiteral("\\0"));
        addMenuManager.addEntry(QStringLiteral("\\x{????}"), QString(), i18n("Hex character 0000 to FFFF (2^16-1)"), QStringLiteral("\\x{....}"));
        addMenuManager.addEntry(QStringLiteral("\\\\"), QString(), i18n("Backslash"));

        if (forPattern && regexMode) {
            addMenuManager.addSeparator();
            addMenuManager.addEntry(QStringLiteral("(?:E"), QStringLiteral(")"), i18n("Group, non-capturing"), QStringLiteral("(?:"));
            addMenuManager.addEntry(QStringLiteral("(?=E"), QStringLiteral(")"), i18n("Positive Lookahead"), QStringLiteral("(?="));
            addMenuManager.addEntry(QStringLiteral("(?!E"), QStringLiteral(")"), i18n("Negative lookahead"), QStringLiteral("(?!"));
            // variable length positive/negative lookbehind is an experimental feature in Perl 5.30
            // see: https://perldoc.perl.org/perlre.html
            // currently QRegularExpression only supports fixed-length positive/negative lookbehind (2020-03-01)
            addMenuManager.addEntry(QStringLiteral("(?<=E"), QStringLiteral(")"), i18n("Fixed-length positive lookbehind"), QStringLiteral("(?<="));
            addMenuManager.addEntry(QStringLiteral("(?<!E"), QStringLiteral(")"), i18n("Fixed-length negative lookbehind"), QStringLiteral("(?<!"));
        }

        if (!forPattern) {
            addMenuManager.addSeparator();
            addMenuManager.addEntry(QStringLiteral("\\L"), QString(), i18n("Begin lowercase conversion"));
            addMenuManager.addEntry(QStringLiteral("\\U"), QString(), i18n("Begin uppercase conversion"));
            addMenuManager.addEntry(QStringLiteral("\\E"), QString(), i18n("End case conversion"));
            addMenuManager.addEntry(QStringLiteral("\\l"), QString(), i18n("Lowercase first character conversion"));
            addMenuManager.addEntry(QStringLiteral("\\u"), QString(), i18n("Uppercase first character conversion"));
            addMenuManager.addEntry(QStringLiteral("\\#[#..]"), QString(), i18n("Replacement counter (for Replace All)"), QStringLiteral("\\#"));
        }
    }

    // Show menu
    QAction *const result = contextMenu->exec(comboBox->mapToGlobal(pos));
    if (result != nullptr) {
        addMenuManager.handle(result, comboBox->lineEdit());
    }
}

void KateSearchBar::onPowerModeChanged(int /*index*/)
{
    if (m_powerUi->searchMode->currentIndex() == MODE_REGEX) {
        m_powerUi->matchCase->setChecked(true);
    }

    sendConfig();
    indicateMatch(MatchNothing);

    givePatternFeedback();
}

// static void addSecondarySelection(KTextEditor::ViewPrivate *view, KTextEditor::Range range)
// {
//     view->addSecondaryCursorWithSelection(range);
// }

void KateSearchBar::nextMatchForSelection(KTextEditor::ViewPrivate *view, SearchDirection searchDirection)
{
    if (!view->selection()) {
        // Select current word so we can search for that
        const Cursor cursorPos = view->cursorPosition();
        KTextEditor::Range wordRange = view->document()->wordRangeAt(cursorPos);
        if (wordRange.isValid()) {
            selectRange(view, wordRange);
            return;
        }
    }
    if (view->selection()) {
        // We only want text of one of the selection ranges
        const QString pattern = view->doc()->text(view->selectionRange());

        // How to find?
        SearchOptions enabledOptions(KTextEditor::Default);
        if (searchDirection == SearchBackward) {
            enabledOptions |= Backwards;
        }

        // Where to find?
        const Range selRange = view->selectionRange();
        //         const Range selRange = view->lastSelectionRange();
        Range inputRange;
        if (searchDirection == SearchForward) {
            inputRange.setRange(selRange.end(), view->doc()->documentEnd());
        } else {
            inputRange.setRange(Cursor(0, 0), selRange.start());
        }

        // Find, first try
        KateMatch match(view->doc(), enabledOptions);
        match.searchText(inputRange, pattern);

        if (match.isValid()) {
            selectRange(view, match.range());
            //             addSecondarySelection(view, match.range());
        } else {
            // Find, second try
            m_view->showSearchWrappedHint(searchDirection == SearchBackward);
            if (searchDirection == SearchForward) {
                inputRange.setRange(Cursor(0, 0), selRange.start());
            } else {
                inputRange.setRange(selRange.end(), view->doc()->documentEnd());
            }
            KateMatch match2(view->doc(), enabledOptions);
            match2.searchText(inputRange, pattern);
            if (match2.isValid()) {
                selectRange(view, match2.range());
                //                 addSecondarySelection(view, match2.range());
            }
        }
    }
}

void KateSearchBar::enterPowerMode()
{
    QString initialPattern;
    bool selectionOnly = false;

    // Guess settings from context: init pattern with current selection
    const bool selected = m_view->selection();
    if (selected) {
        const Range &selection = m_view->selectionRange();
        if (selection.onSingleLine()) {
            // ... with current selection
            initialPattern = m_view->selectionText();
        } else {
            // Enable selection only
            selectionOnly = true;
        }
    }

    // If there's no new selection, we'll use the existing pattern
    if (initialPattern.isNull()) {
        // Coming from power search?
        const bool fromReplace = (m_powerUi != nullptr) && (m_widget->isVisible());
        if (fromReplace) {
            QLineEdit *const patternLineEdit = m_powerUi->pattern->lineEdit();
            Q_ASSERT(patternLineEdit != nullptr);
            patternLineEdit->selectAll();
            m_powerUi->pattern->setFocus(Qt::MouseFocusReason);
            return;
        }

        // Coming from incremental search?
        const bool fromIncremental = (m_incUi != nullptr) && (m_widget->isVisible());
        if (fromIncremental) {
            initialPattern = m_incUi->pattern->currentText();
        } else {
            // Search bar probably newly opened. Reset initial replacement text to empty
            m_replacement.clear();
        }
    }

    // Create dialog
    const bool create = (m_powerUi == nullptr);
    if (create) {
        // Kill incremental widget
        if (m_incUi != nullptr) {
            // Backup current settings
            const bool OF_INCREMENTAL = false;
            backupConfig(OF_INCREMENTAL);

            // Kill widget
            delete m_incUi;
            m_incUi = nullptr;
            m_layout->removeWidget(m_widget);
            m_widget->deleteLater(); // I didn't get a crash here but for symmetrie to the other mutate slot^
        }

        // Add power widget
        m_widget = new QWidget(this);
        m_powerUi = new Ui::PowerSearchBar;
        m_powerUi->setupUi(m_widget);
        m_layout->addWidget(m_widget);

        // Bind to shared history models
        m_powerUi->pattern->setDuplicatesEnabled(false);
        m_powerUi->pattern->setInsertPolicy(QComboBox::InsertAtTop);
        m_powerUi->pattern->setMaxCount(m_config->maxHistorySize());
        m_powerUi->pattern->setModel(KTextEditor::EditorPrivate::self()->searchHistoryModel());
        m_powerUi->pattern->lineEdit()->setClearButtonEnabled(true);
        m_powerUi->pattern->setCompleter(nullptr);
        m_powerUi->replacement->setDuplicatesEnabled(false);
        m_powerUi->replacement->setInsertPolicy(QComboBox::InsertAtTop);
        m_powerUi->replacement->setMaxCount(m_config->maxHistorySize());
        m_powerUi->replacement->setModel(KTextEditor::EditorPrivate::self()->replaceHistoryModel());
        m_powerUi->replacement->lineEdit()->setClearButtonEnabled(true);
        m_powerUi->replacement->setCompleter(nullptr);

        // Filter Up/Down arrow key inputs to save unfinished search/replace text
        m_powerUi->pattern->installEventFilter(this);
        m_powerUi->replacement->installEventFilter(this);

        // Icons
        // Gnome does not seem to have all icons we want, so we use fall-back icons for those that are missing.
        QIcon mutateIcon = QIcon::fromTheme(QStringLiteral("games-config-options"), QIcon::fromTheme(QStringLiteral("preferences-system")));
        QIcon matchCaseIcon = QIcon::fromTheme(QStringLiteral("format-text-superscript"), QIcon::fromTheme(QStringLiteral("format-text-bold")));
        m_powerUi->mutate->setIcon(mutateIcon);
        m_powerUi->mutate->setChecked(true);
        m_powerUi->findNext->setIcon(QIcon::fromTheme(QStringLiteral("go-down-search")));
        m_powerUi->findPrev->setIcon(QIcon::fromTheme(QStringLiteral("go-up-search")));
        m_powerUi->findAll->setIcon(QIcon::fromTheme(QStringLiteral("edit-find")));
        m_powerUi->matchCase->setIcon(matchCaseIcon);
        m_powerUi->selectionOnly->setIcon(QIcon::fromTheme(QStringLiteral("edit-select-all")));

        // Focus proxy
        centralWidget()->setFocusProxy(m_powerUi->pattern);
    }

    m_powerUi->selectionOnly->setChecked(selectionOnly);

    // Restore previous settings
    if (create) {
        m_powerUi->matchCase->setChecked(m_powerMatchCase);
        m_powerUi->searchMode->setCurrentIndex(m_powerMode);
    }

    // force current index of -1 --> <cursor down> shows 1st completion entry instead of 2nd
    m_powerUi->pattern->setCurrentIndex(-1);
    m_powerUi->replacement->setCurrentIndex(-1);

    // Set initial search pattern
    QLineEdit *const patternLineEdit = m_powerUi->pattern->lineEdit();
    Q_ASSERT(patternLineEdit != nullptr);
    patternLineEdit->setText(initialPattern);
    patternLineEdit->selectAll();

    // Set initial replacement text
    QLineEdit *const replacementLineEdit = m_powerUi->replacement->lineEdit();
    Q_ASSERT(replacementLineEdit != nullptr);
    replacementLineEdit->setText(m_replacement);

    // Propagate settings (slots are still inactive on purpose)
    onPowerPatternChanged(initialPattern);
    givePatternFeedback();

    if (create) {
        // Slots
        connect(m_powerUi->mutate, &QToolButton::clicked, this, &KateSearchBar::enterIncrementalMode);
        connect(patternLineEdit, &QLineEdit::textChanged, this, &KateSearchBar::onPowerPatternChanged);
        connect(m_powerUi->findNext, &QToolButton::clicked, this, &KateSearchBar::findNext);
        connect(m_powerUi->findPrev, &QToolButton::clicked, this, &KateSearchBar::findPrevious);
        connect(m_powerUi->replaceNext, &QPushButton::clicked, this, &KateSearchBar::replaceNext);
        connect(m_powerUi->replaceAll, &QPushButton::clicked, this, &KateSearchBar::replaceAll);
        connect(m_powerUi->searchMode, &QComboBox::currentIndexChanged, this, &KateSearchBar::onPowerModeChanged);
        connect(m_powerUi->matchCase, &QToolButton::toggled, this, &KateSearchBar::onMatchCaseToggled);
        connect(m_powerUi->findAll, &QPushButton::clicked, this, &KateSearchBar::findAll);
        connect(m_powerUi->cancel, &QPushButton::clicked, this, &KateSearchBar::onPowerCancelFindOrReplace);

        // Make [return] in pattern line edit trigger <find next> action
        connect(patternLineEdit, &QLineEdit::returnPressed, this, &KateSearchBar::onReturnPressed);
        connect(replacementLineEdit, &QLineEdit::returnPressed, this, &KateSearchBar::replaceNext);

        // Hook into line edit context menus
        m_powerUi->pattern->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(m_powerUi->pattern, &QComboBox::customContextMenuRequested, this, qOverload<const QPoint &>(&KateSearchBar::onPowerPatternContextMenuRequest));
        m_powerUi->replacement->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_powerUi->replacement,
                &QComboBox::customContextMenuRequested,
                this,
                qOverload<const QPoint &>(&KateSearchBar::onPowerReplacmentContextMenuRequest));
    }

    // Focus
    if (m_widget->isVisible()) {
        m_powerUi->pattern->setFocus(Qt::MouseFocusReason);
    }

    // move close button to right layout, ensures properly at top for both incremental + advanced mode
    m_powerUi->gridLayout->addWidget(closeButton(), 0, 2, 1, 1);
}

void KateSearchBar::enterIncrementalMode()
{
    QString initialPattern;

    // Guess settings from context: init pattern with current selection
    const bool selected = m_view->selection();
    if (selected) {
        const Range &selection = m_view->selectionRange();
        if (selection.onSingleLine()) {
            // ... with current selection
            initialPattern = m_view->selectionText();
        }
    }

    // If there's no new selection, we'll use the existing pattern
    if (initialPattern.isNull()) {
        // Coming from incremental search?
        const bool fromIncremental = (m_incUi != nullptr) && (m_widget->isVisible());
        if (fromIncremental) {
            m_incUi->pattern->lineEdit()->selectAll();
            m_incUi->pattern->setFocus(Qt::MouseFocusReason);
            return;
        }

        // Coming from power search?
        const bool fromReplace = (m_powerUi != nullptr) && (m_widget->isVisible());
        if (fromReplace) {
            initialPattern = m_powerUi->pattern->currentText();
            // current text will be used as initial replacement text later
            m_replacement = m_powerUi->replacement->currentText();
        }
    }

    // Still no search pattern? Use the word under the cursor
    if (initialPattern.isNull()) {
        const KTextEditor::Cursor cursorPosition = m_view->cursorPosition();
        initialPattern = m_view->doc()->wordAt(cursorPosition);
    }

    // Create dialog
    const bool create = (m_incUi == nullptr);
    if (create) {
        // Kill power widget
        if (m_powerUi != nullptr) {
            // Backup current settings
            const bool OF_POWER = true;
            backupConfig(OF_POWER);

            // Kill widget
            delete m_powerUi;
            m_powerUi = nullptr;
            m_layout->removeWidget(m_widget);
            m_widget->deleteLater(); // deleteLater, because it's not a good idea too delete the widget and there for the button triggering this slot
        }

        // Add incremental widget
        m_widget = new QWidget(this);
        m_incUi = new Ui::IncrementalSearchBar;
        m_incUi->setupUi(m_widget);
        m_layout->addWidget(m_widget);

        // Filter Up/Down arrow key inputs to save unfinished search text
        m_incUi->pattern->installEventFilter(this);

        //         new QShortcut(KStandardShortcut::paste().primary(), m_incUi->pattern, SLOT(paste()), 0, Qt::WidgetWithChildrenShortcut);
        //         if (!KStandardShortcut::paste().alternate().isEmpty())
        //             new QShortcut(KStandardShortcut::paste().alternate(), m_incUi->pattern, SLOT(paste()), 0, Qt::WidgetWithChildrenShortcut);

        // Icons
        // Gnome does not seem to have all icons we want, so we use fall-back icons for those that are missing.
        QIcon mutateIcon = QIcon::fromTheme(QStringLiteral("games-config-options"), QIcon::fromTheme(QStringLiteral("preferences-system")));
        QIcon matchCaseIcon = QIcon::fromTheme(QStringLiteral("format-text-superscript"), QIcon::fromTheme(QStringLiteral("format-text-bold")));
        m_incUi->mutate->setIcon(mutateIcon);
        m_incUi->next->setIcon(QIcon::fromTheme(QStringLiteral("go-down-search")));
        m_incUi->prev->setIcon(QIcon::fromTheme(QStringLiteral("go-up-search")));
        m_incUi->matchCase->setIcon(matchCaseIcon);

        // Ensure minimum size
        m_incUi->pattern->setMinimumWidth(12 * m_incUi->pattern->fontMetrics().height());

        // Customize status area
        m_incUi->status->setTextElideMode(Qt::ElideLeft);

        // Focus proxy
        centralWidget()->setFocusProxy(m_incUi->pattern);

        m_incUi->pattern->setDuplicatesEnabled(false);
        m_incUi->pattern->setInsertPolicy(QComboBox::InsertAtTop);
        m_incUi->pattern->setMaxCount(m_config->maxHistorySize());
        m_incUi->pattern->setModel(KTextEditor::EditorPrivate::self()->searchHistoryModel());
        m_incUi->pattern->lineEdit()->setClearButtonEnabled(true);
        m_incUi->pattern->setCompleter(nullptr);
    }

    // Restore previous settings
    if (create) {
        m_incUi->matchCase->setChecked(m_incMatchCase);
    }

    // force current index of -1 --> <cursor down> shows 1st completion entry instead of 2nd
    m_incUi->pattern->setCurrentIndex(-1);

    // Set initial search pattern
    if (!create) {
        disconnect(m_incUi->pattern, &QComboBox::editTextChanged, this, &KateSearchBar::onIncPatternChanged);
    }
    m_incUi->pattern->setEditText(initialPattern);
    connect(m_incUi->pattern, &QComboBox::editTextChanged, this, &KateSearchBar::onIncPatternChanged);
    m_incUi->pattern->lineEdit()->selectAll();

    // Propagate settings (slots are still inactive on purpose)
    if (initialPattern.isEmpty()) {
        // Reset edit color
        indicateMatch(MatchNothing);
    }

    // Enable/disable next/prev
    m_incUi->next->setDisabled(initialPattern.isEmpty());
    m_incUi->prev->setDisabled(initialPattern.isEmpty());

    if (create) {
        // Slots
        connect(m_incUi->mutate, &QToolButton::clicked, this, &KateSearchBar::enterPowerMode);
        connect(m_incUi->pattern->lineEdit(), &QLineEdit::returnPressed, this, &KateSearchBar::onReturnPressed);
        connect(m_incUi->next, &QToolButton::clicked, this, &KateSearchBar::findNext);
        connect(m_incUi->prev, &QToolButton::clicked, this, &KateSearchBar::findPrevious);
        connect(m_incUi->matchCase, &QToolButton::toggled, this, &KateSearchBar::onMatchCaseToggled);
    }

    // Focus
    if (m_widget->isVisible()) {
        m_incUi->pattern->setFocus(Qt::MouseFocusReason);
    }

    // move close button to right layout, ensures properly at top for both incremental + advanced mode
    m_incUi->hboxLayout->addWidget(closeButton());
}

bool KateSearchBar::clearHighlights()
{
    // Remove ScrollBarMarks
    const QHash<int, KTextEditor::Mark *> &marks = m_view->document()->marks();
    QHashIterator<int, KTextEditor::Mark *> i(marks);
    while (i.hasNext()) {
        i.next();
        if (i.value()->type & KTextEditor::Document::SearchMatch) {
            m_view->document()->removeMark(i.value()->line, KTextEditor::Document::SearchMatch);
        }
    }

    if (m_infoMessage) {
        delete m_infoMessage;
    }

    if (m_hlRanges.isEmpty()) {
        return false;
    }
    qDeleteAll(m_hlRanges);
    m_hlRanges.clear();
    return true;
}

void KateSearchBar::updateHighlightColors()
{
    const QColor foregroundColor = m_view->defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle::Normal)->foreground().color();
    const QColor &searchColor = m_view->rendererConfig()->searchHighlightColor();
    const QColor &replaceColor = m_view->rendererConfig()->replaceHighlightColor();

    // init match attribute
    highlightMatchAttribute->setForeground(foregroundColor);
    highlightMatchAttribute->setBackground(searchColor);
    highlightMatchAttribute->dynamicAttribute(Attribute::ActivateMouseIn)->setBackground(searchColor);
    highlightMatchAttribute->dynamicAttribute(Attribute::ActivateMouseIn)->setForeground(foregroundColor);
    highlightMatchAttribute->dynamicAttribute(Attribute::ActivateCaretIn)->setBackground(searchColor);
    highlightMatchAttribute->dynamicAttribute(Attribute::ActivateCaretIn)->setForeground(foregroundColor);

    // init replacement attribute
    highlightReplacementAttribute->setBackground(replaceColor);
    highlightReplacementAttribute->setForeground(foregroundColor);
}

void KateSearchBar::showEvent(QShowEvent *event)
{
    // Update init cursor
    if (m_incUi != nullptr) {
        m_incInitCursor = m_view->cursorPosition();
    }

    // We don't want to update if a "findOrReplaceAll" is running
    // other we end up deleting our working range and crash
    if (m_cancelFindOrReplace) {
        updateSelectionOnly();
    }

    KateViewBarWidget::showEvent(event);
}

bool KateSearchBar::eventFilter(QObject *obj, QEvent *event)
{
    QComboBox *combo = qobject_cast<QComboBox *>(obj);
    if (combo && event->type() == QEvent::KeyPress) {
        const int key = static_cast<QKeyEvent *>(event)->key();
        const int currentIndex = combo->currentIndex();
        const QString currentText = combo->currentText();
        QString &unfinishedText = (m_powerUi && combo == m_powerUi->replacement) ? m_replacement : m_unfinishedSearchText;
        if (key == Qt::Key_Up && currentIndex <= 0 && unfinishedText != currentText) {
            // Only restore unfinished text if we are already in the latest entry
            combo->setCurrentIndex(-1);
            combo->setCurrentText(unfinishedText);
        } else if (key == Qt::Key_Down || key == Qt::Key_Up) {
            // Only save unfinished text if it is not empty and it is modified
            const bool isUnfinishedSearch = (!currentText.trimmed().isEmpty() && (currentIndex == -1 || combo->itemText(currentIndex) != currentText));
            if (isUnfinishedSearch && unfinishedText != currentText) {
                unfinishedText = currentText;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void KateSearchBar::updateSelectionOnly()
{
    // Make sure the previous selection-only search range is not used anymore
    if (m_workingRange && !m_selectionChangedByUndoRedo) {
        delete m_workingRange;
        m_workingRange = nullptr;
    }

    if (m_powerUi == nullptr) {
        return;
    }

    // Re-init "Selection only" checkbox if power search bar open
    const bool selected = m_view->selection();
    bool selectionOnly = selected;
    if (selected) {
        Range const &selection = m_view->selectionRange();
        selectionOnly = !selection.onSingleLine();
    }
    m_powerUi->selectionOnly->setChecked(selectionOnly);
}

void KateSearchBar::updateIncInitCursor()
{
    if (m_incUi == nullptr) {
        return;
    }

    // Update init cursor
    m_incInitCursor = m_view->cursorPosition();
}

void KateSearchBar::onPowerPatternContextMenuRequest(const QPoint &pos)
{
    const bool FOR_PATTERN = true;
    showExtendedContextMenu(FOR_PATTERN, pos);
}

void KateSearchBar::onPowerPatternContextMenuRequest()
{
    onPowerPatternContextMenuRequest(m_powerUi->pattern->mapFromGlobal(QCursor::pos()));
}

void KateSearchBar::onPowerReplacmentContextMenuRequest(const QPoint &pos)
{
    const bool FOR_REPLACEMENT = false;
    showExtendedContextMenu(FOR_REPLACEMENT, pos);
}

void KateSearchBar::onPowerReplacmentContextMenuRequest()
{
    onPowerReplacmentContextMenuRequest(m_powerUi->replacement->mapFromGlobal(QCursor::pos()));
}

void KateSearchBar::onPowerCancelFindOrReplace()
{
    m_cancelFindOrReplace = true;
}

bool KateSearchBar::isPower() const
{
    return m_powerUi != nullptr;
}

void KateSearchBar::slotReadWriteChanged()
{
    if (!KateSearchBar::isPower()) {
        return;
    }

    // perhaps disable/enable
    m_powerUi->replaceNext->setEnabled(m_view->doc()->isReadWrite() && isPatternValid());
    m_powerUi->replaceAll->setEnabled(m_view->doc()->isReadWrite() && isPatternValid());
}

#include "moc_katesearchbar.cpp"
