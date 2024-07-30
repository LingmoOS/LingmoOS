/*
    SPDX-FileCopyrightText: 2002 John Firebaugh <jfirebaugh@kde.org>
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001-2010 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kate_view_h
#define kate_view_h

#include <ktexteditor/linerange.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/view.h>

#include <QJsonDocument>
#include <QPointer>
#include <QTimer>

#include <array>

#include "katetextfolding.h"
#include "katetextrange.h"

namespace KTextEditor
{
class AnnotationModel;
class Message;
class InlineNoteProvider;
class DocumentPrivate;
}

namespace Kate
{
class TextCursor;
}
class KateBookmarks;
class KateRendererConfig;
class KateViewConfig;
class KateRenderer;
class KateSpellCheckDialog;
class KateCompletionWidget;
class KateViewInternal;
class KateViewBar;
class KateTextPreview;
class KateGotoBar;
class KateDictionaryBar;
class KateSpellingMenu;
class KateMessageWidget;
class KateIconBorder;
class KateStatusBar;
class KateViewEncodingAction;
class KateModeMenu;
class KateAbstractInputMode;
class KateScriptActionMenu;
class KateMessageLayout;
class KateInlineNoteData;
class MulticursorTest;

class KToggleAction;
class KSelectAction;

class QAction;
class QTextLayout;
class QSpacerItem;
class QMenu;
class QActionGroup;

namespace KTextEditor
{
//
// Kate KTextEditor::View class ;)
//
class KTEXTEDITOR_EXPORT ViewPrivate final : public KTextEditor::View
{
    Q_OBJECT

    friend class KTextEditor::View;
    friend class ::KateViewInternal;
    friend class ::KateIconBorder;
    friend class ::KateTextPreview;
    friend MulticursorTest;

public:
    ViewPrivate(KTextEditor::DocumentPrivate *doc, QWidget *parent, KTextEditor::MainWindow *mainWindow = nullptr);
    ~ViewPrivate() override;

    /**
     * Get the view's main window, if any
     * \return the view's main window
     */
    KTextEditor::MainWindow *mainWindow() const override
    {
        return m_mainWindow;
    }

    KTextEditor::Document *document() const override;

    ViewMode viewMode() const override;
    QString viewModeHuman() const override;
    InputMode viewInputMode() const override;
    QString viewInputModeHuman() const override;

    void setViewInputMode(InputMode mode) override
    {
        setInputMode(mode);
    }

    void setInputMode(InputMode mode, const bool rememberInConfig = true);

public:
    KateViewInternal *getViewInternal()
    {
        return m_viewInternal;
    }

    //
    // KTextEditor::ClipboardInterface
    //
public Q_SLOTS:
    void paste(const QString *textToPaste = nullptr);
    void cut();
    void copy();
    void screenshot();

private Q_SLOTS:
    /**
     * Paste the global mouse selection. Support for Selection is provided only
     * on systems with a global mouse selection (e.g. X11).
     */
    void pasteSelection();

    /**
     * Copy current selected stuff and paste previous content of clipboard as one operation.
     */
    void swapWithClipboard();

    /**
     * Wrap lines touched by the selection with respect of existing paragraphs.
     * Work is done by KTextEditor::DocumentPrivate::wrapParagraph
     */
    void applyWordWrap();

    /**
     * Copy the current file name and line number
     */
    void copyFileLocation() const;

    //
    // KTextEditor::PopupMenuInterface
    //
public:
    void setContextMenu(QMenu *menu) override;
    QMenu *contextMenu() const override;
    QMenu *defaultContextMenu(QMenu *menu = nullptr) const override;

private Q_SLOTS:
    void aboutToShowContextMenu();
    void aboutToHideContextMenu();

private:
    QPointer<QMenu> m_contextMenu;

    //
    // KTextEditor::ViewCursorInterface
    //
