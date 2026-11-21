// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The settings pages name the framework clients with lower-case prefixes
// (accounts::, upower::, audio::, ...). The clients live in Lingmo::Accounts,
// Lingmo::upower, Lingmo::Audio, ...; the spellings differ per client. The
// project's compile options force-include this header.
#pragma once

#ifdef __cplusplus

namespace Lingmo {
namespace Accounts {}
namespace appearance {}
namespace Audio {}
namespace bluez {}
namespace Display {}
namespace Files {}
namespace Networkmanager {}
namespace powerprofiles {}
namespace timedate {}
namespace upower {}
} // namespace Lingmo

namespace accounts = Lingmo::Accounts;
namespace appearance = Lingmo::appearance;
namespace audio = Lingmo::Audio;
namespace bluez = Lingmo::bluez;
namespace bluetooth = Lingmo::bluez;
namespace display = Lingmo::Display;
namespace files = Lingmo::Files;
namespace networkmanager = Lingmo::Networkmanager;
namespace powerprofiles = Lingmo::powerprofiles;
namespace timedate = Lingmo::timedate;
namespace upower = Lingmo::upower;

#endif // __cplusplus
