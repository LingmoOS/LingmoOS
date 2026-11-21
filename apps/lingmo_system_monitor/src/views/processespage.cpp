// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "processespage.h"

#include "colors.h"
#include "components.h"
#include "monitor.h"
#include "processlist.h"

#include "base/i18n.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/search_field.h"
#include "ui/segmented_control.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <array>

using namespace Lingmo::ui;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

static constexpr double kMargin = 12;
static constexpr double kTop = 22;
static constexpr double kSpacing = 16;
static constexpr double kScopeWidth = 357;
static constexpr double kRowHeight = 36;
static constexpr double kFieldHeight = 30;
static constexpr double kHeaderHeight = 30;
static constexpr double kCpuWidth = 72;
static constexpr double kMemoryWidth = 88;
static constexpr double kIoWidth = 88;
static constexpr double kPidWidth = 64;
static constexpr double kNumbersWidth = kCpuWidth + kMemoryWidth + 2 * kIoWidth + kPidWidth;

struct TableColumn
{
    const char* key;
    const char* title;
    double width;
};

// The name column takes what the others leave.
static constexpr std::array<TableColumn, 6> kColumns{{
    {"name", trNoop("Name"), 0},
    {"cpu", trNoop("CPU"), kCpuWidth},
    {"memory", trNoop("Memory"), kMemoryWidth},
    {"read", trNoop("Read"), kIoWidth},
    {"write", trNoop("Write"), kIoWidth},
    {"pid", trNoop("PID"), kPidWidth},
}};

bool ProcessListView::keyPress(const KeyEvent& event)
{
    if (event.modifiers != NoModifier)
        return ListView::keyPress(event);
    if (event.key == key::Up || event.key == key::Down) {
        if (onStep)
            onStep(event.key == key::Up ? -1 : 1);
        return true;
    }
    // Qt's Keys.onReturnPressed: the keypad's Enter is another key.
    if (event.key == key::Return) {
        if (onOpen)
            onOpen();
        return true;
    }
    if (event.key == key::Delete) {
        if (onQuit)
            onQuit();
        return true;
    }
    return ListView::keyPress(event);
}

// A round button with the quit glyph; disabled, it sinks into the page.
class ProcessesPage::QuitButton : public View
{
public:
    QuitButton()
        : m_icon(addChild<Icon>("quit", 16))
    {
        m_icon->setName("endProcessIcon");
    }

    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_icon->setGeometry({rounding::centeredStart(geometry().width, 16), rounding::centeredStart(geometry().height, 16), 16, 16});
        m_icon->setColor(Theme::current().text);
        m_icon->setOpacity(isEnabled() ? 1.0 : 0.35);
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        Color fill = theme.secondBackground;
        if (!isEnabled())
            fill = color(MonitorColor::MutedPlate);
        else if (m_pressed)
            fill = tint(theme.secondBackground, theme.text.withAlpha(0.06));
        else if (m_hovered)
            fill = tint(theme.secondBackground, theme.text.withAlpha(0.025));
        painter.fillRoundedRect(localRect(), geometry().height / 2, fill);
    }

    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }

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
    Icon* m_icon;
    bool m_hovered = false;
    bool m_pressed = false;
};

// A column's title, which sorts the list by the column.
class ProcessesPage::ColumnHeader : public View
{
public:
    ColumnHeader(std::string column, std::string title, bool numeric)
        : m_column(std::move(column))
        , m_numeric(numeric)
        , m_label(addLabel(this, std::move(title), 8, 500))
        , m_indicator(addChild<Icon>("chevron-down", 10))
    {
    }

    const std::string& column() const { return m_column; }

    void setIndicator(bool visible, bool ascending)
    {
        m_indicator->setVisible(visible);
        m_indicator->setIconName(ascending ? "chevron-up" : "chevron-down");
    }

    bool acceptsPointer() const override { return true; }
    // A Tab stop without a focus ring, as FishUI's AbstractButton.
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    bool keyPress(const KeyEvent& event) override
    {
        if (event.modifiers != NoModifier || event.key != key::Space)
            return false;
        m_spacePressed = true;
        return true;
    }

    bool keyRelease(const KeyEvent& event) override
    {
        if (event.key != key::Space || !m_spacePressed)
            return false;
        m_spacePressed = false;
        if (onClicked)
            onClicked();
        return true;
    }