public:
    bool setCursorPosition(KTextEditor::Cursor position) override;

    KTextEditor::Cursor cursorPosition() const override;

    KTextEditor::Cursor cursorPositionVirtual() const override;

    QPoint cursorToCoordinate(KTextEditor::Cursor cursor) const override;

    KTextEditor::Cursor coordinatesToCursor(const QPoint &coord) const override;

    QPoint cursorPositionCoordinates() const override;

    bool setCursorPositionVisual(const KTextEditor::Cursor position);

    QScrollBar *verticalScrollBar() const override;
    QScrollBar *horizontalScrollBar() const override;

    /**
     * Return the virtual cursor column, each tab is expanded into the
     * document's tabWidth characters. If word wrap is off, the cursor may be
     * behind the line's length.
     */
    int virtualCursorColumn() const;

    bool mouseTrackingEnabled() const override;
    bool setMouseTrackingEnabled(bool enable) override;

    /*
     * multicursor stuff
     */

    // Helper structs to work with multicursors
    struct PlainSecondaryCursor {
        KTextEditor::Cursor pos;
        KTextEditor::Range range;
        friend bool operator<(const PlainSecondaryCursor &l, const PlainSecondaryCursor &r)
        {
            return l.pos < r.pos;
        }
    };
    struct SecondaryCursor {
        std::unique_ptr<Kate::TextCursor> pos;
        std::unique_ptr<Kate::TextRange> range;
        KTextEditor::Cursor anchor = KTextEditor::Cursor::invalid();

        KTextEditor::Cursor cursor() const
        {
            return pos->toCursor();
        }

        friend bool operator<(const SecondaryCursor &l, const SecondaryCursor &r)
        {
            return l.cursor() < r.cursor();
        }

        friend bool operator<(const SecondaryCursor &l, KTextEditor::Cursor r)
        {
            return l.cursor() < r;
        }

        friend bool operator==(const SecondaryCursor &l, const SecondaryCursor &r)
        {
            return l.cursor() == r.cursor();
        }

        void clearSelection()
        {
            range.reset();
            anchor = KTextEditor::Cursor::invalid();
        }
    };

    // Just a helper to control the states in which we disallow multicursor
    bool isMulticursorNotAllowed() const;

    // Adds a secondary cursor
    void addSecondaryCursor(KTextEditor::Cursor cursor);
    void setSecondaryCursors(const QList<KTextEditor::Cursor> &positions);

    const std::vector<SecondaryCursor> &secondaryCursors() const;
    QList<PlainSecondaryCursor> plainSecondaryCursors() const;
    void addSecondaryCursorsWithSelection(const QList<PlainSecondaryCursor> &cursorsWithSelection);

    void clearSecondaryCursors();
    void clearSecondarySelections();

    // Check all the cursors, and remove cursors which have the same positions
    // if @p matchLine is true, cursors whose line are same will be considered equal
    void ensureUniqueCursors(bool matchLine = false);

    // For multicursor external api
    QList<KTextEditor::Cursor> cursors() const;
    QList<KTextEditor::Range> selectionRanges() const;

    void setCursors(const QList<KTextEditor::Cursor> &cursorPositions);
    void setSelections(const QList<KTextEditor::Range> &selectionRanges);

    // Returns true if primary or secondary cursors have selection
    bool hasSelections() const;

private:
    KTEXTEDITOR_NO_EXPORT
    bool removeSecondaryCursors(const std::vector<KTextEditor::Cursor> &cursorToRemove, bool removeIfOverlapsSelection = false);
    KTEXTEDITOR_NO_EXPORT
    Kate::TextRange *newSecondarySelectionRange(KTextEditor::Range);
    KTEXTEDITOR_NO_EXPORT
    void sortCursors();
    KTEXTEDITOR_NO_EXPORT
    void paintCursors();

    std::vector<SecondaryCursor> m_secondaryCursors;
    bool m_skipCurrentSelection = false;

    void addSecondaryCursorDown();
    // exported for multicursortest
    void addSecondaryCursorUp();
    // exported for multicursortest

private:
    KTEXTEDITOR_NO_EXPORT
    void notifyMousePositionChanged(const KTextEditor::Cursor newPosition);

    // Internal
public:
    bool setCursorPositionInternal(const KTextEditor::Cursor position, uint tabwidth = 1, bool calledExternally = false);

    //
    // KTextEditor::ConfigInterface
    //
public:
    QStringList configKeys() const override;
    QVariant configValue(const QString &key) override;
    void setConfigValue(const QString &key, const QVariant &value) override;

