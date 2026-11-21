// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "base/signal.h"

namespace Lingmo::ui {

// Which of the seat's selections.
enum class ClipboardMode {
    // Copy and paste.
    Clipboard,
    // The primary selection: what was selected last, pasted with a middle
    // click.
    Selection,
};

// Data in several MIME types.
class MimeData
{
public:
    // Replaces the bytes of a type already set.
    void setData(const std::string& mimeType, std::string bytes);
    // Null when the type is not set.
    const std::string* data(const std::string& mimeType) const;
    bool hasFormat(const std::string& mimeType) const { return data(mimeType) != nullptr; }
    // In the order set.
    std::vector<std::string> formats() const;
    bool isEmpty() const { return m_formats.empty(); }

    // As "text/plain;charset=utf-8"; offered under the other text types too.
    void setText(const std::string& text);
    // The first text type set, in wayland::kTextMimeTypes' order.
    std::optional<std::string> text() const;
    // As "text/uri-list": one URL per line, CRLF ended.
    void setUrls(const std::vector<std::string>& urls);
    // The "text/uri-list" lines, comments and blank lines left out.
    std::vector<std::string> urls() const;

    const std::vector<std::pair<std::string, std::string>>& entries() const { return m_formats; }

private:
    std::vector<std::pair<std::string, std::string>> m_formats;
};

// The session's clipboard and primary selection. Setting one makes this
// application its owner until another one sets it. Without a compositor
// connection both stay within the process.
class Clipboard
{
public:
    static void setText(const std::string& text);
    static void setText(const std::string& text, ClipboardMode mode);
    // Answers from the event loop, or at once in a headless application; the
    // text is empty when the clipboard holds none.
    static void requestText(std::function<void(std::string text)> callback);
    static void requestText(std::function<void(std::string text)> callback, ClipboardMode mode);

    // Several types at once, such as a file copy's "text/uri-list",
    // "x-special/gnome-copied-files" and text. Empty data clears.
    static void setMimeData(const MimeData& data, ClipboardMode mode = ClipboardMode::Clipboard);
    static void clear(ClipboardMode mode = ClipboardMode::Clipboard);
    // What the owner offers now; empty when nothing is held.
    static std::vector<std::string> mimeTypes(ClipboardMode mode = ClipboardMode::Clipboard);
    static bool hasText(ClipboardMode mode = ClipboardMode::Clipboard);
    // The bytes in one type, from the event loop (at once when headless);
    // nothing when the owner offers no such type or failed.
    static void requestData(const std::string& mimeType,
                            std::function<void(std::optional<std::string> data)> callback,
                            ClipboardMode mode = ClipboardMode::Clipboard);
    // Whether this application set what is held now.
    static bool ownsData(ClipboardMode mode = ClipboardMode::Clipboard);
    // False when the compositor has no primary selection.
    static bool supportsSelection();

    // After the held data changed, this application's own changes
    // included.
    static base::Signal<ClipboardMode>& changed();
};

} // namespace Lingmo::ui
