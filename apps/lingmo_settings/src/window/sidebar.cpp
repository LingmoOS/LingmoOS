// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/sidebar.h"

#include <algorithm>
#include <cmath>

#include "base/i18n.h"
#include "ui/color_provider.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"
#include "widgets/rowicon.h"
#include "widgets/searchfield.h"
#include "window/pagelist.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kSearchHeight = 34;
static constexpr double kTitlePointSize = 12;
static constexpr double kItemPointSize = 10;
static constexpr double kIconTile = 24;

static bool windowActive(const View* view)
{
    const Window* window = view->window();
    return !window || window->isActive();
}

// Sidebar tiles use the live system accent in both normal and selected states.
// Keep the shared RowIcon's semantic colours for status icons inside pages.
class SidebarIcon : public RowIcon
{
public:
    explicit SidebarIcon(const std::string& glyph)
        : RowIcon(glyph, Theme::current().highlight)
    {
    }

protected:
    void paint(Painter& painter) override
    {
        const Color fill = Theme::current().highlight;
        painter.fillRoundedRect(localRect(), 6,
                                LinearGradient::vertical(localRect(), fill.lighter(1.15), fill));
    }
};

// One entry: its tile and title, with the press and current backgrounds.
class SidebarItem : public View
{
public:
    explicit SidebarItem(const PageInfo& page)
        : m_page(page)
    {
        m_icon = addChild<SidebarIcon>(page.icon);
        m_title = addChild<Label>(tr(page.title.c_str()));
        m_title->setFontSize(metrics::points(kItemPointSize));
        m_title->setVerticallyCentered(true);
        setTracksHover(true);
        setCursorShape(CursorShape::Pointer);
    }

    const PageInfo& page() const { return m_page; }

    void setCurrent(bool current)
    {
        if (current == m_current)
            return;
        m_current = current;
        m_icon->setInverted(current);
        // The title's colour is decided in layout, beside the inactive tint.
        requestLayout();
        requestPaint();
    }

    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const double height = geometry().height;
        m_icon->setGeometry({metrics::sidebarItemPadding, (height - kIconTile) / 2, kIconTile, kIconTile});
        const double x = metrics::sidebarItemPadding + kIconTile + metrics::s;
        m_title->setGeometry({x, 0, std::max(0.0, geometry().width - x), height});
        // 8pt text reads lighter than its colour, so the inactive grey leans
        // towards the text colour.
        const Color text = theme.text;
        const Color grey = theme.disabledText;
        const double weight = 0.35 * text.a / 255.0;
        const auto tint = [weight](uint8_t from, uint8_t to) {
            return uint8_t(std::lround(from + (to - from) * weight));
        };
        // The current entry sits on the accent, so its title goes to the
        // colour that reads on it rather than the window's text colour.
        m_title->setColor(m_current ? theme.highlightedText
                                    : windowActive(this)
                                    ? text
                                    : Color{tint(grey.r, text.r), tint(grey.g, text.g), tint(grey.b, text.b),
                                            grey.a});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        // The current entry keeps the accent; a hovered row gets only a hint
        // of a surface, and the pressed row deepens it.
        if (m_pressed || m_current || isHovered()) {
            const Color fill = m_pressed ? color(ColorId::ItemPressed)
                                : m_current ? color(SettingsColor::SidebarItemCurrent)
                                            : color(SettingsColor::SidebarItemHovered);
            painter.drawSmoothRectangle(localRect(), theme.mediumRadius, fill);
        }
    }

    void hoverChanged() override { requestPaint(); }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    const PageInfo& m_page;
    RowIcon* m_icon;
    Label* m_title;
    bool m_current = false;
    bool m_pressed = false;
};

// A category's name above its entries. With a count it is a search result
// group: brighter than an ordinary category and followed by its result count.
class SidebarSection : public View
{
public:
    explicit SidebarSection(std::string text, int count = -1)
        : m_count(count >= 0)
    {
        m_label = addChild<Label>(std::move(text));
        m_label->setFontSize(metrics::points(kItemPointSize));
        if (m_count) {
            m_countLabel = addChild<Label>(std::to_string(count));
            m_countLabel->setFontSize(metrics::points(kItemPointSize));
        }
        m_metrics = addChild<Label>("X");
        m_metrics->setVisible(false);
    }

