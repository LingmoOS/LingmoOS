// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "preview/previewcontroller.h"
#include "core/fileitem.h"
#include "core/standardfolders.h"
#include "preview/imageinfo.h"

#include "base/application_registry.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "base/mime_database.h"
#include "base/url.h"

#include <dirent.h>
#include <fstream>
#include <sys/stat.h>
#include <unicode/ucnv.h>
#include <unicode/ucsdet.h>

#include <algorithm>

using Lingmo::i18n::tr;

namespace fm {

// The text item lays out everything it holds when it is shown.
static constexpr size_t kTextLimit = 256 * 1024;

static bool validUtf8(std::string_view text)
{
    size_t i = 0;
    while (i < text.size()) {
        const unsigned char c = text[i];
        size_t length = c < 0x80 ? 1 : (c >> 5) == 6 ? 2 : (c >> 4) == 14 ? 3 : (c >> 3) == 30 ? 4 : 0;
        if (length == 0 || i + length > text.size())
            return false;
        for (size_t k = 1; k < length; ++k) {
            if ((static_cast<unsigned char>(text[i + k]) & 0xc0) != 0x80)
                return false;
        }
        i += length;
    }
    return true;
}

// Text in an ICU converter's encoding, as UTF-8; nullopt when it cannot be read.
static std::optional<std::string> convert(std::string_view data, const char* encoding)
{
    UErrorCode status = U_ZERO_ERROR;
    std::string out(data.size() * 3 + 16, '\0');
    const int32_t length = ucnv_convert("UTF-8", encoding, out.data(), int32_t(out.size()), data.data(),
                                        int32_t(data.size()), &status);
    if (U_FAILURE(status))
        return std::nullopt;
    out.resize(size_t(length));
    return out;
}

// UTF-8 with what is not valid replaced by U+FFFD.
static std::string repairedUtf8(std::string_view data)
{
    if (auto converted = convert(data, "UTF-8"))
        return *converted;
    return std::string(data);
}

// A decoder for text that is neither UTF-8 nor marked by a byte order mark
// (GBK, Shift JIS, Big5, Windows code pages), guessed from byte statistics.
static std::optional<std::string> decodeLegacy(std::string_view data)
{
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCharsetDetector, decltype(&ucsdet_close)> detector(ucsdet_open(&status), ucsdet_close);
    ucsdet_setText(detector.get(), data.data(), int32_t(data.size()), &status);
    int32_t count = 0;
    const UCharsetMatch** matches = ucsdet_detectAll(detector.get(), &count, &status);
    if (U_FAILURE(status))
        return std::nullopt;

    // A few characters score the same in several encodings; the UI language settles a tie.
    const std::string language = base::Locale::system().language();
    std::string best;
    int32_t bestConfidence = 0;
    bool bestInLanguage = false;
    for (int32_t i = 0; i < count; ++i) {
        status = U_ZERO_ERROR;
        const std::string name = ucsdet_getName(matches[i], &status);
        const int32_t confidence = ucsdet_getConfidence(matches[i], &status);
        const char* matchLanguage = ucsdet_getLanguage(matches[i], &status);
        const bool inLanguage = matchLanguage && language == matchLanguage;
        // Without a byte order mark, NUL bytes already decided UTF-16 and UTF-32.
        if (U_FAILURE(status) || name.starts_with("UTF-"))
            continue;
        if (confidence > bestConfidence || (confidence == bestConfidence && inLanguage && !bestInLanguage)) {
            best = name;
            bestConfidence = confidence;
            bestInLanguage = inLanguage;
        }
    }
    if (best.empty())
        return std::nullopt;
    return convert(data, best.c_str());
}

// The start of a text file, or nothing for content that is not text.
static std::optional<std::string> readText(const std::string& path, bool* truncated)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return std::nullopt;
    std::string data(kTextLimit + 1, '\0');
    file.read(data.data(), std::streamsize(data.size()));
    data.resize(size_t(file.gcount()));
    *truncated = data.size() > kTextLimit;
    if (*truncated) {
        data.resize(kTextLimit);
        // Ends on a whole line where there is one.
        const size_t end = data.rfind('\n');
        if (end != std::string::npos && end > 0)
            data.resize(end + 1);
    }

