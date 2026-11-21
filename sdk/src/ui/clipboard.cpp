namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/clipboard.h"

#include <algorithm>

#include "base/event_loop.h"
#include "ui/application.h"
#include "wayland/connection.h"
#include "wayland/data_device.h"
#include "wayland/primary_selection.h"

namespace Lingmo::ui {

void MimeData::setData(const std::string& mimeType, std::string bytes)
{
    for (auto& [type, data] : m_formats) {
        if (type == mimeType) {
            data = std::move(bytes);
            return;
        }
    }
    m_formats.emplace_back(mimeType, std::move(bytes));
}

const std::string* MimeData::data(const std::string& mimeType) const
{
    for (const auto& [type, data] : m_formats) {
        if (type == mimeType)
            return &data;
    }
    return nullptr;
}

std::vector<std::string> MimeData::formats() const
{
    std::vector<std::string> types;
    types.reserve(m_formats.size());
    for (const auto& [type, data] : m_formats)
        types.push_back(type);
    return types;
}

void MimeData::setText(const std::string& text)
{
    setData(wayland::kTextMimeTypes[0], text);
}

std::optional<std::string> MimeData::text() const
{
    for (const char* type : wayland::kTextMimeTypes) {
        if (const std::string* bytes = data(type))
            return *bytes;
    }
    return std::nullopt;
}

void MimeData::setUrls(const std::vector<std::string>& urls)
{
    std::string list;
    for (const std::string& url : urls)
        list += url + "\r\n";
    setData("text/uri-list", std::move(list));
}

std::vector<std::string> MimeData::urls() const
{
    std::vector<std::string> urls;
    const std::string* list = data("text/uri-list");
    if (!list)
        return urls;
    size_t start = 0;
    while (start < list->size()) {
        size_t end = list->find('\n', start);
        if (end == std::string::npos)
            end = list->size();
        std::string line = list->substr(start, end - start);
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!line.empty() && line.front() != '#')
            urls.push_back(std::move(line));
        start = end + 1;
    }
    return urls;
}

// Without a compositor connection both selections stay within the process.
static MimeData s_headless[2];

static wayland::Selection* selectionFor(ClipboardMode mode)
{
    Application* application = Application::instance();
    if (!application->hasConnection())
        return nullptr;
    if (mode == ClipboardMode::Selection)
        return application->connection().primarySelection();
    return application->connection().dataDevice();
}

static bool headless()
{
    return !Application::instance()->hasConnection();
}

base::Signal<ClipboardMode>& Clipboard::changed()
{
    static base::Signal<ClipboardMode> signal;
    return signal;
}

void Clipboard::setText(const std::string& text)
{
    setText(text, ClipboardMode::Clipboard);
}

void Clipboard::setText(const std::string& text, ClipboardMode mode)
{
    MimeData data;
    data.setText(text);
    setMimeData(data, mode);
}

void Clipboard::requestText(std::function<void(std::string text)> callback)
{
    requestText(std::move(callback), ClipboardMode::Clipboard);
}

void Clipboard::requestText(std::function<void(std::string text)> callback, ClipboardMode mode)
{
    if (headless()) {
        callback(s_headless[int(mode)].text().value_or(std::string()));
        return;
    }
    wayland::Selection* selection = selectionFor(mode);
    const std::string type = selection ? selection->textMimeType() : std::string();
    if (type.empty()) {
        Application::instance()->loop().post([callback = std::move(callback)] { callback({}); });
        return;
    }
    selection->request(type, [callback = std::move(callback)](std::optional<std::string> text) {
        callback(text.value_or(std::string()));
    });
}

void Clipboard::setMimeData(const MimeData& data, ClipboardMode mode)
{
    if (headless()) {
        s_headless[int(mode)] = data;
        changed().emit(mode);
        return;
    }
    if (wayland::Selection* selection = selectionFor(mode))
        selection->set(data);
}

void Clipboard::clear(ClipboardMode mode)
{
    setMimeData(MimeData(), mode);
}

std::vector<std::string> Clipboard::mimeTypes(ClipboardMode mode)
{
    if (headless()) {
        // As offered on Wayland: text under every text type.
        std::vector<std::string> types = s_headless[int(mode)].formats();
        if (s_headless[int(mode)].text()) {
            for (const char* type : wayland::kTextMimeTypes) {
                if (std::find(types.begin(), types.end(), type) == types.end())
                    types.push_back(type);
            }
        }
        return types;
    }
    wayland::Selection* selection = selectionFor(mode);
    return selection ? selection->mimeTypes() : std::vector<std::string>();
}

bool Clipboard::hasText(ClipboardMode mode)
{
    if (headless())
        return s_headless[int(mode)].text().has_value();
    wayland::Selection* selection = selectionFor(mode);
    return selection && !selection->textMimeType().empty();
}

void Clipboard::requestData(const std::string& mimeType,
                            std::function<void(std::optional<std::string> data)> callback, ClipboardMode mode)
{
    if (headless()) {
        const MimeData& held = s_headless[int(mode)];
        if (const std::string* bytes = held.data(mimeType)) {
            callback(*bytes);
        } else {
            const bool textType = std::find(std::begin(wayland::kTextMimeTypes), std::end(wayland::kTextMimeTypes),
                                            mimeType) != std::end(wayland::kTextMimeTypes);
            callback(textType ? held.text() : std::nullopt);
        }
        return;
    }
    wayland::Selection* selection = selectionFor(mode);
    if (!selection) {
        Application::instance()->loop().post([callback = std::move(callback)] { callback(std::nullopt); });
        return;
    }
    selection->request(mimeType, std::move(callback));
}

bool Clipboard::ownsData(ClipboardMode mode)
{
    if (headless())
        return !s_headless[int(mode)].isEmpty();
    wayland::Selection* selection = selectionFor(mode);
    return selection && selection->ownsData();
}

bool Clipboard::supportsSelection()
{
    return headless() || selectionFor(ClipboardMode::Selection) != nullptr;
}

} // namespace Lingmo::ui
