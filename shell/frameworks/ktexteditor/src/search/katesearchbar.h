/*
    SPDX-FileCopyrightText: 2009-2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2007 Sebastian Pipping <webmaster@hartwork.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_SEARCH_BAR_H
#define KATE_SEARCH_BAR_H 1

#include "kateviewhelpers.h"
#include <ktexteditor_export.h>

#include <ktexteditor/attribute.h>
#include <ktexteditor/document.h>

namespace KTextEditor
{
class ViewPrivate;
}
class KateViewConfig;
class QVBoxLayout;
class QComboBox;

namespace Ui
{
class IncrementalSearchBar;
class PowerSearchBar;
}

namespace KTextEditor
{
class MovingRange;
class Message;
}

class KTEXTEDITOR_EXPORT KateSearchBar : public KateViewBarWidget
{
    Q_OBJECT

    friend class SearchBarTest;

public:
    enum SearchMode {
        // NOTE: Concrete values are important here
        // to work with the combobox index!
        MODE_PLAIN_TEXT = 0,
        MODE_WHOLE_WORDS = 1,
        MODE_ESCAPE_SEQUENCES = 2,
        MODE_REGEX = 3
    };

    enum MatchResult { MatchFound, MatchWrappedForward, MatchWrappedBackward, MatchMismatch, MatchNothing, MatchNeutral };

    enum SearchDirection { SearchForward, SearchBackward };

public:
    explicit KateSearchBar(bool initAsPower, KTextEditor::ViewPrivate *view, KateViewConfig *config);
    ~KateSearchBar() override;

    void closed() override;

    bool isPower() const;

    QString searchPattern() const;
    QString replacementPattern() const;

    bool selectionOnly() const;
    bool matchCase() const;

    void nextMatchForSelection(KTextEditor::ViewPrivate *view, SearchDirection searchDirection);

public Q_SLOTS:
    /**
     * Set the current search pattern.
     * @param searchPattern the search pattern
     */
    void setSearchPattern(const QString &searchPattern);

    /**
     * Set the current replacement pattern.
     * @param replacementPattern the replacement pattern
     */
    void setReplacementPattern(const QString &replacementPattern);

    void setSearchMode(SearchMode mode);
    void setSelectionOnly(bool selectionOnly);
    void setMatchCase(bool matchCase);

    // Called by buttons and typically <F3>/<Shift>+<F3> shortcuts
    void findNext();
    void findPrevious();

    // PowerMode stuff
    void findAll();
    void replaceNext();
    void replaceAll();

    // Also used by KTextEditor::ViewPrivate
    void enterPowerMode();
    void enterIncrementalMode();

    bool clearHighlights();
    void updateHighlightColors();

    // read write status of document changed
    void slotReadWriteChanged();

protected:
    // Overridden
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private Q_SLOTS:
    void onIncPatternChanged(const QString &pattern);
    void onMatchCaseToggled(bool matchCase);

    void onReturnPressed();
    void updateSelectionOnly();
    void updateIncInitCursor();

    void onPowerPatternChanged(const QString &pattern);
    void onPowerModeChanged(int index);
    void onPowerPatternContextMenuRequest();
    void onPowerPatternContextMenuRequest(const QPoint &);
    void onPowerReplacmentContextMenuRequest();
    void onPowerReplacmentContextMenuRequest(const QPoint &);
    void onPowerCancelFindOrReplace();

    /**
     * This function do the hard search & replace work in time slice steps.
     * When all is done @ref m_matchCounter is set and the signal
     * @ref findOrReplaceAllFinished() is emitted.
     */
    void findOrReplaceAll();

    /**
     * Restore needed settings when signal @ref findOrReplaceAllFinished()
     * was received.
     */
    void endFindOrReplaceAll();

Q_SIGNALS:
    /**
     * Will emitted by @ref findOrReplaceAll() when all is done.
     */
    void findOrReplaceAllFinished();

private:
    // Helpers
    bool find(SearchDirection searchDirection = SearchForward)
    {
        return findOrReplace(searchDirection, nullptr);
    };
    KTEXTEDITOR_NO_EXPORT
    bool findOrReplace(SearchDirection searchDirection, const QString *replacement);

    /**
     * The entry point to start a search & replace task.
     * Set needed member variables and call @ref findOrReplaceAll() to do the work.
     */
    KTEXTEDITOR_NO_EXPORT
    void beginFindOrReplaceAll(KTextEditor::Range inputRange, const QString &replacement, bool replaceMode = true);
    KTEXTEDITOR_NO_EXPORT
    void beginFindAll(KTextEditor::Range inputRange)
    {
        beginFindOrReplaceAll(inputRange, QString(), false);
    };

    KTEXTEDITOR_NO_EXPORT
    bool isPatternValid() const;

    KTEXTEDITOR_NO_EXPORT
    KTextEditor::SearchOptions searchOptions(SearchDirection searchDirection = SearchForward) const;

    KTEXTEDITOR_NO_EXPORT
    void highlightMatch(KTextEditor::Range range);
    KTEXTEDITOR_NO_EXPORT
    void highlightReplacement(KTextEditor::Range range);
    KTEXTEDITOR_NO_EXPORT
    void indicateMatch(MatchResult matchResult);
    KTEXTEDITOR_NO_EXPORT
    static void selectRange(KTextEditor::ViewPrivate *view, KTextEditor::Range range);
    KTEXTEDITOR_NO_EXPORT
    void selectRange2(KTextEditor::Range range);

    KTEXTEDITOR_NO_EXPORT
    QList<QString> getCapturePatterns(const QString &pattern) const;
    KTEXTEDITOR_NO_EXPORT
    void showExtendedContextMenu(bool forPattern, const QPoint &pos);

    KTEXTEDITOR_NO_EXPORT
    void givePatternFeedback();
    KTEXTEDITOR_NO_EXPORT
    void addCurrentTextToHistory(QComboBox *combo);
    KTEXTEDITOR_NO_EXPORT
    void backupConfig(bool ofPower);
    KTEXTEDITOR_NO_EXPORT
    void sendConfig();

    KTEXTEDITOR_NO_EXPORT
    void showResultMessage();

private:
    KTextEditor::ViewPrivate *const m_view;
    KateViewConfig *const m_config;
    QList<KTextEditor::MovingRange *> m_hlRanges;
    QPointer<KTextEditor::Message> m_infoMessage;

    // Shared by both dialogs
    QVBoxLayout *const m_layout;
    QWidget *m_widget;
    QString m_unfinishedSearchText;

    // Incremental search related
    Ui::IncrementalSearchBar *m_incUi;
    KTextEditor::Cursor m_incInitCursor;

    // Power search related
    Ui::PowerSearchBar *m_powerUi = nullptr;
    KTextEditor::MovingRange *m_workingRange = nullptr;
    KTextEditor::Range m_inputRange;
    QString m_replacement;
    uint m_matchCounter = 0;
    bool m_replaceMode = false;
    bool m_cancelFindOrReplace = true;
    bool m_selectionChangedByUndoRedo = false;
    std::vector<KTextEditor::Range> m_highlightRanges;

    // attribute to highlight matches with
    KTextEditor::Attribute::Ptr highlightMatchAttribute;
    KTextEditor::Attribute::Ptr highlightReplacementAttribute;

    // Status backup
    bool m_incHighlightAll : 1;
    bool m_incFromCursor : 1;
    bool m_incMatchCase : 1;
    bool m_powerMatchCase : 1;
    bool m_powerFromCursor : 1;
    bool m_powerHighlightAll : 1;
    unsigned int m_powerMode : 2;
};

#endif // KATE_SEARCH_BAR_H
