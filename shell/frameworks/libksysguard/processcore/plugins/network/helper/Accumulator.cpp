/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "Accumulator.h"

#include "Capture.h"
#include "ConnectionMapping.h"

using namespace std::chrono_literals;

Accumulator::Accumulator(std::shared_ptr<Capture> capture, std::shared_ptr<ConnectionMapping> mapping)
    : m_capture(capture)
    , m_mapping(mapping)
    , m_running(true)
{
    m_thread = std::thread{&Accumulator::loop, this};
}

Accumulator::~Accumulator()
{
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

Accumulator::PidDataCounterHash Accumulator::data()
{
    std::lock_guard<std::mutex> lock{m_mutex};

    auto tmp = m_data;

    auto toErase = std::vector<int>{};
    for (auto &entry : m_data) {
        if (entry.second.first == 0 && entry.second.second == 0) {
            toErase.push_back(entry.first);
        } else {
            entry.second.first = 0;
            entry.second.second = 0;
        }
    }

    std::for_each(toErase.cbegin(), toErase.cend(), [this](int pid) {
        m_data.erase(pid);
    });

    return tmp;
}

void Accumulator::loop()
{
    while (m_running) {
        auto packet = m_capture->nextPacket();

        auto result = m_mapping->pidForPacket(packet);
        if (result.pid == 0) {
            continue;
        }

        addData(result.direction, packet, result.pid);
    }
}

void Accumulator::addData(Packet::Direction direction, const Packet &packet, int pid)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    auto itr = m_data.find(pid);
    if (itr == m_data.end()) {
        m_data.emplace(pid, InboundOutboundData{0, 0});
    }

    if (direction == Packet::Direction::Inbound) {
        m_data[pid].first += packet.size();
    } else {
        m_data[pid].second += packet.size();
    };
}