    void layout() override
    {
        const Color color = Theme::current().disabledText;
        const SizeF size = m_label->sizeHint();
        const double x = m_numeric ? geometry().width - size.width - 12 : 12;
        m_label->setColor(color);
        m_label->setGeometry({x, rounding::centeredStart(geometry().height, size.height), size.width, size.height});
        // Numeric titles stay flush with their values; the indicator sits in the cell's 12 px margin.
        m_indicator->setColor(color);
        m_indicator->setGeometry({x + size.width + 3, rounding::centeredStart(geometry().height, 10), 10, 10});
    }

    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }

    void pointerRelease(const PointerEvent& event) override
    {
        const bool pressed = m_pressed;
        m_pressed = false;
        if (pressed && localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    std::string m_column;
    bool m_numeric;
    Label* m_label;
    Icon* m_indicator;
    bool m_pressed = false;
    bool m_spacePressed = false;
};

// One process or application of the list.
class ProcessesPage::Row : public View
{
public:
    explicit Row(ProcessesPage* page)
        : m_page(page)
        , m_icon(addChild<ThemeIcon>("application-x-executable", 22))
        , m_name(addLabel(this, {}, 9))
    {
        m_name->setVerticallyCentered(true);
        for (size_t i = 0; i < m_values.size(); ++i) {
            m_values[i] = addLabel(this, {}, 9);
            m_values[i]->setVerticallyCentered(true);
            m_values[i]->setTextAlignment(Label::TextAlignment::Right);
        }
    }

    void bind(size_t row, const ProcessRecord& process, bool highlighted)
    {
        m_row = row;
        m_highlighted = highlighted;
        m_icon->setIconName(process.iconName.empty() ? "application-x-executable" : process.iconName);
        m_name->setText(process.displayName);
        m_values[0]->setText(Monitor::formatPercent(process.cpu));
        m_values[1]->setText(Monitor::formatBytes(double(process.memory)));
        m_values[2]->setText(process.ioAvailable ? Monitor::formatRate(process.readRate) : "—");
        m_values[3]->setText(process.ioAvailable ? Monitor::formatRate(process.writeRate) : "—");
        m_values[4]->setText(std::to_string(process.pid));
        requestLayout();
        requestPaint();
    }

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const double height = geometry().height;
        const double nameWidth = m_page->nameWidth();
        m_width = m_page->m_table->geometry().width;
        m_icon->setGeometry({12, rounding::centeredStart(height, 22), 22, 22});
        m_name->setColor(m_highlighted ? theme.highlightedText : theme.text);
        m_name->setGeometry({44, 0, std::max(0.0, nameWidth - 44 - 12), height});
        double x = nameWidth;
        for (size_t i = 0; i < m_values.size(); ++i) {
            const double width = kColumns[i + 1].width;
            // Rates and PIDs are secondary.
            const Color normal = i >= 2 ? theme.disabledText : theme.text;
            m_values[i]->setColor(m_highlighted ? theme.highlightedText : normal);
            m_values[i]->setGeometry({x, 0, width - 12, height});
            x += width;
        }
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        const RectF rect{0, 0, m_width, geometry().height};
        // Masked to the table's rounded body, as the rows reach its bottom corners.
        const View* table = m_page->m_table;
        const PointF tableOrigin = table->mapToWindow({}) - mapToWindow({});
        painter.clipRoundedRect({tableOrigin.x, tableOrigin.y, table->geometry().width, table->geometry().height},
                                theme.bigRadius, theme.continuousCorners ? Corners::Continuous : Corners::Circular);
        if (m_highlighted)
            painter.fillRect(rect, theme.highlight);
        else if (m_row % 2)
            painter.fillRect(rect, theme.text.withAlpha(0.018));
    }

    void pointerPress(const PointerEvent& event) override
    {
        // The list reaches past the table to the window's edge for its scroll bar.
        if (event.position.x >= m_width)
            return;
        if (event.button == MouseButton::Right) {
            m_page->select(m_row);
            if (m_page->onContextMenu)
                m_page->onContextMenu(mapToWindow(event.position));
            return;
        }
        if (event.button != MouseButton::Left)
            return;
        m_page->select(m_row);
        m_page->m_list->setFocus(FocusReason::Mouse);
        if (event.clickCount == 2 && m_page->onOpenDetails)
            m_page->onOpenDetails();
    }

private:
    ProcessesPage* m_page;
    size_t m_row = 0;
    bool m_highlighted = false;
    double m_width = 0;
    ThemeIcon* m_icon;
    Label* m_name;
    std::array<Label*, 5> m_values{};
};