public:
    /**
     * Try to fold an unfolded range starting at @p line
     * @return the new folded range on success, otherwise an unvalid range
     */
    KTextEditor::Range foldLine(int line);

    /**
     * Try to unfold a folded range starting at @p line
     * @return true when a range was unfolded, otherwise false
     */
    bool unfoldLine(int line);

    /**
     * Try to toggle the folding state of a range starting at line @p line
     * @return true when the line was toggled, false when not
     */
    bool toggleFoldingOfLine(int line);

    /**
     * Try to change all the foldings inside a folding range starting at @p line
     * but not the range itself starting there.
     * However, should the range itself be folded, will only the range unfolded
     * and the containing ranges kept untouched.
     * Should the range not contain other ranges will the range itself folded,
     * @return true when any range was folded or unfolded, otherwise false
     */
    bool toggleFoldingsInRange(int line);

    // Code Completion Interface
public:
    bool isCompletionActive() const override;
    void startCompletion(KTextEditor::Range word, KTextEditor::CodeCompletionModel *model) override;
    void startCompletion(const Range &word,
                         const QList<KTextEditor::CodeCompletionModel *> &models = QList<KTextEditor::CodeCompletionModel *>(),
                         KTextEditor::CodeCompletionModel::InvocationType invocationType = KTextEditor::CodeCompletionModel::ManualInvocation) override;
    void abortCompletion() override;
    void forceCompletion() override;
    void registerCompletionModel(KTextEditor::CodeCompletionModel *model) override;
    void unregisterCompletionModel(KTextEditor::CodeCompletionModel *model) override;
    bool isCompletionModelRegistered(KTextEditor::CodeCompletionModel *model) const;
    QList<KTextEditor::CodeCompletionModel *> codeCompletionModels() const override;
    bool isAutomaticInvocationEnabled() const override;
    void setAutomaticInvocationEnabled(bool enabled = true) override;

Q_SIGNALS:
    void completionExecuted(KTextEditor::View *view, KTextEditor::Cursor position, KTextEditor::CodeCompletionModel *model, const QModelIndex &);
    void completionAborted(KTextEditor::View *view);

public Q_SLOTS:
    void userInvokedCompletion();

public:
    KateCompletionWidget *completionWidget() const;
    mutable KateCompletionWidget *m_completionWidget;
    void sendCompletionExecuted(const KTextEditor::Cursor position, KTextEditor::CodeCompletionModel *model, const QModelIndex &index);
    void sendCompletionAborted();

    //
    // KTextEditor::TextHintInterface
    //
public:
    void registerTextHintProvider(KTextEditor::TextHintProvider *provider) override;
    void unregisterTextHintProvider(KTextEditor::TextHintProvider *provider) override;
    void setTextHintDelay(int delay) override;
    int textHintDelay() const override;

public:
    bool dynWordWrap() const
    {
        return m_hasWrap;
    }

    //
    // Inline Notes Interface
    //
public:
    void registerInlineNoteProvider(KTextEditor::InlineNoteProvider *provider) override;
    void unregisterInlineNoteProvider(KTextEditor::InlineNoteProvider *provider) override;
    QRect inlineNoteRect(const KateInlineNoteData &note) const;

    QVarLengthArray<KateInlineNoteData, 8> inlineNotes(int line) const;

private:
    std::vector<KTextEditor::InlineNoteProvider *> m_inlineNoteProviders;

private Q_SLOTS:
    void inlineNotesReset();
    void inlineNotesLineChanged(int line);

    //
    // KTextEditor::SelectionInterface stuff
    //
public Q_SLOTS:
    bool setSelection(KTextEditor::Range selection) override;

    bool removeSelection() override
    {
        return clearSelection();
    }

    bool removeSelectionText() override
    {
        return removeSelectedText();
    }

    bool setBlockSelection(bool on) override;
    bool toggleBlockSelection();

    bool clearSelection();
    bool clearSelection(bool redraw, bool finishedChangingSelection = true);

    bool removeSelectedText();

    bool selectAll();

public:
    bool selection() const override;
    QString selectionText() const override;
    bool blockSelection() const override;
    KTextEditor::Range selectionRange() const override;

    static void blockFix(KTextEditor::Range &range);

    //
    // Arbitrary Syntax HL + Action extensions
    //
public:
    // Action association extension
    void deactivateEditActions();
    void activateEditActions();

    //
    // internal helper stuff, for katerenderer and so on
    //
