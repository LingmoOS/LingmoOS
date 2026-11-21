// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/networkspeedwidget.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

#include "base/i18n.h"
#include "base/locale.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr double kSparkLineWidth   = 1.5;
constexpr double kBarHeight        = 5.0;
constexpr int    kHistoryLength    = 20;

} // namespace

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

NetworkSpeedWidget::NetworkSpeedWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::NetworkSpeed, size)
{
    setName("network-speed-widget");

    m_downSpeed = addChild<ui::Label>();
    m_downSpeed->setWeight(300);
    m_downSpeed->setElide(ui::Label::Elide::Right);

    m_upSpeed = addChild<ui::Label>();
    m_upSpeed->setWeight(300);
    m_upSpeed->setElide(ui::Label::Elide::Right);

    m_downLabel = addChild<ui::Label>();
    m_downLabel->setElide(ui::Label::Elide::Right);

    m_upLabel = addChild<ui::Label>();
    m_upLabel->setElide(ui::Label::Elide::Right);

    m_combined = addChild<ui::Label>();
    m_combined->setWeight(300);
    m_combined->setElide(ui::Label::Elide::Right);

    const ui::Theme& theme = widgetTheme(this);
    const ui::Color sub = theme.text.withAlpha(0.62);
    m_downLabel->setColor(ui::ColorRecipe(sub));
    m_upLabel->setColor(ui::ColorRecipe(sub));

    m_timer.setInterval(std::chrono::seconds(1));
    m_timer.setSingleShot(false);
    m_timer.onTimeout = [this] { sample(); };
    m_timer.start();
    sample();
}

NetworkSpeedWidget::~NetworkSpeedWidget()
{
    m_timer.stop();
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

std::pair<uint64_t, uint64_t> NetworkSpeedWidget::readProcNet()
{
    std::ifstream file("/proc/net/dev");
    if (!file.is_open())
        return {0, 0};

    std::string line;
    // Skip the two header lines.
    std::getline(file, line);
    std::getline(file, line);

    uint64_t totalRx = 0;
    uint64_t totalTx = 0;

    while (std::getline(file, line)) {
        // Skip leading spaces and find the interface name.
        const char* p = line.c_str();
        while (*p == ' ')
            ++p;
        // Skip the loopback interface.
        if (std::strncmp(p, "lo ", 3) == 0 || std::strncmp(p, "lo:", 3) == 0)
            continue;

        // Advance past the interface name (ends at ':').
        const char* colon = std::strchr(p, ':');
        if (!colon)
            continue;

        // Parse the 16 columns after the colon.
        // col[0] = rx bytes, col[8] = tx bytes.
        uint64_t cols[16] = {};
        std::istringstream ss(std::string(colon + 1));
        for (int i = 0; i < 16; ++i)
            ss >> cols[i];

        totalRx += cols[0];
        totalTx += cols[8];
    }

    return {totalRx, totalTx};
}

std::string NetworkSpeedWidget::formatSpeed(double bytesPerSec)
{
    if (bytesPerSec < 1024.0) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f B/s", bytesPerSec);
        return buf;
    }
    if (bytesPerSec < 1024.0 * 1024.0) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.0f KB/s", bytesPerSec / 1024.0);
        return buf;
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f MB/s", bytesPerSec / (1024.0 * 1024.0));
    return buf;
}

// ---------------------------------------------------------------------------
// Sampling
// ---------------------------------------------------------------------------

void NetworkSpeedWidget::sample()
{
    const auto [rx, tx] = readProcNet();
    const auto now=std::chrono::steady_clock::now();

    if (m_firstSample) {
        m_prevRx = rx;
        m_prevTx = tx;
        m_firstSample = false;
    } else {
        const uint64_t deltaRx = rx >= m_prevRx ? rx - m_prevRx : 0;
        const uint64_t deltaTx = tx >= m_prevTx ? tx - m_prevTx : 0;
        const double elapsed=std::max(0.001,std::chrono::duration<double>(now-m_lastSample).count());
        m_rxSpeed = double(deltaRx)/elapsed;
        m_txSpeed = double(deltaTx)/elapsed;
        m_prevRx  = rx;
        m_prevTx  = tx;

        m_history.push_back(m_rxSpeed + m_txSpeed);
        if (int(m_history.size()) > kHistoryLength)
            m_history.pop_front();  // O(1) with deque
    }

    m_lastSample=now;
    m_combined->setText(formatSpeed(m_rxSpeed + m_txSpeed));
    m_downSpeed->setText(formatSpeed(m_rxSpeed));
    m_upSpeed->setText(formatSpeed(m_txSpeed));
    if (size() == WidgetSize::Small) {
        m_combined->setText(formatSpeed(m_rxSpeed + m_txSpeed));
    } else {
        m_downSpeed->setText(formatSpeed(m_rxSpeed));
        m_upSpeed->setText(formatSpeed(m_txSpeed));
    }

    requestLayout();
    requestPaint();
}

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

void NetworkSpeedWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;

    const ui::Theme& theme = widgetTheme(this);
    const ui::Color sub = theme.text.withAlpha(0.62);

    if (size() == WidgetSize::Small) {
        // "Network" subtitle above a large combined speed label.
        m_downLabel->setFontSize(11);
        m_downLabel->setColor(ui::ColorRecipe(sub));
        m_downLabel->setText(tr("Network"));
        m_downLabel->setTextAlignment(ui::Label::TextAlignment::Center);

        m_combined->setFontSize(26);
        m_combined->setWeight(300);
        m_combined->setTextAlignment(ui::Label::TextAlignment::Center);

        const double labelH   = std::max(14.0, m_downLabel->sizeHint().height);
        const double speedH   = std::max(30.0, m_combined->sizeHint().height);
        const double totalH   = labelH + 4 + speedH;
        const double topY     = content.y + (content.height - totalH) / 2.0;

        m_downLabel->setGeometry({content.x, topY, content.width, labelH});
        m_combined->setGeometry({content.x, topY + labelH + 4, content.width, speedH});

        // Hide the per-direction labels and speeds.
        m_downSpeed->setGeometry({});
        m_upSpeed->setGeometry({});
        m_upLabel->setGeometry({});

        m_sparkRect = {};
        return;
    }

    // Medium and Large share the speed-section layout.
    // For Large the speed section occupies the top 60 %; the sparkline the bottom 40 %.
    const double speedSectionH = size() == WidgetSize::Large
            ? std::floor(content.height * 0.60)
            : content.height;
    const double halfW    = std::floor(content.width / 2.0) - 8;
    const double rightX   = content.x + halfW + 16;

    m_downSpeed->setFontSize(20);
    m_downSpeed->setWeight(300);
    m_upSpeed->setFontSize(20);
    m_upSpeed->setWeight(300);

    m_downLabel->setFontSize(10);
    m_downLabel->setColor(ui::ColorRecipe(sub));
    m_downLabel->setText(tr("Download"));
    m_downLabel->setTextAlignment(ui::Label::TextAlignment::Left);

    m_upLabel->setFontSize(10);
    m_upLabel->setColor(ui::ColorRecipe(sub));
    m_upLabel->setText(tr("Upload"));
    m_upLabel->setTextAlignment(ui::Label::TextAlignment::Left);

    const double labelH = std::max(13.0, m_downLabel->sizeHint().height);
    const double speedH = std::max(24.0, m_downSpeed->sizeHint().height);
    const double blockH = speedH + 4 + labelH;
    const double blockY = content.y + (speedSectionH - blockH) / 2.0;

    // Left: download
    m_downSpeed->setGeometry({content.x, blockY, halfW, speedH});
    m_downLabel->setGeometry({content.x, blockY + speedH + 4, halfW, labelH});

    // Right: upload
    m_upSpeed->setGeometry({rightX, blockY, halfW, speedH});
    m_upLabel->setGeometry({rightX, blockY + speedH + 4, halfW, labelH});

    // Combined label unused in Medium/Large.
    m_combined->setGeometry({});

    // Sparkline rect.
    if (size() == WidgetSize::Large) {
        const double sparkY = content.y + speedSectionH + 6;
        m_sparkRect = {content.x, sparkY, content.width, content.bottom() - sparkY};
    } else {
        // Medium: a thin strip at the bottom.
        constexpr double kStripH = 36;
        m_sparkRect = {content.x, content.bottom() - kStripH, content.width, kStripH};
    }
}

// ---------------------------------------------------------------------------
// Painting
// ---------------------------------------------------------------------------

