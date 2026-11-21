// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <unicode/ucasemap.h>

#include "base/i18n.h"
#include "pages/input/checkrow.h"
#include "pages/keyboard/keyboard.h"
#include "ui/label.h"
#include "ui/list_view.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "widgets/searchfield.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

// Unicode's default lower case, as JavaScript's toLowerCase() gives it.
static std::string lowerCase(const std::string& text)
{
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCaseMap, decltype(&ucasemap_close)> map(ucasemap_open("", 0, &status), ucasemap_close);
    if (U_FAILURE(status))
        return text;
    std::string result(text.size() + 16, '\0');
    int32_t length = ucasemap_utf8ToLower(map.get(), result.data(), int32_t(result.size()), text.data(),
                                          int32_t(text.size()), &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        result.resize(size_t(length));
        length = ucasemap_utf8ToLower(map.get(), result.data(), int32_t(result.size()), text.data(),
                                      int32_t(text.size()), &status);
    }
    if (U_FAILURE(status))
        return text;
    result.resize(size_t(length));
    return result;
}

static std::string trimmed(const std::string& text)
{
    const auto space = [](unsigned char c) { return c == ' ' || (c >= '\t' && c <= '\r'); };
    size_t start = 0;
    size_t end = text.size();
    while (start < end && space(text[start]))
        ++start;
    while (end > start && space(text[end - 1]))
        --end;
    return text.substr(start, end - start);
}

namespace {

// Opens with a row at the top, once it has its size. The Qt version asked
// for the row centred before its list had a height, which put it there.
class LayoutListView : public ListView
{
public:
    void openAt(std::optional<size_t> row) { m_openRow = row; }

protected:
    void layout() override
    {
        ListView::layout();
        if (!m_openRow || geometry().height <= 0)
            return;
        const RectF rect = rowRect(*m_openRow);
        m_openRow.reset();
        setScrollPosition({0, rect.y});
    }

private:
    std::optional<size_t> m_openRow;
};

// The list of layouts, with a note over it while nothing matches.
class LayoutList : public View
{
public:
    LayoutList()
    {
        m_list = addChild<LayoutListView>();
        m_list->setElastic(false);
        m_list->setRowHeight(metrics::rowHeight);
        m_empty = addChild<Label>(tr("No layouts found"));
        m_empty->setName("keyboardlayout:empty");
        m_empty->setVisible(false);
    }

    LayoutListView* list() const { return m_list; }
    void setEmpty(bool empty) { m_empty->setVisible(empty); }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        m_list->setGeometry(bounds);
        m_empty->setColor(Theme::current().disabledText);
        const SizeF size = m_empty->sizeHint();
        m_empty->setGeometry({rounding::centeredStart(bounds.width, size.width),
                              rounding::centeredStart(bounds.height, size.height), size.width, size.height});
    }

private:
    LayoutListView* m_list;
    Label* m_empty;
};

class LayoutPage : public Page
{
public:
    explicit LayoutPage(SettingsWindow& window)
        : Page(window, tr("Layout"))
    {
        // The list scrolls, not the page.
        setScrollDirections(false, false);
        content()->setSpacing(metrics::l);

        m_search = content()->addChild<SearchField>();
        m_search->setName("keyboardlayout:search");
        m_search->setPlaceholder(tr("Search layouts"));
        m_search->setLayoutHints({.fillWidth = true});
        m_search->onTextChanged = [this] {
            filter();
            m_layouts->list()->setScrollPosition({0, 0});
        };

        Card* card = content()->addChild<Card>(Card::Style::List);
        card->setLayoutHints({.fillWidth = true, .fillHeight = true});
        m_layouts = card->addChild<LayoutList>();
        m_layouts->setName("keyboardlayout:list");
        m_layouts->setLayoutHints({.fillWidth = true, .fillHeight = true});
        ListView* list = m_layouts->list();
        list->createItem = [] { return std::make_unique<CheckRow>(); };
        list->bindItem = [this](View* item, size_t row) {
            auto* check = static_cast<CheckRow*>(item);
            const KeyboardLayout& layout = *m_shown[row];
            check->setViewIndex(m_layouts->list(), int(row));
            check->setText(layout.name);
            check->setChecked(layout.id == m_current);
            check->onClicked = [this, id = layout.id] { m_keyboard.selectLayout(id); };
        };

        m_current = m_keyboard.layout();
        m_changed = m_keyboard.changed().connect([this] {
            m_current = m_keyboard.layout();
            m_layouts->list()->reload();
        });
        filter();
        for (size_t row = 0; row < m_shown.size(); ++row) {
            if (m_shown[row]->id == m_current)
                m_layouts->list()->openAt(row);
        }
    }

private:
    void filter()
    {
        const std::string query = lowerCase(trimmed(m_search->text()));
        m_shown.clear();
        for (const KeyboardLayout& layout : keyboardLayouts()) {
            if (query.empty() || lowerCase(layout.name).find(query) != std::string::npos
                || lowerCase(layout.id).find(query) != std::string::npos)
                m_shown.push_back(&layout);
        }
        m_layouts->list()->setRowCount(m_shown.size());
        m_layouts->list()->reload();
        m_layouts->setEmpty(m_shown.empty());
    }

    Keyboard m_keyboard;
    base::Subscription m_changed;
    SearchField* m_search;
    LayoutList* m_layouts;
    std::vector<const KeyboardLayout*> m_shown;
    std::string m_current;
};

} // namespace

static const PageRegistration registration("keyboardlayout", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<LayoutPage>(window);
});

} // namespace settings
