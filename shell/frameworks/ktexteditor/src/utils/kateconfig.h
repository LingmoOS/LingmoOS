/*
    SPDX-FileCopyrightText: 2003 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_CONFIG_H
#define KATE_CONFIG_H

#include <ktexteditor_export.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <functional>
#include <map>
#include <memory>

#include <QBitArray>
#include <QColor>
#include <QList>
#include <QObject>

class KConfigGroup;
namespace KTextEditor
{
class ViewPrivate;
}
namespace KTextEditor
{
class DocumentPrivate;
}
class KateRenderer;

namespace KTextEditor
{
class EditorPrivate;
}

class KConfig;

/**
 * Base Class for the Kate Config Classes
 * Current childs are KateDocumentConfig/KateDocumentConfig/KateDocumentConfig
 */
class KTEXTEDITOR_EXPORT KateConfig
{
public:
    /**
     * Start some config changes.
     * This method is needed to init some kind of transaction for config changes,
     * update will only be done once, at configEnd() call.
     */
    void configStart();

    /**
     * End a config change transaction, update the concerned
     * KateDocumentConfig/KateDocumentConfig/KateDocumentConfig
     */
    void configEnd();

    /**
     * Is this a global config object?
     * @return true when this is a global config object
     */
    bool isGlobal() const
    {
        return !m_parent;
    }

    /**
     * All known config keys.
     * This will use the knowledge about all registered keys of the global object.
     * @return all known config keys
     */
    QStringList configKeys() const
    {
        return m_parent ? m_parent->configKeys() : *m_configKeys.get();
    }

    /**
     * Is given key set in this config object?
     * @param key config key, aka enum from KateConfig* classes
     * @return is the wanted key set?
     */
    bool isSet(const int key) const
    {
        return m_configEntries.find(key) != m_configEntries.end();
    }

    /**
     * Get a config value.
     * @param key config key, aka enum from KateConfig* classes
     * @return value for the wanted key, will assert if key is not valid
     */
    QVariant value(const int key) const;

    /**
     * Set a config value.
     * Will assert if key is invalid.
     * Might not alter the value if given value fails validation.
     * @param key config key, aka enum from KateConfig* classes
     * @param value value to set
     * @return true on success
     */
    bool setValue(const int key, const QVariant &value);

    /**
     * Get a config value for the string key.
     * @param key config key, aka commandName from KateConfig* classes
     * @return value for the wanted key, will return invalid variant if key is not known
     */
    QVariant value(const QString &key) const;

    /**
     * Set a config value.
     * Will do nothing if key is not known or the given value fails validation.
     * @param key config key, aka commandName from KateConfig* classes
     * @param value value to set
     * @return true on success
     */
    bool setValue(const QString &key, const QVariant &value);

protected:
    /**
     * Construct a KateConfig.
     * @param parent parent config object, if any
     */
    KateConfig(const KateConfig *parent = nullptr);

    /**
     * Virtual Destructor
     */
    virtual ~KateConfig();

    /**
     * One config entry.
     */
    class ConfigEntry
    {
    public:
        /**
         * Construct one config entry.
         * @param enumId value of the enum for this config entry
         * @param configId value of the key for the KConfig file for this config entry
         * @param command command name
         * @param defaultVal default value
         * @param valid validator function, default none
         */
        ConfigEntry(int enumId, const char *configId, QString command, QVariant defaultVal, std::function<bool(const QVariant &)> valid = nullptr)
            : enumKey(enumId)
            , configKey(configId)
            , commandName(command)
            , defaultValue(defaultVal)
            , value(defaultVal)
            , validator(valid)
        {
        }

        /**
         * Enum key for this config entry, shall be unique
         */
        const int enumKey;

        /**
         * KConfig entry key for this config entry, shall be unique in its group
         * e.g. "Tab Width"
         */
        const char *const configKey;

        /**
         * Command name as used in e.g. ConfigInterface or modeline/command line
         * e.g. tab-width
         */
        const QString commandName;

        /**
         * Default value if nothing special was configured
         */
        const QVariant defaultValue;

        /**
         * The concrete value, per default == defaultValue
         */
        QVariant value;

        /**
         * An optional validator function, only when these returns true
         * we accept a given new value.
         * Is no validator set, we accept any value.
         */
        std::function<bool(const QVariant &)> validator;
    };

    /**
     * Read all config entries from given config group.
     * @param config config group to read from
     */
    void readConfigEntries(const KConfigGroup &config);

