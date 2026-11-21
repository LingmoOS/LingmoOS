// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "document.h"
#include "fileoptions.h"
#include "session.h"
#include "unicode.h"

#include "base/elapsed_timer.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/uuid.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std::chrono_literals;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace fs = std::filesystem;
namespace strings = Lingmo::base::strings;

struct LoadResult
{
    DecodedText decoded;
    int error = Document::NoNotice;
    std::string errorString;
    DiskStamp stamp;
    bool cancelled = false;
};

static std::function<void(const std::string &)> s_recentFilesHook;

void Document::setRecentFilesHook(std::function<void(const std::string &)> hook)
{
    s_recentFilesHook = std::move(hook);
}

// The file through symbolic links: what they point at.
static std::optional<struct stat> statOf(const std::string &path)
{
    struct stat info;
    if (::stat(path.c_str(), &info) != 0)
        return std::nullopt;
    return info;
}

static bool exists(const std::string &path)
{
    return statOf(path).has_value();
}

static DiskStamp stampOf(const std::string &path)
{
    DiskStamp stamp;
    if (const auto info = statOf(path)) {
        stamp.modified = info->st_mtim;
        stamp.size = int64_t(info->st_size);
    }
    return stamp;
}

static std::string absolutePath(const std::string &path)
{
    if (path.empty())
        return path;
    std::error_code error;
    return fs::absolute(fs::path(path), error).lexically_normal().string();
}

static std::string parentOf(const std::string &path)
{
    return fs::path(path).parent_path().string();
}

static std::string errorText(int error)
{
    return std::strerror(error);
}

static LoadResult readFile(const std::string &path, const std::string &encoding,
                           const std::shared_ptr<std::atomic<int>> &progress,
                           const std::shared_ptr<std::atomic<bool>> &cancel)
{
    LoadResult result;
    const auto info = statOf(path);
    if (!info) {
        result.error = Document::LoadFailed;
        result.errorString = tr("The file does not exist.");
        return result;
    }
    if (S_ISDIR(info->st_mode)) {
        result.error = Document::LoadFailed;
        result.errorString = tr("This is a folder.");
        return result;
    }
    if (info->st_size > Document::maximumFileSize) {
        result.error = Document::TooLarge;
        return result;
    }
    const int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        const int error = errno;
        result.error = error == EACCES || error == EPERM ? Document::PermissionDenied : Document::LoadFailed;
        result.errorString = errorText(error);
        return result;
    }
    result.stamp.modified = info->st_mtim;
    result.stamp.size = int64_t(info->st_size);

    std::string data;
    const int64_t expected = std::max<int64_t>(1, info->st_size);
    data.reserve(size_t(info->st_size));
    static constexpr size_t chunk = 1024 * 1024;
    // Sizes of special files (e.g. in /proc) are 0; they are read to the end anyway.
    while (true) {
        if (cancel->load()) {
            ::close(fd);
            result.cancelled = true;
            return result;
        }
        const size_t before = data.size();
        data.resize(before + chunk);
        const ssize_t got = ::read(fd, data.data() + before, chunk);
        if (got < 0 && errno == EINTR) {
            data.resize(before);
            continue;
        }
        if (got < 0) {
            const int error = errno;
            ::close(fd);
            result.error = Document::LoadFailed;
            result.errorString = errorText(error);
            return result;
        }
        data.resize(before + size_t(got));
        if (got == 0)
            break;
        if (int64_t(data.size()) > Document::maximumFileSize) {
            ::close(fd);
            result.error = Document::TooLarge;
            return result;
        }
        progress->store(int(std::min<int64_t>(900, int64_t(data.size()) * 900 / expected)));
    }
    ::close(fd);

    result.decoded = TextCodec::decode(data, encoding);
    progress->store(1000);
    return result;
}

bool Document::isTooLongToHighlight(const std::string &line)
{
    // UTF-8 takes at most three bytes per UTF-16 unit: a long line needs no count.
    return line.size() > size_t(maximumHighlightedLine)
            && (line.size() > size_t(maximumHighlightedLine) * 3 || Unicode::utf16Length(line) > size_t(maximumHighlightedLine));
}

void SyntaxStates::reset(const Language *language, int lineCount)
{
    m_language = language;
    m_end.assign(size_t(lineCount), 0);
    m_valid = 0;
    m_changedEnd = -1;
    m_resume = -1;
}

void SyntaxStates::linesChanged(int line, int removed, int added)
{
    const int oldValid = m_valid;
    if (removed > added)
        m_end.erase(m_end.begin() + line + added, m_end.begin() + line + removed);
    else if (added > removed)
        m_end.insert(m_end.begin() + line + removed, size_t(added - removed), 0);
    if (line >= oldValid)
        return;

    m_valid = line;
    m_changedEnd = line + added - 1;
    const int resume = oldValid >= line + removed ? oldValid + added - removed : -1;
    m_resume = std::max(m_resume >= line + removed ? m_resume + added - removed : -1, resume);
}

Syntax::State SyntaxStates::before(int line) const
{
    if (line <= 0 || line - 1 >= int(m_end.size()))
        return 0;
    return m_end[size_t(line - 1)];
}

int SyntaxStates::advance(const TextBuffer &buffer, int line, int64_t budgetMs, int *last)
{
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    const int target = std::min(line, buffer.lineCount() - 1);
    int first = -1;
    std::vector<SyntaxSpan> spans;
    while (m_valid <= target) {
        const int i = m_valid;
        const std::string &text = buffer.line(i);
        spans.clear();
        const Syntax::State entering = before(i);
        const Syntax::State state = Document::isTooLongToHighlight(text)
                ? entering : Syntax::highlight(m_language, text, entering, &spans);
        const Syntax::State old = m_end[size_t(i)];
        m_end[size_t(i)] = state;
        m_valid = i + 1;
        if (state != old && i + 1 < buffer.lineCount()) {
            if (first < 0)
                first = i + 1;
            if (last)
                *last = i + 1;
        }
        if (m_resume > 0 && i >= m_changedEnd && state == old) {
            m_valid = std::min<int>(m_resume, int(m_end.size()));
            m_resume = -1;
        }
        if ((i & 63) == 0 && timer.elapsedMs() >= budgetMs)
            break;
    }
    return first;
}

Document::Document(const std::string &draftId)
    : m_draftId(draftId.empty() ? Lingmo::base::Uuid::generate().toString() : draftId)
{
    m_states.reset(nullptr, 1);

    m_highlightTimer.setInterval(0ms);
    m_highlightTimer.setSingleShot(true);
    m_highlightTimer.onTimeout = [this] { highlightSlice(); };

    // Drafts are written a moment after typing stops, never to the file itself.
    m_draftTimer.setInterval(3000ms);
    m_draftTimer.setSingleShot(true);
    m_draftTimer.onTimeout = [this] { writeDraftAsync(); };

    m_diskCheckTimer.setInterval(200ms);
    m_diskCheckTimer.setSingleShot(true);
    m_diskCheckTimer.onTimeout = [this] { checkDisk(); };

    m_matchTimer.setInterval(60ms);
    m_matchTimer.setSingleShot(true);
    m_matchTimer.onTimeout = [this] { computeMatches(); };

    m_progressTimer.setInterval(100ms);
    m_progressTimer.onTimeout = [this] {
        if (!m_loadProgressShared)
            return;
        const double progress = m_loadProgressShared->load() / 1000.0;
        if (progress != m_loadProgress) {
            m_loadProgress = progress;
            loadProgressChanged();
        }
    };
}