public:
    // should cursor be wrapped ? take config + blockselection state in account
    bool wrapCursor() const;

    // some internal functions to get selection state of a line/col
    bool cursorSelected(const KTextEditor::Cursor cursor);
    bool lineSelected(int line);
    bool lineEndSelected(const KTextEditor::Cursor lineEndPos);
    bool lineHasSelected(int line);
    bool lineIsSelection(int line);

    void ensureCursorColumnValid();

    void tagSelection(KTextEditor::Range oldSelection);

    void selectWord(const KTextEditor::Cursor cursor);
    void selectLine(const KTextEditor::Cursor cursor);

    // BEGIN EDIT STUFF
public:
    void editStart();
    void editEnd(int editTagLineStart, int editTagLineEnd, bool tagFrom);

    void editSetCursor(const KTextEditor::Cursor cursor);
    // END

    // BEGIN TAG & CLEAR
public:
    bool tagLine(const KTextEditor::Cursor virtualCursor);

    bool tagRange(KTextEditor::Range range, bool realLines = false);
    bool tagLines(KTextEditor::LineRange lineRange, bool realLines = false);
    bool tagLines(KTextEditor::Cursor start, KTextEditor::Cursor end, bool realCursors = false);
    bool tagLines(KTextEditor::Range range, bool realRange = false);

    void tagAll();

    void clear();

    void repaintText(bool paintOnlyDirty = false);

    void updateView(bool changed = false);
    // END

    //
    // KTextEditor::AnnotationView
    //
public:
    void setAnnotationModel(KTextEditor::AnnotationModel *model) override;
    KTextEditor::AnnotationModel *annotationModel() const override;
    void setAnnotationBorderVisible(bool visible) override;
    bool isAnnotationBorderVisible() const override;
    void setAnnotationItemDelegate(KTextEditor::AbstractAnnotationItemDelegate *delegate) override;
    KTextEditor::AbstractAnnotationItemDelegate *annotationItemDelegate() const override;
    void setAnnotationUniformItemSizes(bool enable) override;
    bool uniformAnnotationItemSizes() const override;

Q_SIGNALS:
    void navigateLeft();
    void navigateRight();
    void navigateUp();
    void navigateDown();
    void navigateAccept();
    void navigateBack();

private:
    KTextEditor::AnnotationModel *m_annotationModel;

    //
    // KTextEditor::View
    //
public:
    void emitNavigateLeft()
    {
        Q_EMIT navigateLeft();
    }
    void emitNavigateRight()
    {
        Q_EMIT navigateRight();
    }
    void emitNavigateUp()
    {
        Q_EMIT navigateUp();
    }
    void emitNavigateDown()
    {
        Q_EMIT navigateDown();
    }
    void emitNavigateAccept()
    {
        Q_EMIT navigateAccept();
    }
    void emitNavigateBack()
    {
        Q_EMIT navigateBack();
    }
    /**
     Return values for "save" related commands.
    */
    bool isOverwriteMode() const;

    bool isLineRTL(int line) const;

    QTextLayout *textLayout(const KTextEditor::Cursor pos) const;