    /**
     * Write all config entries to given config group.
     * @param config config group to write to
     */
    void writeConfigEntries(KConfigGroup &config) const;

    /**
     * Register a new config entry.
     * Used by the sub classes to register all there known ones.
     * @param entry new entry to add
     */
    void addConfigEntry(ConfigEntry &&entry);

    /**
     * Finalize the config entries.
     * Called by the sub classes after all entries are registered
     */
    void finalizeConfigEntries();

    /**
     * do the real update
     */
    virtual void updateConfig() = 0;

private:
    /**
     * Get full map of config entries, aka the m_configEntries of the top config object
     * @return full map with all config entries
     */
    const std::map<int, ConfigEntry> &fullConfigEntries() const
    {
        return m_parent ? m_parent->fullConfigEntries() : m_configEntries;
    }
    /**
     * Get hash of config entries, aka the m_configKeyToEntry of the top config object
     * @return full hash with all config entries
     */
    const QHash<QString, const ConfigEntry *> &fullConfigKeyToEntry() const
    {
        return m_parent ? m_parent->fullConfigKeyToEntry() : *m_configKeyToEntry.get();
    }

private:
    /**
     * parent config object, if any
     */
    const KateConfig *const m_parent = nullptr;

    /**
     * two cases:
     *   - we have m_parent == nullptr => this contains all known config entries
     *   - we have m_parent != nullptr => this contains all set config entries for this level of configuration
     *
     * uses a map ATM for deterministic iteration e.g. for read/writeConfig
     */
    std::map<int, ConfigEntry> m_configEntries;

    /**
     * All known config keys, filled only for the object with m_parent == nullptr
     */
    std::unique_ptr<QStringList> m_configKeys;

    /**
     * Hash of config keys => config entry, filled only for the object with m_parent == nullptr
     */
    std::unique_ptr<QHash<QString, const ConfigEntry *>> m_configKeyToEntry;

protected:
    /**
     * recursion depth
     */
    int configSessionNumber = 0;
};

class KTEXTEDITOR_EXPORT KateGlobalConfig : public KateConfig
{
private:
    friend class KTextEditor::EditorPrivate;

    /**
     * only used in KTextEditor::EditorPrivate for the static global fallback !!!
     */
    KateGlobalConfig();

public:
    static KateGlobalConfig *global()
    {
        return s_global;
    }

    /**
     * Known config entries
     */
    enum ConfigEntryTypes {
        /**
         * Encoding prober
         */
        EncodingProberType,

        /**
         * Fallback encoding
         */
        FallbackEncoding
    };

public:
    /**
     * Read config from object
     */
    void readConfig(const KConfigGroup &config);

    /**
     * Write config to object
     */
    void writeConfig(KConfigGroup &config);

protected:
    void updateConfig() override;

public:
    QString fallbackEncoding() const
    {
        return value(FallbackEncoding).toString();
    }

    bool setFallbackEncoding(const QString &encoding)
    {
        return setValue(FallbackEncoding, encoding);
    }

private:
    static KateGlobalConfig *s_global;
};

class KTEXTEDITOR_EXPORT KateDocumentConfig : public KateConfig
{
private:
    friend class KTextEditor::EditorPrivate;

    KateDocumentConfig();

public:
    /**
     * Construct a DocumentConfig
     */
    explicit KateDocumentConfig(KTextEditor::DocumentPrivate *doc);

    inline static KateDocumentConfig *global()
    {
        return s_global;
    }

    /**
     * Known config entries
     */
    enum ConfigEntryTypes {
        /**
         * Tabulator width
         */
        TabWidth,

        /**
         * Indentation width
         */
        IndentationWidth,

        /**
         * On-the-fly spellcheck enabled?
         */
        OnTheFlySpellCheck,

        /**
         * Indent pasted text?
         */
        IndentOnTextPaste,

        /**
         * Replace tabs with spaces?
         */
        ReplaceTabsWithSpaces,

        /**
         * Backup files for local files?
         */
        BackupOnSaveLocal,

        /**
         * Backup files for remote files?
         */
        BackupOnSaveRemote,

        /**
         * Prefix for backup files
         */
        BackupOnSavePrefix,

        /**
         * Suffix for backup files
         */
        BackupOnSaveSuffix,

        /**
         * Indentation mode, like "normal"
         */
        IndentationMode,

        /**
         * Tab handling, like indent, insert tab, smart
         */
        TabHandlingMode,

        /**
         * Static word wrap?
         */
        StaticWordWrap,

