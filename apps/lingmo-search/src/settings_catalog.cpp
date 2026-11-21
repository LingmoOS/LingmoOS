// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "search.h"
#include "base/i18n.h"
namespace Search {
std::vector<Setting> settingsCatalog(const std::function<std::string(const char*)>& translate) {
    struct Item { const char *id, *title, *keywords; };
    // IDs are Settings' public -m names (settings/src/window/pagelist.cpp).
    static const Item items[] = {
        {"wlan", Lingmo::I18n::trNoop("Wi-Fi"), Lingmo::I18n::trNoop("wireless network internet")},
        {"bluetooth", Lingmo::I18n::trNoop("Bluetooth"), Lingmo::I18n::trNoop("headphones pairing devices")},
        {"appearance", Lingmo::I18n::trNoop("Appearance"), Lingmo::I18n::trNoop("theme dark light accent font")},
        {"background", Lingmo::I18n::trNoop("Wallpaper"), Lingmo::I18n::trNoop("background desktop picture")},
        {"display", Lingmo::I18n::trNoop("Display"), Lingmo::I18n::trNoop("screen resolution scaling brightness")},
        {"sound", Lingmo::I18n::trNoop("Sound"), Lingmo::I18n::trNoop("audio volume microphone speakers")},
        {"keyboard", Lingmo::I18n::trNoop("Keyboard"), Lingmo::I18n::trNoop("input method layout typing")},
        {"mouse", Lingmo::I18n::trNoop("Mouse"), Lingmo::I18n::trNoop("pointer speed scrolling")},
        {"touchpad", Lingmo::I18n::trNoop("Touchpad"), Lingmo::I18n::trNoop("trackpad gestures scrolling")},
        {"accounts", Lingmo::I18n::trNoop("Users"), Lingmo::I18n::trNoop("account password login")},
        {"language", Lingmo::I18n::trNoop("Language"), Lingmo::I18n::trNoop("region locale translation")},
        {"datetime", Lingmo::I18n::trNoop("Date & Time"), Lingmo::I18n::trNoop("clock time zone calendar")},
        {"defaultapps", Lingmo::I18n::trNoop("Default Applications"), Lingmo::I18n::trNoop("browser email defaults")},
        {"privacy", Lingmo::I18n::trNoop("Privacy"), Lingmo::I18n::trNoop("permissions computer control")},
        {"power", Lingmo::I18n::trNoop("Power"), Lingmo::I18n::trNoop("battery sleep energy")},
        {"update", Lingmo::I18n::trNoop("Software Update"), Lingmo::I18n::trNoop("upgrade packages system version")},
        {"about", Lingmo::I18n::trNoop("About"), Lingmo::I18n::trNoop("system information version hardware")},
    };
    std::vector<Setting> result;
    for (const auto& item : items) result.push_back({item.id, translate(item.title), translate(item.keywords) + " " + item.title + " " + item.keywords});
    return result;
}
}
