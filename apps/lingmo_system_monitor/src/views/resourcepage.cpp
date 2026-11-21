// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resourcepage.h"

#include "chartcard.h"
#include "components.h"
#include "historygraph.h"
#include "monitor.h"

#include "base/i18n.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;
using Lingmo::i18n::trNoop;

static constexpr double kTileWidth = 150;
static constexpr double kTileSpacing = 12;

// A logical processor's share of time, in the grid under the chart.
struct ResourcePage::Tile : public Plate
{
    Tile()
        : Plate(Theme::current().smallRadius)
        , name(addLabel(this, {}, 8))
        , value(addLabel(this, {}, 8))
        , bar(addChild<ProgressBar>())
    {
        setContinuousCorners(std::nullopt);
        setLayoutHints({.fillWidth = true, .preferredHeight = 66});
    }

    SizeF sizeHint() const override { return {kTileWidth, 66}; }

    void layout() override
    {
        const double width = geometry().width;
        const SizeF valueSize = value->sizeHint();
        name->setGeometry({12, 12, std::max(0.0, width - 24 - valueSize.width), name->sizeHint().height});
        value->setGeometry({width - 12 - valueSize.width, 12, valueSize.width, valueSize.height});
        bar->setGeometry({12, geometry().height - 12 - 6, std::max(0.0, width - 24), 6});
    }

    Label* name;
    Label* value;
    ProgressBar* bar;
};

ResourcePage::ResourcePage(Monitor& monitor, int page)
    : m_monitor(monitor)
    , m_page(page)
    , m_content(setContent<Column>())
    , m_layout(m_content->addChild<ColumnLayout>())
{
    setName("resourcePage");
    m_content->setMargins({12, 8, 12, 24});
    m_layout->setLayoutHints({.fillWidth = true});
    m_layout->setSpacing(20);
    m_updated = m_monitor.updated.connect([this] { update(); });
    rebuild();
}

void ResourcePage::setPage(int page)
{
    if (page == m_page)
        return;
    m_page = page;
    rebuild();
}

Label* ResourcePage::addHeading(const char* text)
{
    // FishUI's leftPadding: the text starts at the cards' text edge.
    auto* row = m_layout->addChild<Row>();
    row->setMargins({16, 0, 0, 0});
    row->setLayoutHints({.fillWidth = true});
    return addLabel(row, text, 9, 500);
}

ResourcePage::Section ResourcePage::addSection(View* parent, size_t rows)
{
    Section section;
    section.column = parent->addChild<ColumnLayout>();
    section.column->setSpacing(8);
    section.column->setLayoutHints({.fillWidth = true});
    section.title = section.column->addChild<SectionLabel>();
    section.title->setLayoutHints({.fillWidth = true});
    section.card = section.column->addChild<ListCard>();
    section.card->setLayoutHints({.fillWidth = true});
    for (size_t i = 0; i < rows; ++i)
        section.rows.push_back(section.card->addRow({}));
    return section;
}