        /**
         * Static word wrap column
         */
        StaticWordWrapColumn,

        /**
         * PageUp/Down moves cursor?
         */
        PageUpDownMovesCursor,

        /**
         * Smart Home key?
         */
        SmartHome,

        /**
         * Show Tabs?
         */
        ShowTabs,

        /**
         * Indent on tab?
         */
        IndentOnTab,

        /**
         * Keep extra space?
         */
        KeepExtraSpaces,

        /**
         * Backspace key indents?
         */
        BackspaceIndents,

        /**
         * Show spaces mode like none, all, ...
         */
        ShowSpacesMode,

        /**
         * Trailing Marker Size
         */
        TrailingMarkerSize,

        /**
         * Remove spaces mode
         */
        RemoveSpacesMode,

        /**
         * Ensure newline at end of file
         */
        NewlineAtEOF,

        /**
         * Overwrite mode?
         */
        OverwriteMode,

        /**
         * Encoding
         */
        Encoding,

        /**
         * End of line mode: dos, mac, unix
         */
        EndOfLine,

        /**
         * Allow EOL detection
         */
        AllowEndOfLineDetection,

        /**
         * Use Byte Order Mark
         */
        ByteOrderMark,

        /**
         * Swap file mode
         */
        SwapFile,

        /**
         * Swap file directory
         */
        SwapFileDirectory,

        /**
         * Swap file sync interval
         */
        SwapFileSyncInterval,

        /**
         * Line length limit
         */
        LineLengthLimit,

        /**
         * Camel Cursor Movement?
         */
        CamelCursor,

        /**
         * Automatically detect file indentation
         */
        AutoDetectIndent,

        /**
         * Automatically save?
         */
        AutoSave,
        /**
         * Automatically save on focus lost
         */
        AutoSaveOnFocusOut,
        /**
         * Auto save interval
         */
        AutoSaveInteral,

        /**
         * Should we auto-reload if the old state is in version control?
         */
        AutoReloadIfStateIsInVersionControl
    };

public:
    /**
     * Read config from object
     */
    void readConfig(const KConfigGroup &config);

    /**
     * Write config to object
     */
    void writeConfig(KConfigGroup &config);

protected:
    void updateConfig() override;

public:
    int tabWidth() const
    {
        return value(TabWidth).toInt();
    }

    void setTabWidth(int tabWidth)
    {
        setValue(TabWidth, QVariant(tabWidth));
    }

    int indentationWidth() const
    {
        return value(IndentationWidth).toInt();
    }

    void setIndentationWidth(int indentationWidth)
    {
        setValue(IndentationWidth, QVariant(indentationWidth));
    }

    bool onTheFlySpellCheck() const
    {
        return value(OnTheFlySpellCheck).toBool();
    }

    void setOnTheFlySpellCheck(bool on)
    {
        setValue(OnTheFlySpellCheck, QVariant(on));
    }

    bool indentPastedText() const
    {
        return value(IndentOnTextPaste).toBool();
    }

    void setIndentPastedText(bool on)
    {
        setValue(IndentOnTextPaste, QVariant(on));
    }

    bool replaceTabsDyn() const
    {
        return value(ReplaceTabsWithSpaces).toBool();
    }

    void setReplaceTabsDyn(bool on)
    {
        setValue(ReplaceTabsWithSpaces, QVariant(on));
    }

    bool backupOnSaveLocal() const
    {
        return value(BackupOnSaveLocal).toBool();
    }

    void setBackupOnSaveLocal(bool on)
    {
        setValue(BackupOnSaveLocal, QVariant(on));
    }

    bool backupOnSaveRemote() const
    {
        return value(BackupOnSaveRemote).toBool();
    }

    void setBackupOnSaveRemote(bool on)
    {
        setValue(BackupOnSaveRemote, QVariant(on));
    }

    QString backupPrefix() const
    {
        return value(BackupOnSavePrefix).toString();
    }

    void setBackupPrefix(const QString &prefix)
    {
        setValue(BackupOnSavePrefix, QVariant(prefix));
    }

    QString backupSuffix() const
    {
        return value(BackupOnSaveSuffix).toString();
    }

    void setBackupSuffix(const QString &suffix)
    {
        setValue(BackupOnSaveSuffix, QVariant(suffix));
    }

    QString indentationMode() const
    {
        return value(IndentationMode).toString();
    }

    void setIndentationMode(const QString &identationMode)
    {
        setValue(IndentationMode, identationMode);
    }