    const char* bom = nullptr;
    size_t bomLength = 0;
    if (data.starts_with("\xef\xbb\xbf")) {
        bom = "UTF-8";
        bomLength = 3;
    } else if (data.starts_with(std::string_view("\xff\xfe\0\0", 4))) {
        bom = "UTF-32LE";
        bomLength = 4;
    } else if (data.starts_with(std::string_view("\0\0\xfe\xff", 4))) {
        bom = "UTF-32BE";
        bomLength = 4;
    } else if (data.starts_with("\xff\xfe")) {
        bom = "UTF-16LE";
        bomLength = 2;
    } else if (data.starts_with("\xfe\xff")) {
        bom = "UTF-16BE";
        bomLength = 2;
    }
    // Only UTF-16 and UTF-32 text holds NUL bytes.
    if (!bom && data.find('\0') != std::string::npos)
        return std::nullopt;

    std::string text;
    if (bom) {
        text = convert(std::string_view(data).substr(bomLength), bom).value_or(std::string());
    } else if (validUtf8(data)) {
        text = data;
    } else if (auto legacy = decodeLegacy(data)) {
        text = *legacy;
    } else {
        text = repairedUtf8(data);
    }

    std::string normalized;
    normalized.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            normalized += '\n';
            if (i + 1 < text.size() && text[i + 1] == '\n')
                ++i;
        } else {
            normalized += text[i];
        }
    }
    return normalized;
}

PreviewFile describeFile(const std::string& url)
{
    PreviewFile result;
    result.url = url;
    result.name = fileName(url);
    const auto local = base::url::toLocalFile(url);
    if (!local) {
        result.message = tr("Preview is unavailable for this location.");
        return result;
    }

    const std::string path = cleanPath(*local);
    result.name = fileName(path).empty() ? path : fileName(path);
    struct stat st;
    if (::stat(path.c_str(), &st) != 0 || (!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode))) {
        result.message = tr("This file is no longer available.");
        return result;
    }

    const std::string mime = base::mime::typeForFile(path);
    result.type = mimeComment(mime);
    result.icon = base::mime::iconName(mime);
    result.size = formatSize(uint64_t(st.st_size));
    if (S_ISDIR(st.st_mode)) {
        result.folder = true;
        result.icon = "folder";
        uint64_t count = 0;
        if (DIR* directory = ::opendir(path.c_str())) {
            while (dirent* entry = ::readdir(directory)) {
                const std::string_view name = entry->d_name;
                if (name != "." && name != "..")
                    ++count;
            }
            ::closedir(directory);
        }
        result.itemCount = count;
        return result;
    }

    if (mime.starts_with("image/")) {
        const ImageInfo info = readImageInfo(path, mime);
        if (info.readable) {
            result.kind = PreviewFile::Kind::Image;
            result.width = info.width;
            result.height = info.height;
            result.vector = info.vector;
            result.animated = info.animated;
        }
    } else if (base::mime::inherits(mime, "text/plain")) {
        bool truncated = false;
        if (auto text = readText(path, &truncated)) {
            result.kind = PreviewFile::Kind::Text;
            result.text = std::move(*text);
            result.truncated = truncated;
        }
    }
    result.mimeType = mime;
    return result;
}

static uint64_t totalSize(const std::string& path, const std::atomic_bool& cancelled)
{
    struct stat st;
    if (::lstat(path.c_str(), &st) != 0)
        return 0;
    uint64_t total = uint64_t(st.st_size);
    if (!S_ISDIR(st.st_mode))
        return total;
    if (DIR* directory = ::opendir(path.c_str())) {
        while (dirent* entry = ::readdir(directory)) {
            if (cancelled.load())
                break;
            const std::string name = entry->d_name;
            if (name != "." && name != "..")
                total += totalSize(path + '/' + name, cancelled);
        }
        ::closedir(directory);
    }
    return total;
}

PreviewController::PreviewController()
{
    // Scans the desktop entries in the background, for the "Open with" button.
    m_registryChanged = base::ApplicationRegistry::instance().changed.connect([this] {
        if (m_visible && !m_loading && !m_file.application && updateApplication())
            fileChanged.emit();
    });

    m_showTimer.setSingleShot(true);
    m_showTimer.setInterval(std::chrono::milliseconds(250));
    m_showTimer.onTimeout = [this] {
        // Open at the size of what is shown, unless describing it stalls.
        if (m_showPending) {
            m_showPending = false;
            showRequested.emit();
        }
    };
}

