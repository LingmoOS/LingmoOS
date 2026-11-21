// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/keyboard/keyboard.h"

#include <algorithm>
#include <cstdio>
#include <libintl.h>
#include <memory>

#include <expat.h>
#include <unicode/ucol.h>

#include "base/file.h"

namespace settings {

static constexpr const char* kRulesFile = "/usr/share/X11/xkb/rules/evdev.xml";

namespace {

// The layoutList of a rules file, walked the way xkeyboard-config nests it:
// a layout's configItem, then its variants' configItems.
struct RulesReader
{
    std::vector<KeyboardLayout> layouts;
    bool inLayoutList = false;
    bool inVariant = false;
    // The element whose text is being read: name or description.
    std::string* reading = nullptr;
    std::string layout;
    std::string name;
    std::string description;

    void start(std::string_view element)
    {
        if (element == "layoutList") {
            inLayoutList = true;
        } else if (!inLayoutList) {
            return;
        } else if (element == "variant") {
            inVariant = true;
        } else if (element == "configItem") {
            name.clear();
            description.clear();
        } else if (element == "name") {
            name.clear();
            reading = &name;
        } else if (element == "description") {
            description.clear();
            reading = &description;
        }
    }

    void end(std::string_view element)
    {
        reading = nullptr;
        if (!inLayoutList)
            return;
        if (element == "layoutList") {
            inLayoutList = false;
        } else if (element == "variant") {
            inVariant = false;
        } else if (element == "configItem" && !name.empty()) {
            if (!inVariant)
                layout = name;
            // A placeholder for a layout file the user writes by hand.
            if (layout == "custom")
                return;
            layouts.push_back({inVariant ? layout + "-" + name : layout,
                               dgettext("xkeyboard-config", description.c_str())});
        }
    }
};

} // namespace

std::vector<KeyboardLayout> readKeyboardLayouts(const std::string& path)
{
    const std::optional<std::string> contents = Lingmo::base::file::read(path);
    if (!contents) {
        fprintf(stderr, "Cannot read keyboard layouts from %s\n", path.c_str());
        return {};
    }
    bind_textdomain_codeset("xkeyboard-config", "UTF-8");

    RulesReader reader;
    std::unique_ptr<XML_ParserStruct, decltype(&XML_ParserFree)> parser(XML_ParserCreate("UTF-8"), XML_ParserFree);
    XML_SetUserData(parser.get(), &reader);
    XML_SetElementHandler(
            parser.get(),
            [](void* data, const XML_Char* element, const XML_Char**) { static_cast<RulesReader*>(data)->start(element); },
            [](void* data, const XML_Char* element) { static_cast<RulesReader*>(data)->end(element); });
    XML_SetCharacterDataHandler(parser.get(), [](void* data, const XML_Char* text, int length) {
        auto* reader = static_cast<RulesReader*>(data);
        if (reader->reading)
            reader->reading->append(text, size_t(length));
    });
    if (XML_Parse(parser.get(), contents->data(), int(contents->size()), XML_TRUE) == XML_STATUS_ERROR) {
        fprintf(stderr, "Cannot parse %s: %s\n", path.c_str(), XML_ErrorString(XML_GetErrorCode(parser.get())));
    }

    // By name in the user's language, ignoring case.
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCollator, decltype(&ucol_close)> collator(ucol_open(nullptr, &status), ucol_close);
    if (U_SUCCESS(status)) {
        ucol_setStrength(collator.get(), UCOL_SECONDARY);
        std::stable_sort(reader.layouts.begin(), reader.layouts.end(),
                         [&collator](const KeyboardLayout& a, const KeyboardLayout& b) {
                             UErrorCode error = U_ZERO_ERROR;
                             return ucol_strcollUTF8(collator.get(), a.name.data(), int32_t(a.name.size()),
                                                     b.name.data(), int32_t(b.name.size()), &error)
                                     == UCOL_LESS;
                         });
    }
    return reader.layouts;
}

const std::vector<KeyboardLayout>& keyboardLayouts()
{
    static const std::vector<KeyboardLayout> layouts = readKeyboardLayouts(kRulesFile);
    return layouts;
}

Keyboard::Keyboard()
    : m_service("org.lingmo.Keyboard1")
{
}

bool Keyboard::repeatEnabled() const
{
    return m_service.boolValue("repeatEnabled", true);
}

void Keyboard::setRepeatEnabled(bool enabled)
{
    m_service.call("setRepeatEnabled", enabled);
}

int Keyboard::repeatDelay() const
{
    return m_service.intValue("repeatDelay", 0);
}

void Keyboard::setRepeatDelay(int milliseconds)
{
    m_service.call("setRepeatDelay", int32_t(milliseconds));
}

int Keyboard::repeatInterval() const
{
    return m_service.intValue("repeatInterval", 0);
}

void Keyboard::setRepeatInterval(int milliseconds)
{
    m_service.call("setRepeatInterval", int32_t(milliseconds));
}

std::string Keyboard::layout() const
{
    return m_service.stringValue("layout");
}

std::string Keyboard::layoutName() const
{
    const std::string id = layout();
    for (const KeyboardLayout& entry : keyboardLayouts()) {
        if (entry.id == id)
            return entry.name;
    }
    return id;
}

void Keyboard::selectLayout(const std::string& id)
{
    m_service.callWithReply("setLayout", [this, id](const Lingmo::base::dbus::Reply& reply) {
        if (!reply.isError() && !reply.value<bool>().value_or(false))
            fprintf(stderr, "Cannot select keyboard layout %s\n", id.c_str());
        m_service.changed.emit();
    }, id);
}

std::string Keyboard::shortcutConvention() const
{
    return m_service.isAvailable() ? m_service.stringValue("shortcutConvention") : "pc";
}

void Keyboard::setShortcutConvention(const std::string& convention)
{
    m_service.call("setShortcutConvention", convention);
}

} // namespace settings