Document::~Document()
{
    if (m_loadCancel)
        m_loadCancel->store(true);
    m_loadJob.cancel();
    waitForDraftWrite();
}

std::string Document::draftDirectory()
{
    return Session::dataDirectory() + "/drafts";
}

void Document::setDraftId(const std::string &id)
{
    if (id.empty() || id == m_draftId)
        return;
    m_draftId = id;
    draftIdChanged();
}

std::string Document::draftPath() const
{
    return draftDirectory() + "/" + m_draftId + ".txt";
}

// The bytes of the first `units` UTF-16 code units of `text`.
static size_t utf16Prefix(std::string_view text, size_t units)
{
    size_t i = 0;
    size_t counted = 0;
    while (i < text.size() && counted < units) {
        counted += static_cast<unsigned char>(text[i]) >= 0xF0 ? 2 : 1;
        i = Unicode::next(text, i);
    }
    return i;
}

std::string Document::title() const
{
    if (!m_filePath.empty())
        return fileName();
    // Named after its first line until it is saved.
    const std::string &line = m_buffer.line(0);
    const std::string first = strings::simplified(std::string_view(line).substr(0, utf16Prefix(line, 200)));
    if (first.empty())
        return tr("Untitled");
    if (Unicode::utf16Length(first) > 40)
        return first.substr(0, utf16Prefix(first, 39)) + "…";
    return first;
}

std::string Document::fileName() const
{
    return fs::path(m_filePath).filename().string();
}

std::string Document::folder() const
{
    if (m_filePath.empty())
        return std::string();
    std::string dir = parentOf(m_filePath);
    const std::string home = Lingmo::base::paths::home();
    if (dir == home || strings::startsWith(dir, home + "/"))
        dir.replace(0, home.size(), "~");
    return dir;
}

void Document::setFilePath(const std::string &path)
{
    const std::string absolute = absolutePath(path);
    if (m_filePath == absolute)
        return;
    m_filePath = absolute;
    watchFile();
    filePathChanged();
    titleChanged();
}

void Document::watchFile()
{
    // The watch is on the folder for the file's name, so a file replaced by
    // renaming another over it, or deleted and made again, is still followed.
    if (m_watchedFile == m_filePath && (m_watch.isActive() || m_filePath.empty()))
        return;
    m_watch.reset();
    m_watchedFile = m_filePath;
    if (!m_watchedFile.empty())
        m_watch = Lingmo::base::FileWatch::file(m_watchedFile, [this] { m_diskCheckTimer.start(); });
}

void Document::updateWritable()
{
    bool writable = true;
    if (!m_filePath.empty())
        writable = exists(m_filePath) ? ::access(m_filePath.c_str(), W_OK) == 0
                                      : ::access(parentOf(m_filePath).c_str(), W_OK) == 0;
    if (writable != m_writable) {
        m_writable = writable;
        writableChanged();
    }
}

void Document::load(const std::string &path, const std::string &encoding, bool addToRecent)
{
    cancelLoading();
    setFilePath(path);
    m_addToRecent = addToRecent;
    m_loading = true;
    m_loadProgress = 0;
    loadingChanged();
    loadProgressChanged();

    m_loadProgressShared = std::make_shared<std::atomic<int>>(0);
    m_loadCancel = std::make_shared<std::atomic<bool>>(false);
    m_loadJob = Lingmo::base::runInThread(
            [path = m_filePath, encoding, progress = m_loadProgressShared, cancel = m_loadCancel] {
                return std::make_shared<LoadResult>(readFile(path, encoding, progress, cancel));
            },
            [this](std::shared_ptr<LoadResult> result) {
                if (!result->cancelled)
                    finishLoad(*result);
            });
    m_progressTimer.start();
}

bool Document::waitForLoaded(int timeoutMs)
{
    Lingmo::base::EventLoop *loop = Lingmo::base::EventLoop::current();
    Lingmo::base::ElapsedTimer timer;
    timer.start();
    while (m_loading && loop && timer.elapsedMs() < timeoutMs) {
        pollfd descriptor{loop->fd(), POLLIN, 0};
        ::poll(&descriptor, 1, 20);
        loop->dispatch();
    }
    return !m_loading;
}

void Document::cancelLoading()
{
    if (!m_loading)
        return;
    if (m_loadCancel)
        m_loadCancel->store(true);
    m_loadJob.cancel();
    m_progressTimer.stop();
    m_loading = false;
    m_reloading = false;
    loadingChanged();
}

void Document::reload()
{
    if (m_filePath.empty())
        return;
    m_reloading = true;
    m_reloadCursor = m_cursor;
    load(m_filePath, m_encoding);
}

void Document::reloadWithEncoding(const std::string &encoding)
{
    if (m_filePath.empty()) {
        setEncoding(encoding);
        return;
    }
    m_reloading = true;
    m_reloadCursor = m_cursor;
    load(m_filePath, encoding);
}

void Document::finishLoad(LoadResult &result)
{
    m_progressTimer.stop();
    m_loading = false;
    const bool reloading = m_reloading;
    m_reloading = false;

    if (result.error != NoNotice) {
        loadingChanged();
        // Unsaved text of a file that can no longer be read still comes back.
        if (m_restoreDraftAfterLoad) {
            m_restoreDraftAfterLoad = false;
            if (restoreDraft()) {
                m_diskDiffers = true;
                updateModified();
            }
        }
        setNotice(result.error == LoadFailed && !exists(m_filePath) && m_modified ? Deleted : Notice(result.error),
                  result.errorString);
        return;
    }

    DecodedText &decoded = result.decoded;
    // Reloading the same encoding is one more undo step; another encoding's text
    // must not come back through undo.
    if (reloading && !m_locked && !decoded.binary && !decoded.malformed && !m_malformed
        && decoded.encoding == m_encoding && result.stamp.size <= largeFileSize && m_buffer.length() <= largeFileSize) {
        replaceWithReloaded(decoded.lines);
    } else {
        const int oldCount = m_buffer.lineCount();
        m_buffer.setLines(std::move(decoded.lines));
        // Views follow the buffer before anything else can make them look at it.
        m_states.reset(nullptr, m_buffer.lineCount());
        m_cursor = m_anchor = reloading ? m_buffer.clamp(m_reloadCursor) : TextPosition();
        linesChanged(0, oldCount, m_buffer.lineCount());
        m_undo.clear();
    }
    m_formatModified = false;
    m_diskDiffers = false;
    m_ignoreDisk = false;
    m_diskStamp = result.stamp;
    m_encoding = decoded.encoding;
    m_bom = decoded.bom;
    m_lineEnding = decoded.lineEnding;

    const bool large = result.stamp.size > largeFileSize;
    if (large != m_large) {
        m_large = large;
        largeChanged();
    }
    m_highlightingEnabled = !large;
    if (!m_languageChosen)
        m_language = Syntax::detect(m_filePath, m_buffer.line(0));
    m_states.reset(Syntax::language(m_language), m_buffer.lineCount());

    if (!m_indentationFromFile || !reloading)
        applyEditorConfig();

    if (m_locked != decoded.binary) {
        m_locked = decoded.binary;
        lockedChanged();
    }

    updateWritable();
    watchFile();

    loadingChanged();
    encodingChanged();
    lineEndingChanged();
    languageChanged();
    titleChanged();
    undoChanged();
    cursorChanged();
    contentsChanged();
    updateModified();
    updateMatches();

    m_malformed = decoded.malformed;
    m_loadedEncoding = decoded.encoding;
    if (decoded.binary)
        setNotice(Binary);
    else if (decoded.malformed)
        setNotice(Malformed, m_encoding);
    else if (large && m_states.language())
        setNotice(LargeFile);
    else if (!m_writable)
        setNotice(ReadOnly);
    else if (decoded.mixedLineEndings)
        setNotice(MixedLineEndings);
    else
        setNotice(NoNotice);

    if (m_restoreDraftAfterLoad) {
        m_restoreDraftAfterLoad = false;
        restoreDraft();
    } else if (!reloading) {
        removeDraft();
    }

    if (!reloading && m_addToRecent && s_recentFilesHook)
        s_recentFilesHook(m_filePath);
    loaded();
}

