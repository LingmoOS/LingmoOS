// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "searcher.h"
#include "syntax.h"
#include "textbuffer.h"
#include "textcodec.h"
#include "undostack.h"

#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/thread_pool.h"
#include "base/timer.h"

#include <atomic>
#include <ctime>
#include <functional>
#include <future>
#include <memory>
#include <optional>

// The end states of each line's highlighting, computed lazily from the top.
// After an edit only the changed lines are recomputed when the state they
// end in is the same as before.
class SyntaxStates
{
public:
    void reset(const Language *language, int lineCount);
    void linesChanged(int line, int removed, int added);

    const Language *language() const { return m_language; }
    int validLines() const { return m_valid; }
    // The state entering `line`; a guess while it is beyond validLines().
    Syntax::State before(int line) const;
    // Computes states up to `line` or until `budgetMs` runs out. Returns the
    // first line whose entering state changed, or -1, and sets `last` to the
    // last such line.
    int advance(const TextBuffer &buffer, int line, int64_t budgetMs, int *last);

private:
    const Language *m_language = nullptr;
    std::vector<Syntax::State> m_end;
    int m_valid = 0;
    // Lines from m_changedEnd on were valid before the edit; once a line there
    // ends in the state it had, everything up to m_resume is valid again.
    int m_changedEnd = -1;
    int m_resume = -1;
};

struct LoadResult;

// When a file was last seen: its modification time and size.
struct DiskStamp
{
    timespec modified{};
    int64_t size = -1;

    bool isValid() const { return size >= 0; }
    friend bool operator==(const DiskStamp &a, const DiskStamp &b)
    {
        return a.modified.tv_sec == b.modified.tv_sec && a.modified.tv_nsec == b.modified.tv_nsec && a.size == b.size;
    }
};

struct DocumentStatistics
{
    int64_t lines = 0;
    int64_t characters = 0;
    int64_t charactersWithoutSpaces = 0;
    int64_t words = 0;
    bool hasSelection = false;
    int64_t selectionLines = 0;
    int64_t selectionCharacters = 0;
    int64_t selectionWords = 0;
};

// One open text: its buffer, undo history, file, encoding, search state and
// draft. Changes are announced through the signals below.
class Document
{
public:
    enum Notice {
        NoNotice,
        Restored,
        ChangedOnDisk,
        Deleted,
        Malformed,
        Binary,
        ReadOnly,
        PermissionDenied,
        LoadFailed,
        TooLarge,
        SaveFailed,
        LossyEncoding,
        LargeFile,
        MixedLineEndings,
        MalformedSave,
    };

    enum CaseChange {
        Upper,
        Lower,
        Title,
    };

    // The biggest file that is opened; anything larger is refused.
    static constexpr int64_t maximumFileSize = 256ll * 1024 * 1024;
    // Above this, highlighting and bracket matching are off.
    static constexpr int64_t largeFileSize = 4ll * 1024 * 1024;
    // Lines longer than this, in UTF-16 units, are not highlighted.
    static constexpr int maximumHighlightedLine = 20000;
    static bool isTooLongToHighlight(const std::string &line);

    explicit Document(const std::string &draftId = std::string());
    ~Document();

    Document(const Document &) = delete;
    Document &operator=(const Document &) = delete;

    // Opens `path`; empty `encoding` detects it.
    // A tab restored from the session was opened before: it stays where it is in the recent files.
    void load(const std::string &path, const std::string &encoding = std::string(), bool addToRecent = true);
    void reload();
    void reloadWithEncoding(const std::string &encoding);
    void cancelLoading();
    // Waits for a load in progress, running the event loop, for tests and the command line.
    bool waitForLoaded(int timeoutMs = 30000);

    bool save();
    bool saveAs(const std::string &path);
    // Saves to the same file in UTF-8, after a LossyEncoding notice.
    bool saveAsUtf8();
    // Saves after a MalformedSave notice, giving up the bytes that did not decode.
    bool saveReplacingMalformed();
    void setEncoding(const std::string &encoding);
    void dismissNotice();
    void keepLocalVersion();

    std::string title() const;
    const std::string &filePath() const { return m_filePath; }
    std::string fileName() const;
    std::string folder() const;
    bool isUntitled() const { return m_filePath.empty(); }
    bool isModified() const { return m_modified; }
    bool isEmpty() const { return m_buffer.length() == 0; }
    bool isLoading() const { return m_loading; }
    double loadProgress() const { return m_loadProgress; }
    // Editing is locked, e.g. for a binary file until the user unlocks it.
    bool isLocked() const { return m_locked; }
    void setLocked(bool locked);
    // The file on disk cannot be written; Save turns into Save As.
    bool isWritable() const { return m_writable; }