    // The default font's line height, whole, so the rows below stay on the
    // pixel grid.
    SizeF sizeHint() const override
    {
        return {0, std::ceil(m_metrics->sizeHint().height) + metrics::sidebarCategorySpacing + metrics::xs};
    }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const double padding = metrics::sidebarItemPadding;
        m_label->setColor(m_count ? theme.text.withAlpha(0.62) : theme.disabledText);
        const SizeF label = m_label->sizeHint();
        const double countWidth = m_countLabel ? m_countLabel->sizeHint().width : 0;
        const double gap = m_countLabel ? metrics::s : 0;
        m_label->setGeometry({padding, metrics::sidebarCategorySpacing,
                              std::max(0.0, geometry().width - 2 * padding - countWidth - gap), label.height});
        if (m_countLabel) {
            const SizeF count = m_countLabel->sizeHint();
            m_countLabel->setColor(theme.disabledText);
            m_countLabel->setGeometry({geometry().width - padding - count.width,
                                       metrics::sidebarCategorySpacing +
                                               (std::ceil(m_metrics->sizeHint().height) - count.height) / 2,
                                       count.width, count.height});
        }
    }

private:
    Label* m_label;
    Label* m_countLabel = nullptr;
    Label* m_metrics;
    bool m_count = false;
};

// The entries of the sidebar or of a search, by category, as a list view
// with sections lays them out: a section sits right above its first entry,
// and the spacing runs between entries.
class SidebarList : public View
{
public:
    struct Entry
    {
        const PageInfo* page;
        const PageInfo* rootPage;
    };

    void setEntries(std::vector<Entry> entries)
    {
        while (!children().empty())
            takeChild(children().front().get());
        m_entries = std::move(entries);
        m_items.clear();
        for (size_t i = 0; i < m_entries.size();) {
            const std::string category = m_entries[i].rootPage->category;
            size_t end = i + 1;
            while (end < m_entries.size() && m_entries[end].rootPage->category == category)
                ++end;
            addChild<SidebarSection>(tr(category.c_str()), m_resultMode ? int(end - i) : -1);
            for (size_t j = i; j < end; ++j) {
                SidebarItem* item = addChild<SidebarItem>(*m_entries[j].page);
                item->setName(m_namePrefix + m_entries[j].page->id);
                item->onClicked = [this, j] {
                    if (onFocusRequested)
                        onFocusRequested();
                    if (onClicked)
                        onClicked(int(j));
                };
                m_items.push_back(item);
            }
            i = end;
        }
        setCurrentIndex(m_current);
        requestLayout();
    }

    void setResultMode(bool result) { m_resultMode = result; }

    // Scrolls the current entry into view at the next layout, with the
    // list's bottom margin below it.
    void revealCurrent(ScrollView* scroll)
    {
        m_revealIn = scroll;
        requestLayout();
    }

    // Names the entries for layout dumps: the prefix, then the page's id.
    void setNamePrefix(std::string prefix) { m_namePrefix = std::move(prefix); }

    const std::vector<Entry>& entries() const { return m_entries; }
    int count() const { return int(m_entries.size()); }

    void setCurrentIndex(int index)
    {
        m_current = index;
        for (size_t i = 0; i < m_items.size(); ++i)
            m_items[i]->setCurrent(int(i) == index);
    }
    int currentIndex() const { return m_current; }

    RectF itemRect(int index) const
    {
        return index >= 0 && index < int(m_items.size()) ? m_items[size_t(index)]->geometry() : RectF{};
    }

    SizeF sizeHint() const override { return {0, measure(nullptr)}; }
    double heightForWidth(double) const override { return measure(nullptr); }