public Q_SLOTS:
    void indent();
    void unIndent();
    void cleanIndent();
    void formatIndent();
    void align(); // alias of formatIndent, for backward compatibility
    void alignOn();
    void comment();
    void uncomment();
    void toggleComment();
    void killLine();

    /**
     * Sets the cursor to the previous editing position in this document
     */
    void goToPreviousEditingPosition();

    /**
     * Sets the cursor to the next editing position in this document.
     */
    void goToNextEditingPosition();

    /**
     * Uppercases selected text, or an alphabetic character next to the cursor.
     */
    void uppercase();

    /**
     * Lowercases selected text, or an alphabetic character next to the cursor.
     */
    void lowercase();

    /**
     * Capitalizes the selection (makes each word start with an uppercase) or
     * the word under the cursor.
     */
    void capitalize();

    /**
     * Joins lines touched by the selection.
     */
    void joinLines();

    /**
     * Performs a line break (insert a new line char) at current cursor position
     * and indent the new line.
     *
     * Most work is done by @c KTextEditor::DocumentPrivate::newLine and
     * @c KateAutoIndent::userTypedChar
     * @see KTextEditor::DocumentPrivate::newLine, KateAutoIndent
     */
    void keyReturn();

    /**
     * Performs a line break (insert a new line char) at current cursor position
     * but keep all leading non word char as indent for the new line.
     */
    void smartNewline();

    /**
     * Inserts a new line character at the current cursor position, with
     * the newly inserted line having no indentation regardless of indentation
     * settings. Useful e.g. for inserting a new, empty, line to separate
     * blocks of code inside a function.
     */
    void noIndentNewline();

    void newLineAbove();

    void newLineBelow();

    /**
     * Insert a tabulator char at current cursor position.
     */
    void backspace();

    /**
     * Remove the word left from the current cursor position including all leading
     * space.
     * @see KateViewInternal::wordPrev
     */
    void deleteWordLeft();

    /**
     * Remove the current selection. When nothing is selected the char right
     * from the current cursor position is removed.
     * @see KTextEditor::DocumentPrivate::del
     */
    void keyDelete();

    /**
     * When the char right from the current cursor position is a space is all
     * space to the right removed. Otherwise is the word to the right including
     * all trialling space removed.
     * @see KateViewInternal::wordNext
     */
    void deleteWordRight();

    /**
     * Transpose the characters left and right from the current cursor position
     * and move the cursor one position to the right. If the char right to the
     * current cursor position is a new line char, nothing is done.
     * @see KTextEditor::DocumentPrivate::transpose
     */
    void transpose();

    /**
     * Transpose the word at the current cursor position with the word to the right (or to the left for RTL layouts).
     * If the word is the last in the line, try swapping with the previous word instead.
     * If the word is the only one in the line, no swapping is done.
     * @see KTextEditor::DocumentPrivate::swapTextRanges
     */
    void transposeWord();
    void cursorLeft();
    void shiftCursorLeft();
    void cursorRight();
    void shiftCursorRight();
    void wordLeft();
    void shiftWordLeft();
    void wordRight();
    void shiftWordRight();
    void markSelection();
    void home();
    void shiftHome();
    void end();
    void shiftEnd();
    void up();
    void shiftUp();
    void down();
    void shiftDown();
    void scrollUp();
    void scrollDown();
    void topOfView();
    void shiftTopOfView();
    void bottomOfView();
    void shiftBottomOfView();
    void pageUp();
    void shiftPageUp();
    void pageDown();
    void shiftPageDown();
    void top();
    void shiftTop();
    void bottom();
    void shiftBottom();
    void toMatchingBracket();
    void shiftToMatchingBracket();
    void toPrevModifiedLine();
    void toNextModifiedLine();
    /**
     * Insert a tabulator char at current cursor position.
     */
    void insertTab();

    void gotoLine();

    // config file / session management functions
public:
    /**
     * Read session settings from the given \p config.
     *
     * Known flags:
     *  "SkipUrl" => don't save/restore the file
     *  "SkipMode" => don't save/restore the mode
     *  "SkipHighlighting" => don't save/restore the highlighting
     *  "SkipEncoding" => don't save/restore the encoding
     *
     * \param config read the session settings from this KConfigGroup
     * \param flags additional flags
     * \see writeSessionConfig()
     */
    void readSessionConfig(const KConfigGroup &config, const QSet<QString> &flags = QSet<QString>()) override;

    /**
     * Write session settings to the \p config.
     * See readSessionConfig() for more details.
     *
     * \param config write the session settings to this KConfigGroup
     * \param flags additional flags
     * \see readSessionConfig()
     */
    void writeSessionConfig(KConfigGroup &config, const QSet<QString> &flags = QSet<QString>()) override;

public Q_SLOTS:
    void setEol(int eol);
    void setAddBom(bool enabled);
    void find();
    void findSelectedForwards();
    void findSelectedBackwards();
    void findNextOccurunceAndSelect();
    void findAllOccuruncesAndSelect();
    void skipCurrentOccurunceSelection();
    void replace();
    void findNext();
    void findPrevious();
    void showSearchWrappedHint(bool isReverseSearch);
    void createMultiCursorsFromSelection();
    void removeCursorsFromEmptyLines();

    void setFoldingMarkersOn(bool enable); // Not in KTextEditor::View, but should be
    void setIconBorder(bool enable);
    void setLineNumbersOn(bool enable);
    void setScrollBarMarks(bool enable);
    void setScrollBarMiniMap(bool enable);
    void setScrollBarMiniMapAll(bool enable);
    void setScrollBarMiniMapWidth(int width);
    void toggleFoldingMarkers();
    void toggleIconBorder();
    void toggleLineNumbersOn();
    void toggleScrollBarMarks();
    void toggleScrollBarMiniMap();
    void toggleScrollBarMiniMapAll();
    void toggleShowSpaces();
    void toggleDynWordWrap();
    void setDynWrapIndicators(int mode);