// A hairline the width of the table, one device pixel at any scale.
class Hairline : public View
{
protected:
    void paint(Painter& painter) override
    {
        painter.drawHorizontalLine(0, 0, geometry().width, Theme::current().text.withAlpha(0.07), 1);
    }
};

ProcessesPage::ProcessesPage(Monitor& monitor, Selection& selection)
    : m_monitor(monitor)
    , m_selection(selection)
    , m_scope(addChild<SegmentedControl>(std::vector<std::string>{tr("Applications"), tr("My Processes"), tr("All Processes")}))
    , m_search(addChild<SearchField>(tr("Search processes")))
    , m_quit(addChild<QuitButton>())
    , m_table(addChild<Plate>(Theme::current().bigRadius))
    , m_hairline(m_table->addChild<Hairline>())
    , m_list(addChild<ProcessListView>())
    , m_empty(m_table->addChild<Column>())
{
    m_scope->setName("processScope");
    m_search->setName("processSearch");
    m_quit->setName("endProcessButton");
    m_table->setName("processTable");
    m_list->setName("processList");
    // Qt's ListView takes focus on a click only.
    m_list->setClickFocusOnly(true);
    m_scope->setFontSize(points(9));
    m_scope->setCurrentIndex(m_monitor.scope());
    m_scope->onCurrentChanged = [this](int index) {
        m_monitor.setScope(index);
        clearSelection();
        reload();
    };

    m_search->setFontSize(points(9));
    m_search->setText(m_monitor.processes().search());
    m_search->onTextChanged = [this] {
        m_monitor.processes().setSearch(m_search->text());
        m_monitor.applications().setSearch(m_search->text());
        clearSelection();
    };

    m_quit->onClicked = [this] {
        if (onConfirmEnd)
            onConfirmEnd(false);
    };

    m_table->setClipsChildren(true);
    for (const TableColumn& column : kColumns) {
        ColumnHeader* header = m_table->addChild<ColumnHeader>(column.key, tr(column.title), column.width > 0);
        header->onClicked = [this, key = std::string(column.key)] { m_monitor.sortBy(key); };
        m_headers.push_back(header);
    }

    m_list->setRowHeight(kRowHeight);
    m_list->createItem = [this] { return std::make_unique<Row>(this); };
    m_list->bindItem = [this](View* item, size_t row) {
        const ProcessRecord* process = list().at(row);
        if (!process)
            return;
        const bool highlighted = m_selection.pid == process->pid && m_selection.startTicks == process->startTicks
                                 && m_selection.application == applicationsMode();
        static_cast<Row*>(item)->bind(row, *process, highlighted);
    };
    m_list->onStep = [this](int step) {
        const int count = int(list().count());
        if (count == 0)
            return;
        const int current = list().indexOf(m_selection.pid);
        const int row = std::clamp(current + step, 0, count - 1);
        select(size_t(row));
        m_list->scrollToRow(size_t(row));
    };
    m_list->onOpen = [this] {
        if (onOpenDetails)
            onOpenDetails();
    };
    m_list->onQuit = [this] {
        if (onConfirmEnd)
            onConfirmEnd(false);
    };

    auto* empty = static_cast<Column*>(m_empty);
    empty->setSpacing(8);
    m_emptyTitle = addLabel(empty, {}, 0, 500);
    m_emptyTitle->setTextAlignment(Label::TextAlignment::Center);
    m_emptyTitle->setLayoutHints({.fillWidth = true});
    m_emptyText = addLabel(empty, {}, 9);
    m_emptyText->setTextAlignment(Label::TextAlignment::Center);
    m_emptyText->setLayoutHints({.fillWidth = true});

    for (ProcessList* processes : {&m_monitor.processes(), &m_monitor.applications()}) {
        m_subscriptions.push_back(processes->changed.connect([this, processes] {
            if (processes == &list())
                reload();
        }));
        m_subscriptions.push_back(processes->sortingChanged.connect([this] { updateSorting(); }));
    }
    m_subscriptions.push_back(m_monitor.updated.connect([this] { reload(); }));
    updateSorting();
    reload();
}

bool ProcessesPage::applicationsMode() const
{
    return m_scope->currentIndex() == 0;
}