PreviewController::~PreviewController()
{
    stopFolderSize();
}

void PreviewController::close()
{
    if (!m_visible)
        return;
    m_visible = false;
    m_showPending = false;
    m_showTimer.stop();
    ++m_generation;
    m_describe = {};
    stopFolderSize();
    m_loading = false;
    changed.emit();
    fileChanged.emit();
    closed.emit();
}

void PreviewController::present(std::vector<PreviewItem> items, int currentIndex, bool activate, double ratio)
{
    if (items.empty())
        return;
    if (ratio > 0)
        m_sourceRatio = ratio;
    const bool wasVisible = m_visible;
    m_visible = true;
    if (items != m_items || !wasVisible) {
        const std::string previous = m_index < int(m_items.size()) ? m_items[size_t(m_index)].url : std::string();
        m_items = std::move(items);
        m_index = std::clamp(currentIndex, 0, int(m_items.size()) - 1);
        if (wasVisible && !activate) {
            for (size_t i = 0; i < m_items.size(); ++i) {
                if (m_items[i].url == previous)
                    m_index = int(i);
            }
        }
        changed.emit();
        load();
    }
    if (activate) {
        if (m_loading) {
            m_showPending = true;
            m_showTimer.start();
        } else {
            showRequested.emit();
        }
    }
}

void PreviewController::select(int index)
{
    if (!m_visible || index < 0 || index >= int(m_items.size()) || index == m_index)
        return;
    m_index = index;
    changed.emit();
    load();
}

void PreviewController::step(int delta)
{
    select(m_index + delta);
}

void PreviewController::reload()
{
    if (m_visible && m_index >= 0 && m_index < int(m_items.size()))
        load();
}

void PreviewController::openCurrent()
{
    if (!m_visible || !m_file.application)
        return;
    const auto path = base::url::toLocalFile(m_file.url);
    if (!path)
        return;
    if (launcher::launchApplication(m_file.application->desktopFile, *path))
        close();
}

void PreviewController::load()
{
    stopFolderSize();
    const PreviewItem& item = m_items[size_t(m_index)];
    const std::string url = item.url;
    const uint64_t generation = ++m_generation;
    if (m_file.url.empty()) {
        m_file.url = item.url;
        m_file.name = item.name;
        m_file.icon = item.icon;
    }
    m_loading = true;
    fileChanged.emit();
    m_describe = base::ThreadPool::global().run([url] { return describeFile(url); },
                                                [this, generation](PreviewFile file) {
                                                    if (generation != m_generation || !m_visible)
                                                        return;
                                                    m_file = std::move(file);
                                                    updateApplication();
                                                    m_loading = false;
                                                    fileChanged.emit();
                                                    if (m_showPending) {
                                                        m_showPending = false;
                                                        m_showTimer.stop();
                                                        showRequested.emit();
                                                    }
                                                    if (m_file.folder) {
                                                        if (const auto path = base::url::toLocalFile(m_file.url))
                                                            loadFolderSize(*path);
                                                    }
                                                });
}

// The application registry belongs to this thread, not the describing one.
bool PreviewController::updateApplication()
{
    if (m_file.folder || m_file.mimeType.empty())
        return false;
    const auto path = base::url::toLocalFile(m_file.url);
    const auto applications = launcher::applicationsFor(path.value_or(std::string()), m_file.mimeType);
    if (applications.empty())
        return false;
    m_file.application = applications.front();
    return true;
}

void PreviewController::stopFolderSize()
{
    if (m_sizeCancelled)
        m_sizeCancelled->store(true);
    m_sizeCancelled.reset();
    m_sizeJob = {};
}

void PreviewController::loadFolderSize(const std::string& path)
{
    m_sizeCancelled = std::make_shared<std::atomic_bool>(false);
    m_sizeJob = base::ThreadPool::global().run([path, cancelled = m_sizeCancelled] { return totalSize(path, *cancelled); },
                                               [this](uint64_t total) {
                                                   m_file.size = formatSize(total);
                                                   fileChanged.emit();
                                               });
}

} // namespace fm