    enum TabHandling {
        tabInsertsTab = 0,
        tabIndents = 1,
        tabSmart = 2 //!< indents in leading space, otherwise inserts tab
    };

    enum WhitespaceRendering { None, Trailing, All };

    int tabHandling() const
    {
        return value(TabHandlingMode).toInt();
    }

    void setTabHandling(int tabHandling)
    {
        setValue(TabHandlingMode, tabHandling);
    }

    bool wordWrap() const
    {
        return value(StaticWordWrap).toBool();
    }

    void setWordWrap(bool on)
    {
        setValue(StaticWordWrap, on);
    }

    int wordWrapAt() const
    {
        return value(StaticWordWrapColumn).toInt();
    }

    void setWordWrapAt(int col)
    {
        setValue(StaticWordWrapColumn, col);
    }

    bool pageUpDownMovesCursor() const
    {
        return value(PageUpDownMovesCursor).toBool();
    }

    void setPageUpDownMovesCursor(bool on)
    {
        setValue(PageUpDownMovesCursor, on);
    }

    void setKeepExtraSpaces(bool on)
    {
        setValue(KeepExtraSpaces, on);
    }

    bool keepExtraSpaces() const
    {
        return value(KeepExtraSpaces).toBool();
    }

    void setBackspaceIndents(bool on)
    {
        setValue(BackspaceIndents, on);
    }

    bool backspaceIndents() const
    {
        return value(BackspaceIndents).toBool();
    }

    void setSmartHome(bool on)
    {
        setValue(SmartHome, on);
    }

    bool smartHome() const
    {
        return value(SmartHome).toBool();
    }

    void setShowTabs(bool on)
    {
        setValue(ShowTabs, on);
    }

    bool showTabs() const
    {
        return value(ShowTabs).toBool();
    }

    void setShowSpaces(WhitespaceRendering mode)
    {
        setValue(ShowSpacesMode, mode);
    }

    WhitespaceRendering showSpaces() const
    {
        return WhitespaceRendering(value(ShowSpacesMode).toInt());
    }

    void setMarkerSize(int markerSize)
    {
        setValue(TrailingMarkerSize, markerSize);
    }

    int markerSize() const
    {
        return value(TrailingMarkerSize).toInt();
    }

    /**
     * Remove trailing spaces on save.
     * triState = 0: never remove trailing spaces
     * triState = 1: remove trailing spaces of modified lines (line modification system)
     * triState = 2: remove trailing spaces in entire document
     */
    void setRemoveSpaces(int triState)
    {
        setValue(RemoveSpacesMode, triState);
    }

    int removeSpaces() const
    {
        return value(RemoveSpacesMode).toInt();
    }

    void setNewLineAtEof(bool on)
    {
        setValue(NewlineAtEOF, on);
    }

    bool newLineAtEof() const
    {
        return value(NewlineAtEOF).toBool();
    }

    void setOvr(bool on)
    {
        setValue(OverwriteMode, on);
    }

    bool ovr() const
    {
        return value(OverwriteMode).toBool();
    }

    void setTabIndents(bool on)
    {
        setValue(IndentOnTab, on);
    }

    bool tabIndentsEnabled() const
    {
        return value(IndentOnTab).toBool();
    }

    QString encoding() const
    {
        return value(Encoding).toString();
    }

    bool setEncoding(const QString &encoding)
    {
        return setValue(Encoding, encoding);
    }

    enum Eol { eolUnix = 0, eolDos = 1, eolMac = 2 };

    int eol() const
    {
        return value(EndOfLine).toInt();
    }

    /**
     * Get current end of line string.
     * Based on current set eol mode.
     * @return current end of line string
     */
    QString eolString() const;

    void setEol(int mode)
    {
        setValue(EndOfLine, mode);
    }

    bool bom() const
    {
        return value(ByteOrderMark).toBool();
    }

    void setBom(bool bom)
    {
        setValue(ByteOrderMark, bom);
    }

    bool allowEolDetection() const
    {
        return value(AllowEndOfLineDetection).toBool();
    }

    void setAllowEolDetection(bool on)
    {
        setValue(AllowEndOfLineDetection, on);
    }

    QString swapDirectory() const
    {
        return value(SwapFileDirectory).toString();
    }

    void setSwapDirectory(const QString &directory)
    {
        setValue(SwapFileDirectory, directory);
    }

    enum SwapFileMode { DisableSwapFile = 0, EnableSwapFile, SwapFilePresetDirectory };

