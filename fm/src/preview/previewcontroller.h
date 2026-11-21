// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "core/launcher.h"

#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace fm {

// One item a source asked to preview, as its snapshot lists them.
struct PreviewItem
{
    std::string url;
    std::string name;
    std::string icon;
    std::string thumbnail;

    bool operator==(const PreviewItem&) const = default;
};

// What the window shows for an item, described on a worker.
struct PreviewFile
{
    enum class Kind {
        Generic,
        Image,
        Text,
    };

    std::string url;
    std::string name;
    Kind kind = Kind::Generic;
    // Instead of the contents: the file is gone, or not local.
    std::string message;
    std::string type;
    std::string mimeType;
    std::string icon;
    std::string size;
    bool folder = false;
    uint64_t itemCount = 0;
    // Images: pixels after the EXIF orientation, 0 when unknown.
    int width = 0;
    int height = 0;
    bool vector = false;
    bool animated = false;
    // Text: the start of the file, and whether there is more.
    std::string text;
    bool truncated = false;
    // The "Open with" button's application.
    std::optional<launcher::Application> application;

    bool isImage() const { return kind == Kind::Image && width > 0 && height > 0; }
};

// The items of the source that holds the
// preview, the one shown and its description.
class PreviewController
{
public:
    PreviewController();
    ~PreviewController();

    bool isVisible() const { return m_visible; }
    const std::vector<PreviewItem>& items() const { return m_items; }
    int currentIndex() const { return m_index; }
    const PreviewFile& file() const { return m_file; }
    bool isLoading() const { return m_loading; }
    // The requesting window's device pixel ratio; 0 when it gave none.
    double sourceRatio() const { return m_sourceRatio; }

    void present(std::vector<PreviewItem> items, int currentIndex, bool activate, double ratio);
    void close();
    void select(int index);
    void step(int delta);
    void openCurrent();
    // Describes the item shown again, in the language of now.
    void reload();

    // A change of the items, the index or the visibility.
    base::Signal<> changed;
    // The description changed, or loading started or ended.
    base::Signal<> fileChanged;
    // Open the window, once the first item is described or that takes too long.
    base::Signal<> showRequested;
    base::Signal<> closed;

private:
    void load();
    bool updateApplication();
    void stopFolderSize();
    void loadFolderSize(const std::string& path);

    std::vector<PreviewItem> m_items;
    PreviewFile m_file;
    int m_index = 0;
    uint64_t m_generation = 0;
    bool m_visible = false;
    bool m_loading = false;
    bool m_showPending = false;
    double m_sourceRatio = 0;
    base::Timer m_showTimer;
    base::Job m_describe;
    base::Job m_sizeJob;
    std::shared_ptr<std::atomic_bool> m_sizeCancelled;
    base::Subscription m_registryChanged;
};

// The description of a file, read now; any thread.
PreviewFile describeFile(const std::string& url);

} // namespace fm
