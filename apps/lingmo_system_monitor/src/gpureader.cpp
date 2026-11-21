// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gpureader.h"
#include "fileutil.h"

#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static bool isDirectory(const std::string& path)
{
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

// Subdirectories, by name.
static std::vector<std::string> subdirectories(const std::string& path)
{
    std::vector<std::string> names;
    for (std::string& name : directoryEntries(path)) {
        if (isDirectory(path + '/' + name))
            names.push_back(std::move(name));
    }
    std::sort(names.begin(), names.end());
    return names;
}

static bool isCard(std::string_view name)
{
    return name.size() > 4 && name.starts_with("card")
           && std::all_of(name.begin() + 4, name.end(), [](char c) { return c >= '0' && c <= '9'; });
}

static std::string lower(std::string_view text)
{
    std::string result(text);
    for (char& c : result) {
        if (c >= 'A' && c <= 'Z')
            c = char(c - 'A' + 'a');
    }
    return result;
}

GpuReader::GpuReader(std::string procRoot, std::string sysRoot)
    : m_procRoot(std::move(procRoot))
    , m_sysRoot(std::move(sysRoot))
    , m_uid(getuid())
{
}

void GpuReader::scan()
{
    m_scanned = true;
    const std::string drm = m_sysRoot + "/class/drm";
    for (const std::string& entry : subdirectories(drm)) {
        if (!isCard(entry))
            continue;

        Card card;
        card.device = drm + '/' + entry + "/device";
        const auto monitors = subdirectories(card.device + "/hwmon");
        if (!monitors.empty())
            card.hwmon = card.device + "/hwmon/" + monitors.front();
        card.hasBusy = fileExists(card.device + "/gpu_busy_percent");
        card.hasTemperature = !card.hwmon.empty() && fileExists(card.hwmon + "/temp1_input");
        card.hasFrequency = !card.hwmon.empty() && fileExists(card.hwmon + "/freq1_input");
        card.hasVram = fileExists(card.device + "/mem_info_vram_total");
        for (const std::string& node : subdirectories(card.device + "/drm")) {
            if (node.starts_with("card") || node.starts_with("renderD"))
                card.nodes.push_back(node);
        }
        if (!card.hasBusy && !card.hasTemperature && card.nodes.empty())
            continue;

        card.primary = trimmed(readFile(card.device + "/boot_vga")) == "1";
        card.driver = validUtf8(fileName(symLinkTarget(card.device + "/driver")));
        card.name = validUtf8(trimmed(readFile(card.device + "/product_name")));
        if (card.name.empty())
            card.name = pciName(trimmed(readFile(card.device + "/vendor")), trimmed(readFile(card.device + "/device")));
        if (card.name.empty())
            card.name = card.driver.empty() ? entry : card.driver;
        m_cards.push_back(std::move(card));
    }
    std::stable_sort(m_cards.begin(), m_cards.end(),
                     [](const Card& left, const Card& right) { return left.primary && !right.primary; });
    for (size_t index = 0; index < m_cards.size(); ++index) {
        if (m_cards[index].hasBusy)
            continue;
        for (const std::string& node : m_cards[index].nodes)
            m_nodeOwners.emplace("/dev/dri/" + node, int(index));
    }
}

std::string GpuReader::pciName(std::string_view vendor, std::string_view device)
{
    const std::string vendorId = lower(vendor.starts_with("0x") ? vendor.substr(2) : vendor);
    const std::string deviceId = lower(device.starts_with("0x") ? device.substr(2) : device);
    if (vendorId.size() != 4 || deviceId.size() != 4)
        return {};

    for (const char* path : {"/usr/share/misc/pci.ids", "/usr/share/hwdata/pci.ids"}) {
        const std::string ids = readFile(path);
        if (ids.empty())
            continue;
        bool inVendor = false;
        for (const std::string_view line : split(ids, '\n')) {
            if (line.empty() || line.starts_with('#'))
                continue;
            if (line[0] != '\t') {
                if (inVendor)
                    return {};
                inVendor = line.starts_with(vendorId + "  ");
            } else if (inVendor && line.starts_with("\t" + deviceId + "  ")) {
                return validUtf8(trimmed(line.substr(7)));
            }
        }
        return {};
    }
    return {};
}

DrmClient GpuReader::parseDrmFdinfo(std::string_view contents)
{
    DrmClient client;
    std::map<std::string, int> capacities;
    for (const std::string_view line : split(contents, '\n')) {
        const size_t colon = line.find(':');
        if (colon == std::string_view::npos)
            continue;
        const std::string_view key = line.substr(0, colon);
        const auto values = splitFields(line.substr(colon + 1));
        const std::string_view value = values.empty() ? std::string_view() : values.front();
        if (key == "drm-client-id") {
            client.id = std::string(value);
            continue;
        }

        uint64_t number = 0;
        if (!parseNumber(value, &number))
            continue;
        if (key.starts_with("drm-engine-capacity-")) {
            capacities[std::string(key.substr(20))] = std::max(1, int(number));
        } else if (key.starts_with("drm-engine-")) {
            DrmEngine& engine = client.engines[std::string(key.substr(11))];
            engine.busyTime = number;
            engine.hasTime = true;
        } else if (key.starts_with("drm-total-cycles-")) {
            client.engines[std::string(key.substr(17))].totalCycles = number;
        } else if (key.starts_with("drm-cycles-")) {
            DrmEngine& engine = client.engines[std::string(key.substr(11))];
            engine.cycles = number;
            engine.hasCycles = true;
        }
    }
    for (auto& [name, engine] : client.engines) {
        const auto capacity = capacities.find(name);
        engine.capacity = capacity == capacities.end() ? 1 : capacity->second;
    }
    return client;
}

double GpuReader::drmBusy(const DrmClients& current, const DrmClients& previous, double elapsedTime)
{
    std::map<std::string, double> loads;
    std::map<std::string, int> capacities;
    for (const auto& [key, client] : current) {
        // A new client's counters include work done before the interval.
        const auto earlier = previous.find(key);
        if (earlier == previous.end())
            continue;

        for (const auto& [name, engine] : client.engines) {
            const auto found = earlier->second.engines.find(name);
            const DrmEngine before = found == earlier->second.engines.end() ? DrmEngine() : found->second;
            double load = 0;
            if (engine.hasCycles && before.hasCycles && engine.totalCycles > before.totalCycles)
                load = double(counterDelta(engine.cycles, before.cycles)) / (engine.totalCycles - before.totalCycles);
            else if (engine.hasTime && before.hasTime && elapsedTime > 0)
                load = counterDelta(engine.busyTime, before.busyTime) / elapsedTime;
            loads[name] += load;
            const auto capacity = capacities.find(name);
            capacities[name] = std::max(capacity == capacities.end() ? 1 : capacity->second, engine.capacity);
        }
    }

    // The busiest engine sets the GPU's activity.
    double busy = 0;
    for (const auto& [name, load] : loads)
        busy = std::max(busy, load / capacities[name]);
    return std::clamp(100.0 * busy, 0.0, 100.0);
}

std::vector<std::pair<std::string, int>> GpuReader::drmDescriptors(const std::string& path) const
{
    std::vector<std::pair<std::string, int>> nodes;
    DIR* descriptors = opendir((path + "/fd").c_str());
    if (!descriptors)
        return nodes;
    while (const dirent* entry = readdir(descriptors)) {
        char target[256];
        const ssize_t length = readlinkat(dirfd(descriptors), entry->d_name, target, sizeof(target));
        if (length <= 0 || size_t(length) == sizeof(target))
            continue;
        const auto owner = m_nodeOwners.find(std::string(target, size_t(length)));
        if (owner != m_nodeOwners.end())
            nodes.emplace_back(entry->d_name, owner->second);
    }
    closedir(descriptors);
    return nodes;
}

std::vector<DrmClients> GpuReader::drmClients(const std::vector<ProcessRecord>& processes,
                                              const std::vector<bool>& wanted)
{
    std::vector<DrmClients> clients(m_cards.size());
    if (std::find(wanted.begin(), wanted.end(), true) == wanted.end())
        return clients;

    ++m_samples;
    std::unordered_map<int, Descriptors> listed;
    for (const ProcessRecord& process : processes) {
        // Another user's descriptors cannot be inspected.
        if (process.uid != m_uid || process.kernelThread)
            continue;

        // Listing descriptors costs about 10 µs each, so a process's are listed
        // when it appears and every fifth sample after, spread across processes.
        const std::string path = m_procRoot + '/' + std::to_string(process.pid);
        const auto known = m_descriptors.find(process.pid);
        Descriptors descriptors = known == m_descriptors.end() ? Descriptors() : known->second;
        if (descriptors.startTicks != process.startTicks || (m_samples + uint64_t(process.pid)) % 5 == 0)
            descriptors = {process.startTicks, drmDescriptors(path)};

        for (const auto& [descriptor, card] : descriptors.nodes) {
            if (!wanted[size_t(card)])
                continue;
            // A closed descriptor's number may now name another file, which has no DRM fdinfo.
            DrmClient client = parseDrmFdinfo(readFile(path + "/fdinfo/" + descriptor));
            if (client.engines.empty())
                continue;
            // Descriptors shared through fork or dup belong to one client.
            const std::string key = client.id.empty() ? std::to_string(process.pid) + ':' + descriptor : client.id;
            clients[size_t(card)][key] = std::move(client);
        }
        listed[process.pid] = std::move(descriptors);
    }
    m_descriptors = std::move(listed);
    return clients;
}

void GpuReader::sample(Metrics* metrics, const std::vector<ProcessRecord>& processes, double elapsed)
{
    if (!m_scanned)
        scan();

    // Reading a runtime-suspended GPU's counters would wake it up.
    std::vector<bool> active(m_cards.size());
    std::vector<bool> clientCounted(m_cards.size());
    for (size_t index = 0; index < m_cards.size(); ++index) {
        const Card& card = m_cards[index];
        active[index] = trimmed(readFile(card.device + "/power/runtime_status")) != "suspended";
        clientCounted[index] = active[index] && !card.hasBusy && !card.nodes.empty();
    }
    auto clients = drmClients(processes, clientCounted);

    std::vector<Gpu> gpus;
    for (size_t index = 0; index < m_cards.size(); ++index) {
        Card& card = m_cards[index];
        int busy = 0;
        // Without a device-wide counter, activity sums the busy time that
        // DRM fdinfo reports for each of the user's clients.
        if (clientCounted[index]) {
            if (!clients[index].empty())
                card.hasClientBusy = true;
            busy = int(std::lround(drmBusy(clients[index], card.clients, elapsed * 1e9)));
            card.clients = std::move(clients[index]);
        }
        if (!card.hasBusy && !card.hasClientBusy && !card.hasTemperature)
            continue;

        Gpu gpu;
        gpu.name = card.name;
        gpu.driver = card.driver;
        gpu.active = active[index];
        gpu.hasBusy = card.hasBusy || card.hasClientBusy;
        gpu.hasTemperature = card.hasTemperature;
        gpu.hasFrequency = card.hasFrequency;
        gpu.hasVram = card.hasVram;
        if (active[index] && card.hasBusy) {
            int64_t value = 0;
            parseNumber(trimmed(readFile(card.device + "/gpu_busy_percent")), &value);
            busy = int(std::clamp<int64_t>(value, 0, 100));
        }
        gpu.busy = busy;
        if (active[index] && card.hasTemperature) {
            int64_t value = 0;
            if (parseNumber(trimmed(readFile(card.hwmon + "/temp1_input")), &value))
                gpu.temperature = value / 1000.0;
        }
        if (active[index] && card.hasFrequency)
            gpu.frequency = toUnsigned(trimmed(readFile(card.hwmon + "/freq1_input"))) / 1e6;
        if (active[index] && card.hasVram) {
            gpu.vramUsed = toUnsigned(trimmed(readFile(card.device + "/mem_info_vram_used")));
            gpu.vramTotal = toUnsigned(trimmed(readFile(card.device + "/mem_info_vram_total")));
        }
        gpus.push_back(std::move(gpu));
    }

    // The overview follows the primary GPU, or the first one with an activity counter.
    const Gpu* main = gpus.empty() ? nullptr : &gpus.front();
    for (const Gpu& gpu : gpus) {
        if (gpu.hasBusy) {
            if (!main->hasBusy)
                main = &gpu;
            break;
        }
    }
    metrics->gpuAvailable = !gpus.empty();
    metrics->gpuName = main ? main->name : std::string();
    metrics->gpuHasBusy = main && main->hasBusy;
    metrics->gpuHasTemperature = main && main->hasTemperature;
    metrics->gpuPercent = main ? main->busy : 0;
    metrics->gpuTemperature = main ? std::max(0.0, main->temperature) : 0;
    metrics->gpus = std::move(gpus);
}