public:
    int getEol() const;
    QMenu *getEolMenu();

public:
    KateRenderer *renderer();
    KateRendererConfig *rendererConfig();

    bool iconBorder();
    bool lineNumbersOn();
    bool scrollBarMarks();
    bool scrollBarMiniMap();
    bool scrollBarMiniMapAll();
    int dynWrapIndicators();
    bool foldingMarkersOn();
    bool forceRTLDirection() const;

private Q_SLOTS:
    /**
     * used to update actions after selection changed
     */
    void slotSelectionChanged();

    void toggleInputMode();
    void cycleInputMode();

public:
    /**
     * accessor to katedocument pointer
     * @return pointer to document
     */
    KTextEditor::DocumentPrivate *doc()
    {
        return m_doc;
    }
    const KTextEditor::DocumentPrivate *doc() const
    {
        return m_doc;
    }

public Q_SLOTS:
    void slotUpdateUndo();
    void toggleInsert();
    void reloadFile();
    void toggleWWMarker();
    void toggleNPSpaces();
    void toggleWordCount(bool on);
    void toggleWriteLock();
    void switchToCmdLine();
    void slotReadWriteChanged();
    void toggleCamelCaseCursor();

Q_SIGNALS:
    void dropEventPass(QDropEvent *);

public:
    /**
     * Folding handler for this view.
     * @return folding handler
     */
    Kate::TextFolding &textFolding()
    {
        return m_textFolding;
    }

public:
    void slotTextInserted(KTextEditor::View *view, const KTextEditor::Cursor position, const QString &text);

    void exportHtmlToFile(const QString &file);

private Q_SLOTS:
    void slotDocumentReloaded();
    void slotDocumentAboutToReload();
    void slotGotFocus();
    void slotLostFocus();
    void slotSaveCanceled(const QString &error);
    void slotConfigDialog();
    void exportHtmlToClipboard();
    void exportHtmlToFile();

public Q_SLOTS:
    void slotFoldToplevelNodes();
    void slotExpandToplevelNodes();
    void slotToggleFolding();
    void slotToggleFoldingsInRange();

private:
    KTEXTEDITOR_NO_EXPORT
    void setupLayout();
    KTEXTEDITOR_NO_EXPORT
    void setupConnections();
    KTEXTEDITOR_NO_EXPORT
    void setupActions();
    KTEXTEDITOR_NO_EXPORT
    void setupEditActions();
    KTEXTEDITOR_NO_EXPORT
    void setupCodeFolding();
    KTEXTEDITOR_NO_EXPORT
    void setupSpeechActions();

    std::vector<QAction *> m_editActions;
    QAction *m_editUndo;
    QAction *m_editRedo;
    bool m_gotoBottomAfterReload;
    bool m_markedSelection;
    KToggleAction *m_toggleFoldingMarkers;
    KToggleAction *m_toggleIconBar;
    KToggleAction *m_toggleLineNumbers;
    KToggleAction *m_toggleScrollBarMarks;
    KToggleAction *m_toggleScrollBarMiniMap;
    KToggleAction *m_toggleScrollBarMiniMapAll;
    KToggleAction *m_toggleShowSpace;
    KToggleAction *m_toggleDynWrap;
    KSelectAction *m_setDynWrapIndicators;
    KToggleAction *m_toggleWWMarker;
    KToggleAction *m_toggleNPSpaces;
    KToggleAction *m_toggleWordCount;
    QAction *m_switchCmdLine;
    KToggleAction *m_viInputModeAction;

    KSelectAction *m_setEndOfLine;
    KToggleAction *m_addBom;

    QAction *m_cut;
    QAction *m_copy;
    QAction *m_copyHtmlAction;
    QAction *m_paste;
    QAction *m_clipboardHistory;
    // always nullptr if paste selection isn't supported
    QAction *m_pasteSelection = nullptr;
    QAction *m_swapWithClipboard;
    QAction *m_selectAll;
    QAction *m_deSelect;
    QAction *m_screenshotSelection = nullptr;

    QActionGroup *m_inputModeActions;

    KToggleAction *m_toggleBlockSelection;
    KToggleAction *m_toggleInsert;
    KToggleAction *m_toggleWriteLock;
    KToggleAction *m_forceRTLDirection;

    bool m_hasWrap;
    bool m_forceRTL = false;
    bool m_accessibilityEnabled = false;

    KTextEditor::DocumentPrivate *const m_doc;
    Kate::TextFolding m_textFolding;
    KateViewConfig *const m_config;
    KateRenderer *const m_renderer;
    KateViewInternal *const m_viewInternal;
    KateSpellCheckDialog *m_spell;
    KateBookmarks *const m_bookmarks;

    //* margins
    QSpacerItem *m_topSpacer;
    QSpacerItem *m_leftSpacer;
    QSpacerItem *m_rightSpacer;
    QSpacerItem *m_bottomSpacer;

