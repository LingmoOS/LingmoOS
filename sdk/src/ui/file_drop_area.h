// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {

// Takes files dropped from other applications (text/uri-list), as a
// DropArea with keys ["text/uri-list"] reading drop.urls. Its children fill
// it; a child that takes the drag itself keeps it. The URLs are read when
// the drag enters, so the drop is refused when none of them is wanted.
class FileDropArea : public View
{
public:
    FileDropArea();
    ~FileDropArea() override;

    // Which dropped URLs are taken; unset, all of them.
    std::function<bool(const std::string& url)> filter;
    // The wanted URLs of a drop, as they were listed.
    std::function<void(const std::vector<std::string>& urls)> onDropped;
    // A drag over the area began or ended, as containsDrag.
    std::function<void()> onContainsDragChanged;

    // The action a drop takes when the source allows it, else the first one
    // it allows; CopyAction unless set.
    void setAction(uint32_t action) { m_action = action; }
    bool containsDrag() const { return m_containsDrag; }

protected:
    void layout() override;
    bool canDrop(const DropOffer& offer) const override;
    void dragEnter(const DragEvent& event) override;
    uint32_t dragUpdate(const DragEvent& event) override;
    void dragLeave() override;
    uint32_t drop(const DragEvent& event) override;

private:
    std::vector<std::string> wanted(const std::string& uriList) const;
    uint32_t action(const DropOffer& offer) const;
    void setContainsDrag(bool contains);

    uint32_t m_action = 1;
    bool m_containsDrag = false;
    // The offer's URLs once read; unset while the read is pending.
    std::optional<std::vector<std::string>> m_urls;
    // Each drag's reads report only to that drag.
    std::shared_ptr<int> m_drag = std::make_shared<int>(0);
};

} // namespace Lingmo::ui
