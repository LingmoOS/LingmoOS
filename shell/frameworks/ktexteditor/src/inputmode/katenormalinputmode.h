/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_NORMAL_INPUT_MODE_H
#define KATE_NORMAL_INPUT_MODE_H

#include <memory>

#include "kateabstractinputmode.h"

class KateNormalInputModeFactory;
class KateSearchBar;
class KateCommandLineBar;

class KateNormalInputMode : public KateAbstractInputMode
{
    explicit KateNormalInputMode(KateViewInternal *viewInternal);
    friend KateNormalInputModeFactory;

public:
    KTextEditor::View::ViewMode viewMode() const override;
    QString viewModeHuman() const override;
    KTextEditor::View::InputMode viewInputMode() const override;
    QString viewInputModeHuman() const override;

    void activate() override;
    void deactivate() override;
    void reset() override;

    bool overwrite() const override;
    void overwrittenChar(const QChar &) override;

    void clearSelection() override;
    bool stealKey(QKeyEvent *) override;

    void gotFocus() override;
    void lostFocus() override;

    void readSessionConfig(const KConfigGroup &config) override;
    void writeSessionConfig(KConfigGroup &config) override;
    void updateRendererConfig() override;
    void updateConfig() override;
    void readWriteChanged(bool rw) override;

    void find() override;
    void findSelectedForwards() override;
    void findSelectedBackwards() override;
    void findReplace() override;
    void findNext() override;
    void findPrevious() override;

    void activateCommandLine() override;

    bool keyPress(QKeyEvent *) override;
    bool blinkCaret() const override;
    KTextEditor::caretStyles caretStyle() const override;

    void toggleInsert() override;
    void launchInteractiveCommand(const QString &command) override;

    QString bookmarkLabel(int line) const override;

private:
    /**
     * Search bar mode:
     *   - Setup Incremental mode, among other things: potential new search pattern
     *   - Setup Power mode, aka find & replace: Also potential new search pattern
     *   - Use current mode and current search pattern or if no Search bar exists, launch Incremental mode
     */
    enum SearchBarMode { IncrementalSearchBar, PowerSearchBar, IncrementalSearchBarOrKeepMode };

    /**
     * Get search bar, create it on demand. (with right mode)
     * @param mode wanted search bar mode
     * @return search bar widget
     */
    KateSearchBar *searchBar(const SearchBarMode mode);

    /**
     * search bar around?
     * @return search bar around?
     */
    bool hasSearchBar() const
    {
        return m_searchBar.get();
    }

    /**
     * Get command line bar, create it on demand.
     * @return command line bar, created if not already there
     */
    KateCommandLineBar *cmdLineBar();

private:
    std::unique_ptr<KateSearchBar> m_searchBar;
    std::unique_ptr<KateCommandLineBar> m_cmdLine;
};

#endif