private Q_SLOTS:
    void slotHlChanged();

    /**
     * Configuration
     */
public:
    inline KateViewConfig *config()
    {
        return m_config;
    }

    void updateConfig();

    void updateDocumentConfig();

    void updateRendererConfig();

private Q_SLOTS:
    void updateFoldingConfig();

private:
    bool m_startingUp;
    bool m_updatingDocumentConfig;

    // stores the current selection
    Kate::TextRange m_selection;

    // do we select normal or blockwise ?
    bool blockSelect;

    // templates
public:
    bool insertTemplateInternal(const KTextEditor::Cursor insertPosition, const QString &templateString, const QString &script = QString());

    /**
     * Accessors to the bars...
     */
public:
    KateViewBar *bottomViewBar() const;
    KateDictionaryBar *dictionaryBar();

private:
    KTEXTEDITOR_NO_EXPORT
    KateGotoBar *gotoBar();

    /**
     * viewbar + its widgets
     * they are created on demand...
     */
private:
    // created in constructor of the view
    KateViewBar *m_bottomViewBar;

    // created on demand..., only access them through the above accessors....
    KateGotoBar *m_gotoBar;
    KateDictionaryBar *m_dictionaryBar;

    // input modes
public:
    KateAbstractInputMode *currentInputMode() const;

public:
    KTextEditor::Range visibleRange();

Q_SIGNALS:
    void displayRangeChanged(KTextEditor::ViewPrivate *view);

protected:
    bool event(QEvent *e) override;

    KToggleAction *m_toggleOnTheFlySpellCheck;
    KateSpellingMenu *m_spellingMenu;

protected Q_SLOTS:
    void toggleOnTheFlySpellCheck(bool b);

public Q_SLOTS:
    void changeDictionary();
    void reflectOnTheFlySpellCheckStatus(bool enabled);

public:
    KateSpellingMenu *spellingMenu();

private:
    bool m_userContextMenuSet;

private Q_SLOTS:
    /**
     * save folding state before document reload
     */
    void saveFoldingState();

    /**
     * restore folding state after document reload
     */
    void applyFoldingState();

public:
    /**
     * Clear any saved folding state
     */
    void clearFoldingState();

private:
    KTEXTEDITOR_NO_EXPORT
    void clearHighlights();
    KTEXTEDITOR_NO_EXPORT
    void createHighlights();

private:
    KTEXTEDITOR_NO_EXPORT
    void selectionChangedForHighlights();

    /**
     * saved folding state
     */
    QJsonDocument m_savedFoldingState;

    QString m_currentTextForHighlights;

    std::vector<std::unique_ptr<KTextEditor::MovingRange>> m_rangesForHighlights;

public:
    /**
     * Attribute of a range changed or range with attribute changed in given line range.
     * @param lineRange line range that the change spans
     * @param needsRepaint do we need to trigger repaints? e.g. if ranges with attributes change
     */
    void notifyAboutRangeChange(KTextEditor::LineRange lineRange, bool needsRepaint);

private Q_SLOTS:
    /**
     * Delayed update for view after text ranges changed
     */
    void slotDelayedUpdateOfView();

Q_SIGNALS:
    /**
     * Delayed update for view after text ranges changed
     */
    void delayedUpdateOfView();

    /**
     * Emitted whenever the caret enter or leave a range.
     * ATM only used by KateStatusBar to update the dict button
     */
    void caretChangedRange(KTextEditor::View *);