    std::function<void(int index)> onClicked;
    // Called before opening an entry, so keyboard navigation keeps working
    // after a click on the list.
    std::function<void()> onFocusRequested;

protected:
    void layout() override
    {
        measure(this);
        if (m_revealIn && m_current >= 0 && m_current < int(m_items.size())) {
            const RectF item = m_items[size_t(m_current)]->geometry();
            m_revealIn->ensureVisible({item.x, item.y, item.width, item.height + metrics::sidebarMargin});
        }
        m_revealIn = nullptr;
    }

    // One panel per category, behind its entries and under them in the stack:
    // the list paints first, its children over it. The fill is flat; only the
    // shadow lifts it from the sidebar.
    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const double width = geometry().width - 2 * metrics::sidebarMargin;
        const Color fill = color(SettingsColor::SidebarPanelFill);
        const Color shadow = color(SettingsColor::SidebarPanelShadow);
        for (const Panel& panel : m_panels) {
            const RectF rect{metrics::sidebarMargin, panel.top, width, panel.bottom - panel.top};
            painter.drawShadow(rect, theme.mediumRadius, theme.darkMode ? 22 : 12, shadow, {0, 2});
            painter.fillRoundedRect(rect, theme.mediumRadius, fill);
        }
    }

private:
    // A category's panel, as measure() found it: where its entries begin and
    // end, its own padding included.
    struct Panel
    {
        double top = 0;
        double bottom = 0;
    };

    // The content height; places the children when given the list.
    double measure(SidebarList* place) const
    {
        const double width = geometry().width - 2 * metrics::sidebarMargin;
        // The entries sit inside their category's panel, inset from its edge,
        // so the panel has something to hold them by.
        const double inset = metrics::xs;
        double y = 0;
        bool first = true;
        // The panel of the category being laid out: its first entry's top.
        double panelTop = 0;
        bool inPanel = false;
        if (place)
            m_panels.clear();

        for (const auto& child : children()) {
            const bool section = dynamic_cast<SidebarSection*>(child.get()) != nullptr;
            if (section) {
                // A category's name closes the panel above it.
                if (place && inPanel) {
                    m_panels.push_back({panelTop - inset, y + inset});
                    inPanel = false;
                }
            } else if (!first) {
                y += metrics::sidebarItemSpacing;
            }
            const double height = section ? child->sizeHint().height : metrics::sidebarItemHeight;
            if (place) {
                if (section)
                    child->setGeometry({metrics::sidebarMargin, y, width, height});
                else
                    child->setGeometry({metrics::sidebarMargin + inset, y, width - 2 * inset, height});
            }
            y += height;
            if (!section) {
                if (!inPanel) {
                    panelTop = y - height;
                    inPanel = true;
                }
                first = false;
            }
        }
        if (place && inPanel)
            m_panels.push_back({panelTop - inset, y + inset});
        return y + metrics::sidebarMargin;
    }

    std::string m_namePrefix;
    ScrollView* m_revealIn = nullptr;
    std::vector<Entry> m_entries;
    std::vector<SidebarItem*> m_items;
    bool m_resultMode = false;
    // Filled by measure(), which is const: the panels are what it found, not
    // state the list keeps.
    mutable std::vector<Panel> m_panels;
    int m_current = -1;
};

// A short gradient at the top or bottom of the scrolling list, so the rows
// fade into the sidebar instead of meeting the search field or the window edge.
class SidebarFade : public View
{
public:
    explicit SidebarFade(bool top)
        : m_top(top)
    {
    }

protected:
    void paint(Painter& painter) override
    {
        const Color background = color(SettingsColor::SidebarBackground);
        const Color clear = background.withAlpha(0);
        painter.fillRoundedRect(localRect(), 0,
                                LinearGradient::vertical(localRect(), m_top ? background : clear,
                                                         m_top ? clear : background));
    }

private:
    bool m_top;
};