void Document::replaceWithReloaded(const std::vector<std::string> &lines)
{
    const std::string before = m_buffer.text();
    const std::string after = strings::join(lines, "\n");
    const size_t shorter = std::min(before.size(), after.size());
    size_t head = 0;
    while (head < shorter && before[head] == after[head])
        ++head;
    // Both texts share the bytes, so a character boundary in one is one in the other.
    head = Unicode::boundary(before, head);
    size_t tail = 0;
    while (tail < shorter - head && before[before.size() - 1 - tail] == after[after.size() - 1 - tail])
        ++tail;
    while (tail > 0 && (static_cast<unsigned char>(before[before.size() - tail]) & 0xC0) == 0x80)
        --tail;

    const TextPosition cursor = m_reloadCursor;
    if (head + tail < before.size() || head + tail < after.size()) {
        const EditStep step{m_buffer.position(int64_t(head)), before.substr(head, before.size() - head - tail),
                            after.substr(head, after.size() - head - tail)};
        m_undo.seal();
        m_undo.record(step, UndoGroup::Other, {m_cursor, m_anchor}, {cursor, cursor},
                      std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch()).count());
        m_undo.seal();
        applyStep(step, false);
    }
    m_cursor = m_anchor = m_buffer.clamp(cursor);
    m_undo.setClean();
}

void Document::applyEditorConfig()
{
    const EditorConfig config = m_filePath.empty() ? EditorConfig() : EditorConfig::forFile(m_filePath);
    m_indentationFromFile = false;
    if (config.insertSpaces || config.indentSize) {
        m_insertSpaces = config.insertSpaces.value_or(m_insertSpaces);
        m_indentWidth = config.indentSize.value_or(m_indentWidth);
        m_tabWidth = config.tabWidth.value_or(m_insertSpaces ? m_tabWidth : m_indentWidth);
        m_indentationFromFile = true;
    } else if (const auto detected = FileOptions::detectIndentation(m_buffer.lines())) {
        m_insertSpaces = detected->insertSpaces;
        if (detected->insertSpaces)
            m_indentWidth = detected->width;
        else
            m_indentWidth = m_tabWidth;
        m_indentationFromFile = true;
    }
    m_trimTrailingWhitespace = config.trimTrailingWhitespace.value_or(false);
    m_finalNewline = config.insertFinalNewline;
    indentationChanged();
}

void Document::setIndentationDefaults(bool insertSpaces, int width)
{
    if (m_indentationFromFile)
        return;
    m_insertSpaces = insertSpaces;
    m_tabWidth = width;
    m_indentWidth = width;
    indentationChanged();
}

void Document::setNotice(Notice notice, const std::string &detail)
{
    if (m_notice == notice && m_noticeDetail == detail)
        return;
    m_notice = notice;
    m_noticeDetail = detail;
    noticeChanged();
}

void Document::dismissNotice()
{
    setNotice(NoNotice);
}

void Document::keepLocalVersion()
{
    // The disk copy is no longer what the buffer started from.
    m_ignoreDisk = true;
    m_diskDiffers = true;
    m_diskStamp = stampOf(m_filePath);
    updateModified();
    setNotice(NoNotice);
}

void Document::setLocked(bool locked)
{
    if (m_locked == locked)
        return;
    m_locked = locked;
    lockedChanged();
    if (!locked && m_notice == Binary)
        setNotice(NoNotice);
}

void Document::updateModified()
{
    const bool modified = !m_undo.isClean() || m_formatModified || m_diskDiffers;
    if (modified == m_modified)
        return;
    m_modified = modified;
    modifiedChanged();
}

void Document::setEncoding(const std::string &encoding)
{
    if (encoding == m_encoding || !TextCodec::isSupported(encoding))
        return;
    m_encoding = encoding;
    if (!strings::startsWith(m_encoding, "UTF-"))
        m_bom = false;
    m_formatModified = true;
    encodingChanged();
    updateModified();
    scheduleDraft();
}

void Document::setBom(bool bom)
{
    if (m_bom == bom || (bom && !strings::startsWith(m_encoding, "UTF-")))
        return;
    m_bom = bom;
    m_formatModified = true;
    encodingChanged();
    updateModified();
}

void Document::setLineEnding(int lineEnding)
{
    if (lineEnding < 0 || lineEnding > int(LineEnding::ClassicMac) || LineEnding(lineEnding) == m_lineEnding)
        return;
    m_lineEnding = LineEnding(lineEnding);
    m_formatModified = true;
    lineEndingChanged();
    updateModified();
    if (m_notice == MixedLineEndings)
        setNotice(NoNotice);
}

std::string Document::languageName() const
{
    return Syntax::name(m_language);
}

void Document::setLanguage(const std::string &language)
{
    m_languageChosen = true;
    const bool enabling = !m_highlightingEnabled;
    m_highlightingEnabled = true;
    if (m_language == language && !enabling)
        return;
    m_language = language;
    m_states.reset(Syntax::language(language), m_buffer.lineCount());
    languageChanged();
    syntaxChanged(0, m_buffer.lineCount() - 1);
    if (m_notice == LargeFile)
        setNotice(NoNotice);
}

void Document::detectLanguage()
{
    if (m_languageChosen)
        return;
    const std::string detected = Syntax::detect(m_filePath, m_buffer.line(0));
    if (detected == m_language)
        return;
    m_language = detected;
    m_states.reset(Syntax::language(detected), m_buffer.lineCount());
    languageChanged();
    syntaxChanged(0, m_buffer.lineCount() - 1);
}

void Document::setInsertSpaces(bool insertSpaces)
{
    if (m_insertSpaces == insertSpaces)
        return;
    m_insertSpaces = insertSpaces;
    m_indentationFromFile = true;
    indentationChanged();
}

void Document::setTabWidth(int width)
{
    width = std::clamp(width, 1, 16);
    if (m_tabWidth == width)
        return;
    m_tabWidth = width;
    m_indentationFromFile = true;
    indentationChanged();
}

void Document::setIndentWidth(int width)
{
    width = std::clamp(width, 1, 16);
    if (m_indentWidth == width)
        return;
    m_indentWidth = width;
    m_indentationFromFile = true;
    indentationChanged();
}

void Document::requestHighlighting(int line)
{
    if (!isHighlighting())
        return;
    m_highlightTarget = std::min(line, m_buffer.lineCount() - 1);
    if (m_states.validLines() <= m_highlightTarget && !m_highlightTimer.isActive())
        m_highlightTimer.start();
}

