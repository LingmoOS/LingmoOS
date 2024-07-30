/*
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <atomic>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>

#include <getopt.h>

#include "Accumulator.h"
#include "Capture.h"
#include "ConnectionMapping.h"
#include "Packet.h"
#include "TimeStamps.h"

static std::atomic_bool g_running{false};

int main(int argc, char **argv)
{
    static struct option long_options[] = {
        {"help", 0, nullptr, 'h'},
        {"stats", 0, nullptr, 's'},
        {nullptr, 0, nullptr, 0},
    };

    auto statsRequested = false;
    auto optionIndex = 0;
    auto option = -1;
    while ((option = getopt_long(argc, argv, "", long_options, &optionIndex)) != -1) {
        switch (option) {
        case 's':
            statsRequested = true;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [options]\n";
            std::cerr << "This is a helper application for tracking per-process network usage.\n";
            std::cerr << "\n";
            std::cerr << "Options:\n";
            std::cerr << "  --stats     Print packet capture statistics.\n";
            std::cerr << "  --help      Display this help.\n";
            return 0;
        }
    }

    auto mapping = std::make_shared<ConnectionMapping>();

    auto capture = std::make_shared<Capture>();
    if (!capture->start()) {
        std::cerr << capture->lastError() << std::endl;
        return 1;
    }

    auto accumulator = std::make_shared<Accumulator>(capture, mapping);

    signal(SIGINT, [](int) {
        g_running = false;
    });
    signal(SIGTERM, [](int) {
        g_running = false;
    });

    g_running = true;
    while (g_running) {
        auto data = accumulator->data();
        auto timeStamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        if (statsRequested != 0) {
            capture->reportStatistics();
        }

        if (data.empty()) {
            std::cout << std::put_time(std::localtime(&timeStamp), "%T") << std::endl;
        } else {
            for (auto itr = data.begin(); itr != data.end(); ++itr) {
                std::cout << std::put_time(std::localtime(&timeStamp), "%T");
                std::cout << "|PID|" << (*itr).first << "|IN|" << (*itr).second.first << "|OUT|" << (*itr).second.second;
                std::cout << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
