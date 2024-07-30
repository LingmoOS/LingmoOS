/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Packet.h"
#include "TimeStamps.h"

class Capture;
class ConnectionMapping;
class Packet;

class Accumulator
{
public:
    using InboundOutboundData = std::pair<int, int>;
    using PidDataCounterHash = std::unordered_map<int, InboundOutboundData>;

    Accumulator(std::shared_ptr<Capture> capture, std::shared_ptr<ConnectionMapping> mapping);
    ~Accumulator();

    PidDataCounterHash data();

private:
    void addData(Packet::Direction direction, const Packet &packet, int pid);
    void loop();

    std::shared_ptr<Capture> m_capture;
    std::shared_ptr<ConnectionMapping> m_mapping;

    std::thread m_thread;
    std::atomic_bool m_running;
    std::mutex m_mutex;

    PidDataCounterHash m_data;
};

#endif // ACCUMULATOR_H
