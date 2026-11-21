// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ui/clipboard.h"

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::ui::wayland {

class Connection;

// Writes the bytes into the descriptor from the loop, never blocking it,
// then closes it.
void writeToPipe(base::EventLoop& loop, int fd, std::string data);
// Reads the descriptor to its end from the loop, then closes it; nothing on
// a read error.
void readFromPipe(base::EventLoop& loop, int fd, std::function<void(std::optional<std::string>)> done);

// The text types in order of preference, text/plain;charset=utf-8 first.
extern const char* const kTextMimeTypes[5];

// One of the seat's selections, through a protocol a subclass speaks:
// wl_data_device for the clipboard, zwp_primary_selection_v1 for the
// primary selection. Offers and sources are that protocol's objects.
class Selection
{
public:
    virtual ~Selection();

    Selection(const Selection&) = delete;
    Selection& operator=(const Selection&) = delete;

    // Makes the application the owner; empty data gives the selection up.
    void set(const MimeData& data);
    std::vector<std::string> mimeTypes() const;
    bool ownsData() const { return m_source != nullptr; }
    void request(const std::string& mimeType, std::function<void(std::optional<std::string>)> done);
    // The first text type held, or empty.
    std::string textMimeType() const;

protected:
    Selection(Connection& connection, base::EventLoop& loop, ClipboardMode mode);

    virtual void* createSource(const std::vector<std::string>& mimeTypes) = 0;
    virtual void destroySource(void* source) = 0;
    // A null source clears the selection.
    virtual void setSource(void* source) = 0;
    virtual void receive(void* offer, const char* mimeType, int fd) = 0;
    virtual void destroyOffer(void* offer) = 0;

    // From the protocol's listeners.
    void offerAnnounced(void* offer);
    void offerMimeType(void* offer, const char* mimeType);
    void selectionChanged(void* offer);
    void sourceSend(void* source, const char* mimeType, int fd);
    void sourceCancelled(void* source);
    // A drag's offer that is not the selection's: forgotten, not destroyed.
    void forgetOffer(void* offer);
    const std::vector<std::string>* offerTypes(void* offer) const;
    // Destroys every protocol object; for the subclass's destructor.
    void releaseAll();

    Connection& m_connection;
    base::EventLoop& m_loop;

private:
    ClipboardMode m_mode;
    // Announced offers and their MIME types, until they become the selection or go.
    std::unordered_map<void*, std::vector<std::string>> m_offers;
    void* m_selection = nullptr;
    void* m_source = nullptr;
    // What the source sends, every alias of a type included.
    std::vector<std::pair<std::string, std::string>> m_sourceData;
};

} // namespace Lingmo::ui::wayland