void Document::highlightSlice()
{
    if (!isHighlighting())
        return;
    int last = -1;
    const int first = m_states.advance(m_buffer, m_highlightTarget, 6, &last);
    if (first >= 0)
        syntaxChanged(first, last);
    if (m_states.validLines() <= m_highlightTarget)
        m_highlightTimer.start();
}

void Document::beginEdit()
{
    m_undo.begin({m_cursor, m_anchor});
}

void Document::endEdit()
{
    m_undo.end({m_cursor, m_anchor});
    undoChanged();
}

void Document::applyStep(const EditStep &step, bool reverse)
{
    const std::string &remove = reverse ? step.inserted : step.removed;
    const std::string &insert = reverse ? step.removed : step.inserted;
    const TextRange range{step.at, TextBuffer::endOf(step.at, remove)};
    m_buffer.remove(range);
    const TextPosition end = m_buffer.insert(step.at, insert);
    const int removedLines = range.end.line - range.start.line + 1;
    const int addedLines = end.line - step.at.line + 1;
    m_states.linesChanged(step.at.line, removedLines, addedLines);
    linesChanged(step.at.line, removedLines, addedLines);
    if (step.at.line == 0) {
        titleChanged();
        if (m_filePath.empty())
            detectLanguage();
    }
}

static int64_t now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
}

void Document::replace(const TextRange &range, std::string_view input, UndoGroup::Kind kind, const Selection &after)
{
    if (m_locked || m_loading)
        return;
    const std::string text = Unicode::isValid(input) ? std::string(input) : Unicode::sanitized(input);
    const TextRange r{m_buffer.clamp(range.normalized().start), m_buffer.clamp(range.normalized().end)};
    std::string removed = m_buffer.text(r);
    if (removed.empty() && text.empty()) {
        setCursor(after.cursor, after.anchor);
        return;
    }
    const EditStep step{r.start, std::move(removed), text};
    m_undo.record(step, kind, {m_cursor, m_anchor}, after, now());
    applyStep(step, false);

    m_cursor = m_buffer.clamp(after.cursor);
    m_anchor = m_buffer.clamp(after.anchor);
    cursorChanged();
    undoChanged();
    contentsChanged();
    updateModified();
    scheduleDraft();
    updateMatches();
    revealCursor();
}

void Document::undo()
{
    if (!m_undo.canUndo() || m_locked)
        return;
    const UndoGroup group = m_undo.undo();
    for (auto it = group.steps.crbegin(); it != group.steps.crend(); ++it)
        applyStep(*it, true);
    m_cursor = m_buffer.clamp(group.before.cursor);
    m_anchor = m_buffer.clamp(group.before.anchor);
    cursorChanged();
    undoChanged();
    contentsChanged();
    updateModified();
    scheduleDraft();
    updateMatches();
    revealCursor();
}

void Document::redo()
{
    if (!m_undo.canRedo() || m_locked)
        return;
    const UndoGroup group = m_undo.redo();
    for (const EditStep &step : group.steps)
        applyStep(step, false);
    m_cursor = m_buffer.clamp(group.after.cursor);
    m_anchor = m_buffer.clamp(group.after.anchor);
    cursorChanged();
    undoChanged();
    contentsChanged();
    updateModified();
    scheduleDraft();
    updateMatches();
    revealCursor();
}

void Document::setText(std::string_view text)
{
    const TextPosition end = TextBuffer::endOf({0, 0}, text);
    replace({{0, 0}, m_buffer.end()}, text, UndoGroup::Other, {end, end});
}

void Document::setCursor(const TextPosition &cursor, const TextPosition &anchor)
{
    const TextPosition c = m_buffer.clamp(cursor);
    const TextPosition a = m_buffer.clamp(anchor);
    if (c == m_cursor && a == m_anchor)
        return;
    m_cursor = c;
    m_anchor = a;
    m_undo.seal();
    cursorChanged();
    updateMatches();
}

int Document::visualColumn() const
{
    const std::string &line = m_buffer.line(m_cursor.line);
    int column = 0;
    for (size_t i = 0; i < size_t(m_cursor.column) && i < line.size(); i = Unicode::next(line, i)) {
        if (line[i] == '\t')
            column += m_tabWidth - column % m_tabWidth;
        else
            ++column;
    }
    return column + 1;
}

int64_t Document::selectionLength() const
{
    if (!hasSelection())
        return 0;
    const TextRange r = selection();
    if (r.start.line == r.end.line)
        return int64_t(Unicode::utf16Length(std::string_view(m_buffer.line(r.start.line))
                                                    .substr(size_t(r.start.column), size_t(r.end.column - r.start.column))));
    int64_t length = int64_t(Unicode::utf16Length(std::string_view(m_buffer.line(r.start.line)).substr(size_t(r.start.column))));
    for (int line = r.start.line + 1; line < r.end.line; ++line)
        length += 1 + int64_t(Unicode::utf16Length(m_buffer.line(line)));
    length += 1 + int64_t(Unicode::utf16Length(std::string_view(m_buffer.line(r.end.line)).substr(0, size_t(r.end.column))));
    return length;
}

std::string Document::selectedText() const
{
    return m_buffer.text(selection());
}

void Document::selectAll()
{
    setCursor(m_buffer.end(), {0, 0});
}

void Document::goToLine(int line, int column)
{
    const int index = std::clamp(line - 1, 0, m_buffer.lineCount() - 1);
    // The column counts characters; positions are bytes.
    const std::string &text = m_buffer.line(index);
    size_t offset = 0;
    for (int i = 1; i < column && offset < text.size(); ++i)
        offset = Unicode::next(text, offset);
    setCursor({index, int(offset)});
    revealCursor();
}

void Document::insertText(std::string_view input, bool overwrite)
{
    std::string text = strings::replaceAll(input, "\r\n", "\n");
    std::replace(text.begin(), text.end(), '\r', '\n');
    TextRange range = selection();
    const bool singleLine = text.find('\n') == std::string::npos;
    if (overwrite && range.isEmpty() && singleLine) {
        // As many characters as are typed.
        const std::string &line = m_buffer.line(m_cursor.line);
        size_t end = size_t(m_cursor.column);
        for (size_t i = 0; i < text.size() && end < line.size(); i = Unicode::next(text, i))
            end = Unicode::next(line, end);
        range.end.column = int(end);
    }
    const TextPosition end = TextBuffer::endOf(range.start, text);
    const bool typed = Unicode::utf16Length(text) <= 2 && singleLine;
    replace(range, text, typed ? UndoGroup::Typing : UndoGroup::Other, {end, end});
}

void Document::insertNewline(bool autoIndent)
{
    std::string text = "\n";
    if (autoIndent) {
        const std::string &line = m_buffer.line(selection().start.line);
        size_t indent = 0;
        while (indent < line.size() && int(indent) < selection().start.column && (line[indent] == ' ' || line[indent] == '\t'))
            ++indent;
        text.append(line, 0, indent);
    }
    const TextRange range = selection();
    const TextPosition end = TextBuffer::endOf(range.start, text);
    replace(range, text, UndoGroup::Other, {end, end});
}

static bool isWordCharacter(char32_t c)
{
    return Unicode::isLetterOrNumber(c) || c == '_';
}

