// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"

#include "components.h"
#include "monitor.h"
#include "processdetails.h"
#include "resourcepage.h"
#include "summarycard.h"
#include "textmetrics.h"

#include "base/i18n.h"
#include "ui/application.h"
#include "ui/clipboard.h"
#include "ui/dialog_window.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/toast.h"
#include "ui/window_header.h"

#include <algorithm>
#include <optional>

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;

static constexpr double kSidebarWidth = 240;
static constexpr double kHeaderHeight = 48;
// The title and window buttons end on this line; content spacing is measured from it.
static constexpr double kHeaderBaseline = 28;
static const char* const kPageTitles[] = {trNoop("Processes"), trNoop("CPU"), trNoop("Memory"),
                                          trNoop("Network"), trNoop("Disk"), trNoop("GPU")};

static std::vector<double> sumSeries(const std::vector<double>& first, const std::vector<double>& second)
{
    std::vector<double> sum = first;
    for (size_t i = 0; i < sum.size(); ++i)
        sum[i] += i < second.size() ? second[i] : 0;
    return sum;
}

// The header's titles and the update interval button.
class HeaderContent : public View
{
public:
    explicit HeaderContent(WindowHeader* header)
        : m_header(header)
        , m_applicationTitle(addLabel(this, tr("System Monitor"), 11))
        , m_pageTitle(addLabel(this, tr(kPageTitles[0]), 11))
        , m_interval(addChild<HeaderButton>("menu", 32))
    {
        m_applicationTitle->setName("applicationTitle");
        m_pageTitle->setName("pageTitle");
        m_interval->setName("refreshInterval");
    }

    Label* pageTitle() const { return m_pageTitle; }
    HeaderButton* intervalButton() const { return m_interval; }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const Window* window = this->window();
        const Color color = !window || window->isActive() ? theme.text : theme.disabledText;
        const TextMetrics metrics = textMetrics(m_applicationTitle);
        const double top = snapToDevice(this, kHeaderBaseline - metrics.ascent);
        // The window buttons centre on the title's capitals.
        const double capCenter = snapToDevice(this, snapToDevice(this, kHeaderBaseline) - metrics.capHeight / 2);
        const double buttonTop = capCenter - 16;
        m_header->setButtonMargin(buttonTop);

        m_applicationTitle->setColor(color);
        m_applicationTitle->setGeometry({20, top, kSidebarWidth - 20, metrics.lineHeight});
        const double right = geometry().width;
        m_interval->setGeometry({right - 32, buttonTop, 32, 32});
        const double pageX = 20 + kSidebarWidth - 20 + 12;
        m_pageTitle->setColor(color);
        m_pageTitle->setGeometry({pageX, top, std::max(0.0, right - 32 - 12 - pageX), metrics.lineHeight});
    }

private:
    WindowHeader* m_header;
    Label* m_applicationTitle;
    Label* m_pageTitle;
    HeaderButton* m_interval;
};

class MainWindow::Root : public View
{
public:
    explicit Root(MainWindow* window)
        : m_owner(window)
    {
        header = addChild<WindowHeader>(kHeaderHeight);
        content = header->setContent<HeaderContent>(header);

        sidebar = addChild<ScrollView>();
        auto* column = sidebar->setContent<Column>();
        column->setMargins({12, 4, 12, 16});
        column->setSpacing(12);
        processesButton = column->addChild<ProcessesButton>();
        processesButton->setLayoutHints({.fillWidth = true});
        cpu = column->addChild<SummaryCard>(tr("CPU"), "cpu", accent::blue);
        memory = column->addChild<SummaryCard>(tr("Memory"), "memory", accent::purple);
        network = column->addChild<SummaryCard>(tr("Network"), "network", accent::green);
        disk = column->addChild<SummaryCard>(tr("Disk"), "disk", accent::orange);
        gpu = column->addChild<SummaryCard>(tr("GPU"), "gpu", accent::pink);
        for (SummaryCard* card : {cpu, memory, network, disk, gpu})
            card->setLayoutHints({.fillWidth = true});
        sidebar->setName("resourceSidebar");
        processesButton->setName("processesNavigation");
        cpu->setName("cpuSummary");
        memory->setName("memorySummary");
        network->setName("networkSummary");
        disk->setName("diskSummary");
        gpu->setName("gpuSummary");
        network->setMaximum(0);
        network->setScaleFloor(1024 * 1024);
        disk->setMaximum(0);
        disk->setScaleFloor(1024 * 1024);
        gpu->setVisible(false);

        pages = addChild<View>();
        pages->setClipsChildren(true);
        error = addLabel(this, {});
        error->setVisible(false);
    }

    bool acceptsPointer() const override { return false; }