    SwapFileMode swapFileMode() const
    {
        return SwapFileMode(value(SwapFile).toInt());
    }

    void setSwapFileMode(int mode)
    {
        setValue(SwapFile, mode);
    }

    int swapSyncInterval() const
    {
        return value(SwapFileSyncInterval).toInt();
    }

    void setSwapSyncInterval(int interval)
    {
        setValue(SwapFileSyncInterval, interval);
    }

    int lineLengthLimit() const
    {
        return value(LineLengthLimit).toInt();
    }

    void setLineLengthLimit(int limit)
    {
        setValue(LineLengthLimit, limit);
    }

    void setCamelCursor(bool on)
    {
        setValue(CamelCursor, on);
    }

    bool camelCursor() const
    {
        return value(CamelCursor).toBool();
    }

    void setAutoDetectIndent(bool on)
    {
        setValue(AutoDetectIndent, on);
    }

    bool autoDetectIndent() const
    {
        return value(AutoDetectIndent).toBool();
    }

    bool autoSave() const
    {
        return value(AutoSave).toBool();
    }

    bool autoSaveOnFocusOut() const
    {
        return value(AutoSaveOnFocusOut).toBool();
    }

    int autoSaveInterval() const
    {
        return value(AutoSaveInteral).toInt();
    }

private:
    static KateDocumentConfig *s_global;
    KTextEditor::DocumentPrivate *m_doc = nullptr;
};

class KTEXTEDITOR_EXPORT KateViewConfig : public KateConfig
{
private:
    friend class KTextEditor::EditorPrivate;

    /**
     * only used in KTextEditor::EditorPrivate for the static global fallback !!!
     */
    KTEXTEDITOR_NO_EXPORT
    KateViewConfig();

public:
    /**
     * Construct a ViewConfig
     */
    explicit KateViewConfig(KTextEditor::ViewPrivate *view);

    /**
     * Cu ViewConfig
     */
    ~KateViewConfig() override;

    inline static KateViewConfig *global()
    {
        return s_global;
    }

    /**
     * All known config keys
     * Keep them sorted alphabetically for our convenience.
     * Keep the same order when adding config entries with addConfigEntry() in
     * KateViewConfig::KateViewConfig() otherwise the code will assert.
     */
    enum ConfigEntryTypes {
        AllowMarkMenu,
        AutoBrackets,
        AutoCenterLines,
        AutomaticCompletionInvocation,
        AutomaticCompletionPreselectFirst,
        BackspaceRemoveComposedCharacters,
        BookmarkSorting,
        CharsToEncloseSelection,
        ClipboardHistoryEntries,
        DefaultMarkType,
        DynWordWrapAlignIndent,
        DynWordWrapIndicators,
        DynWrapAnywhere,
        DynWrapAtStaticMarker,
        DynamicWordWrap,
        EnterToInsertCompletion,
        FoldFirstLine,
        InputMode,
        KeywordCompletion,
        MaxHistorySize,
        MousePasteAtCursorPosition,
        PersistentSelection,
        ScrollBarMiniMapWidth,
        ScrollPastEnd,
        SearchFlags,
        TabCompletion,
        ShowBracketMatchPreview,
        ShowFoldingBar,
        ShowFoldingPreview,
        ShowIconBar,
        ShowLineCount,
        ShowLineModification,
        ShowLineNumbers,
        ShowScrollBarMarks,
        ShowScrollBarMiniMap,
        ShowScrollBarMiniMapAll,
        ShowScrollBarPreview,
        ShowScrollbars,
        ShowWordCount,
        TextDragAndDrop,
        SmartCopyCut,
        UserSetsOfCharsToEncloseSelection,
        ViInputModeStealKeys,
        ViRelativeLineNumbers,
        WordCompletion,
        WordCompletionMinimalWordLength,
        WordCompletionRemoveTail,
        ShowDocWithCompletion,
        MultiCursorModifier,
        ShowFoldingOnHoverOnly,
        ShowStatusbarLineColumn,
        ShowStatusbarDictionary,
        ShowStatusbarInputMode,
        ShowStatusbarHighlightingMode,
        ShowStatusbarTabSettings,
        ShowStatusbarFileEncoding,
        StatusbarLineColumnCompact,
        ShowStatusbarEOL,
        EnableAccessibility,
    };

public:
    /**
     * Read config from object
     */
    void readConfig(const KConfigGroup &config);

