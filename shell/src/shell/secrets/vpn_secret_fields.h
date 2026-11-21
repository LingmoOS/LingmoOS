// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <map>
#include <string>
#include <vector>

namespace Lingmo::Shell {

struct VpnSecretField {
    std::string key;
    std::string label;
    bool secret = true;
};

// What to ask for to connect a VPN, as the KDE and GNOME authentication
// dialogs decide it: the hinted secrets when NetworkManager names them, else
// the plugin's own rules (lingmo-framework's VpnUtils::secretFields).
std::vector<VpnSecretField> vpnSecretFields(const std::string &serviceType,
                                            const std::map<std::string, std::string> &data,
                                            const std::vector<std::string> &hints);
// The server's message among the hints, if it sent one.
std::string vpnHintMessage(const std::vector<std::string> &hints);

} // namespace Lingmo::Shell