public:
    /**
     * set of ranges which had the mouse inside last time, used for rendering
     * @return set of ranges which had the mouse inside last time checked
     */
    const QSet<Kate::TextRange *> *rangesMouseIn() const
    {
        return &m_rangesMouseIn;
    }

    /**
     * set of ranges which had the caret inside last time, used for rendering
     * @return set of ranges which had the caret inside last time checked
     */
    const QSet<Kate::TextRange *> *rangesCaretIn() const
    {
        return &m_rangesCaretIn;
    }

    /**
     * check if ranges changed for mouse in and caret in
     * @param activationType type of activation to check
     */
    void updateRangesIn(KTextEditor::Attribute::ActivationType activationType);

    //
    // helpers for delayed view update after ranges changes
    //
private:
    /**
     * delayed update timer
     */
    QTimer m_delayedUpdateTimer;

    /**
     * line range to update
     */
    KTextEditor::LineRange m_lineToUpdateRange;

    /**
     * set of ranges which had the mouse inside last time
     */
    QSet<Kate::TextRange *> m_rangesMouseIn;

    /**
     * set of ranges which had the caret inside last time
     */
    QSet<Kate::TextRange *> m_rangesCaretIn;

    //
    // forward impl for KTextEditor::MessageInterface
    //
public:
    /**
     * Used by Document::postMessage().
     */
    void postMessage(KTextEditor::Message *message, QList<std::shared_ptr<QAction>> actions);

private:
    /**
     * Message widgets showing KTextEditor::Messages.
     * The index of the array maps to the enum KTextEditor::Message::MessagePosition.
     */
    std::array<KateMessageWidget *, 5> m_messageWidgets{{nullptr}};
    /** Layout for floating notifications */
    KateMessageLayout *m_notificationLayout = nullptr;

    // for unit test 'tests/messagetest.cpp'
public:
    KateMessageWidget *messageWidget();

private:
    /**
     * The main window responsible for this view, if any
     */
    QPointer<KTextEditor::MainWindow> m_mainWindow;

    //
    // KTextEditor::PrintInterface
    //
public Q_SLOTS:
    bool print() override;
    void printPreview() override;

public:
    /**
     * Get the view status bar
     * @return status bar, in enabled
     */
    KateStatusBar *statusBar() const
    {
        return m_statusBar;
    }

    /**
     * Toogle status bar, e.g. create or remove it
     */
    void toggleStatusBar();

    /**
     * Get the encoding menu
     * @return the encoding menu
     */
    KateViewEncodingAction *encodingAction() const
    {
        return m_encodingAction;
    }

    /**
     * Get the mode menu
     * @return the mode menu
     */
    KateModeMenu *modeAction() const
    {
        return m_modeAction;
    }

private:
    /**
     * the status bar of this view
     */
    KateStatusBar *m_statusBar;

    /**
     * the encoding selection menu, used by view + status bar
     */
    KateViewEncodingAction *m_encodingAction;

    /**
     * the mode selection menu, used by view
     */
    KateModeMenu *m_modeAction;

    /**
     * is automatic invocation of completion disabled temporarily?
     */
    bool m_temporaryAutomaticInvocationDisabled;

public:
    /**
     * Returns the attribute for the default style \p defaultStyle.
     */
    Attribute::Ptr defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle defaultStyle) const override;

    /**
     * Get the list of AttributeBlocks for a given \p line in the document.
     *
     * \return list of AttributeBlocks for given \p line.
     */
    QList<KTextEditor::AttributeBlock> lineAttributes(int line) override;

private:
    // remember folding state to prevent refolding the first line if it was manually unfolded,
    // e.g. when saving a file or changing other config vars
    bool m_autoFoldedFirstLine;

public:
    void setScrollPositionInternal(KTextEditor::Cursor cursor);

    void setHorizontalScrollPositionInternal(int x);

    KTextEditor::Cursor maxScrollPositionInternal() const;

    int firstDisplayedLineInternal(LineType lineType) const;

    int lastDisplayedLineInternal(LineType lineType) const;

    QRect textAreaRectInternal() const;

private:
    /**
     * script action menu, stored in scoped pointer to ensure
     * destruction before other QObject auto-cleanup as it
     * manage sub objects on its own that have this view as parent
     */
    std::unique_ptr<KateScriptActionMenu> m_scriptActionMenu;

    // for showSearchWrappedHint()
    QPointer<KTextEditor::Message> m_wrappedMessage;
    bool m_isLastSearchReversed = false;
};

}

#endif
