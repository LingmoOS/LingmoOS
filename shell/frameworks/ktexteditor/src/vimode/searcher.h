/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_SEARCHER_H
#define KATEVI_SEARCHER_H

#include "ktexteditor/attribute.h"
#include "ktexteditor/range.h"
#include <vimode/range.h>

#include <QString>

namespace KTextEditor
{
class Cursor;
class ViewPrivate;
class MovingRange;
}

namespace KateVi
{
class InputModeManager;

class Searcher
{
public:
    explicit Searcher(InputModeManager *viInputModeManager);
    ~Searcher();

    /** Command part **/
    void findNext();
    void findPrevious();

    /** Simple searchers **/
    Range motionFindNext(int count = 1);
    Range motionFindPrev(int count = 1);
    Range findWordForMotion(const QString &pattern, bool backwards, const KTextEditor::Cursor startFrom, int count);

    /** Extended searcher for Emulated Command Bar. **/
    struct SearchParams {
        QString pattern;
        bool isBackwards = false;
        bool isCaseSensitive = false;
        bool shouldPlaceCursorAtEndOfMatch = false;
    };
    KTextEditor::Range findPattern(const SearchParams &searchParams, const KTextEditor::Cursor startFrom, int count, bool addToSearchHistory = true);

    const QString getLastSearchPattern() const;
    bool lastSearchWrapped() const;
    void setLastSearchParams(const SearchParams &searchParams);

    void enableHighlightSearch(bool enable);
    bool isHighlightSearchEnabled() const;
    void hideCurrentHighlight();
    void updateHighlightColors();
    void clearHighlights();
    void patternDone(bool wasAborted);

private:
    Range findPatternForMotion(const SearchParams &searchParams, const KTextEditor::Cursor startFrom, int count = 1);
    KTextEditor::Range findPatternWorker(const SearchParams &searchParams, const KTextEditor::Cursor startFrom, int count);

    void highlightVisibleResults(const SearchParams &searchParams, bool force = false);
    void disconnectSignals();
    void connectSignals();

private:
    enum class HighlightMode {
        Disable, /** vi :set nohls[earch] **/
        Enable, /** vi :set hls[earch] **/
        HideCurrent /** vi :noh[lsearch] - stop highlighting until next search **/
    };

    InputModeManager *m_viInputModeManager;
    KTextEditor::ViewPrivate *m_view;

    SearchParams m_lastSearchConfig;
    bool m_lastSearchWrapped;

    HighlightMode m_hlMode{HighlightMode::Enable};
    QList<KTextEditor::MovingRange *> m_hlRanges;
    SearchParams m_lastHlSearchConfig;
    KTextEditor::Range m_lastHlSearchRange;
    KTextEditor::Attribute::Ptr highlightMatchAttribute;
    QMetaObject::Connection m_displayRangeChangedConnection;
    QMetaObject::Connection m_textChangedConnection;
    bool newPattern{true};
};
}

#endif // KATEVI_SEARCHER_H