TextPosition Document::wordBoundary(const TextPosition &from, bool forward) const
{
    const TextPosition p = m_buffer.clamp(from);
    const std::string &line = m_buffer.line(p.line);
    const size_t size = line.size();
    if (forward) {
        if (size_t(p.column) >= size)
            return p.line + 1 < m_buffer.lineCount() ? TextPosition{p.line + 1, 0} : p;
        size_t i = size_t(p.column);
        while (i < size && Unicode::isSpace(Unicode::at(line, i)))
            i = Unicode::next(line, i);
        if (i < size && isWordCharacter(Unicode::at(line, i))) {
            while (i < size && isWordCharacter(Unicode::at(line, i)))
                i = Unicode::next(line, i);
        } else {
            while (i < size && !isWordCharacter(Unicode::at(line, i)) && !Unicode::isSpace(Unicode::at(line, i)))
                i = Unicode::next(line, i);
        }
        return {p.line, int(i)};
    }

    if (p.column == 0)
        return p.line > 0 ? TextPosition{p.line - 1, int(m_buffer.line(p.line - 1).size())} : p;
    size_t i = size_t(p.column);
    while (i > 0 && Unicode::isSpace(Unicode::before(line, i)))
        i = Unicode::previous(line, i);
    if (i > 0 && isWordCharacter(Unicode::before(line, i))) {
        while (i > 0 && isWordCharacter(Unicode::before(line, i)))
            i = Unicode::previous(line, i);
    } else {
        while (i > 0 && !isWordCharacter(Unicode::before(line, i)) && !Unicode::isSpace(Unicode::before(line, i)))
            i = Unicode::previous(line, i);
    }
    return {p.line, int(i)};
}

TextRange Document::wordAt(const TextPosition &position) const
{
    const TextPosition p = m_buffer.clamp(position);
    const std::string &line = m_buffer.line(p.line);
    auto same = [](char32_t c, char32_t reference) {
        if (isWordCharacter(reference))
            return isWordCharacter(c);
        if (Unicode::isSpace(reference))
            return Unicode::isSpace(c);
        return !isWordCharacter(c) && !Unicode::isSpace(c);
    };
    if (line.empty())
        return {p, p};
    const char32_t reference = size_t(p.column) < line.size() ? Unicode::at(line, size_t(p.column))
                                                              : Unicode::before(line, size_t(p.column));
    size_t start = size_t(p.column);
    size_t end = size_t(p.column);
    while (start > 0 && same(Unicode::before(line, start), reference))
        start = Unicode::previous(line, start);
    while (end < line.size() && same(Unicode::at(line, end), reference))
        end = Unicode::next(line, end);
    return {{p.line, int(start)}, {p.line, int(end)}};
}

void Document::deleteBackward(bool word)
{
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    TextPosition start;
    if (word) {
        start = wordBoundary(m_cursor, false);
    } else if (m_cursor.column > 0) {
        const std::string &line = m_buffer.line(m_cursor.line);
        int column = int(Unicode::previous(line, size_t(m_cursor.column)));
        // In leading spaces, Backspace goes back to the previous indentation stop.
        if (m_insertSpaces && line[size_t(column)] == ' ') {
            const bool leading = std::all_of(line.begin(), line.begin() + m_cursor.column, [](char c) { return c == ' '; });
            if (leading)
                column = ((m_cursor.column - 1) / m_indentWidth) * m_indentWidth;
        }
        start = {m_cursor.line, column};
    } else if (m_cursor.line > 0) {
        start = {m_cursor.line - 1, int(m_buffer.line(m_cursor.line - 1).size())};
    } else {
        return;
    }
    replace({start, m_cursor}, std::string(), UndoGroup::Backspace, {start, start});
}

void Document::deleteForward(bool word)
{
    if (hasSelection()) {
        deleteSelection();
        return;
    }
    TextPosition end;
    const std::string &line = m_buffer.line(m_cursor.line);
    if (word) {
        end = wordBoundary(m_cursor, true);
    } else if (size_t(m_cursor.column) < line.size()) {
        end = {m_cursor.line, int(Unicode::next(line, size_t(m_cursor.column)))};
    } else if (m_cursor.line + 1 < m_buffer.lineCount()) {
        end = {m_cursor.line + 1, 0};
    } else {
        return;
    }
    const TextPosition at = m_cursor;
    replace({at, end}, std::string(), UndoGroup::Delete, {at, at});
}

void Document::deleteSelection()
{
    if (!hasSelection())
        return;
    const TextRange range = selection();
    replace(range, std::string(), UndoGroup::Other, {range.start, range.start});
}

// The lines a line-wise command works on: the selected ones, without a
// last line the selection only touches at its start.
static std::pair<int, int> selectedLines(const TextRange &selection)
{
    int last = selection.end.line;
    if (last > selection.start.line && selection.end.column == 0)
        --last;
    return {selection.start.line, last};
}

void Document::indentSelection(bool unindent)
{
    const TextRange range = selection();
    const auto [first, last] = selectedLines(range);
    const std::string indent = m_insertSpaces ? std::string(size_t(m_indentWidth), ' ') : std::string("\t");
    const TextPosition oldCursor = m_cursor;
    const TextPosition oldAnchor = m_anchor;
    int cursorShift = 0;
    int anchorShift = 0;

    beginEdit();
    for (int line = first; line <= last; ++line) {
        const std::string &text = m_buffer.line(line);
        int shift = 0;
        if (unindent) {
            int remove = 0;
            if (!text.empty() && text[0] == '\t')
                remove = 1;
            else
                while (remove < m_indentWidth && remove < int(text.size()) && text[size_t(remove)] == ' ')
                    ++remove;
            if (remove == 0)
                continue;
            replace({{line, 0}, {line, remove}}, std::string(), UndoGroup::Other, {m_cursor, m_anchor});
            shift = -remove;
        } else {
            if (text.empty() && first != last)
                continue;
            replace({{line, 0}, {line, 0}}, indent, UndoGroup::Other, {m_cursor, m_anchor});
            shift = int(indent.size());
        }
        if (line == oldCursor.line)
            cursorShift = shift;
        if (line == oldAnchor.line)
            anchorShift = shift;
    }
    m_cursor = m_buffer.clamp({oldCursor.line, std::max(0, oldCursor.column + cursorShift)});
    m_anchor = m_buffer.clamp({oldAnchor.line, std::max(0, oldAnchor.column + anchorShift)});
    if (oldAnchor.column == 0 && oldAnchor != oldCursor)
        m_anchor.column = 0;
    if (oldCursor.column == 0 && oldAnchor != oldCursor)
        m_cursor.column = 0;
    endEdit();
    cursorChanged();
}

void Document::moveLines(bool up)
{
    const auto [first, last] = selectedLines(selection());
    if ((up && first == 0) || (!up && last + 1 >= m_buffer.lineCount()))
        return;
    std::string block;
    for (int line = first; line <= last; ++line) {
        if (line > first)
            block += '\n';
        block += m_buffer.line(line);
    }
    const int delta = up ? -1 : 1;
    const Selection after{{m_cursor.line + delta, m_cursor.column}, {m_anchor.line + delta, m_anchor.column}};
    if (up) {
        const std::string above = m_buffer.line(first - 1);
        replace({{first - 1, 0}, {last, int(m_buffer.line(last).size())}}, block + '\n' + above, UndoGroup::Other, after);
    } else {
        const std::string below = m_buffer.line(last + 1);
        replace({{first, 0}, {last + 1, int(below.size())}}, below + '\n' + block, UndoGroup::Other, after);
    }
}