    const std::string &encoding() const { return m_encoding; }
    bool hasBom() const { return m_bom; }
    void setBom(bool bom);
    int lineEnding() const { return int(m_lineEnding); }
    void setLineEnding(int lineEnding);
    const std::string &language() const { return m_language; }
    void setLanguage(const std::string &language);
    std::string languageName() const;
    bool isHighlighting() const { return m_highlightingEnabled && m_states.language(); }
    bool isLarge() const { return m_large; }
    bool insertSpaces() const { return m_insertSpaces; }
    void setInsertSpaces(bool insertSpaces);
    int tabWidth() const { return m_tabWidth; }
    void setTabWidth(int width);
    int indentWidth() const { return m_indentWidth; }
    void setIndentWidth(int width);
    // Uses the defaults when the file says nothing about its indentation.
    void setIndentationDefaults(bool insertSpaces, int width);

    bool canUndo() const { return m_undo.canUndo(); }
    bool canRedo() const { return m_undo.canRedo(); }
    void undo();
    void redo();

    const TextBuffer &buffer() const { return m_buffer; }
    int lineCount() const { return m_buffer.lineCount(); }
    std::string text() const { return m_buffer.text(); }
    // Replaces everything as one undoable step.
    void setText(std::string_view text);

    // The editing primitive: replaces `range` with `text` and records it.
    void replace(const TextRange &range, std::string_view text, UndoGroup::Kind kind, const Selection &after);
    void beginEdit();
    void endEdit();

    TextPosition cursor() const { return m_cursor; }
    TextPosition anchor() const { return m_anchor; }
    void setCursor(const TextPosition &cursor, const TextPosition &anchor);
    void setCursor(const TextPosition &cursor) { setCursor(cursor, cursor); }
    TextRange selection() const { return TextRange{m_anchor, m_cursor}.normalized(); }
    bool hasSelection() const { return m_cursor != m_anchor; }
    int cursorLine() const { return m_cursor.line; }
    int cursorColumn() const { return m_cursor.column; }
    // The column counted with tabs expanded, as shown to the user (from 1).
    int visualColumn() const;
    // In UTF-16 units, as the status bar counts.
    int64_t selectionLength() const;
    std::string selectedText() const;
    void selectAll();
    // From 1; the column in characters.
    void goToLine(int line, int column = 1);

    // Typing: replaces the selection; in overwrite mode replaces the next character.
    void insertText(std::string_view text, bool overwrite = false);
    void paste(std::string_view text) { insertText(text); }
    void insertNewline(bool autoIndent);
    void deleteBackward(bool word);
    void deleteForward(bool word);
    void deleteSelection();
    void indentSelection(bool unindent);
    void moveLines(bool up);
    void duplicateLines();
    void deleteLines();
    void changeCase(int change);

    // Word movement: the start of the previous word or the end of the next.
    TextPosition wordBoundary(const TextPosition &from, bool forward) const;
    TextRange wordAt(const TextPosition &position) const;
    // The bracket matching the one before or after the cursor, or {-1,-1}.
    TextRange matchingBracket(const TextPosition &position) const;

    DocumentStatistics statistics() const;

    int notice() const { return m_notice; }
    const std::string &noticeDetail() const { return m_noticeDetail; }

    // Search; the matches stay highlighted while the pattern is set.
    void setSearch(const std::string &text, bool caseSensitive, bool wholeWords, bool regex, bool inSelection);
    void clearSearch();
    // Selects the next match; returns false when there is none. Sets wrapped.
    bool findNext(bool backwards = false);
    // While typing the pattern: the first match from where the selection starts.
    bool findIncremental();
    bool replaceCurrent(const std::string &replacement);
    int replaceAll(const std::string &replacement);
    const std::string &searchText() const { return m_searcher.pattern(); }
    bool searchValid() const { return m_searcher.isValid(); }
    std::string searchError() const { return m_searcher.errorString(); }
    const Searcher &searcher() const { return m_searcher; }
    TextRange searchScope() const { return m_searchScope; }
    int matchCount() const { return m_matchCount; }
    int matchIndex() const { return m_matchIndex; }
    bool lastSearchWrapped() const { return m_wrapped; }

    SyntaxStates &syntaxStates() { return m_states; }
    // Computes highlighting states up to `line`, a slice at a time.
    void requestHighlighting(int line);

    const std::string &draftId() const { return m_draftId; }
    void setDraftId(const std::string &id);
    static std::string draftDirectory();
    std::string draftPath() const;
    // Writes the unsaved text to the draft now, on this thread.
    void writeDraftNow();
    void removeDraft();
    // Takes the text of a draft as the unsaved state of this document.
    bool restoreDraft();
    // Starts a draft write on a worker thread, as the draft timer does.
    void writeDraftAsync();

