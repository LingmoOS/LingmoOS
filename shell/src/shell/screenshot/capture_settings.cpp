// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/screenshot/capture_settings.h"

#include <algorithm>

#include "base/paths.h"
#include "base/settings.h"
#include "base/strings.h"
#include "shell/common/file_thread.h"
#include <string_view>

namespace Lingmo::Shell {

static const char kGroup[] = "Screenshot/";

static std::string key(const char *name)
{
    return std::string(kGroup) + name;
}

static std::string validDestination(const std::string &destination, bool video)
{
    static const std::string allowed[] = {CaptureSettings::kDefault, CaptureSettings::kDesktop,
                                          CaptureSettings::kDocuments, CaptureSettings::kClipboard,
                                          CaptureSettings::kCustom};
    if (video && destination == CaptureSettings::kClipboard)
        return CaptureSettings::kDefault;
    return std::find(std::begin(allowed), std::end(allowed), destination) != std::end(allowed)
            ? destination
            : std::string(CaptureSettings::kDefault);
}

// base::Settings' own rectangle value.
static base::Rect readRect(const base::Settings &settings, const std::string &name)
{
    const std::optional<base::Settings::Rect> rect = settings.rectValue(name);
    return rect ? base::Rect(rect->x, rect->y, rect->width, rect->height) : base::Rect();
}

CaptureSettings::CaptureSettings(std::string path)
    : m_path(std::move(path))
{
    m_load = runOnFileThread([path = m_path] { return load(path); }, [this](Values values) {
        m_imageDestination = values.imageDestination;
        m_videoDestination = values.videoDestination;
        m_customImageFolder = values.customImageFolder;
        m_customVideoFolder = values.customVideoFolder;
        m_timer = values.timer;
        m_microphone = values.microphone;
        m_showPointer = values.showPointer;
        m_recordPointer = values.recordPointer;
        m_rememberSelection = values.rememberSelection;
        m_lastSelection = values.lastSelection;
        m_toolbarGeometry = values.toolbarGeometry;
        m_lastType = values.lastType;
        m_lastSource = values.lastSource;
        m_ready = true;
        changed();
    });
}

CaptureSettings::Values CaptureSettings::load(const std::string &path)
{
    const base::Settings settings(path);
    Values values;
    values.imageDestination = validDestination(settings.stringValue(key("ImageDestination")), false);
    values.videoDestination = validDestination(settings.stringValue(key("VideoDestination")), true);
    values.customImageFolder = settings.stringValue(key("CustomImageFolder"));
    values.customVideoFolder = settings.stringValue(key("CustomVideoFolder"));
    values.timer = std::max(0, int(settings.intValue(key("Timer"), 0)));
    values.microphone = settings.stringValue(key("Microphone"));
    values.showPointer = settings.boolValue(key("ShowPointer"), false);
    values.recordPointer = settings.boolValue(key("RecordPointer"), true);
    values.rememberSelection = settings.boolValue(key("RememberSelection"), true);
    values.lastSelection = readRect(settings, key("LastSelection"));
    values.toolbarGeometry = readRect(settings, key("ToolbarGeometry"));
    values.lastType = int(settings.intValue(key("LastType"), values.lastType));
    values.lastSource = int(settings.intValue(key("LastSource"), values.lastSource));
    return values;
}

void CaptureSettings::write(const std::string &name, const std::string &value)
{
    writeSettings(m_path, [name = key(name.c_str()), value](base::Settings &settings) {
        settings.setValue(name, std::string_view(value));
    });
}

void CaptureSettings::write(const std::string &name, int64_t value)
{
    writeSettings(m_path, [name = key(name.c_str()), value](base::Settings &settings) {
        settings.setValue(name, value);
    });
}

void CaptureSettings::write(const std::string &name, bool value)
{
    writeSettings(m_path, [name = key(name.c_str()), value](base::Settings &settings) {
        settings.setValue(name, value);
    });
}

void CaptureSettings::writeRect(const std::string &name, const base::Rect &rect)
{
    const base::Settings::Rect value {rect.x(), rect.y(), rect.width(), rect.height()};
    writeSettings(m_path, [name = key(name.c_str()), value](base::Settings &settings) {
        settings.setValue(name, value);
    });
}

void CaptureSettings::setImageDestination(const std::string &destination)
{
    const std::string valid = validDestination(destination, false);
    if (valid == m_imageDestination)
        return;
    m_imageDestination = valid;
    write("ImageDestination", valid);
    changed();
}

void CaptureSettings::setVideoDestination(const std::string &destination)
{
    const std::string valid = validDestination(destination, true);
    if (valid == m_videoDestination)
        return;
    m_videoDestination = valid;
    write("VideoDestination", valid);
    changed();
}

std::string CaptureSettings::imageFolder() const
{
    return folderFor(m_imageDestination, m_customImageFolder, false);
}

std::string CaptureSettings::videoFolder() const
{
    return folderFor(m_videoDestination, m_customVideoFolder, true);
}

void CaptureSettings::setCustomFolder(bool video, const std::string &folder)
{
    if (folder.empty())
        return;
    if (video) {
        m_customVideoFolder = folder;
        m_videoDestination = kCustom;
        write("CustomVideoFolder", folder);
        write("VideoDestination", m_videoDestination);
    } else {
        m_customImageFolder = folder;
        m_imageDestination = kCustom;
        write("CustomImageFolder", folder);
        write("ImageDestination", m_imageDestination);
    }
    changed();
}

void CaptureSettings::setTimer(int seconds)
{
    seconds = std::max(0, seconds);
    if (seconds == m_timer)
        return;
    m_timer = seconds;
    write("Timer", int64_t(seconds));
    changed();
}

void CaptureSettings::setMicrophone(const std::string &node)
{
    if (node == m_microphone)
        return;
    m_microphone = node;
    write("Microphone", node);
    changed();
}

void CaptureSettings::setShowPointer(bool show)
{
    if (show == m_showPointer)
        return;
    m_showPointer = show;
    write("ShowPointer", show);
    changed();
}

void CaptureSettings::setRecordPointer(bool show)
{
    if (show == m_recordPointer)
        return;
    m_recordPointer = show;
    write("RecordPointer", show);
    changed();
}

void CaptureSettings::setRememberSelection(bool remember)
{
    if (remember == m_rememberSelection)
        return;
    m_rememberSelection = remember;
    write("RememberSelection", remember);
    changed();
}

void CaptureSettings::setLastSelection(const base::Rect &selection)
{
    if (selection == m_lastSelection)
        return;
    m_lastSelection = selection;
    writeRect("LastSelection", selection);
}

void CaptureSettings::setLastMode(int type, int source)
{
    if (type == m_lastType && source == m_lastSource)
        return;
    m_lastType = type;
    m_lastSource = source;
    write("LastType", int64_t(type));
    write("LastSource", int64_t(source));
}

void CaptureSettings::setToolbarGeometry(const base::Rect &geometry)
{
    if (geometry == m_toolbarGeometry)
        return;
    m_toolbarGeometry = geometry;
    writeRect("ToolbarGeometry", geometry);
    changed();
}

std::string CaptureSettings::folderFor(const std::string &destination, const std::string &custom,
                                       bool video) const
{
    using base::paths::UserDirectory;
    if (destination == kClipboard)
        return {};
    if (destination == kDesktop)
        return base::paths::userDirectory(UserDirectory::Desktop);
    if (destination == kDocuments)
        return base::paths::userDirectory(UserDirectory::Documents);
    if (destination == kCustom && !custom.empty())
        return custom;
    if (video)
        return base::paths::userDirectory(UserDirectory::Videos);
    return base::paths::userDirectory(UserDirectory::Pictures) + "/Screenshots";
}

} // namespace Lingmo::Shell
