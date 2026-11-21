// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/clipboard/clipboard_history.h"

#include <ctime>
#include <filesystem>
#include <memory>

#include "base/i18n.h"
#include "base/locale.h"
#include "base/logging.h"
#include "base/strings.h"
#include "shell/common/file_thread.h"
#include "shell/common/mime_icons.h"
#include "shell/common/text_match.h"

using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trn;

namespace Lingmo::Shell {

static const base::LogCategory lingmoClipboard("lingmo.shell.clipboard");

// The list shows one line; a longer title only costs layout.
static constexpr size_t kTitleLength = 300;

static std::string fileName(const std::string &path)
{
    return std::filesystem::path(path).filename().string();
}

static bool matches(const ClipboardEntry &entry, const std::string &filter)
{
    if (filter.empty())
        return true;
    switch (entry.kind) {
    case ClipboardEntry::Kind::Text:
        return containsIgnoringCase(entry.text, filter);
    case ClipboardEntry::Kind::Files:
        for (const std::string &path : entry.files) {
            if (containsIgnoringCase(fileName(path), filter))
                return true;
        }
        return false;
    case ClipboardEntry::Kind::Image:
        return false;
    }
    return false;
}

ClipboardHistoryModel::ClipboardHistoryModel(ClipboardStore &store)
    : m_store(store)
{
    m_changed = m_store.changed.connect([this] {
        rebuild();
        modelReset();
    });
    rebuild();
}

const ClipboardEntry &ClipboardHistoryModel::entry(int row) const
{
    return m_store.entries()[m_rows[size_t(row)]];
}

std::string ClipboardHistoryModel::keyAt(int row) const
{
    if (row < 0 || row >= count())
        return {};
    return entry(row).key;
}

void ClipboardHistoryModel::setFilterText(const std::string &text)
{
    if (m_filterText == text)
        return;
    m_filterText = text;
    rebuild();
    modelReset();
}

void ClipboardHistoryModel::rebuild()
{
    m_rows.clear();
    const auto &entries = m_store.entries();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (matches(entries[i], m_filterText))
            m_rows.push_back(i);
    }
    lookUpIcons();
}

void ClipboardHistoryModel::lookUpIcons()
{
    std::erase_if(m_icons, [this](const auto &item) { return m_store.indexOf(item.first) < 0; });

    std::vector<std::pair<std::string, std::string>> wanted;
    for (const ClipboardEntry &entry : m_store.entries()) {
        if (entry.kind == ClipboardEntry::Kind::Files && !entry.files.empty() && !m_icons.count(entry.key))
            wanted.emplace_back(entry.key, entry.files.front());
    }
    if (wanted.empty())
        return;
    // A file's type may need a look into it.
    m_iconJob = runOnFileThread(
            [wanted] {
                std::map<std::string, std::string> icons;
                for (const auto &[key, path] : wanted)
                    icons[key] = mimeIconNameForFile(path);
                return icons;
            },
            [this](std::map<std::string, std::string> icons) {
                m_icons.merge(icons);
                if (!m_rows.empty())
                    dataChanged(0, count() - 1);
            });
}

std::string ClipboardHistoryModel::kind(const ClipboardEntry &entry)
{
    switch (entry.kind) {
    case ClipboardEntry::Kind::Text:
        return "text";
    case ClipboardEntry::Kind::Image:
        return "image";
    case ClipboardEntry::Kind::Files:
        return "files";
    }
    return {};
}

std::string ClipboardHistoryModel::title(int row) const
{
    const ClipboardEntry &item = entry(row);
    switch (item.kind) {
    case ClipboardEntry::Kind::Text: {
        std::string text = item.text;
        if (base::strings::utf16Length(text) > kTitleLength) {
            const std::u16string units = base::strings::utf8ToUtf16(text);
            text = base::strings::utf16ToUtf8(std::u16string_view(units).substr(0, kTitleLength));
        }
        return base::strings::simplified(text);
    }
    case ClipboardEntry::Kind::Image: {
        // An image in the clipboard history, with the time it was copied.
        std::string time;
        if (item.copied) {
            const time_t seconds = time_t(*item.copied / 1000);
            struct tm local = {};
            localtime_r(&seconds, &local);
            time = base::Locale::system().toString(base::ClockTime{local.tm_hour, local.tm_min, local.tm_sec, 0},
                                                   base::Locale::FormatType::Short);
        }
        // TRANSLATORS: an image in the clipboard history and the time it was copied, such as "Image, 10:42:07".
        return format(tr("Image, {time}"), {{"time", time}});
    }
    case ClipboardEntry::Kind::Files:
        if (item.files.size() == 1)
            return fileName(item.files.front());
        return trn("{n} File", "{n} Files", item.files.size());
    }
    return {};
}

std::string ClipboardHistoryModel::detail(int row) const
{
    return entry(row).text;
}

std::string ClipboardHistoryModel::iconName(int row) const
{
    const ClipboardEntry &item = entry(row);
    if (item.kind != ClipboardEntry::Kind::Files)
        return {};
    const auto it = m_icons.find(item.key);
    return it == m_icons.end() ? std::string() : it->second;
}

ClipboardHistory::ClipboardHistory(base::dbus::Connection *bus, clipboard::SelectionController &selection,
                                   std::string directory, size_t limit)
    : m_bus(bus)
    , m_store(std::move(directory), limit)
    , m_monitor(selection)
    , m_model(m_store)
{
    m_subscriptions.push_back(m_monitor.copied.connect([this](const ClipboardPayload &payload) { m_store.add(payload); }));
    m_subscriptions.push_back(
            m_monitor.ownSelectionShown.connect([this](const std::string &key) { m_store.moveToTop(key); }));
}

ClipboardHistory::~ClipboardHistory()
{
    if (m_object && m_bus)
        m_bus->releaseName("org.lingmo.ClipboardHistory");
}

bool ClipboardHistory::registerService()
{
    if (m_object)
        return true;
    if (!m_bus)
        return false;
    auto object = std::make_unique<base::dbus::Object>(m_bus, "/ClipboardHistory", "org.lingmo.ClipboardHistory");
    object->addMethod("toggle", [this] { toggle(); });
    if (!object->publish()
        || !m_bus->requestName("org.lingmo.ClipboardHistory", base::dbus::Connection::DoNotQueue)) {
        LM_WARNING(lingmoClipboard) << "Cannot own org.lingmo.ClipboardHistory";
        return false;
    }
    m_object = std::move(object);
    return true;
}

void ClipboardHistory::toggle()
{
    if (togglePicker)
        togglePicker();
}

void ClipboardHistory::select(int row, std::function<void()> then)
{
    const std::string key = m_model.keyAt(row);
    if (key.empty())
        return;
    const uint64_t serial = ++m_selectSerial;
    m_store.payload(key, [this, key, serial, then = std::move(then)](ClipboardPayload payload) {
        if (serial != m_selectSerial || payload.empty() || !m_monitor.setSelection(payload, key))
            return;
        m_store.moveToTop(key);
        then();
    });
}

void ClipboardHistory::paste(int row)
{
    select(row, [this] {
        // In this order: the selection, the keyboard back to the
        // application, then the keys.
        if (closePicker)
            closePicker();
        if (sendPaste)
            sendPaste();
    });
}

void ClipboardHistory::copy(int row)
{
    select(row, [this] {
        if (closePicker)
            closePicker();
    });
}

void ClipboardHistory::remove(int row)
{
    m_store.remove(m_model.keyAt(row));
}

void ClipboardHistory::clear()
{
    m_store.clear();
}

} // namespace Lingmo::Shell