void NetworkSpeedWidget::paintContent(ui::Painter& painter)
{
    const ui::Theme& theme = widgetTheme(this);

    if (size() == WidgetSize::Small) {
        // Draw a small ↓↑ icon in the upper portion of the card.
        const ui::RectF content = contentRect();
        const double cx   = content.x + content.width / 2.0;
        const double iconY = content.y + 8;
        const double sz   = 10.0;
        const ui::Color ink = theme.text.withAlpha(0.55);

        // Down arrow (↓): centred slightly left
        {
            const double ax = cx - sz * 0.65;
            ui::Path p;
            p.moveTo(ax,          iconY);
            p.lineTo(ax,          iconY + sz * 1.2);
            p.moveTo(ax - sz * 0.4, iconY + sz * 0.75);
            p.lineTo(ax,          iconY + sz * 1.2);
            p.lineTo(ax + sz * 0.4, iconY + sz * 0.75);
            painter.strokePath(p, ink, 1.5, ui::LineCap::Round, ui::LineJoin::Round);
        }
        // Up arrow (↑): centred slightly right
        {
            const double ax = cx + sz * 0.65;
            ui::Path p;
            p.moveTo(ax,          iconY + sz * 1.2);
            p.lineTo(ax,          iconY);
            p.moveTo(ax - sz * 0.4, iconY + sz * 0.45);
            p.lineTo(ax,          iconY);
            p.lineTo(ax + sz * 0.4, iconY + sz * 0.45);
            painter.strokePath(p, ink, 1.5, ui::LineCap::Round, ui::LineJoin::Round);
        }
        return;
    }

    // Medium / Large: mini speed bars + sparkline.
    const ui::Color rxColor = theme.highlight;
    const ui::Color txColor = theme.highlight.withAlpha(0.59); // ~150/255
    const ui::Color track   = theme.text.withAlpha(0.12);

    // Find max for bar scaling — use the greater of rx and tx independently.
    const double maxSpeed = std::max({m_rxSpeed, m_txSpeed, 1.0});

    // Draw bars above the sparkline area or just above the label rows.
    // Place them just below the speed values, before the label row.
    const ui::RectF content = contentRect();
    const double speedSectionH = size() == WidgetSize::Large
            ? std::floor(content.height * 0.60)
            : content.height;
    const double halfW  = std::floor(content.width / 2.0) - 8;
    const double rightX = content.x + halfW + 16;

    // Bar Y: just above the label text rows (label sits at blockY + speedH + 4).
    // Re-derive blockY for bar placement.
    const double labelH = kBarHeight + 2; // small
    const double speedH = 24.0;
    const double blockH = speedH + 4 + labelH;
    const double blockY = content.y + (speedSectionH - blockH) / 2.0;
    const double barY   = blockY + speedH + 4;

    // Download bar (track + filled).
    const ui::RectF rxTrack = {content.x, barY, halfW, kBarHeight};
    painter.fillRoundedRect(rxTrack, kBarHeight / 2, track);
    const double rxFrac = std::clamp(m_rxSpeed / maxSpeed, 0.0, 1.0);
    if (rxFrac > 0) {
        ui::RectF rxFill = rxTrack;
        rxFill.width = rxTrack.width * rxFrac;
        painter.fillRoundedRect(rxFill, kBarHeight / 2, rxColor);
    }

    // Upload bar.
    const ui::RectF txTrack = {rightX, barY, halfW, kBarHeight};
    painter.fillRoundedRect(txTrack, kBarHeight / 2, track);
    const double txFrac = std::clamp(m_txSpeed / maxSpeed, 0.0, 1.0);
    if (txFrac > 0) {
        ui::RectF txFill = txTrack;
        txFill.width = txTrack.width * txFrac;
        painter.fillRoundedRect(txFill, kBarHeight / 2, txColor);
    }

    // Sparkline.
    if (m_sparkRect.width <= 0 || m_sparkRect.height <= 0 || m_history.size() < 2)
        return;

    const double maxH = *std::max_element(m_history.begin(), m_history.end());
    if (maxH <= 0)
        return;

    const double stepX = m_sparkRect.width / double(kHistoryLength - 1);
    const int    count = int(m_history.size());
    const int    startIdx = kHistoryLength - count; // offset if history not yet full

    // Faint background strip for sparkline area.
    painter.fillRoundedRect(m_sparkRect, 4, theme.text.withAlpha(0.05));

    ui::Path spark;
    bool first = true;
    for (int i = 0; i < count; ++i) {
        const double val = m_history[std::size_t(i)];
        const double nx  = m_sparkRect.x + (startIdx + i) * stepX;
        const double ny  = m_sparkRect.bottom()
                - std::clamp(val / maxH, 0.0, 1.0) * (m_sparkRect.height - 4) - 2;
        if (first) {
            spark.moveTo(nx, ny);
            first = false;
        } else {
            spark.lineTo(nx, ny);
        }
    }
    painter.strokePath(spark, rxColor.withAlpha(0.78), kSparkLineWidth,
                       ui::LineCap::Round, ui::LineJoin::Round);
}

// ---------------------------------------------------------------------------
// Retranslation
// ---------------------------------------------------------------------------

void NetworkSpeedWidget::retranslateContent()
{
    const ui::Theme& theme = widgetTheme(this);
    const ui::Color sub = theme.text.withAlpha(0.62);
    m_downLabel->setColor(ui::ColorRecipe(sub));
    m_upLabel->setColor(ui::ColorRecipe(sub));

    if (size() == WidgetSize::Small)
        m_downLabel->setText(tr("Network"));
    else {
        m_downLabel->setText(tr("Download"));
        m_upLabel->setText(tr("Upload"));
    }
    requestLayout();
}

} // namespace fm