Sidebar::Sidebar()
{
    m_title = addChild<Label>(tr("Settings"));
    m_title->setFontSize(metrics::points(kTitlePointSize));
    m_title->setVerticallyCentered(true);
    m_search = addChild<SearchField>();
    m_search->setName("search");
    m_search->onTextChanged = [this] { updateResults(); };
    m_search->onAccepted = [this] {
        if (isSearching() && m_currentResult >= 0)
            openResult(m_currentResult);
    };

    m_listScroll = addChild<ScrollView>();
    m_list = m_listScroll->setContent<SidebarList>();
    m_list->setNamePrefix("sidebar:");
    m_list->onFocusRequested = [this] { setFocus(FocusReason::Mouse); };
    m_list->onClicked = [this](int index) {
        const auto& entry = m_list->entries()[size_t(index)];
        if (onActivated)
            onActivated(entry.page->id, entry.rootPage->id);
    };
    m_resultsScroll = addChild<ScrollView>();
    m_resultsScroll->setVisible(false);
    m_resultsList = m_resultsScroll->setContent<SidebarList>();
    m_resultsList->setNamePrefix("result:");
    m_resultsList->setResultMode(true);
    m_resultsList->onFocusRequested = [this] { setFocus(FocusReason::Mouse); };
    m_resultsList->onClicked = [this](int index) { openResult(index); };
    m_topFade = addChild<SidebarFade>(true);
    m_bottomFade = addChild<SidebarFade>(false);
    m_noResults = addChild<Label>(tr("No results"));
    m_noResults->setElide(Label::Elide::None);
    m_noResults->setVisible(false);
}

Sidebar::~Sidebar() = default;

SizeF Sidebar::sizeHint() const
{
    return {metrics::sidebarWidth, 0};
}

void Sidebar::setEntries(std::vector<const PageInfo*> entries)
{
    m_entries = std::move(entries);
    std::vector<SidebarList::Entry> list;
    for (const PageInfo* page : m_entries)
        list.push_back({page, page});
    m_list->setEntries(std::move(list));
    setCurrent(m_currentPage, m_currentRootPage);
    if (isSearching())
        updateResults();
}

void Sidebar::retranslate()
{
    m_title->setText(tr("Settings"));
    m_search->setPlaceholder(tr("Search"));
    m_noResults->setText(tr("No results"));
    setEntries(m_entries);
}

void Sidebar::setSubPageAvailable(const std::string& id, bool available)
{
    auto it = std::find(m_unavailableSubPages.begin(), m_unavailableSubPages.end(), id);
    if (available && it != m_unavailableSubPages.end())
        m_unavailableSubPages.erase(it);
    else if (!available && it == m_unavailableSubPages.end())
        m_unavailableSubPages.push_back(id);
    if (isSearching())
        updateResults();
}

void Sidebar::setCurrent(const std::string& page, const std::string& rootPage)
{
    m_currentPage = page;
    m_currentRootPage = rootPage;
    int index = -1;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i]->id == rootPage)
            index = int(i);
    }
    if (index != m_list->currentIndex())
        m_list->revealCurrent(m_listScroll);
    m_list->setCurrentIndex(index);
    if (isSearching()) {
        // Back, Forward and hardware changes move the page without a click in the results.
        for (size_t i = 0; i < m_results.size(); ++i) {
            if (m_results[i].page->id == page) {
                m_currentResult = int(i);
                m_resultsList->setCurrentIndex(m_currentResult);
                return;
            }
        }
        m_currentResult = -1;
        m_resultsList->setCurrentIndex(-1);
    }
}

void Sidebar::focusSearch()
{
    m_search->setFocus(FocusReason::Keyboard);
    m_search->selectAll();
}

static std::string trimmed(const std::string& text)
{
    const size_t first = text.find_first_not_of(" \t\n");
    if (first == std::string::npos)
        return {};
    return text.substr(first, text.find_last_not_of(" \t\n") - first + 1);
}

void Sidebar::releaseSearch(PointF windowPosition)
{
    if (m_search->hasFocus() && !m_search->localRect().contains(m_search->mapFromWindow(windowPosition)))
        m_search->window()->contentView()->setFocus();
}

bool Sidebar::isSearching() const
{
    return !trimmed(m_search->text()).empty();
}

static std::string lowered(std::string text)
{
    for (char& c : text) {
        if (c >= 'A' && c <= 'Z')
            c = char(c - 'A' + 'a');
    }
    return text;
}