ProcessList& ProcessesPage::list() const
{
    return applicationsMode() ? m_monitor.applications() : m_monitor.processes();
}

bool ProcessesPage::isEditingSearch() const
{
    return m_search->hasFocus();
}

void ProcessesPage::focusSearch()
{
    m_search->setFocus(FocusReason::Other);
    m_search->selectAll();
}

void ProcessesPage::clearSearch()
{
    m_search->setText({});
    m_monitor.processes().setSearch({});
    m_monitor.applications().setSearch({});
    clearSelection();
    m_list->setFocus(FocusReason::Other);
}

void ProcessesPage::selectionChanged()
{
    const ProcessRecord* process = m_selection.application ? m_monitor.application(m_selection.pid, m_selection.startTicks)
                                                           : m_monitor.process(m_selection.pid, m_selection.startTicks);
    m_quit->setEnabled(process && Monitor::canManage(*process));
    m_list->reload();
}

void ProcessesPage::select(size_t row)
{
    const ProcessRecord* process = list().at(row);
    m_selection.application = applicationsMode();
    m_selection.pid = process ? process->pid : 0;
    m_selection.startTicks = process ? process->startTicks : 0;
    selectionChanged();
    if (onSelectionChanged)
        onSelectionChanged();
}

void ProcessesPage::clearSelection()
{
    m_selection.pid = 0;
    m_selection.startTicks = 0;
    m_selection.application = applicationsMode();
    selectionChanged();
    if (onSelectionChanged)
        onSelectionChanged();
}

void ProcessesPage::reload()
{
    const size_t count = list().count();
    m_list->setRowCount(count);
    m_list->reload();
    m_empty->setVisible(count == 0 && m_monitor.ready());
    m_emptyTitle->setText(applicationsMode() ? tr("No applications found") : tr("No processes found"));
    m_emptyText->setText(tr("Try a different name, user, or process ID."));
    selectionChanged();
    requestLayout();
}

void ProcessesPage::updateSorting()
{
    const ProcessList& processes = m_monitor.processes();
    for (ColumnHeader* header : m_headers)
        header->setIndicator(header->column() == processes.sortColumn(), processes.ascending());
}

double ProcessesPage::nameWidth() const
{
    return std::max(0.0, m_table->geometry().width - kNumbersWidth);
}

void ProcessesPage::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    const double right = width - kMargin;
    m_scope->setTrackColor(color(MonitorColor::MutedPlate));
    m_scope->setGeometry({kMargin, kTop, kScopeWidth, 36});
    const double fieldTop = kTop + (36 - kFieldHeight) / 2;
    m_quit->setGeometry({right - kFieldHeight, fieldTop, kFieldHeight, kFieldHeight});
    const double searchX = kMargin + kScopeWidth + kSpacing;
    const double searchWidth = std::max(0.0, right - kFieldHeight - kSpacing - searchX);
    m_search->setGeometry({searchX, fieldTop, searchWidth, kFieldHeight});

    const double tableTop = kTop + 36 + kSpacing;
    m_table->setGeometry({kMargin, tableTop, width - 2 * kMargin, std::max(0.0, height - tableTop - kMargin)});
    const double tableWidth = m_table->geometry().width;
    double x = 0;
    for (size_t i = 0; i < m_headers.size(); ++i) {
        const double columnWidth = kColumns[i].width > 0 ? kColumns[i].width : std::max(0.0, tableWidth - kNumbersWidth);
        m_headers[i]->setGeometry({x, 0, columnWidth, kHeaderHeight});
        x += columnWidth;
    }
    const Window* window = this->window();
    const double hairline = 1 / (window ? window->scale() : 1.0);
    m_hairline->setGeometry({0, kHeaderHeight, tableWidth, hairline});
    // Its scroll bar lives at the window's right edge, beside the table rather than over its rows.
    const double listTop = kHeaderHeight + hairline;
    m_list->setGeometry({kMargin, tableTop + listTop, width - kMargin,
                         std::max(0.0, m_table->geometry().height - listTop)});
    m_emptyText->setColor(Theme::current().disabledText);
    const double emptyWidth = std::max(0.0, tableWidth - 64);
    const double emptyHeight = m_empty->heightForWidth(emptyWidth);
    m_empty->setGeometry({rounding::centeredStart(tableWidth, emptyWidth), rounding::centeredStart(m_table->geometry().height, emptyHeight),
                          emptyWidth, emptyHeight});
}