    /**
     * Write config to object
     */
    void writeConfig(KConfigGroup &config);

protected:
    void updateConfig() override;

public:
    bool dynWordWrap() const
    {
        return value(DynamicWordWrap).toBool();
    }
    void setDynWordWrap(bool on)
    {
        setValue(DynamicWordWrap, on);
    }
    bool dynWrapAnywhere() const
    {
        return value(DynWrapAnywhere).toBool();
    }

    bool dynWrapAtStaticMarker() const
    {
        return value(DynWrapAtStaticMarker).toBool();
    }

    int dynWordWrapIndicators() const
    {
        return value(DynWordWrapIndicators).toInt();
    }

    int dynWordWrapAlignIndent() const
    {
        return value(DynWordWrapAlignIndent).toInt();
    }

    bool lineNumbers() const
    {
        return value(ShowLineNumbers).toBool();
    }

    bool scrollBarMarks() const
    {
        return value(ShowScrollBarMarks).toBool();
    }

    bool scrollBarPreview() const
    {
        return value(ShowScrollBarPreview).toBool();
    }

    bool scrollBarMiniMap() const
    {
        return value(ShowScrollBarMiniMap).toBool();
    }

    bool scrollBarMiniMapAll() const
    {
        return value(ShowScrollBarMiniMapAll).toBool();
    }

    int scrollBarMiniMapWidth() const
    {
        return value(ScrollBarMiniMapWidth).toInt();
    }

    /* Whether to show scrollbars */
    enum ScrollbarMode { AlwaysOn = 0, ShowWhenNeeded, AlwaysOff };

    int showScrollbars() const
    {
        return value(ShowScrollbars).toInt();
    }

    bool showDocWithCompletion() const
    {
        return value(ShowDocWithCompletion).toBool();
    }

    Qt::KeyboardModifiers multiCursorModifiers() const
    {
        return static_cast<Qt::KeyboardModifiers>(value(MultiCursorModifier).toInt());
    }

    void setMultiCursorModifiers(Qt::KeyboardModifiers m)
    {
        setValue(MultiCursorModifier, (int)m);
    }

    bool iconBar() const
    {
        return value(ShowIconBar).toBool();
    }

    bool foldingBar() const
    {
        return value(ShowFoldingBar).toBool();
    }

    bool foldingPreview() const
    {
        return value(ShowFoldingPreview).toBool();
    }

    bool lineModification() const
    {
        return value(ShowLineModification).toBool();
    }

    int bookmarkSort() const
    {
        return value(BookmarkSorting).toInt();
    }

    int autoCenterLines() const
    {
        return value(AutoCenterLines).toInt();
    }

    enum SearchFlags {
        IncMatchCase = 1 << 0,
        IncHighlightAll = 1 << 1,
        IncFromCursor = 1 << 2,
        PowerMatchCase = 1 << 3,
        PowerHighlightAll = 1 << 4,
        PowerFromCursor = 1 << 5,
        // PowerSelectionOnly = 1 << 6, Better not save to file // Sebastian
        PowerModePlainText = 1 << 7,
        PowerModeWholeWords = 1 << 8,
        PowerModeEscapeSequences = 1 << 9,
        PowerModeRegularExpression = 1 << 10,
        PowerUsePlaceholders = 1 << 11
    };

    uint searchFlags() const
    {
        return value(SearchFlags).toUInt();
    }
    void setSearchFlags(uint flags)
    {
        setValue(SearchFlags, flags);
    }

    int maxHistorySize() const
    {
        return value(MaxHistorySize).toInt();
    }

    uint defaultMarkType() const
    {
        return value(DefaultMarkType).toUInt();
    }

    bool allowMarkMenu() const
    {
        return value(AllowMarkMenu).toBool();
    }

    bool persistentSelection() const
    {
        return value(PersistentSelection).toBool();
    }

    KTextEditor::View::InputMode inputMode() const
    {
        return static_cast<KTextEditor::View::InputMode>(value(InputMode).toUInt());
    }

    bool viInputModeStealKeys() const
    {
        return value(ViInputModeStealKeys).toBool();
    }

    bool viRelativeLineNumbers() const
    {
        return value(ViRelativeLineNumbers).toBool();
    }

    // Do we still need the enum and related functions below?
    enum TextToSearch { Nowhere = 0, SelectionOnly = 1, SelectionWord = 2, WordOnly = 3, WordSelection = 4 };

    bool automaticCompletionInvocation() const
    {
        return value(AutomaticCompletionInvocation).toBool();
    }

    bool automaticCompletionPreselectFirst() const
    {
        return value(AutomaticCompletionPreselectFirst).toBool();
    }