static std::string withoutHyphens(std::string text)
{
    std::erase(text, '-');
    return text;
}

// Chinese, Japanese and Korean put no spaces between words, so their words
// match anywhere.
static bool isCjk(const std::string& word)
{
    for (size_t i = 0; i < word.size();) {
        const auto byte = uint8_t(word[i]);
        uint32_t code = byte;
        size_t length = 1;
        if (byte >= 0xf0 && i + 3 < word.size()) {
            code = (byte & 0x07) << 18 | (word[i + 1] & 0x3f) << 12 | (word[i + 2] & 0x3f) << 6 | (word[i + 3] & 0x3f);
            length = 4;
        } else if (byte >= 0xe0 && i + 2 < word.size()) {
            code = (byte & 0x0f) << 12 | (word[i + 1] & 0x3f) << 6 | (word[i + 2] & 0x3f);
            length = 3;
        } else if (byte >= 0xc0 && i + 1 < word.size()) {
            code = (byte & 0x1f) << 6 | (word[i + 1] & 0x3f);
            length = 2;
        }
        if ((code >= 0x3040 && code <= 0x30ff) || (code >= 0x3400 && code <= 0x9fff)
            || (code >= 0xac00 && code <= 0xd7af))
            return true;
        i += length;
    }
    return false;
}

// Each word of the query must match the title, the category or a keyword.
// Latin words match from their start, so "fo" finds Fonts but not Performance.
static bool matches(const std::vector<std::string>& words, const std::vector<std::string>& fields)
{
    std::string text;
    for (const std::string& field : fields) {
        if (!text.empty())
            text += ' ';
        text += field;
    }
    text = lowered(text);
    for (char& c : text) {
        if (c == ',' || c == '&' || c == '(' || c == ')')
            c = ' ';
    }
    std::string spaced = text;
    std::replace(spaced.begin(), spaced.end(), '-', ' ');
    text = " " + spaced + " " + withoutHyphens(text);
    return std::all_of(words.begin(), words.end(), [&text](const std::string& word) {
        return text.find(isCjk(word) ? word : " " + word) != std::string::npos;
    });
}

static std::vector<std::string> queryWords(const std::string& query)
{
    std::vector<std::string> words;
    std::string word;
    for (const char c : withoutHyphens(lowered(query))) {
        if (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '&' || c == '(' || c == ')') {
            if (!word.empty())
                words.push_back(std::move(word));
            word.clear();
        } else {
            word += c;
        }
    }
    if (!word.empty())
        words.push_back(std::move(word));
    return words;
}

void Sidebar::updateResults()
{
    const bool searching = isSearching();
    m_listScroll->setVisible(!searching);
    m_resultsScroll->setVisible(searching);
    m_results.clear();
    if (searching) {
        const std::vector<std::string> words = queryWords(m_search->text());
        for (const PageInfo* entry : m_entries) {
            if (matches(words, {tr(entry->title.c_str()), tr(entry->category.c_str()), tr(entry->keywords.c_str())}))
                m_results.push_back({entry, entry});
            for (const PageInfo& sub : pages()) {
                if (sub.parentId != entry->id
                    || std::find(m_unavailableSubPages.begin(), m_unavailableSubPages.end(), sub.id)
                            != m_unavailableSubPages.end()
                    || !matches(words, {tr(sub.title.c_str()), tr(entry->title.c_str()), tr(entry->category.c_str()),
                                        tr(sub.keywords.c_str())}))
                    continue;
                m_results.push_back({&sub, entry});
            }
        }
    }
    std::vector<SidebarList::Entry> list;
    for (const Result& result : m_results)
        list.push_back({result.page, result.rootPage});
    m_resultsList->setEntries(std::move(list));
    m_currentResult = m_results.empty() ? -1 : 0;
    for (size_t i = 0; i < m_results.size(); ++i) {
        if (m_results[i].page->id == m_currentPage)
            m_currentResult = int(i);
    }
    m_resultsList->setCurrentIndex(m_currentResult);
    m_noResults->setVisible(searching && m_results.empty());
    requestLayout();
}