    // Compares the file with what was loaded or saved; reloads or reports.
    void checkDisk();

    static void setRecentFilesHook(std::function<void(const std::string &)> hook);

    Lingmo::base::Signal<> titleChanged;
    Lingmo::base::Signal<> filePathChanged;
    Lingmo::base::Signal<> modifiedChanged;
    Lingmo::base::Signal<> loadingChanged;
    Lingmo::base::Signal<> loadProgressChanged;
    Lingmo::base::Signal<> lockedChanged;
    Lingmo::base::Signal<> writableChanged;
    Lingmo::base::Signal<> encodingChanged;
    Lingmo::base::Signal<> lineEndingChanged;
    Lingmo::base::Signal<> languageChanged;
    Lingmo::base::Signal<> largeChanged;
    Lingmo::base::Signal<> indentationChanged;
    Lingmo::base::Signal<> undoChanged;
    Lingmo::base::Signal<> cursorChanged;
    Lingmo::base::Signal<> noticeChanged;
    Lingmo::base::Signal<> searchChanged;
    Lingmo::base::Signal<> matchesChanged;
    // Lines [line, line + removed) became [line, line + added).
    Lingmo::base::Signal<int, int, int> linesChanged;
    Lingmo::base::Signal<> contentsChanged;
    // The entering highlight state of lines [first, last] changed.
    Lingmo::base::Signal<int, int> syntaxChanged;
    // A view should scroll the cursor into view.
    Lingmo::base::Signal<> revealCursor;
    Lingmo::base::Signal<> loaded;
    Lingmo::base::Signal<> draftIdChanged;
    Lingmo::base::Signal<> saved;

private:
    void applyStep(const EditStep &step, bool reverse);
    void finishLoad(LoadResult &result);
    void replaceWithReloaded(const std::vector<std::string> &lines);
    void setNotice(Notice notice, const std::string &detail = std::string());
    void updateModified();
    void setFilePath(const std::string &path);
    void watchFile();
    void updateWritable();
    void detectLanguage();
    void updateMatches();
    void computeMatches();
    void scheduleDraft();
    void applyEditorConfig();
    bool write(const std::string &path, const std::string &encoding, bool bom);
    void highlightSlice();
    void waitForDraftWrite();

    TextBuffer m_buffer;
    UndoStack m_undo;
    TextPosition m_cursor;
    TextPosition m_anchor;

    std::string m_filePath;
    bool m_modified = false;
    // The encoding, BOM or line ending changed since the last save.
    bool m_formatModified = false;
    // The file is gone or its text was replaced by a draft: the disk copy is not the buffer.
    bool m_diskDiffers = false;
    bool m_restoreDraftAfterLoad = false;
    // Some bytes did not decode; saving as is would replace them for good.
    bool m_malformed = false;
    std::string m_loadedEncoding;
    bool m_loading = false;
    double m_loadProgress = 0;
    bool m_locked = false;
    bool m_writable = true;
    bool m_large = false;
    std::string m_encoding = "UTF-8";
    bool m_bom = false;
    LineEnding m_lineEnding = LineEnding::Unix;
    std::string m_language;
    bool m_languageChosen = false;
    bool m_highlightingEnabled = true;
    bool m_insertSpaces = true;
    int m_tabWidth = 4;
    int m_indentWidth = 4;
    bool m_indentationFromFile = false;
    bool m_trimTrailingWhitespace = false;
    std::optional<bool> m_finalNewline;

    int m_notice = NoNotice;
    std::string m_noticeDetail;

    Searcher m_searcher;
    TextRange m_searchScope;
    int m_matchCount = 0;
    int m_matchIndex = 0;
    bool m_wrapped = false;
    Lingmo::base::Timer m_matchTimer;

    SyntaxStates m_states;
    int m_highlightTarget = 0;
    Lingmo::base::Timer m_highlightTimer;

    std::string m_draftId;
    Lingmo::base::Timer m_draftTimer;
    std::future<void> m_draftWrite;

    Lingmo::base::FileWatch m_watch;
    std::string m_watchedFile;
    DiskStamp m_diskStamp;
    Lingmo::base::Timer m_diskCheckTimer;
    // The disk changed while the buffer was modified and the user kept theirs.
    bool m_ignoreDisk = false;

    Lingmo::base::Job m_loadJob;
    std::shared_ptr<std::atomic<int>> m_loadProgressShared;
    std::shared_ptr<std::atomic<bool>> m_loadCancel;
    Lingmo::base::Timer m_progressTimer;
    // A reload keeps the cursor and scroll position.
    bool m_reloading = false;
    bool m_addToRecent = true;
    TextPosition m_reloadCursor;
};

#endif
