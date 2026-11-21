// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

namespace settings {

// What the About page tells about the system.
namespace about {

// The os-release PRETTY_NAME.
std::string prettyProductName();
// Vendor and model family from the firmware tables, or the device tree
// model; empty when the firmware names neither.
std::string computerModel();
// The rule behind computerModel(), on the raw firmware strings.
std::string computerModel(const std::string& vendor, const std::string& family, const std::string& product,
                          const std::string& deviceTreeModel);
// As Qt names it: x86_64, arm64, i386.
std::string architecture();
std::string kernelVersion();
// The kernel's own build string, from uname().version.
std::string kernelBuild();
// How long the machine has been up, in the largest units that fit.
std::string uptime();
// The session type and desktop the page is running in, when they are set.
std::string sessionType();
// The firmware version and date, when the firmware tables provide them.
std::string firmwareVersion();
// "<model> x <count>" from /proc/cpuinfo.
std::string processor();
std::string memorySize();
// "<used> / <total>" of the root file system.
std::string internalStorage();
// The machine's name, as the kernel holds it.
std::string hostName();
// The Debian release this system is built on, from /etc/debian_version.
std::string debianVersion();
// The installed Lingmo Aurora compositor version.
std::string auroraVersion();

// With binary units, B through YB, whole numbers.
std::string formatByteSize(double size);

} // namespace about

} // namespace settings
