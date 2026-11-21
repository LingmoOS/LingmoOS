// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct MemoryMetrics
{
    uint64_t total = 0;
    uint64_t used = 0;
    uint64_t available = 0;
    uint64_t cache = 0;
    double percent = 0;
    uint64_t swapTotal = 0;
    uint64_t swapUsed = 0;
    double swapPercent = 0;
};

struct NetworkInterface
{
    std::string name;
    std::string state;
    std::string address;
    double download = 0;
    double upload = 0;
    uint64_t received = 0;
    uint64_t sent = 0;
    bool physical = false;
};

struct Disk
{
    std::string name;
    std::string model;
    double read = 0;
    double write = 0;
    uint64_t size = 0;
};

struct Volume
{
    std::string mount;
    std::string device;
    std::string type;
    uint64_t total = 0;
    uint64_t available = 0;
    uint64_t used = 0;
};

struct Gpu
{
    std::string name;
    std::string driver;
    bool active = false;
    bool hasBusy = false;
    int busy = 0;
    bool hasTemperature = false;
    double temperature = -1;
    bool hasFrequency = false;
    double frequency = 0;
    bool hasVram = false;
    uint64_t vramUsed = 0;
    uint64_t vramTotal = 0;
};

struct Processor
{
    // The kernel's number, which skips offline processors.
    int number = 0;
    double usage = 0;
};

struct Metrics
{
    MemoryMetrics memory;

    double cpuPercent = 0;
    std::vector<Processor> cores;
    std::string cpuName;
    int processCount = 0;

    double download = 0;
    double upload = 0;
    uint64_t received = 0;
    uint64_t sent = 0;
    std::vector<NetworkInterface> interfaces;

    double diskRead = 0;
    double diskWrite = 0;
    std::vector<Disk> disks;
    std::vector<Volume> volumes;

    std::vector<Gpu> gpus;
    bool gpuAvailable = false;
    std::string gpuName;
    bool gpuHasBusy = false;
    bool gpuHasTemperature = false;
    double gpuPercent = 0;
    double gpuTemperature = 0;
};
