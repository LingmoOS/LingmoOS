// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/systemwidget.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <sstream>

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr double kBarHeight = 6;

struct CpuSample
{
    uint64_t total = 0;
    uint64_t idle = 0;
};

CpuSample readCpu()
{
    CpuSample sample;
    std::ifstream file("/proc/stat");
    std::string line;
    if (!std::getline(file, line))
        return sample;
    std::istringstream stream(line);
    std::string label;
    uint64_t value = 0;
    stream >> label;
    while (stream >> value)
        sample.total += value;
    // The fourth field is idle, the fifth iowait: neither is busy time.
    std::istringstream again(line);
    again >> label;
    uint64_t user = 0, nice = 0, system = 0, idle = 0, iowait = 0;
    again >> user >> nice >> system >> idle >> iowait;
    sample.idle = idle + iowait;
    return sample;
}

double readMemory()
{
    std::ifstream file("/proc/meminfo");
    std::string line;
    uint64_t total = 0;
    uint64_t available = 0;
    while (std::getline(file, line)) {
        const std::string key = line.substr(0, line.find(':'));
        std::istringstream stream(line.substr(line.find(':') + 1));
        uint64_t kilobytes = 0;
        stream >> kilobytes;
        if (key == "MemTotal")
            total = kilobytes;
        else if (key == "MemAvailable")
            available = kilobytes;
    }
    if (total == 0)
        return 0;
    return std::clamp(1.0 - double(available) / double(total), 0.0, 1.0);
}

std::string percentText(const base::Locale& locale, double value)
{
    return locale.toString(std::round(value * 100.0), 0) + "%";
}

} // namespace

SystemWidget::SystemWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::System, size)
{
    setName("system-widget");
    m_cpu = addChild<ui::Label>();
    m_memory = addChild<ui::Label>();
    m_cpu->setWeight(600);
    m_memory->setWeight(600);

    m_timer.setInterval(std::chrono::seconds(1));
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { sample(); };
    m_timer.start();
    sample();
}

SystemWidget::~SystemWidget() = default;

void SystemWidget::sample()
{
    const CpuSample cpu = readCpu();
    if (m_previousTotal > 0 && cpu.total > m_previousTotal) {
        const double total = double(cpu.total - m_previousTotal);
        const double idle = double(cpu.idle - m_previousIdle);
        m_cpuUsage = std::clamp(1.0 - idle / total, 0.0, 1.0);
    }
    m_previousTotal = cpu.total;
    m_previousIdle = cpu.idle;
    m_memoryUsage = readMemory();

    const base::Locale& locale = base::Locale::system();
    m_cpu->setText(std::string(tr("CPU")) + " " + percentText(locale, m_cpuUsage));
    m_memory->setText(std::string(tr("Memory")) + " " + percentText(locale, m_memoryUsage));
    requestLayout();
    requestPaint();
}

void SystemWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    const double labelHeight = size()==WidgetSize::Small ? 24 : 28;
    const double rowHeight = labelHeight + 4 + kBarHeight;
    const double gap = std::max(14.0, content.height * 0.18);
    m_cpu->setFontSize(size() == WidgetSize::Small ? 14 : 18);
    m_cpu->setWeight(600);
    m_memory->setFontSize(size() == WidgetSize::Small ? 14 : 18);
    m_memory->setWeight(600);
    m_cpu->setGeometry({content.x, content.y, content.width, labelHeight});
    m_cpuBar = {content.x, content.y + labelHeight + 4, content.width, kBarHeight};
    const double memY = content.y + rowHeight + gap;
    m_memory->setGeometry({content.x, memY, content.width,
                           std::max(0.0, content.bottom() - memY - kBarHeight - 4)});
    m_memoryBar = {content.x, std::min(memY + labelHeight + 4, content.bottom() - kBarHeight),
                   content.width, kBarHeight};
}

void SystemWidget::paintContent(ui::Painter& painter)
{
    const ui::Theme& theme = widgetTheme(this);
    const ui::Color track = theme.text.withAlpha(0.14);
    const ui::Color fill = theme.highlight;
    const auto bar = [&painter, &track, &fill](const ui::RectF& rect, double value) {
        if (rect.width <= 0)
            return;
        painter.fillRoundedRect(rect, rect.height / 2, track);
        ui::RectF filled = rect;
        filled.width = std::max(0.0, rect.width * std::clamp(value, 0.0, 1.0));
        if (filled.width > 0)
            painter.fillRoundedRect(filled, rect.height / 2, fill);
    };
    bar(m_cpuBar, m_cpuUsage);
    bar(m_memoryBar, m_memoryUsage);
}

} // namespace fm
