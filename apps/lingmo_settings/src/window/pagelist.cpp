// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/pagelist.h"

#include <map>

#include "base/i18n.h"
#include "window/page.h"

using namespace Lingmo::ui;
using Lingmo::i18n::trNoop;

namespace settings {

static std::vector<PageInfo> makePages()
{
    std::vector<PageInfo> pages;
    const auto add = [&pages](PageInfo page) { pages.push_back(std::move(page)); };

    // Sidebar pages, in sidebar order. Ethernet and Bluetooth join it once
    // their hardware is found, after Wi-Fi.
    add({.id = "wlan", .name = "wlan", .title = trNoop("Wi-Fi"), .icon = "network-wireless-connected-100",
         .iconColor = Color::rgb(0x0067FF), .category = trNoop("Network"),
         .keywords = trNoop("wireless, WLAN, network, internet, password, DNS, IP address")});
    add({.id = "ethernet", .name = "ethernet", .title = trNoop("Ethernet"), .icon = "network.svg",
         .iconColor = Color::rgb(0x0067FF), .category = trNoop("Network"),
         .keywords = trNoop("wired, LAN, network, internet, cable, IP address"), .initiallyAvailable = false});
    add({.id = "bluetooth", .name = "bluetooth", .title = trNoop("Bluetooth"), .icon = "bluetooth.svg",
         .iconColor = Color::rgb(0x0067FF), .category = trNoop("Network"),
         .keywords = trNoop("wireless, devices, pairing, headphones"), .initiallyAvailable = false});
    add({.id = "network", .name = "network", .title = trNoop("Network"), .icon = "globe.svg",
         .iconColor = Color::rgb(0x0067FF), .category = trNoop("Network"),
         .keywords = trNoop("VPN, proxy, hotspot, tethering, internet")});
    add({.id = "appearance", .name = "appearance", .title = trNoop("Appearance"), .icon = "appearance.svg",
         .iconColor = Color::rgb(0x03B4CB), .category = trNoop("Desktop"),
         .keywords = trNoop("theme, dark mode, light mode, accent color, dim wallpaper")});
    add({.id = "background", .name = "background", .title = trNoop("Wallpaper"), .icon = "wallpaper.svg",
         .iconColor = Color::rgb(0x34B4A7), .category = trNoop("Desktop"),
         .keywords = trNoop("background, desktop picture, image")});
    add({.id = "dock", .name = "dock", .title = trNoop("Desktop & Dock"), .icon = "dock.svg",
         .iconColor = Color::rgb(0x8585FC), .category = trNoop("Desktop"),
         .keywords = trNoop("taskbar, position, size, auto hide, glass effect")});
    add({.id = "statusbar", .name = "statusbar", .title = trNoop("Menu Bar"), .icon = "topbar.svg",
         .iconColor = Color::rgb(0x8585FC), .category = trNoop("Desktop"),
         .keywords = trNoop("top bar, panel, blur, transparency")});
    add({.id = "desktopwidgets", .name = "desktopwidgets", .title = trNoop("Desktop Widgets"),
         .icon = "appearance.svg", .iconColor = Color::rgb(0x8585FC), .category = trNoop("Desktop"),
         .keywords = trNoop("widgets, clock, calendar, battery, desktop cards")});
    add({.id = "notifications", .name = "notifications", .title = trNoop("Notifications"),
         .icon = "notifications.svg", .iconColor = Color::rgb(0xF16884), .category = trNoop("Desktop"),
         .keywords = trNoop("do not disturb, banners, alerts")});
    add({.id = "display", .name = "display", .title = trNoop("Display"), .icon = "display.svg",
         .iconColor = Color::rgb(0x0087ED), .category = trNoop("Devices"),
         .keywords = trNoop("monitor, screen, resolution, refresh rate, scaling, brightness, rotation")});
    add({.id = "sound", .name = "sound", .title = trNoop("Sound"), .icon = "sound.svg",
         .iconColor = Color::rgb(0xF16884), .category = trNoop("Devices"),
         .keywords = trNoop("audio, volume, speakers, headphones, microphone, input, output")});
    add({.id = "keyboard", .name = "keyboard", .title = trNoop("Keyboard"), .icon = "keyboard.svg",
         .iconColor = Color::rgb(0x5E6AD2), .category = trNoop("Devices"),
         .keywords = trNoop("input method, key repeat, keyboard style")});
    add({.id = "mouse", .name = "mouse", .title = trNoop("Mouse"), .icon = "cursor.svg",
         .iconColor = Color::rgb(0x3385FF), .category = trNoop("Devices"),
         .keywords = trNoop("pointer speed, acceleration, natural scrolling, left-handed")});
    add({.id = "touchpad", .name = "touchpad", .title = trNoop("Touchpad"), .icon = "touchpad.svg",
         .iconColor = Color::rgb(0x999999), .category = trNoop("Devices"),
         .keywords = trNoop("trackpad, tap to click, natural scrolling, secondary click, pointer speed")});
    add({.id = "touchscreen", .name = "touchscreen", .title = trNoop("Touchscreen"), .icon = "touchscreen.svg",
         .iconColor = Color::rgb(0x999999), .category = trNoop("Devices"),
         .keywords = trNoop("touch, touch screen, touch panel")});
    add({.id = "accounts", .name = "accounts", .title = trNoop("Users"), .icon = "accounts.svg",
         .iconColor = Color::rgb(0xDA7C43), .category = trNoop("System"),
         .keywords = trNoop("accounts, password, login, administrator, avatar")});
    add({.id = "language", .name = "language", .title = trNoop("Language"), .icon = "language.svg",
         .iconColor = Color::rgb(0x20A7FF), .category = trNoop("System"),
         .keywords = trNoop("region, locale, system language")});
    add({.id = "datetime", .name = "datetime", .title = trNoop("Date & Time"), .icon = "datetime.svg",
         .iconColor = Color::rgb(0x418CFF), .category = trNoop("System"),
         .keywords = trNoop("clock, time zone, 24-hour time, NTP, sync")});
    add({.id = "defaultapps", .name = "defaultapps", .title = trNoop("Default Applications"),
         .icon = "defaultapps.svg", .iconColor = Color::rgb(0x418CFF), .category = trNoop("System"),
         .keywords = trNoop("web browser, email client, terminal")});
    add({.id = "privacy", .name = "privacy", .title = trNoop("Privacy"), .icon = "lock",
         .iconColor = Color::rgb(0x418CFF), .category = trNoop("System"),
         .keywords = trNoop("computer control, agents, allowed apps, permissions")});
    add({.id = "battery", .name = "battery", .title = trNoop("Battery"), .icon = "battery.svg",
         .iconColor = Color::rgb(0x2EC347), .category = trNoop("System"),
         .keywords = trNoop("charge, charging limit, battery health, capacity, history, percentage")});
    add({.id = "power", .name = "power", .title = trNoop("Power"), .icon = "power.svg",
         .iconColor = Color::rgb(0xFE8433), .category = trNoop("System"),
         .keywords = trNoop("energy, performance, power saving, sleep, turn off display")});
    add({.id = "about", .name = "about", .title = trNoop("About"), .icon = "about.svg",
         .iconColor = Color::rgb(0x24A7FD), .category = trNoop("System"),
         .keywords = trNoop("system, version, kernel, processor, memory, storage, hardware")});
    add({.id = "update", .name = "update", .title = trNoop("Software Update"), .icon = "update.svg",
         .iconColor = Color::rgb(0x0067FF), .category = trNoop("System"),
         .keywords = trNoop("system update, software update, upgrade, version, apt, packages")});

    // Sub pages, opened from inside a sidebar page; search lists them after it.
    add({.id = "update-advanced", .name = "update-advanced", .title = trNoop("Advanced Settings"),
         .icon = "update.svg", .iconColor = Color::rgb(0x0067FF),
         .keywords = trNoop("automatic checks, frequency, schedule, update time"),
         .parentId = "update"});
    add({.id = "vpn", .name = "vpn", .title = trNoop("VPN"), .icon = "network-vpn",
         .iconColor = Color::rgb(0x48484C), .keywords = trNoop("VPN, WireGuard, OpenVPN, tunnel, network"),
         .parentId = "network"});
    add({.id = "proxy", .name = "proxy", .title = trNoop("Proxy"), .icon = "proxy.svg",
         .iconColor = Color::rgb(0x0067FF), .keywords = trNoop("HTTP, HTTPS, SOCKS, FTP, PAC, network"),
         .parentId = "network"});
    add({.id = "hotspot", .name = "hotspot", .title = trNoop("Hotspot"), .icon = "network-wireless-hotspot",
         .iconColor = Color::rgb(0x2EBC6B),
         .keywords = trNoop("tethering, share internet, access point, Wi-Fi, wireless"), .parentId = "network"});
    add({.id = "fonts", .title = trNoop("Fonts"), .icon = "fonts.svg", .iconColor = Color::rgb(0xFE8433),
         .keywords = trNoop("general font, fixed font, font size, anti-aliasing, hinting"), .parentId = "appearance"});
    add({.id = "icontheme", .title = trNoop("Icon Theme"), .icon = "icontheme.svg", .iconColor = Color::rgb(0x2EC347),
         .keywords = trNoop("icons"), .parentId = "appearance"});
    add({.id = "cursortheme", .title = trNoop("Cursor Style"), .icon = "cursor.svg", .iconColor = Color::rgb(0x3385FF),
         .keywords = trNoop("cursor theme, cursor size, pointer"), .parentId = "appearance"});
    add({.id = "keyboardlayout", .title = trNoop("Keyboard Layout"), .icon = "keyboard.svg",
         .iconColor = Color::rgb(0x5E6AD2), .keywords = trNoop("input source"), .parentId = "keyboard"});
    add({.id = "hotcorners", .name = "hotcorners", .title = trNoop("Hot Corners"), .icon = "dock.svg",
         .iconColor = Color::rgb(0x8585FC), .keywords = trNoop("screen corners, show desktop, show all windows"),
         .parentId = "dock"});
    add({.id = "shortcuts", .title = trNoop("Keyboard Shortcuts"), .icon = "keyboard.svg",
         .iconColor = Color::rgb(0x5E6AD2), .keywords = trNoop("hotkeys, key bindings"), .parentId = "keyboard"});
    return pages;
}

struct Registration
{
    PageFactory create;
    AvailabilityFactory watchAvailability;
};

// Filled before main() by the pages' static registrations.
static std::map<std::string, Registration>& registrations()
{
    static std::map<std::string, Registration> map;
    return map;
}

PageRegistration::PageRegistration(const char* id, PageFactory create, AvailabilityFactory watchAvailability)
{
    registrations()[id] = {std::move(create), std::move(watchAvailability)};
}

const std::vector<PageInfo>& pages()
{
    static const std::vector<PageInfo> list = [] {
        std::vector<PageInfo> pages = makePages();
        for (PageInfo& page : pages) {
            const auto it = registrations().find(page.id);
            if (it == registrations().end())
                continue;
            page.create = it->second.create;
            page.watchAvailability = it->second.watchAvailability;
        }
        return pages;
    }();
    return list;
}

const PageInfo* findPage(const std::string& id)
{
    for (const PageInfo& page : pages()) {
        if (page.id == id)
            return &page;
    }
    return nullptr;
}

} // namespace settings
