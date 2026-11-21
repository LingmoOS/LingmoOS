namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wayland/selection.h"

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "wayland/connection.h"

namespace Lingmo::ui::wayland {

const char* const kTextMimeTypes[5] = {
    "text/plain;charset=utf-8",
    "text/plain",
    "UTF8_STRING",
    "STRING",
    "TEXT",
};

void writeToPipe(base::EventLoop& loop, int fd, std::string data)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    struct Write
    {
        int fd;
        int watch = 0;
        std::string data;
        size_t offset = 0;
    };
    auto write = std::make_shared<Write>();
    write->fd = fd;
    write->data = std::move(data);
    // Returns true when done, written or failed.
    const auto pump = [write] {
        while (write->offset < write->data.size()) {
            // A target that closes its end early must not end this process.
            const ssize_t count = base::file::writeWithoutSigpipe(write->fd, write->data.data() + write->offset,
                                                                  write->data.size() - write->offset);
            if (count > 0) {
                write->offset += size_t(count);
                continue;
            }
            if (count < 0 && errno == EINTR)
                continue;
            return !(count < 0 && errno == EAGAIN);
        }
        return true;
    };
    if (pump()) {
        close(fd);
        return;
    }
    write->watch = loop.watch(fd, EPOLLOUT, [&loop, write, pump](uint32_t events) {
        if (!(events & (EPOLLERR | EPOLLHUP)) && !pump())
            return;
        loop.unwatch(write->watch);
        close(write->fd);
    });
    if (!write->watch)
        close(fd);
}

// A source sending more than this is dropped rather than read to the end.
static constexpr size_t kMaximumTransfer = size_t(512) << 20;

void readFromPipe(base::EventLoop& loop, int fd, std::function<void(std::optional<std::string>)> done)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    struct Read
    {
        int fd;
        int watch = 0;
        std::string data;
        std::function<void(std::optional<std::string>)> done;
    };
    auto read = std::make_shared<Read>();
    read->fd = fd;
    read->done = std::move(done);
    read->watch = loop.watch(fd, EPOLLIN, [&loop, read](uint32_t) {
        char buffer[16384];
        bool failed = false;
        for (;;) {
            const ssize_t count = ::read(read->fd, buffer, sizeof(buffer));
            if (count > 0) {
                if (read->data.size() + size_t(count) > kMaximumTransfer) {
                    failed = true;
                    break;
                }
                read->data.append(buffer, size_t(count));
                continue;
            }
            if (count < 0 && errno == EINTR)
                continue;
            if (count < 0 && errno == EAGAIN)
                return;
            failed = count < 0;
            break;
        }
        loop.unwatch(read->watch);
        close(read->fd);
        auto callback = std::move(read->done);
        if (failed)
            callback(std::nullopt);
        else
            callback(std::move(read->data));
    });
    if (!read->watch) {
        close(fd);
        loop.post([read] { read->done(std::nullopt); });
    }
}

Selection::Selection(Connection& connection, base::EventLoop& loop, ClipboardMode mode)
    : m_connection(connection)
    , m_loop(loop)
    , m_mode(mode)
{
}

Selection::~Selection() = default;

void Selection::releaseAll()
{
    if (m_source)
        destroySource(m_source);
    m_source = nullptr;
    for (const auto& [offer, types] : m_offers)
        destroyOffer(offer);
    m_offers.clear();
    m_selection = nullptr;
}

void Selection::set(const MimeData& data)
{
    if (m_source)
        destroySource(m_source);
    m_source = nullptr;
    m_sourceData.clear();
    if (data.isEmpty()) {
        setSource(nullptr);
        Clipboard::changed().emit(m_mode);
        return;
    }

    m_sourceData = data.entries();
    // Text goes out under every text type, so any reader finds one it takes.
    if (const std::optional<std::string> text = data.text()) {
        for (const char* type : kTextMimeTypes) {
            if (!data.hasFormat(type))
                m_sourceData.emplace_back(type, *text);
        }
    }
    std::vector<std::string> types;
    types.reserve(m_sourceData.size());
    for (const auto& [type, bytes] : m_sourceData)
        types.push_back(type);
    m_source = createSource(types);
    setSource(m_source);
    m_connection.flush();
    Clipboard::changed().emit(m_mode);
}

std::vector<std::string> Selection::mimeTypes() const
{
    std::vector<std::string> types;
    if (m_source) {
        for (const auto& [type, bytes] : m_sourceData)
            types.push_back(type);
        return types;
    }
    if (const std::vector<std::string>* offered = offerTypes(m_selection))
        return *offered;
    return types;
}

std::string Selection::textMimeType() const
{
    const std::vector<std::string> types = mimeTypes();
    for (const char* candidate : kTextMimeTypes) {
        if (std::find(types.begin(), types.end(), candidate) != types.end())
            return candidate;
    }
    return {};
}

void Selection::request(const std::string& mimeType, std::function<void(std::optional<std::string>)> done)
{
    // Our own data needs no round trip through the compositor.
    if (m_source) {
        std::optional<std::string> data;
        for (const auto& [type, bytes] : m_sourceData) {
            if (type == mimeType) {
                data = bytes;
                break;
            }
        }
        m_loop.post([done = std::move(done), data = std::move(data)]() mutable { done(std::move(data)); });
        return;
    }
    const std::vector<std::string>* types = offerTypes(m_selection);
    int fds[2];
    if (!types || std::find(types->begin(), types->end(), mimeType) == types->end()
        || pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) {
        m_loop.post([done = std::move(done)] { done(std::nullopt); });
        return;
    }
    receive(m_selection, mimeType.c_str(), fds[1]);
    close(fds[1]);
    m_connection.flush();
    readFromPipe(m_loop, fds[0], std::move(done));
}

void Selection::offerAnnounced(void* offer)
{
    m_offers[offer];
}

void Selection::offerMimeType(void* offer, const char* mimeType)
{
    m_offers[offer].push_back(mimeType);
}

void Selection::selectionChanged(void* offer)
{
    if (m_selection && m_selection != offer) {
        m_offers.erase(m_selection);
        destroyOffer(m_selection);
    }
    m_selection = offer;
    // Our own source coming back as an offer changes nothing.
    if (!m_source)
        Clipboard::changed().emit(m_mode);
}

void Selection::sourceSend(void* source, const char* mimeType, int fd)
{
    if (source != m_source) {
        close(fd);
        return;
    }
    for (const auto& [type, bytes] : m_sourceData) {
        if (type == mimeType) {
            writeToPipe(m_loop, fd, bytes);
            return;
        }
    }
    close(fd);
}

void Selection::sourceCancelled(void* source)
{
    destroySource(source);
    if (source != m_source)
        return;
    m_source = nullptr;
    m_sourceData.clear();
    // The selection event with the new owner's offer tells the change, now
    // or when the keyboard comes back.
}

void Selection::forgetOffer(void* offer)
{
    m_offers.erase(offer);
}

const std::vector<std::string>* Selection::offerTypes(void* offer) const
{
    if (!offer)
        return nullptr;
    const auto it = m_offers.find(offer);
    return it == m_offers.end() ? nullptr : &it->second;
}

} // namespace Lingmo::ui::wayland