    WindowHeader* header;
    HeaderContent* content;
    ScrollView* sidebar;
    ProcessesButton* processesButton;
    SummaryCard* cpu;
    SummaryCard* memory;
    SummaryCard* network;
    SummaryCard* disk;
    SummaryCard* gpu;
    // The resource page's viewport, under the header.
    View* pages;
    ProcessesPage* processes = nullptr;
    Label* error;

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        header->setGeometry({0, 0, width, kHeaderHeight});
        sidebar->setGeometry({0, kHeaderHeight, kSidebarWidth, std::max(0.0, height - kHeaderHeight)});
        if (processes)
            processes->setGeometry({kSidebarWidth, kHeaderBaseline, std::max(0.0, width - kSidebarWidth),
                                    std::max(0.0, height - kHeaderBaseline)});
        pages->setGeometry({kSidebarWidth, kHeaderHeight, std::max(0.0, width - kSidebarWidth),
                            std::max(0.0, height - kHeaderHeight)});
        for (const auto& page : pages->children())
            page->setGeometry(pages->localRect());
        const SizeF errorSize = error->sizeHint();
        error->setColor(accent::red);
        error->setGeometry({rounding::centeredStart(width, errorSize.width), height - 12 - errorSize.height, errorSize.width,
                            errorSize.height});
    }

private:
    MainWindow* m_owner;
};

MainWindow::MainWindow(Monitor& monitor)
    : m_monitor(monitor)
    , m_toast(&m_window)
{
    m_window.resize({1040, 700});
    m_window.setMinimumSize({840, 600});
    m_window.setRestoresSize("Main");
    buildContent();

    m_window.addShortcut(ControlModifier, 'f', [this] {
        setCurrentPage(0);
        m_processes->focusSearch();
    });
    m_window.addShortcut(NoModifier, key::Escape, [this] {
        if (m_page == 0 && !m_processes->isEditingSearch())
            return;
        setCurrentPage(0);
        m_processes->clearSearch();
    });
    m_window.onActiveChanged = [this] { m_root->content->requestLayout(); };

    m_updated = m_monitor.updated.connect([this] { update(); });
    m_languageChanged = Application::instance()->languageChanged.connect([this] { retranslate(); });
    update();
    setCurrentPage(0);
}

void MainWindow::buildContent()
{
    m_window.setTitle(tr("System Monitor"));
    auto root = std::make_unique<Root>(this);
    m_root = root.get();
    m_processes = m_root->addChild<ProcessesPage>(m_monitor, m_selection);
    m_root->processes = m_processes;
    // Over the page, whose top reaches into the header.
    m_root->raiseChild(m_root->header);
    m_window.setContentView(std::move(root));

    m_root->processesButton->onClicked = [this] { setCurrentPage(0); };
    int page = 1;
    for (SummaryCard* card : {m_root->cpu, m_root->memory, m_root->network, m_root->disk, m_root->gpu}) {
        card->onClicked = [this, page] { setCurrentPage(page); };
        ++page;
    }

    m_intervalMenu.clear();
    for (const auto& [text, interval] : std::vector<std::pair<const char*, int>>{
             {tr("Update Every Second"), 1000}, {tr("Update Every 2 Seconds"), 2000}, {tr("Update Every 5 Seconds"), 5000}}) {
        m_intervalMenu.addItem(text, [this, interval] { m_monitor.setInterval(interval); });
    }
    HeaderButton* intervalButton = m_root->content->intervalButton();
    intervalButton->onClicked = [this, intervalButton] {
        for (size_t i = 0; i < 3; ++i)
            m_intervalMenu.setItemChecked(i, m_monitor.interval() == (i == 0 ? 1000 : i == 1 ? 2000 : 5000));
        // DesktopMenu.popupFor(intervalButton.background, 3): under the button's 25.33 px plate.
        const double plate = 76.0 / 3;
        const double inset = (32 - plate) / 2;
        intervalButton->setChecked(true);
        m_intervalMenu.popupFor(intervalButton, 3, RectF{inset, inset, plate, plate});
    };
    m_intervalMenu.onClosed = [intervalButton] { intervalButton->setChecked(false); };

    m_contextMenu.clear();
    m_contextMenu.addItem(tr("Details"), [this] { openDetails(); });
    m_contextMenu.addItem(tr("Copy Process ID"), [this] { Clipboard::setText(std::to_string(m_selection.pid)); });
    m_contextMenu.addSeparator();
    m_contextMenu.addItem(tr("Quit"), [this] { confirmEnd(false); });
    m_contextMenu.addItem(tr("Force Quit"), [this] { confirmEnd(true); });

    m_processes->onOpenDetails = [this] { openDetails(); };
    m_processes->onConfirmEnd = [this](bool force) { confirmEnd(force); };
    m_processes->onContextMenu = [this](PointF position) {
        updateContextMenu();
        m_contextMenu.popup(&m_window, position);
    };
}

