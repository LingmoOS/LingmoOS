/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "Capture.h"

#include <iostream>
#include <string>

#include <pcap/pcap.h>

#include "TimeStamps.h"

using namespace std::string_literals;

// Limit the amount of entries waiting in the queue to this size, to prevent
// the queue from getting too full.
static const int MaximumQueueSize = 1000;

void pcapDispatchCallback(uint8_t *user, const struct pcap_pkthdr *h, const uint8_t *bytes)
{
    reinterpret_cast<Capture *>(user)->handlePacket(h, bytes);
}

Capture::Capture(const std::string &interface)
    : m_interface(interface)
{
}

Capture::~Capture()
{
    if (m_pcap) {
        pcap_breakloop(m_pcap);
        if (m_thread.joinable()) {
            m_thread.join();
        }
        pcap_close(m_pcap);
    }
}

bool Capture::start()
{
    auto device = m_interface.empty() ? (const char *)nullptr : m_interface.c_str();

    char errorBuffer[PCAP_ERRBUF_SIZE];
    m_pcap = pcap_create(device, errorBuffer);
    if (!m_pcap) {
        m_error = std::string(errorBuffer, PCAP_ERRBUF_SIZE);
        return false;
    }

    pcap_set_timeout(m_pcap, 500);
    pcap_set_snaplen(m_pcap, 100);
    pcap_set_promisc(m_pcap, 0);
    pcap_set_datalink(m_pcap, DLT_LINUX_SLL);

    if (checkError(pcap_activate(m_pcap))) {
        return false;
    }

    struct bpf_program filter;
    if (checkError(pcap_compile(m_pcap, &filter, "tcp or udp", 1, PCAP_NETMASK_UNKNOWN))) {
        return false;
    }

    if (checkError(pcap_setfilter(m_pcap, &filter))) {
        pcap_freecode(&filter);
        return false;
    }

    pcap_freecode(&filter);

    m_thread = std::thread{&Capture::loop, this};

    return true;
}

std::string Capture::lastError() const
{
    return m_error;
}

void Capture::reportStatistics()
{
    pcap_stat stats;
    pcap_stats(m_pcap, &stats);

    std::cout << "Packet Statistics: " << std::endl;
    std::cout << "  " << stats.ps_recv << " received" << std::endl;
    std::cout << "  " << stats.ps_drop << " dropped (full)" << std::endl;
    std::cout << "  " << stats.ps_ifdrop << " dropped (iface)" << std::endl;
    std::cout << "  " << m_packetCount << " processed" << std::endl;
    std::cout << "  " << m_droppedPackets << " dropped (capture)" << std::endl;
}

Packet Capture::nextPacket()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this]() {
        return m_queue.size() > 0;
    });

    auto packet = std::move(m_queue.front());
    m_queue.pop_front();
    return packet;
}

void Capture::loop()
{
    pcap_loop(m_pcap, -1, &pcapDispatchCallback, reinterpret_cast<uint8_t *>(this));
}

bool Capture::checkError(int result)
{
    switch (result) {
    case PCAP_ERROR_ACTIVATED:
        m_error = "The handle has already been activated"s;
        return true;
    case PCAP_ERROR_NO_SUCH_DEVICE:
        m_error = "The capture source specified when the handle was created doesn't exist"s;
        return true;
    case PCAP_ERROR_PERM_DENIED:
        m_error = "The process doesn't have permission to open the capture source"s;
        return true;
    case PCAP_ERROR_PROMISC_PERM_DENIED:
        m_error = "The process has permission to open the capture source but doesn't have permission to put it into promiscuous mode"s;
        return true;
    case PCAP_ERROR_RFMON_NOTSUP:
        m_error = "Monitor mode was specified but the capture source doesn't support monitor mode"s;
        return true;
    case PCAP_ERROR_IFACE_NOT_UP:
        m_error = "The capture source device is not up"s;
        return true;
    case PCAP_ERROR:
        m_error = std::string(pcap_geterr(m_pcap));
        return true;
    }

    return false;
}

void Capture::handlePacket(const struct pcap_pkthdr *header, const uint8_t *data)
{
    auto timeStamp = std::chrono::time_point_cast<TimeStamp::MicroSeconds::duration>(std::chrono::system_clock::from_time_t(header->ts.tv_sec)
                                                                                     + std::chrono::microseconds{header->ts.tv_usec});

    {
        std::lock_guard<std::mutex> lock{m_mutex};

        m_packetCount++;
        if (m_queue.size() < MaximumQueueSize) {
            m_queue.emplace_back(timeStamp, data, header->caplen, header->len);
        } else {
            m_droppedPackets++;
        }
    }

    m_condition.notify_all();
}