    bool tabCompletion() const
    {
        return value(TabCompletion).toBool();
    }

    bool wordCompletion() const
    {
        return value(WordCompletion).toBool();
    }

    bool keywordCompletion() const
    {
        return value(KeywordCompletion).toBool();
    }

    int wordCompletionMinimalWordLength() const
    {
        return value(WordCompletionMinimalWordLength).toInt();
    }

    bool wordCompletionRemoveTail() const
    {
        return value(WordCompletionRemoveTail).toBool();
    }

    bool textDragAndDrop() const
    {
        return value(TextDragAndDrop).toBool();
    }

    bool smartCopyCut() const
    {
        return value(SmartCopyCut).toBool();
    }

    bool mousePasteAtCursorPosition() const
    {
        return value(MousePasteAtCursorPosition).toBool();
    }

    int clipboardHistoryEntries() const
    {
        return value(ClipboardHistoryEntries).toInt();
    }

    bool scrollPastEnd() const
    {
        return value(ScrollPastEnd).toBool();
    }

    bool foldFirstLine() const
    {
        return value(FoldFirstLine).toBool();
    }

    bool showWordCount() const
    {
        return value(ShowWordCount).toBool();
    }
    void setShowWordCount(bool on)
    {
        setValue(ShowWordCount, on);
    }

    bool showLineCount() const
    {
        return value(ShowLineCount).toBool();
    }

    void setShowLineCount(bool on)
    {
        setValue(ShowLineCount, on);
    }

    bool autoBrackets() const
    {
        return value(AutoBrackets).toBool();
    }

    bool encloseSelectionInChars() const
    {
        return !value(CharsToEncloseSelection).toString().isEmpty();
    }

    QString charsToEncloseSelection() const
    {
        return value(CharsToEncloseSelection).toString();
    }

    bool backspaceRemoveComposed() const
    {
        return value(BackspaceRemoveComposedCharacters).toBool();
    }

    bool showFoldingOnHoverOnly() const
    {
        return value(ShowFoldingOnHoverOnly).toBool();
    }

private:
    static KateViewConfig *s_global;
    KTextEditor::ViewPrivate *m_view = nullptr;
};

class KTEXTEDITOR_EXPORT KateRendererConfig : public KateConfig
{
private:
    friend class KTextEditor::EditorPrivate;

    /**
     * only used in KTextEditor::EditorPrivate for the static global fallback !!!
     */
    KTEXTEDITOR_NO_EXPORT
    KateRendererConfig();

public:
    /**
     * Construct a RendererConfig
     */
    explicit KateRendererConfig(KateRenderer *renderer);

    /**
     * Cu RendererConfig
     */
    ~KateRendererConfig() override;

    inline static KateRendererConfig *global()
    {
        return s_global;
    }

    /**
     * All known config keys
     * Keep them sorted alphabetic for our convenience
     */
    enum ConfigEntryTypes {
        /**
         * auto-select the color theme based on application palette
         */
        AutoColorThemeSelection
    };

public:
    /**
     * Read config from object
     */
    void readConfig(const KConfigGroup &config);

    /**
     * Write config to object
     */
    void writeConfig(KConfigGroup &config);

protected:
    void updateConfig() override;

public:
    const QString &schema() const;
    void setSchema(QString schema);

    /**
     * Reload the schema from the schema manager.
     * For the global instance, have all other instances reload.
     * Used by the schema config page to apply changes.
     */
    void reloadSchema();

    /**
     * Base font to use for the views and co.
     * Will be adjusted there to avoid rendering artifacts for HiDPI stuff.
     * @return base font to use
     */
    const QFont &baseFont() const;

    void setFont(const QFont &font);

    bool wordWrapMarker() const;
    void setWordWrapMarker(bool on);

    const QColor &backgroundColor() const;
    void setBackgroundColor(const QColor &col);

    const QColor &selectionColor() const;
    void setSelectionColor(const QColor &col);

    const QColor &highlightedLineColor() const;
    void setHighlightedLineColor(const QColor &col);

    const QColor &lineMarkerColor(KTextEditor::Document::MarkTypes type = KTextEditor::Document::markType01) const; // markType01 == Bookmark

    const QColor &highlightedBracketColor() const;
    void setHighlightedBracketColor(const QColor &col);

    const QColor &wordWrapMarkerColor() const;
    void setWordWrapMarkerColor(const QColor &col);

    const QColor &tabMarkerColor() const;
    void setTabMarkerColor(const QColor &col);