// The views are made again; the page, the scope, the search, the selection
// and an open details window stay.
void MainWindow::retranslate()
{
    const int page = m_page;
    std::optional<Selection> details;
    if (m_details)
        details = m_details->target();
    m_details.reset();
    m_confirm.reset();
    m_resource = nullptr;
    buildContent();
    update();
    setCurrentPage(page);
    m_processes->selectionChanged();
    if (details) {
        m_details = std::make_unique<ProcessDetails>(&m_window, m_monitor, *details);
        m_details->onClosed = [this] { m_details.reset(); };
        m_details->show();
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::setCurrentPage(int page)
{
    page = std::clamp(page, 0, 5);
    m_page = page;
    m_root->content->pageTitle()->setText(tr(kPageTitles[page]));
    m_root->processesButton->setSelected(page == 0);
    int index = 1;
    for (SummaryCard* card : {m_root->cpu, m_root->memory, m_root->network, m_root->disk, m_root->gpu})
        card->setSelected(page == index++);
    m_processes->setVisible(page == 0);
    // As the Qt version's Loader: made when a resource page opens, gone on the process list.
    if (page == 0 && m_resource) {
        m_root->pages->takeChild(m_resource);
        m_resource = nullptr;
    } else if (page > 0) {
        if (!m_resource)
            m_resource = m_root->pages->addChild<ResourcePage>(m_monitor, page);
        else
            m_resource->setPage(page);
    }
    m_root->requestLayout();
}

void MainWindow::activate()
{
    m_window.show();
    m_window.activate();
}

const ProcessRecord* MainWindow::selectedProcess() const
{
    return m_selection.application ? m_monitor.application(m_selection.pid, m_selection.startTicks)
                                   : m_monitor.process(m_selection.pid, m_selection.startTicks);
}

void MainWindow::openDetails()
{
    if (!selectedProcess())
        return;
    m_details = std::make_unique<ProcessDetails>(&m_window, m_monitor, m_selection);
    m_details->onClosed = [this] { m_details.reset(); };
    m_details->show();
}

void MainWindow::confirmEnd(bool force)
{
    const ProcessRecord* process = selectedProcess();
    if (!process || !Monitor::canManage(*process))
        return;
    const bool application = m_selection.application;
    const std::string name = process->displayName;
    const std::string title = format(force ? tr("Force quit {name}?") : tr("Quit {name}?"), {{"name", name}});
    const char* text = application ? (force ? tr("All of its processes will stop immediately. Unsaved work may be lost.")
                                            : tr("The application will be asked to quit. Unsaved work may be lost."))
                                   : (force ? tr("The process will stop immediately. Unsaved work may be lost.")
                                            : tr("The process will be asked to quit. Unsaved work may be lost."));
    m_confirm = std::make_unique<ConfirmDialog>(&m_window, title, text);
    m_confirm->setConfirmText(force ? tr("Force Quit") : tr("Quit"));
    m_confirm->setDestructive(true);
    m_confirm->onAccepted = [this, target = m_selection, force] {
        const std::string error = target.application ? m_monitor.endApplication(target.pid, target.startTicks, force)
                                                     : m_monitor.endProcess(target.pid, target.startTicks, force);
        if (!error.empty())
            m_toast.show(error);
    };
    m_confirm->show();
}

void MainWindow::updateContextMenu()
{
    const ProcessRecord* process = selectedProcess();
    const bool manage = process && Monitor::canManage(*process);
    m_contextMenu.setItemEnabled(0, process != nullptr);
    m_contextMenu.setItemEnabled(3, manage);
    m_contextMenu.setItemEnabled(4, manage);
}

void MainWindow::update()
{
    const Metrics& metrics = m_monitor.metrics();
    const History& history = m_monitor.history();
    const bool ready = m_monitor.ready();
    const int64_t duration = int64_t(m_monitor.historyDuration()) * 1000;
    m_root->processesButton->setCount(ready ? std::to_string(metrics.processCount) : std::string());

    const auto set = [&](SummaryCard* card, std::string value, std::string subtitle, std::vector<double> values) {
        card->setValue(ready ? std::move(value) : "—");
        card->setSubtitle(ready ? std::move(subtitle) : tr("Collecting data…"));
        card->setHistory(std::move(values), history.timestamps, duration);
    };
    set(m_root->cpu, Monitor::formatPercent(metrics.cpuPercent),
        trn("{n} logical processor", "{n} logical processors", metrics.cores.size()), history.cpuPercent);
    set(m_root->memory, Monitor::formatPercent(metrics.memory.percent),
        Monitor::formatBytes(double(metrics.memory.used)) + " / " + Monitor::formatBytes(double(metrics.memory.total)),
        history.memoryPercent);
    set(m_root->network, "↓ " + Monitor::formatRate(metrics.download), "↑ " + Monitor::formatRate(metrics.upload),
        sumSeries(history.download, history.upload));
    set(m_root->disk, Monitor::formatRate(metrics.diskRead), format(tr("Write {rate}"), {{"rate", Monitor::formatRate(metrics.diskWrite)}}),
        sumSeries(history.diskRead, history.diskWrite));
    m_root->gpu->setVisible(metrics.gpuAvailable);
    set(m_root->gpu,
        metrics.gpuHasBusy ? Monitor::formatPercent(metrics.gpuPercent) : Monitor::formatTemperature(metrics.gpuTemperature),
        metrics.gpuHasBusy && metrics.gpuHasTemperature ? Monitor::formatTemperature(metrics.gpuTemperature)
                                                        : metrics.gpuName,
        metrics.gpuHasBusy ? history.gpuPercent : history.gpuTemperature);

    m_root->error->setText(m_monitor.error());
    m_root->error->setVisible(!m_monitor.error().empty());
    m_root->requestLayout();
}
