namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ui/file_drop_area.h"

#include <utility>

#include "base/url.h"
#include "ui/drag.h"

namespace Lingmo::ui {

static constexpr const char* kUriList = "text/uri-list";

FileDropArea::FileDropArea() = default;

FileDropArea::~FileDropArea() = default;

void FileDropArea::layout()
{
    for (const auto& child : children())
        child->setGeometry(localRect());
}

std::vector<std::string> FileDropArea::wanted(const std::string& uriList) const
{
    std::vector<std::string> urls;
    for (std::string& url : base::url::parseUriList(uriList)) {
        if (!filter || filter(url))
            urls.push_back(std::move(url));
    }
    return urls;
}

uint32_t FileDropArea::action(const DropOffer& offer) const
{
    const uint32_t allowed = offer.sourceActions();
    if (allowed & m_action)
        return m_action;
    for (uint32_t action : {CopyAction, MoveAction, AskAction}) {
        if (allowed & action)
            return action;
    }
    return NoAction;
}

void FileDropArea::setContainsDrag(bool contains)
{
    if (m_containsDrag == contains)
        return;
    m_containsDrag = contains;
    if (onContainsDragChanged)
        onContainsDragChanged();
}

bool FileDropArea::canDrop(const DropOffer& offer) const
{
    return offer.hasMimeType(kUriList);
}

void FileDropArea::dragEnter(const DragEvent& event)
{
    // A new drag: reads of an earlier one no longer count.
    m_drag = std::make_shared<int>(0);
    m_urls.reset();
    setContainsDrag(true);
    event.offer->read(kUriList, [this, drag = std::weak_ptr<int>(m_drag)](std::string data) {
        const std::shared_ptr<int> current = drag.lock();
        if (!current)
            return;
        m_urls = wanted(data);
        // Dropped before the list arrived: delivered now.
        if (*current && !m_urls->empty() && onDropped)
            onDropped(*m_urls);
    });
}

uint32_t FileDropArea::dragUpdate(const DragEvent& event)
{
    // Refused once the list shows nothing wanted; taken until then.
    if (m_urls && m_urls->empty())
        return NoAction;
    return action(*event.offer);
}

void FileDropArea::dragLeave()
{
    m_drag = std::make_shared<int>(0);
    m_urls.reset();
    setContainsDrag(false);
}

uint32_t FileDropArea::drop(const DragEvent& event)
{
    const uint32_t taken = action(*event.offer);
    setContainsDrag(false);
    if (!m_urls) {
        // The list is on its way; the read reports it.
        *m_drag = 1;
        return taken;
    }
    const std::vector<std::string> urls = std::move(*m_urls);
    m_urls.reset();
    m_drag = std::make_shared<int>(0);
    if (urls.empty())
        return NoAction;
    if (onDropped)
        onDropped(urls);
    return taken;
}

} // namespace Lingmo::ui