void ResourcePage::rebuild()
{
    while (!m_layout->children().empty())
        m_layout->takeChild(m_layout->children().back().get());
    m_cores = nullptr;
    m_tiles.clear();
    m_card = nullptr;
    m_sections.clear();
    m_volumes.clear();

    m_chart = m_layout->addChild<ChartCard>();
    m_chart->setName("resourceChart");
    m_chart->setLayoutHints({.fillWidth = true, .preferredHeight = 276});

    const Metrics& metrics = m_monitor.metrics();
    switch (m_page) {
    case 1:
        addHeading(tr("Logical processors"));
        m_cores = m_layout->addChild<GridLayout>(2);
        m_cores->setColumnSpacing(kTileSpacing);
        m_cores->setRowSpacing(kTileSpacing);
        m_cores->setLayoutHints({.fillWidth = true});
        for (size_t i = 0; i < metrics.cores.size(); ++i)
            m_tiles.push_back(m_cores->addChild<Tile>());
        m_built[0] = metrics.cores.size();
        break;
    case 2:
        m_card = m_layout->addChild<ListCard>();
        m_card->setLayoutHints({.fillWidth = true});
        for (const char* text : {trNoop("In use"), trNoop("Available"), trNoop("Cached"), trNoop("Total memory"), trNoop("Swap")})
            m_card->addRow(tr(text));
        break;
    case 3:
        m_card = m_layout->addChild<ListCard>();
        m_card->setLayoutHints({.fillWidth = true});
        for (const char* text : {trNoop("Download"), trNoop("Upload"), trNoop("Total received"), trNoop("Total sent")})
            m_card->addRow(tr(text));
        addHeading(tr("Network interfaces"));
        for (size_t i = 0; i < metrics.interfaces.size(); ++i) {
            Section section = addSection(m_layout, 4);
            section.rows[0]->setText(tr("State"));
            section.rows[1]->setText(tr("Download"));
            section.rows[2]->setText(tr("Upload"));
            section.rows[3]->setText(tr("Hardware address"));
            m_sections.push_back(section);
        }
        m_built[0] = metrics.interfaces.size();
        break;
    case 4:
        for (size_t i = 0; i < metrics.disks.size(); ++i) {
            Section section = addSection(m_layout, 4);
            section.rows[0]->setText(tr("Device"));
            section.rows[1]->setText(tr("Capacity"));
            section.rows[2]->setText(tr("Read"));
            section.rows[3]->setText(tr("Write"));
            m_sections.push_back(section);
        }
        addHeading(tr("Storage"));
        for (size_t i = 0; i < metrics.volumes.size(); ++i) {
            Section section;
            section.column = m_layout->addChild<ColumnLayout>();
            section.column->setSpacing(8);
            section.column->setLayoutHints({.fillWidth = true});
            section.title = section.column->addChild<SectionLabel>();
            section.title->setLayoutHints({.fillWidth = true});
            section.title->setElide(Label::Elide::Middle);
            section.card = section.column->addChild<ListCard>();
            section.card->setLayoutHints({.fillWidth = true});
            // The usage bar sits 16 below the card's top, inset as the rows' text.
            auto* bar = section.card->addChild<Row>();
            bar->setMargins({16, 16, 16, 0});
            bar->setLayoutHints({.preferredHeight = 22});
            section.usage = bar->addChild<ProgressBar>();
            section.usage->setLayoutHints({.fillWidth = true});
            for (const char* text : {trNoop("Used"), trNoop("Available"), trNoop("File system")})
                section.rows.push_back(section.card->addRow(tr(text)));
            m_volumes.push_back(section);
        }
        m_built[0] = metrics.disks.size();
        m_built[1] = metrics.volumes.size();
        break;
    case 5:
        for (size_t i = 0; i < metrics.gpus.size(); ++i) {
            Section section = addSection(m_layout, 6);
            const char* texts[] = {trNoop("Driver"), trNoop("State"), trNoop("Activity"),
                                   trNoop("Temperature"), trNoop("Clock speed"), trNoop("Video memory")};
            for (size_t row = 0; row < 6; ++row)
                section.rows[row]->setText(tr(texts[row]));
            m_sections.push_back(section);
        }
        m_built[0] = metrics.gpus.size();
        break;
    default:
        break;
    }
    update();
}

