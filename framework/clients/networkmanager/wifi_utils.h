// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

#include "base/dbus.h"
#include "lingmo/networkmanager/types.h"

namespace Lingmo::Networkmanager::wifi {

// SSIDs are bytes: a label is for display only and never goes back to
// NetworkManager. Bytes that aren't UTF-8 are read in the charsets of the
// language, by default the session's.
std::string ssidLabel(const std::string& ssid, const std::string& language = {});
// Empty, all NUL bytes, or the single space some routers broadcast.
bool ssidHidden(const std::string& ssid);
// What two SSIDs are compared by: old kernels appended a NUL byte.
std::string ssidKey(const std::string& ssid);
// At most 32 bytes, cut on a character boundary.
std::string ssidBytes(const std::string& name);

// The strongest scheme both the device and the access point support; the
// AP's flags alone when the device reports no capabilities.
SecurityType security(uint32_t deviceCapabilities, uint32_t apFlags, uint32_t wpaFlags, uint32_t rsnFlags,
                      uint32_t mode);
bool enterprise(SecurityType type);
// Networks whose one secret the session's prompt asks for: a PSK, SAE password or WEP key.
bool needsPassword(SecurityType type);
std::string securityLabel(SecurityType type);
// The 802-1x setting for signing in with a user name and password: PEAP or
// TTLS with MSCHAPv2 inside, whichever the server offers.
base::dbus::VariantMap passwordEap(const std::string& identity, const std::string& password,
                                   uint32_t passwordFlags);

// 0, 33, 66 or 100: the step of the signal icon, which the lists sort by.
int strengthStep(int strength);
std::string strengthIcon(int strength);

bool pskValid(const std::string& psk);
std::string generatedPassword();

} // namespace Lingmo::Networkmanager::wifi