void Document::duplicateLines()
{
    const auto [first, last] = selectedLines(selection());
    std::string block;
    for (int line = first; line <= last; ++line) {
        block += '\n';
        block += m_buffer.line(line);
    }
    const int count = last - first + 1;
    const Selection after{{m_cursor.line + count, m_cursor.column}, {m_anchor.line + count, m_anchor.column}};
    const TextPosition end{last, int(m_buffer.line(last).size())};
    replace({end, end}, block, UndoGroup::Other, after);
}

void Document::deleteLines()
{
    const auto [first, last] = selectedLines(selection());
    TextRange range;
    if (last + 1 < m_buffer.lineCount())
        range = {{first, 0}, {last + 1, 0}};
    else if (first > 0)
        range = {{first - 1, int(m_buffer.line(first - 1).size())}, {last, int(m_buffer.line(last).size())}};
    else
        range = {{0, 0}, {last, int(m_buffer.line(last).size())}};
    const int line = std::min(first, m_buffer.lineCount() - 1 - (last - first + 1));
    const TextPosition at{std::max(0, line), 0};
    replace(range, std::string(), UndoGroup::Other, {at, at});
}

void Document::changeCase(int change)
{
    TextRange range = selection();
    if (range.isEmpty())
        range = wordAt(m_cursor);
    if (range.isEmpty())
        return;
    const std::string text = m_buffer.text(range);
    std::string result;
    if (change == Upper) {
        result = Unicode::toUpper(text);
    } else if (change == Lower) {
        result = Unicode::toLower(text);
    } else {
        const std::string lower = Unicode::toLower(text);
        bool start = true;
        for (size_t i = 0; i < lower.size(); i = Unicode::next(lower, i)) {
            char32_t c = Unicode::at(lower, i);
            if (Unicode::isLetter(c)) {
                if (start)
                    c = Unicode::toTitleCase(c);
                start = false;
            } else {
                start = !Unicode::isLetterOrNumber(c) && c != '\'';
            }
            Unicode::append(result, c);
        }
    }
    if (result == text)
        return;
    // Case changes can change lengths (e.g. "ß" to "SS").
    const TextPosition end = TextBuffer::endOf(range.start, result);
    const bool forward = m_anchor <= m_cursor;
    replace(range, result, UndoGroup::Other, forward ? Selection{end, range.start} : Selection{range.start, end});
}

TextRange Document::matchingBracket(const TextPosition &position) const
{
    static constexpr std::string_view open = "([{";
    static constexpr std::string_view close = ")]}";
    const TextRange none{{-1, -1}, {-1, -1}};
    if (m_large)
        return none;
    const TextPosition p = m_buffer.clamp(position);
    const std::string &line = m_buffer.line(p.line);

    // The bracket after the cursor first, then the one before it.
    for (int column : {p.column, p.column - 1}) {
        if (column < 0 || column >= int(line.size()))
            continue;
        const char c = line[size_t(column)];
        size_t kind = open.find(c);
        const bool forward = kind != std::string_view::npos;
        if (!forward)
            kind = close.find(c);
        if (kind == std::string_view::npos)
            continue;
        const char self = c;
        const char other = forward ? close[kind] : open[kind];

        int depth = 0;
        int scanned = 0;
        int l = p.line;
        int i = column;
        while (scanned < 50000) {
            const std::string &text = m_buffer.line(l);
            while (forward ? i < int(text.size()) : i >= 0) {
                const char ch = text[size_t(i)];
                // Brackets are ASCII, so a byte at a time finds them; other
                // characters count once, as before.
                if (ch == self) {
                    ++depth;
                } else if (ch == other && --depth == 0) {
                    return {{p.line, column}, {l, i}};
                }
                i += forward ? 1 : -1;
                if ((static_cast<unsigned char>(ch) & 0xC0) != 0x80)
                    ++scanned;
            }
            l += forward ? 1 : -1;
            if (l < 0 || l >= m_buffer.lineCount() || std::abs(l - p.line) > 2000)
                return none;
            i = forward ? 0 : int(m_buffer.line(l).size()) - 1;
        }
        return none;
    }
    return none;
}

static void countText(std::string_view text, int64_t *characters, int64_t *nonSpace, int64_t *words)
{
    bool inWord = false;
    for (size_t i = 0; i < text.size(); i = Unicode::next(text, i)) {
        const char32_t c = Unicode::at(text, i);
        ++*characters;
        if (Unicode::isSpace(c)) {
            inWord = false;
            continue;
        }
        ++*nonSpace;
        // Ideographs count as words of their own.
        if (Unicode::isIdeograph(c)) {
            ++*words;
            inWord = false;
        } else if (!inWord) {
            ++*words;
            inWord = true;
        }
    }
}

DocumentStatistics Document::statistics() const
{
    DocumentStatistics result;
    for (int i = 0; i < m_buffer.lineCount(); ++i) {
        countText(m_buffer.line(i), &result.characters, &result.charactersWithoutSpaces, &result.words);
        if (i > 0)
            ++result.characters;
    }
    result.lines = m_buffer.lineCount();
    if (hasSelection()) {
        int64_t nonSpace = 0;
        countText(selectedText(), &result.selectionCharacters, &nonSpace, &result.selectionWords);
        const TextRange r = selection();
        result.hasSelection = true;
        result.selectionLines = r.end.line - r.start.line + 1;
    }
    return result;
}

void Document::setSearch(const std::string &text, bool caseSensitive, bool wholeWords, bool regex, bool inSelection)
{
    m_searcher.setPattern(text, {caseSensitive, wholeWords, regex});
    if (!inSelection)
        m_searchScope = TextRange();
    else if (hasSelection() && m_searchScope.isEmpty())
        m_searchScope = selection();
    searchChanged();
    computeMatches();
}

void Document::clearSearch()
{
    m_searcher.setPattern(std::string(), {});
    m_searchScope = TextRange();
    searchChanged();
    computeMatches();
}

void Document::updateMatches()
{
    // Counting walks the whole text; edits and cursor moves are batched.
    if (m_searcher.isValid() || m_matchCount != 0)
        m_matchTimer.start();
}

static bool contains(const std::vector<TextRange> &ranges, const TextRange &range)
{
    return std::find(ranges.begin(), ranges.end(), range) != ranges.end();
}

void Document::computeMatches()
{
    m_matchTimer.stop();
    int count = 0;
    int index = 0;
    if (m_searcher.isValid()) {
        count = m_searcher.count(m_buffer, m_searchScope);
        // The current match is the selected one.
        const TextRange current = selection();
        if (!current.isEmpty()) {
            const TextRange scope = m_searchScope.isEmpty() ? TextRange{{0, 0}, m_buffer.end()} : m_searchScope;
            const TextRange before{scope.start, current.start};
            const int ahead = before.end <= before.start ? 0 : m_searcher.count(m_buffer, before);
            if (contains(m_searcher.matchesInLine(m_buffer.line(current.start.line), current.start.line), current))
                index = ahead + 1;
        }
    }
    if (count != m_matchCount || index != m_matchIndex) {
        m_matchCount = count;
        m_matchIndex = index;
        matchesChanged();
    }
}

