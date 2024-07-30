/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CAPTURE_H
#define CAPTURE_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "Packet.h"

class pcap;

class Capture
{
public:
    Capture(const std::string &interface = std::string{});
    ~Capture();

    bool start();
    std::string lastError() const;
    void reportStatistics();
    Packet nextPacket();

    void handlePacket(const struct pcap_pkthdr *header, const uint8_t *data);

private:
    void loop();
    bool checkError(int result);

    std::string m_interface;
    std::string m_error;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    std::deque<Packet> m_queue;

    int m_packetCount = 0;
    int m_droppedPackets = 0;

    pcap *m_pcap = nullptr;
};

#endif // CAPTURE_H
