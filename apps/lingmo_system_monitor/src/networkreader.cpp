// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "networkreader.h"
#include "fileutil.h"

NetworkReader::NetworkReader(std::string procRoot, std::string sysRoot)
    : m_procRoot(std::move(procRoot))
    , m_sysRoot(std::move(sysRoot))
{
}

void NetworkReader::sample(Metrics* metrics, double elapsed)
{
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> networks;
    std::vector<NetworkInterface> interfaces;
    double download = 0;
    double upload = 0;
    uint64_t received = 0;
    uint64_t sent = 0;
    const std::string devices = readFile(m_procRoot + "/net/dev");
    for (const std::string_view line : split(devices, '\n')) {
        const size_t separator = line.find(':');
        if (separator == std::string_view::npos)
            continue;
        const std::string name = validUtf8(trimmed(line.substr(0, separator)));
        const auto values = splitFields(line.substr(separator + 1));
        if (name == "lo" || values.size() < 16)
            continue;
        const uint64_t rx = toUnsigned(values[0]);
        const uint64_t tx = toUnsigned(values[8]);
        networks[name] = {rx, tx};
        const auto previous = m_previous.find(name);
        const bool known = previous != m_previous.end();
        const double rxRate = known ? byteRate(rx, previous->second.first, elapsed) : 0;
        const double txRate = known ? byteRate(tx, previous->second.second, elapsed) : 0;
        const std::string directory = m_sysRoot + "/class/net/" + name;
        const bool physical = fileExists(directory + "/device");
        if (physical) {
            download += rxRate;
            upload += txRate;
            received += rx;
            sent += tx;
        }
        NetworkInterface interface;
        interface.name = name;
        interface.state = validUtf8(trimmed(readFile(directory + "/operstate")));
        interface.address = validUtf8(trimmed(readFile(directory + "/address")));
        interface.download = rxRate;
        interface.upload = txRate;
        interface.received = rx;
        interface.sent = tx;
        interface.physical = physical;
        interfaces.push_back(std::move(interface));
    }
    metrics->download = download;
    metrics->upload = upload;
    metrics->received = received;
    metrics->sent = sent;
    metrics->interfaces = std::move(interfaces);

    m_previous = std::move(networks);
}
