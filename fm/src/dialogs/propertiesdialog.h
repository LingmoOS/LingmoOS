// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/fileitem.h"
#include "dialogs/framelessdialog.h"

#include "base/thread_pool.h"

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace Lingmo::ui {
class Label;
class ProgressBar;
class SegmentedControl;
}

namespace fm {

// The item's icon and
// name, then type, location, size (counted on a worker for folders and
// several items), and its times.
class PropertiesDialog : public FramelessDialog
{
public:
    explicit PropertiesDialog(FileItemList items);
    ~PropertiesDialog() override;

    void retranslate() override;

    std::function<void()> onDone;

private:
    std::unique_ptr<ui::View> createContent();
    std::unique_ptr<ui::View> createGeneralContent();
    std::unique_ptr<ui::View> createPermissionsContent();
    void setPage(int index);

    FileItemList m_items;
    std::string m_fileName;
    std::string m_iconName;
    std::string m_mimeType;
    std::string m_location;
    std::string m_size;
    // What m_size reads, once known.
    std::optional<uint64_t> m_bytes;
    std::string m_created;
    std::string m_modified;
    std::string m_accessed;
    ui::Label* m_sizeLabel = nullptr;
    ui::ProgressBar* m_storageBar = nullptr;
    ui::SegmentedControl* m_pages = nullptr;
    ui::View* m_pageHost = nullptr;
    int m_page = 0;
    base::Job m_sizeJob;
    std::shared_ptr<std::atomic_bool> m_cancelled = std::make_shared<std::atomic_bool>(false);
};

} // namespace fm