bool Document::findNext(bool backwards)
{
    if (!m_searcher.isValid())
        return false;
    const TextRange current = selection();
    const TextPosition from = backwards ? current.start : current.end;
    const TextRange match = m_searcher.find(m_buffer, from, backwards, m_searchScope, &m_wrapped);
    if (Searcher::isNotFound(match))
        return false;
    setCursor(match.end, match.start);
    computeMatches();
    revealCursor();
    return true;
}

bool Document::findIncremental()
{
    if (!m_searcher.isValid())
        return false;
    const TextRange match = m_searcher.find(m_buffer, selection().start, false, m_searchScope, &m_wrapped);
    if (Searcher::isNotFound(match))
        return false;
    setCursor(match.end, match.start);
    computeMatches();
    revealCursor();
    return true;
}

bool Document::replaceCurrent(const std::string &replacement)
{
    if (!m_searcher.isValid())
        return false;
    const TextRange current = selection();
    if (!current.isEmpty()
            && contains(m_searcher.matchesInLine(m_buffer.line(current.start.line), current.start.line), current)) {
        const std::string text = m_searcher.replacement(m_buffer, current, replacement);
        const TextPosition end = TextBuffer::endOf(current.start, text);
        const TextRange scope = m_searchScope;
        replace(current, text, UndoGroup::Other, {end, end});
        if (!scope.isEmpty() && current.end.line == scope.end.line) {
            m_searchScope.end.column += int(text.size()) - (current.end.column - current.start.column);
            m_searchScope.end.line += int(std::count(text.begin(), text.end(), '\n'));
        }
    }
    return findNext(false);
}

int Document::replaceAll(const std::string &replacement)
{
    if (!m_searcher.isValid() || m_locked)
        return 0;
    const TextRange scope = m_searchScope.isEmpty() ? TextRange{{0, 0}, m_buffer.end()} : m_searchScope;
    int count = 0;
    beginEdit();
    // Bottom up, so earlier positions stay valid.
    TextPosition scopeEnd = scope.end;
    for (int line = scope.end.line; line >= scope.start.line; --line) {
        const std::string text = m_buffer.line(line);
        const int lower = line == scope.start.line ? scope.start.column : 0;
        const int upper = line == scope.end.line ? scope.end.column : int(text.size());
        const std::vector<TextRange> matches = m_searcher.matchesInLine(text, line, lower, upper);
        // Captures come from the line as it was found: a match can look ahead into the next one's text.
        std::vector<std::string> replacements;
        replacements.reserve(matches.size());
        for (const TextRange &match : matches)
            replacements.push_back(m_searcher.replacement(m_buffer, match, replacement));
        for (size_t i = matches.size(); i-- > 0;) {
            const TextRange &match = matches[i];
            const std::string &with = replacements[i];
            if (line == scope.end.line)
                scopeEnd.column += int(with.size()) - (match.end.column - match.start.column);
            replace(match, with, UndoGroup::Other, {m_cursor, m_anchor});
            ++count;
        }
    }
    if (!m_searchScope.isEmpty())
        m_searchScope.end = m_buffer.clamp(scopeEnd);
    endEdit();
    computeMatches();
    return count;
}

void Document::scheduleDraft()
{
    m_draftTimer.start();
}

// Written whole under a temporary name and renamed over the draft, owner only.
static void writeDraftFile(const std::string &path, const std::string &text)
{
    const std::string directory = parentOf(path);
    if (::access(directory.c_str(), F_OK) != 0) {
        std::error_code error;
        fs::create_directories(directory, error);
        ::chmod(directory.c_str(), 0700);
    }
    std::string temporary = path + ".XXXXXX";
    const int fd = ::mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0)
        return;
    ::fchmod(fd, 0600);
    std::string_view rest = text;
    bool ok = true;
    while (!rest.empty()) {
        const ssize_t written = ::write(fd, rest.data(), rest.size());
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0) {
            ok = false;
            break;
        }
        rest.remove_prefix(size_t(written));
    }
    ok = ok && ::fsync(fd) == 0;
    ::close(fd);
    if (!ok || ::rename(temporary.c_str(), path.c_str()) != 0)
        ::unlink(temporary.c_str());
}

void Document::waitForDraftWrite()
{
    if (m_draftWrite.valid())
        m_draftWrite.wait();
}

void Document::writeDraftAsync()
{
    if (!m_modified && !(m_filePath.empty() && !isEmpty())) {
        removeDraft();
        return;
    }
    // One write at a time, so an older text never lands after a newer one.
    waitForDraftWrite();
    m_draftWrite = std::async(std::launch::async, writeDraftFile, draftPath(), m_buffer.text());
}

void Document::writeDraftNow()
{
    m_draftTimer.stop();
    waitForDraftWrite();
    if (!m_modified && !(m_filePath.empty() && !isEmpty())) {
        removeDraft();
        return;
    }
    writeDraftFile(draftPath(), m_buffer.text());
}

void Document::removeDraft()
{
    m_draftTimer.stop();
    waitForDraftWrite();
    ::unlink(draftPath().c_str());
}

bool Document::restoreDraft()
{
    if (m_loading) {
        m_restoreDraftAfterLoad = true;
        return true;
    }
    std::optional<std::string> data = Lingmo::base::file::read(draftPath());
    if (!data)
        return false;
    const std::string text = Unicode::isValid(*data) ? std::move(*data) : Unicode::sanitized(*data);
    if (text == m_buffer.text()) {
        if (m_filePath.empty())
            m_diskDiffers = !text.empty();
        updateModified();
        return true;
    }
    const int oldCount = m_buffer.lineCount();
    m_buffer.setText(text);
    m_undo.clear();
    m_undo.setCleanUnreachable();
    m_states.reset(m_states.language(), m_buffer.lineCount());
    m_cursor = m_anchor = m_buffer.clamp(m_cursor);
    if (m_filePath.empty())
        detectLanguage();
    linesChanged(0, oldCount, m_buffer.lineCount());
    contentsChanged();
    titleChanged();
    undoChanged();
    cursorChanged();
    updateModified();
    if (!m_filePath.empty())
        setNotice(Restored);
    return true;
}

void Document::checkDisk()
{
    if (m_filePath.empty() || m_loading)
        return;
    watchFile();
    updateWritable();
    const DiskStamp stamp = stampOf(m_filePath);

    if (!stamp.isValid()) {
        if (m_notice != Deleted) {
            // Quitting must keep the text: the file is gone.
            m_diskDiffers = true;
            updateModified();
            setNotice(Deleted);
        }
        return;
    }

    const bool returned = m_notice == Deleted;
    if (!returned && stamp == m_diskStamp)
        return;
    if (m_ignoreDisk && !returned) {
        m_diskStamp = stamp;
        return;
    }
    if (!m_modified || (returned && m_undo.isClean() && !m_formatModified)) {
        reload();
        return;
    }
    m_diskStamp = stamp;
    setNotice(ChangedOnDisk);
}

bool Document::save()
{
    if (m_filePath.empty() || m_loading)
        return false;
    return write(m_filePath, m_encoding, m_bom);
}

bool Document::saveAs(const std::string &path)
{
    if (path.empty() || m_loading)
        return false;
    const std::string absolute = absolutePath(path);
    const bool changed = absolute != m_filePath;
    if (!write(absolute, m_encoding, m_bom))
        return false;
    if (changed) {
        setFilePath(absolute);
        updateWritable();
        if (!m_languageChosen)
            detectLanguage();
        if (s_recentFilesHook)
            s_recentFilesHook(m_filePath);
    }
    return true;
}