void ResourcePage::updateChart()
{
    const Metrics& metrics = m_monitor.metrics();
    const History& history = m_monitor.history();
    const bool temperature = m_page == 5 && !metrics.gpuHasBusy;
    static const char* const titles[] = {"", trNoop("Processor activity"), trNoop("Memory usage"), trNoop("Network activity"),
                                          trNoop("Disk activity"), trNoop("Graphics activity")};
    m_chart->setTitle(temperature ? tr("Graphics temperature") : tr(titles[m_page]));
    std::string value;
    std::string subtitle;
    std::vector<double> primary;
    std::vector<double> secondary;
    switch (m_page) {
    case 1:
        value = Monitor::formatPercent(metrics.cpuPercent);
        subtitle = metrics.cpuName;
        primary = history.cpuPercent;
        break;
    case 2:
        value = Monitor::formatPercent(metrics.memory.percent);
        subtitle = format(tr("{used} of {total} in use"), {{"used", Monitor::formatBytes(double(metrics.memory.used))},
                                                          {"total", Monitor::formatBytes(double(metrics.memory.total))}});
        primary = history.memoryPercent;
        break;
    case 3:
        value = Monitor::formatRate(metrics.download);
        subtitle = tr("Physical network interfaces");
        primary = history.download;
        secondary = history.upload;
        break;
    case 4:
        value = Monitor::formatRate(metrics.diskRead);
        subtitle = tr("Physical disks");
        primary = history.diskRead;
        secondary = history.diskWrite;
        break;
    default:
        value = temperature ? Monitor::formatTemperature(metrics.gpuTemperature)
                            : Monitor::formatPercent(metrics.gpuPercent);
        subtitle = metrics.gpuName;
        primary = temperature ? history.gpuTemperature : history.gpuPercent;
        break;
    }
    static const Color colors[] = {accent::blue, accent::blue, accent::purple, accent::green, accent::orange,
                                   accent::pink};
    m_chart->setColors(colors[m_page], m_page == 3 ? accent::blue : accent::purple);
    m_chart->setValue(value);
    m_chart->setSubtitle(subtitle);
    m_chart->setMaximum(m_page <= 2 || m_page == 5 ? 100 : 0);
    m_chart->setScaleLabel(temperature ? Monitor::formatTemperature(100) : std::string());
    m_chart->setLegends(m_page == 3 ? tr("Download") : m_page == 4 ? tr("Read") : "",
                        m_page == 3 ? tr("Upload") : m_page == 4 ? tr("Write") : "");
    m_chart->setDurationText(trn("{n} second", "{n} seconds", m_monitor.historyDuration()));
    m_chart->setSeries(std::move(primary), std::move(secondary), history.timestamps,
                       int64_t(m_monitor.historyDuration()) * 1000);
}

