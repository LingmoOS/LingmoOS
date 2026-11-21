// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// The compositor's components live in Lingmo::Scene, Lingmo::Clipboard,
// Lingmo::Input, Lingmo::Display, ... but the sources name them with
// lower-case prefixes (scene::, clipboard::, input::, display::, ...).
// Declare the namespaces first so the aliases below have a target even at the
// very top of a translation unit, then give the lower-case names a target at
// global scope. The project's compile options force-include this header.
#pragma once

#ifdef __cplusplus

namespace Lingmo {
namespace Accounts {}
namespace appearance {}
namespace Audio {}
namespace bluez {}
namespace Networkmanager {}
namespace powerprofiles {}
namespace timedate {}
namespace Files {}
namespace upower {}
namespace Accelerators {}
namespace Render {}
namespace Toolkit {}
namespace Wayland {}
namespace XWayland {}
namespace Auth {}
namespace Backend {}
namespace Clipboard {}
namespace Compositor {}
namespace ComputerControl {}
namespace Core {}
namespace Cursor {}
namespace Decoration {}
namespace Display {
namespace ipc {}
}
namespace Input {}
namespace Scene {
namespace Motion {}
}
namespace Session {}
namespace Shell {}
namespace Switcher {}
namespace WM {
namespace Overview {}
namespace WindowPositioner {}
}
namespace ui {}
} // namespace Lingmo

namespace accelerators = Lingmo::Accelerators;
namespace auth = Lingmo::Auth;
namespace backend = Lingmo::Backend;
namespace clipboard = Lingmo::Clipboard;
namespace compositor = Lingmo::Compositor;
namespace computer_control = Lingmo::ComputerControl;
namespace core = Lingmo::Core;
namespace cursor = Lingmo::Cursor;
namespace decoration = Lingmo::Decoration;
namespace display = Lingmo::Display;
namespace input = Lingmo::Input;
namespace ipc = Lingmo::Display::ipc;
namespace scene = Lingmo::Scene;
namespace session = Lingmo::Session;
namespace shell = Lingmo::Shell;
namespace switcher = Lingmo::Switcher;
namespace render = Lingmo::Render;
namespace toolkit = Lingmo::Toolkit;
namespace wayland = Lingmo::Wayland;
namespace xwayland = Lingmo::XWayland;
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
namespace ui = Lingmo::ui;
namespace wm = Lingmo::WM;

namespace Lingmo::WM {
namespace overview = Overview;
namespace window_positioner = WindowPositioner;
} // namespace Lingmo::WM

namespace Lingmo::Scene {
namespace motion = Motion;
} // namespace Lingmo::Scene

#endif // __cplusplus