bool Document::saveReplacingMalformed()
{
    m_malformed = false;
    return save();
}

bool Document::saveAsUtf8()
{
    const std::string old = m_encoding;
    m_encoding = "UTF-8";
    if (!save()) {
        m_encoding = old;
        return false;
    }
    encodingChanged();
    return true;
}

// umask() can only be read by changing it, which races with the draft writer.
static mode_t currentUmask()
{
    const std::optional<std::string> status = Lingmo::base::file::read("/proc/self/status");
    const size_t start = status ? status->find("Umask:\t") : std::string::npos;
    if (start == std::string::npos)
        return 022;
    return mode_t(std::strtoul(status->c_str() + start + 7, nullptr, 8));
}

static bool writeAll(int fd, std::string_view bytes)
{
    while (!bytes.empty()) {
        const ssize_t written = ::write(fd, bytes.data(), bytes.size());
        if (written < 0 && errno == EINTR)
            continue;
        if (written <= 0)
            return false;
        bytes.remove_prefix(size_t(written));
    }
    return true;
}

// Writes over the file where it is, keeping its inode, owner and mode.
static bool writeInPlace(const std::string &target, std::string_view bytes, std::string *error)
{
    const int fd = ::open(target.c_str(), O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        if (errno != EACCES && errno != EPERM)
            *error = errorText(errno);
        return false;
    }
    const bool ok = writeAll(fd, bytes) && ::ftruncate(fd, off_t(bytes.size())) == 0 && ::fsync(fd) == 0;
    if (!ok)
        *error = errorText(errno);
    ::close(fd);
    return ok;
}

// Replaces the file atomically: a new file next to it, renamed over it, with
// its permissions and group. A file owned by someone else is written in
// place instead, as a rename would hand it to this user; so is a file in a
// folder the user cannot write to. An empty error means permission was denied.
static bool writeFile(const std::string &target, std::string_view bytes, std::string *error)
{
    struct stat existing;
    const bool fileExists = ::stat(target.c_str(), &existing) == 0;
    if (fileExists && existing.st_uid != ::geteuid())
        return writeInPlace(target, bytes, error);
    if (fileExists && ::access(target.c_str(), W_OK) != 0) {
        *error = format(tr("Existing file {path} is not writable"), {{"path", target}});
        return false;
    }

    std::string temporary = target + ".XXXXXX";
    const int fd = ::mkostemp(temporary.data(), O_CLOEXEC);
    if (fd < 0) {
        if ((errno == EACCES || errno == EPERM || errno == EROFS) && fileExists)
            return writeInPlace(target, bytes, error);
        if (errno != EACCES && errno != EPERM)
            *error = errorText(errno);
        return false;
    }
    if (fileExists) {
        ::fchmod(fd, existing.st_mode & 07777);
        if (existing.st_gid != ::getegid())
            [[maybe_unused]] const int result = ::fchown(fd, uid_t(-1), existing.st_gid);
    } else {
        // As a new file gets: 0666 less the umask.
        ::fchmod(fd, 0666 & ~currentUmask());
    }
    if (!writeAll(fd, bytes) || ::fsync(fd) != 0) {
        *error = errorText(errno);
        ::close(fd);
        ::unlink(temporary.c_str());
        return false;
    }
    ::close(fd);
    if (::rename(temporary.c_str(), target.c_str()) != 0) {
        *error = errorText(errno);
        ::unlink(temporary.c_str());
        return false;
    }
    return true;
}

bool Document::write(const std::string &path, const std::string &encoding, bool bom)
{
    const bool newFile = m_filePath.empty() || !exists(path);
    const bool samePath = !m_filePath.empty() && absolutePath(path) == m_filePath;

    // Someone else's change since it was loaded is not overwritten unasked.
    if (samePath && !newFile && !m_ignoreDisk && m_diskStamp.isValid()) {
        if (!(stampOf(path) == m_diskStamp)) {
            setNotice(ChangedOnDisk);
            return false;
        }
    }
    // Bytes that did not decode are gone from the text; saving makes it final.
    if (m_malformed) {
        setNotice(MalformedSave, m_loadedEncoding);
        return false;
    }

    // EditorConfig's whitespace rules go into the file first and into the buffer,
    // as one undo step, only once it is written.
    const bool finalNewline = m_finalNewline.value_or(newFile);
    const bool needsNewline = finalNewline && !m_buffer.line(m_buffer.lineCount() - 1).empty() && !isEmpty();
    const bool reformat = (m_trimTrailingWhitespace || needsNewline) && !m_locked;
    std::vector<std::string> trimmedLines;
    if (reformat) {
        trimmedLines = m_buffer.lines();
        if (m_trimTrailingWhitespace) {
            for (std::string &text : trimmedLines) {
                size_t end = text.size();
                while (end > 0 && (text[end - 1] == ' ' || text[end - 1] == '\t'))
                    --end;
                text.resize(end);
            }
        }
        if (needsNewline)
            trimmedLines.emplace_back();
    }
    const std::vector<std::string> &lines = reformat ? trimmedLines : m_buffer.lines();

    bool lossy = false;
    const std::string bytes = TextCodec::encode(lines, encoding, bom, m_lineEnding, &lossy);
    if (lossy) {
        setNotice(LossyEncoding, encoding);
        return false;
    }

    // Through a symbolic link to the file it points at, keeping the link, also
    // when that file does not exist yet.
    std::string target = path;
    for (int hops = 0; hops < 40; ++hops) {
        std::error_code error;
        if (!fs::is_symlink(target, error))
            break;
        if (exists(target)) {
            target = fs::canonical(target, error).string();
            break;
        }
        const fs::path link = fs::read_symlink(target, error);
        target = (link.is_absolute() ? link : fs::path(target).parent_path() / link).lexically_normal().string();
    }

    std::string error;
    if (!writeFile(target, bytes, &error)) {
        setNotice(error.empty() ? ReadOnly : SaveFailed, error);
        return false;
    }

    if (reformat) {
        beginEdit();
        if (m_trimTrailingWhitespace) {
            for (int line = 0; line < m_buffer.lineCount(); ++line) {
                const std::string &text = m_buffer.line(line);
                const int end = int(lines[size_t(line)].size());
                if (end < int(text.size())) {
                    const TextPosition cursor = m_cursor.line == line ? TextPosition{line, std::min(m_cursor.column, end)}
                                                                      : m_cursor;
                    replace({{line, end}, {line, int(text.size())}}, std::string(), UndoGroup::Other, {cursor, cursor});
                }
            }
        }
        if (needsNewline)
            replace({m_buffer.end(), m_buffer.end()}, "\n", UndoGroup::Other, {m_cursor, m_anchor});
        endEdit();
    }

    m_diskStamp = stampOf(target);
    m_undo.setClean();
    m_formatModified = false;
    m_diskDiffers = false;
    m_ignoreDisk = false;
    updateModified();
    removeDraft();
    if (m_notice != Malformed && m_notice != Binary)
        setNotice(NoNotice);
    undoChanged();
    saved();
    if (path == m_filePath && s_recentFilesHook)
        s_recentFilesHook(m_filePath);
    return true;
}