    const QColor &indentationLineColor() const;
    void setIndentationLineColor(const QColor &col);

    const QColor &iconBarColor() const;
    void setIconBarColor(const QColor &col);

    const QColor &foldingColor() const;
    void setFoldingColor(const QColor &col);

    // the line number color is used for the line numbers on the left bar
    const QColor &lineNumberColor() const;
    void setLineNumberColor(const QColor &col);
    const QColor &currentLineNumberColor() const;
    void setCurrentLineNumberColor(const QColor &col);

    // the color of the separator between line numbers and icon bar
    const QColor &separatorColor() const;
    void setSeparatorColor(const QColor &col);

    const QColor &spellingMistakeLineColor() const;
    void setSpellingMistakeLineColor(const QColor &col);

    bool showIndentationLines() const;
    void setShowIndentationLines(bool on);

    bool showWholeBracketExpression() const;
    void setShowWholeBracketExpression(bool on);

    static bool animateBracketMatching();
    void setAnimateBracketMatching(bool on);

    const QColor &templateBackgroundColor() const;
    const QColor &templateEditablePlaceholderColor() const;
    const QColor &templateFocusedEditablePlaceholderColor() const;
    const QColor &templateNotEditablePlaceholderColor() const;

    const QColor &modifiedLineColor() const;
    void setModifiedLineColor(const QColor &col);

    const QColor &savedLineColor() const;
    void setSavedLineColor(const QColor &col);

    const QColor &searchHighlightColor() const;
    void setSearchHighlightColor(const QColor &col);

    const QColor &replaceHighlightColor() const;
    void setReplaceHighlightColor(const QColor &col);

    void setLineHeightMultiplier(qreal value);

    qreal lineHeightMultiplier() const
    {
        return s_global->m_lineHeightMultiplier;
    }

private:
    /**
     * Read the schema properties from the config file.
     */
    KTEXTEDITOR_NO_EXPORT
    void setSchemaInternal(const QString &schema);

private:
    QString m_schema;
    QFont m_font;
    QColor m_backgroundColor;
    QColor m_selectionColor;
    QColor m_highlightedLineColor;
    QColor m_highlightedBracketColor;
    QColor m_wordWrapMarkerColor;
    QColor m_tabMarkerColor;
    QColor m_indentationLineColor;
    QColor m_iconBarColor;
    QColor m_foldingColor;
    QColor m_lineNumberColor;
    QColor m_currentLineNumberColor;
    QColor m_separatorColor;
    QColor m_spellingMistakeLineColor;
    std::vector<QColor> m_lineMarkerColor;

    QColor m_templateBackgroundColor;
    QColor m_templateEditablePlaceholderColor;
    QColor m_templateFocusedEditablePlaceholderColor;
    QColor m_templateNotEditablePlaceholderColor;

    QColor m_modifiedLineColor;
    QColor m_savedLineColor;
    QColor m_searchHighlightColor;
    QColor m_replaceHighlightColor;

    qreal m_lineHeightMultiplier = 1.0;

    bool m_wordWrapMarker = false;
    bool m_showIndentationLines = false;
    bool m_showWholeBracketExpression = false;
    bool m_animateBracketMatching = false;

    bool m_schemaSet : 1;
    bool m_fontSet : 1;
    bool m_wordWrapMarkerSet : 1;
    bool m_showIndentationLinesSet : 1;
    bool m_showWholeBracketExpressionSet : 1;
    bool m_backgroundColorSet : 1;
    bool m_selectionColorSet : 1;
    bool m_highlightedLineColorSet : 1;
    bool m_highlightedBracketColorSet : 1;
    bool m_wordWrapMarkerColorSet : 1;
    bool m_tabMarkerColorSet : 1;
    bool m_indentationLineColorSet : 1;
    bool m_iconBarColorSet : 1;
    bool m_foldingColorSet : 1;
    bool m_lineNumberColorSet : 1;
    bool m_currentLineNumberColorSet : 1;
    bool m_separatorColorSet : 1;
    bool m_spellingMistakeLineColorSet : 1;
    bool m_templateColorsSet : 1;
    bool m_modifiedLineColorSet : 1;
    bool m_savedLineColorSet : 1;
    bool m_searchHighlightColorSet : 1;
    bool m_replaceHighlightColorSet : 1;
    QBitArray m_lineMarkerColorSet;

private:
    static KateRendererConfig *s_global;
    KateRenderer *m_renderer = nullptr;
};

#endif
