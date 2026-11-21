// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <string>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/thread_pool.h"

namespace Lingmo::Shell {

// The Options menu and what the capture interface remembers, in the
// [Screenshot] group of shell.conf.
// Pictures and recordings have separate destinations; only pictures can go
// to the clipboard. Rectangles are base::Settings rect values. Read on
// the file thread; the defaults stand until isReady().
class CaptureSettings
{
public:
    // Destinations; "custom" is the folder chosen with Other Location.
    static constexpr const char *kDefault = "default";
    static constexpr const char *kDesktop = "desktop";
    static constexpr const char *kDocuments = "documents";
    static constexpr const char *kClipboard = "clipboard";
    static constexpr const char *kCustom = "custom";

    // `path` is shell.conf.
    explicit CaptureSettings(std::string path);

    bool isReady() const { return m_ready; }

    const std::string &imageDestination() const { return m_imageDestination; }
    void setImageDestination(const std::string &destination);
    const std::string &videoDestination() const { return m_videoDestination; }
    void setVideoDestination(const std::string &destination);

    // Where a picture or a recording is written; empty for the clipboard.
    std::string imageFolder() const;
    std::string videoFolder() const;

    const std::string &customImageFolder() const { return m_customImageFolder; }
    const std::string &customVideoFolder() const { return m_customVideoFolder; }
    // Other Location: the folder becomes the destination.
    void setCustomFolder(bool video, const std::string &folder);

    // Seconds before a capture; zero for none.
    int timer() const { return m_timer; }
    void setTimer(int seconds);

    // A PipeWire node name; empty records no sound.
    const std::string &microphone() const { return m_microphone; }
    void setMicrophone(const std::string &node);

    bool showPointer() const { return m_showPointer; }
    void setShowPointer(bool show);
    bool recordPointer() const { return m_recordPointer; }
    void setRecordPointer(bool show);

    bool rememberSelection() const { return m_rememberSelection; }
    void setRememberSelection(bool remember);

    // The last selected portion, in global logical coordinates, and the mode
    // the toolbar was last left in.
    base::Rect lastSelection() const { return m_lastSelection; }
    void setLastSelection(const base::Rect &selection);
    int lastType() const { return m_lastType; }
    int lastSource() const { return m_lastSource; }
    void setLastMode(int type, int source);

    // Where the toolbar was last dragged to, in global logical coordinates;
    // empty while it has never been moved.
    base::Rect toolbarGeometry() const { return m_toolbarGeometry; }
    void setToolbarGeometry(const base::Rect &geometry);

    // A value the Options menu shows changed, or the file was read.
    base::Signal<> changed;

    struct Values {
        std::string imageDestination = kDefault;
        std::string videoDestination = kDefault;
        std::string customImageFolder;
        std::string customVideoFolder;
        int timer = 0;
        std::string microphone;
        bool showPointer = false;
        bool recordPointer = true;
        bool rememberSelection = true;
        base::Rect lastSelection;
        base::Rect toolbarGeometry;
        // Image and Region: a picture of a portion.
        int lastType = 0;
        int lastSource = 2;
    };
    static Values load(const std::string &path);

private:
    std::string folderFor(const std::string &destination, const std::string &custom, bool video) const;
    void write(const std::string &key, const std::string &value);
    void write(const std::string &key, int64_t value);
    void write(const std::string &key, bool value);
    void writeRect(const std::string &key, const base::Rect &rect);

    const std::string m_path;
    bool m_ready = false;
    std::string m_imageDestination = kDefault;
    std::string m_videoDestination = kDefault;
    std::string m_customImageFolder;
    std::string m_customVideoFolder;
    int m_timer = 0;
    std::string m_microphone;
    bool m_showPointer = false;
    bool m_recordPointer = true;
    bool m_rememberSelection = true;
    base::Rect m_lastSelection;
    base::Rect m_toolbarGeometry;
    int m_lastType = 0;
    int m_lastSource = 2;
    base::Job m_load;
};

} // namespace Lingmo::Shell

