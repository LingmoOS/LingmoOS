// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {
class Label;
class ScrollView;
}

namespace settings {

struct PageInfo;
class SearchField;
class SidebarList;

// The window's left column: its title, a search field, and the pages by
// category. Typing lists matching pages, sub pages included, in place of the
// categories; Up and Down walk the sidebar or the results, Return opens one,
// and Escape clears the search.
class Sidebar : public Lingmo::ui::View
{
public:
    Sidebar();
    ~Sidebar() override;

    // The sidebar pages shown, in order.
    void setEntries(std::vector<const PageInfo*> entries);
    // Takes its text again after the session language changed.
    void retranslate();
    const std::vector<const PageInfo*>& entries() const { return m_entries; }
    // Sub pages whose entry is unavailable stay out of search results.
    void setSubPageAvailable(const std::string& id, bool available);

    // The page shown, and the sidebar page it belongs to.
    void setCurrent(const std::string& page, const std::string& rootPage);

    void focusSearch();
    bool isSearching() const;
    // A press at this point of the window, outside the search field, takes
    // the focus off it.
    void releaseSearch(Lingmo::ui::PointF windowPosition);

    // The title's text, laid out as the header's title lines up with it.
    Lingmo::ui::Label* titleLabel() const { return m_title; }

    // A click on a sidebar page, or a search result opened.
    std::function<void(const std::string& page, const std::string& rootPage)> onActivated;

    Lingmo::ui::SizeF sizeHint() const override;
    bool acceptsFocus() const override { return true; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    bool keyPress(const Lingmo::ui::KeyEvent& event) override;

private:
    struct Result
    {
        const PageInfo* page;
        const PageInfo* rootPage;
    };

    void updateResults();
    void openResult(int index);

    std::vector<const PageInfo*> m_entries;
    std::vector<std::string> m_unavailableSubPages;
    std::vector<Result> m_results;
    int m_currentResult = -1;
    std::string m_currentPage;
    std::string m_currentRootPage;
    Lingmo::ui::Label* m_title;
    SearchField* m_search;
    Lingmo::ui::ScrollView* m_listScroll;
    SidebarList* m_list;
    Lingmo::ui::ScrollView* m_resultsScroll;
    SidebarList* m_resultsList;
    Lingmo::ui::View* m_topFade;
    Lingmo::ui::View* m_bottomFade;
    Lingmo::ui::Label* m_noResults;
};

} // namespace settings
