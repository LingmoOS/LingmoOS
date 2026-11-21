// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <chrono>
#include <deque>
#include <string>
#include <utility>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
}

namespace fm {

// Live network throughput: download and upload rates sampled from /proc/net/dev
// every second, with a sparkline history for the last 20 samples.
class NetworkSpeedWidget : public DesktopWidget
{
public:
    NetworkSpeedWidget(WidgetLayer& layer, WidgetSize size);
    ~NetworkSpeedWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    void sample();
    static std::string formatSpeed(double bytesPerSec);
    static std::pair<uint64_t, uint64_t> readProcNet();

    ui::Label* m_downSpeed  = nullptr;
    ui::Label* m_upSpeed    = nullptr;
    ui::Label* m_downLabel  = nullptr;
    ui::Label* m_upLabel    = nullptr;
    ui::Label* m_combined   = nullptr;

    base::Timer m_timer;
    uint64_t m_prevRx    = 0;
    uint64_t m_prevTx    = 0;
    bool     m_firstSample = true;
    std::chrono::steady_clock::time_point m_lastSample;
    double   m_rxSpeed   = 0;
    double   m_txSpeed   = 0;
    std::deque<double> m_history;
    ui::RectF m_sparkRect;
};

} // namespace fm
