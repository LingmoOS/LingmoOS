// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/clipboard/clipboard_monitor.h"

#include <algorithm>
#include <cerrno>

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/strings.h"
#include "clipboard/selection.h"
#include "clipboard/stored_source.h"

namespace Lingmo::Shell {

// The text types offered for a text, as wl-copy offers them, so X clients
// find one through the selection bridge too.
static const std::vector<std::string> &textAliases()
{
    static const std::vector<std::string> aliases = {"text/plain;charset=utf-8", "text/plain", "UTF8_STRING",
                                                     "STRING", "TEXT"};
    return aliases;
}

static bool isTextAlias(const std::string &type)
{
    const auto &aliases = textAliases();
    return std::find(aliases.begin(), aliases.end(), type) != aliases.end();
}

static std::string textOf(const ClipboardPayload &payload)
{
    for (const auto &[type, data] : payload) {
        if (type == "STRING")
            return base::strings::latin1ToUtf8(data);
        if (isTextAlias(type))
            return data;
    }
    return {};
}

ClipboardMonitor::ClipboardMonitor(clipboard::SelectionController &selection)
    : m_selection(selection)
{
    m_deadline.setSingleShot(true);
    m_deadline.onTimeout = [this] { cancelRead(); };
    m_changed = m_selection.selectionChanged.connect([this] { onSelectionChanged(); });
}

ClipboardMonitor::~ClipboardMonitor()
{
    cancelRead();
}

bool ClipboardMonitor::setSelection(const ClipboardPayload &payload, const std::string &key)
{
    if (payload.empty())
        return false;

    auto source = std::make_shared<clipboard::StoredSource>();
    std::vector<std::string> types;
    for (const auto &[type, data] : payload) {
        if (isTextAlias(type)) {
            for (const std::string &alias : textAliases())
                types.push_back(alias);
        } else {
            types.push_back(type);
        }
    }
    types.push_back(kEntryType);
    types.push_back(kServicesType);

    const std::string text = textOf(payload);
    for (const std::string &type : types) {
        if (source->formats().end() != std::find(source->formats().begin(), source->formats().end(), type))
            continue;
        source->offer(type);
        if (type == kEntryType) {
            source->payloads[type] = key;
        } else if (type == kServicesType) {
            source->payloads[type] = "1";
        } else if (isTextAlias(type)) {
            source->payloads[type] = text;
        } else {
            for (const auto &[format, data] : payload) {
                if (format == type) {
                    source->payloads[type] = data;
                    break;
                }
            }
        }
    }

    m_ownKey = key;
    return m_selection.setSource(source);
}

void ClipboardMonitor::onSelectionChanged()
{
    cancelRead();
    const std::shared_ptr<clipboard::DataSource> source = m_selection.source();
    // A cleared selection leaves the history as it is: the file manager
    // clears the clipboard after pasting a cut.
    if (!source)
        return;

    const std::vector<std::string> &types = source->formats();
    if (std::find(types.begin(), types.end(), kEntryType) != types.end()) {
        if (!m_ownKey.empty())
            ownSelectionShown(m_ownKey);
        return;
    }

    m_pendingFormats = ClipboardStore::formatsToKeep(types);
    if (m_pendingFormats.empty())
        return;
    m_source = source;
    m_deadline.start(kReadDeadline);
    readNext();
}

void ClipboardMonitor::readNext()
{
    if (m_pendingFormats.empty()) {
        m_deadline.stop();
        const ClipboardPayload payload = std::exchange(m_payload, {});
        m_readSize = 0;
        m_source.reset();
        copied(payload);
        return;
    }

    m_readingFormat = m_pendingFormats.front();
    m_pendingFormats.erase(m_pendingFormats.begin());
    const std::shared_ptr<clipboard::DataSource> source = m_source.lock();
    int fds[2];
    if (!source || !source->alive() || pipe2(fds, O_CLOEXEC | O_NONBLOCK) < 0) {
        cancelRead();
        return;
    }
    // The source takes the write end.
    source->send(m_readingFormat, fds[1]);
    m_readFd = fds[0];
    m_buffer.clear();
    m_readWatch = base::EventLoop::current()->watch(m_readFd, EPOLLIN, [this](uint32_t) { onReadable(); });
}

void ClipboardMonitor::onReadable()
{
    char chunk[64 * 1024];
    for (;;) {
        const ssize_t result = read(m_readFd, chunk, sizeof(chunk));
        if (result > 0) {
            m_readSize += result;
            // A cut-off copy is dropped, not kept in part.
            if (m_readSize > ClipboardStore::kMaxPayloadSize) {
                cancelRead();
                return;
            }
            m_buffer.append(chunk, size_t(result));
            continue;
        }
        if (result == 0) {
            finishFormat();
            return;
        }
        if (errno == EINTR)
            continue;
        if (errno != EAGAIN)
            cancelRead();
        return;
    }
}

void ClipboardMonitor::finishFormat()
{
    base::EventLoop::current()->unwatch(std::exchange(m_readWatch, 0));
    close(std::exchange(m_readFd, -1));
    m_payload.emplace_back(m_readingFormat, std::exchange(m_buffer, {}));
    readNext();
}

void ClipboardMonitor::cancelRead()
{
    m_deadline.stop();
    if (m_readWatch)
        base::EventLoop::current()->unwatch(std::exchange(m_readWatch, 0));
    if (m_readFd >= 0)
        close(std::exchange(m_readFd, -1));
    m_pendingFormats.clear();
    m_payload.clear();
    m_buffer.clear();
    m_readSize = 0;
    m_source.reset();
}

} // namespace Lingmo::Shell