void ResourcePage::update()
{
    const Metrics& metrics = m_monitor.metrics();
    const bool changed = (m_page == 1 && metrics.cores.size() != m_built[0])
                         || (m_page == 3 && metrics.interfaces.size() != m_built[0])
                         || (m_page == 4 && (metrics.disks.size() != m_built[0] || metrics.volumes.size() != m_built[1]))
                         || (m_page == 5 && metrics.gpus.size() != m_built[0]);
    if (changed) {
        rebuild();
        return;
    }

    updateChart();
    switch (m_page) {
    case 1:
        for (size_t i = 0; i < m_tiles.size(); ++i) {
            const double usage = metrics.cores[i].usage;
            m_tiles[i]->name->setText(format(tr("CPU {number}"), {{"number", std::to_string(metrics.cores[i].number + 1)}}));
            m_tiles[i]->value->setText(Monitor::formatPercent(usage));
            m_tiles[i]->bar->setValue(std::clamp(usage / 100, 0.0, 1.0));
            m_tiles[i]->requestLayout();
        }
        break;
    case 2: {
        const auto& rows = m_card->children();
        static_cast<InfoRow*>(rows[0].get())->setValue(Monitor::formatBytes(double(metrics.memory.used)));
        static_cast<InfoRow*>(rows[1].get())->setValue(Monitor::formatBytes(double(metrics.memory.available)));
        static_cast<InfoRow*>(rows[2].get())->setValue(Monitor::formatBytes(double(metrics.memory.cache)));
        static_cast<InfoRow*>(rows[3].get())->setValue(Monitor::formatBytes(double(metrics.memory.total)));
        static_cast<InfoRow*>(rows[4].get())->setValue(
            metrics.memory.swapTotal > 0 ? Monitor::formatBytes(double(metrics.memory.swapUsed)) + " / "
                                               + Monitor::formatBytes(double(metrics.memory.swapTotal))
                                         : tr("Not configured"));
        break;
    }
    case 3: {
        const auto& rows = m_card->children();
        static_cast<InfoRow*>(rows[0].get())->setValue(Monitor::formatRate(metrics.download));
        static_cast<InfoRow*>(rows[1].get())->setValue(Monitor::formatRate(metrics.upload));
        static_cast<InfoRow*>(rows[2].get())->setValue(Monitor::formatBytes(double(metrics.received)));
        static_cast<InfoRow*>(rows[3].get())->setValue(Monitor::formatBytes(double(metrics.sent)));
        for (size_t i = 0; i < m_sections.size(); ++i) {
            const NetworkInterface& adapter = metrics.interfaces[i];
            m_sections[i].title->setText(adapter.name);
            m_sections[i].rows[0]->setValue(adapter.state == "up"     ? tr("Connected")
                                            : adapter.state == "down" ? tr("Disconnected")
                                                                      : tr("Unknown"));
            m_sections[i].rows[1]->setValue(Monitor::formatRate(adapter.download));
            m_sections[i].rows[2]->setValue(Monitor::formatRate(adapter.upload));
            m_sections[i].rows[3]->setValue(adapter.address.empty() ? "—" : adapter.address);
        }
        break;
    }
    case 4:
        for (size_t i = 0; i < m_sections.size(); ++i) {
            const Disk& disk = metrics.disks[i];
            m_sections[i].title->setText(disk.model.empty() ? disk.name : disk.model);
            m_sections[i].rows[0]->setValue("/dev/" + disk.name);
            m_sections[i].rows[1]->setValue(Monitor::formatBytes(double(disk.size)));
            m_sections[i].rows[2]->setValue(Monitor::formatRate(disk.read));
            m_sections[i].rows[3]->setValue(Monitor::formatRate(disk.write));
        }
        for (size_t i = 0; i < m_volumes.size(); ++i) {
            const Volume& volume = metrics.volumes[i];
            m_volumes[i].title->setText(volume.mount);
            m_volumes[i].usage->setValue(volume.total > 0 ? double(volume.used) / double(volume.total) : 0);
            m_volumes[i].rows[0]->setValue(format(tr("{used} of {total}"), {{"used", Monitor::formatBytes(double(volume.used))},
                                                                            {"total", Monitor::formatBytes(double(volume.total))}}));
            m_volumes[i].rows[1]->setValue(Monitor::formatBytes(double(volume.available)));
            m_volumes[i].rows[2]->setValue(volume.type.empty() ? "—" : volume.type);
        }
        break;
    case 5:
        for (size_t i = 0; i < m_sections.size(); ++i) {
            const Gpu& gpu = metrics.gpus[i];
            Section& section = m_sections[i];
            section.title->setText(gpu.name);
            section.rows[0]->setValue(gpu.driver.empty() ? "—" : gpu.driver);
            section.rows[1]->setValue(gpu.active ? tr("Active") : tr("Powered off"));
            section.rows[2]->setValue(Monitor::formatPercent(gpu.busy));
            section.rows[2]->setVisible(gpu.hasBusy);
            section.rows[3]->setValue(gpu.active && gpu.temperature >= 0 ? Monitor::formatTemperature(gpu.temperature)
                                                                         : "—");
            section.rows[3]->setVisible(gpu.hasTemperature);
            section.rows[4]->setValue(gpu.active ? std::to_string(std::lround(gpu.frequency)) + " MHz" : "—");
            section.rows[4]->setVisible(gpu.hasFrequency);
            section.rows[5]->setValue(gpu.active ? Monitor::formatBytes(double(gpu.vramUsed)) + " / "
                                                       + Monitor::formatBytes(double(gpu.vramTotal))
                                                 : "—");
            section.rows[5]->setVisible(gpu.hasVram);
            section.card->updateSeparators();
        }
        break;
    default:
        break;
    }
}

void ResourcePage::layout()
{
    if (m_cores) {
        const double width = geometry().width - 24;
        m_cores->setColumns(std::max(2, int(std::floor((width + 12) / 162))));
    }
    ScrollView::layout();
}