void Sidebar::openResult(int index)
{
    if (index < 0 || index >= int(m_results.size()))
        return;
    m_currentResult = index;
    m_resultsList->setCurrentIndex(index);
    if (onActivated)
        onActivated(m_results[size_t(index)].page->id, m_results[size_t(index)].rootPage->id);
}

bool Sidebar::keyPress(const KeyEvent& event)
{
    if (!m_search->hasFocus() && !hasFocus())
        return false;
    if (!isSearching()) {
        // With no query the arrows walk the sidebar's own pages; Return opens
        // the highlighted one.
        if (event.key == key::Down || event.key == key::Up) {
            const int count = m_list->count();
            if (count == 0)
                return true;
            int index = m_list->currentIndex();
            if (index < 0)
                index = 0;
            else
                index = std::clamp(index + (event.key == key::Down ? 1 : -1), 0, count - 1);
            if (index != m_list->currentIndex()) {
                m_list->setCurrentIndex(index);
                m_list->revealCurrent(m_listScroll);
                const auto& entry = m_list->entries()[size_t(index)];
                if (onActivated)
                    onActivated(entry.page->id, entry.rootPage->id);
            }
            return true;
        }
        if (event.key == key::Return) {
            const int index = m_list->currentIndex();
            if (index >= 0 && index < m_list->count()) {
                const auto& entry = m_list->entries()[size_t(index)];
                if (onActivated)
                    onActivated(entry.page->id, entry.rootPage->id);
            }
            return true;
        }
    }
    const int count = int(m_results.size());
    switch (event.key) {
    case key::Down:
        if (count > 0) {
            m_currentResult = std::min(m_currentResult + 1, count - 1);
            m_resultsList->setCurrentIndex(m_currentResult);
            m_resultsScroll->ensureVisible(m_resultsList->itemRect(m_currentResult));
        }
        return true;
    case key::Up:
        if (count > 0) {
            m_currentResult = std::max(m_currentResult - 1, 0);
            m_resultsList->setCurrentIndex(m_currentResult);
            m_resultsScroll->ensureVisible(m_resultsList->itemRect(m_currentResult));
        }
        return true;
    case key::Escape:
        if (!m_search->text().empty())
            m_search->setText({});
        else
            m_search->window()->contentView()->setFocus();
        return true;
    default:
        return false;
    }
}

void Sidebar::layout()
{
    const Theme& theme = Theme::current();
    const double width = geometry().width;
    const double height = geometry().height;
    const bool active = windowActive(this);
    // The title's line is the file manager's sidebar title's: centred in the
    // header height, pushed down by smallSpacing.
    const double titleHeight = 40 + metrics::m;
    const double titleLeft = metrics::sidebarMargin + metrics::sidebarItemPadding;
    m_title->setColor(active ? theme.text : theme.disabledText);
    m_title->setGeometry({titleLeft, theme.smallSpacing, std::max(0.0, width - 2 * titleLeft),
                          titleHeight - theme.smallSpacing - metrics::m});
    const double searchTop = titleHeight + metrics::xs;
    m_search->setGeometry({metrics::sidebarMargin, searchTop, width - 2 * metrics::sidebarMargin, kSearchHeight});
    const double listTop = searchTop + kSearchHeight;
    m_listScroll->setGeometry({0, listTop, width, height - listTop});
    m_resultsScroll->setGeometry({0, listTop, width, height - listTop});
    const double fadeHeight = metrics::m;
    m_topFade->setGeometry({0, listTop, width, fadeHeight});
    m_bottomFade->setGeometry({0, height - fadeHeight, width, fadeHeight});
    m_noResults->setColor(theme.disabledText);
    const SizeF noResults = m_noResults->sizeHint();
    m_noResults->setGeometry({std::round((width - noResults.width) / 2), listTop + metrics::sidebarCategorySpacing,
                              noResults.width, noResults.height});
}

void Sidebar::paint(Painter& painter)
{
    painter.fillRect(localRect(), color(SettingsColor::SidebarBackground));
}

} // namespace settings
