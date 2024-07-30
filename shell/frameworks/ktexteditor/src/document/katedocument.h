/*
    SPDX-FileCopyrightText: 2001-2004 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2001 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KATE_DOCUMENT_H_
#define _KATE_DOCUMENT_H_

#include <QPointer>
#include <QStack>
#include <QTimer>

#include <ktexteditor/document.h>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/movingrangefeedback.h>

#include "katetextline.h"
#include <ktexteditor_export.h>

class KJob;
class KateTemplateHandler;
namespace KTextEditor
{
class Plugin;
class Attribute;
class TemplateScript;
}

namespace KIO
{
class TransferJob;
}

namespace Kate
{
class SwapFile;
}

class KateBuffer;
namespace KTextEditor
{
class Message;
class ViewPrivate;
}
class KateDocumentConfig;
class KateHighlighting;
class KateUndoManager;
class KateOnTheFlyChecker;
class KateDocumentTest;

class KateAutoIndent;
class KateModOnHdPrompt;
class KToggleAction;

/**
 * @brief Backend of KTextEditor::Document related public KTextEditor interfaces.
 *
 * @warning This file is @e private API and not part of the public
 *          KTextEditor interfaces.
 */
class KTEXTEDITOR_EXPORT KTextEditor::DocumentPrivate final : public KTextEditor::Document, private KTextEditor::MovingRangeFeedback
{
    Q_OBJECT

    friend class KTextEditor::Document;
    friend class ::KateDocumentTest;
    friend class ::KateBuffer;

public:
    explicit DocumentPrivate(const KPluginMetaData &data,
                             bool bSingleViewMode = false,
                             bool bReadOnly = false,
                             QWidget *parentWidget = nullptr,
                             QObject * = nullptr);
    explicit DocumentPrivate(bool bSingleViewMode = false, bool bReadOnly = false, QWidget *parentWidget = nullptr, QObject * = nullptr)
        : DocumentPrivate(KPluginMetaData(), bSingleViewMode, bReadOnly, parentWidget)
    {
    }
    ~DocumentPrivate() override;

    using ReadWritePart::closeUrl;
    bool closeUrl() override;

    bool openUrl(const QUrl &url) override;

    KTextEditor::Range rangeOnLine(KTextEditor::Range range, int line) const;

private:
    KTEXTEDITOR_NO_EXPORT
    void showAndSetOpeningErrorAccess();
    /*
     * Overload this to have on-demand view creation
     */
public:
    /**
     * @return The widget defined by this part, set by setWidget().
     */
    QWidget *widget() override;

public:
    bool readOnly() const
    {
        return m_bReadOnly;
    }
    bool singleViewMode() const
    {
        return m_bSingleViewMode;
    }

private:
    // only to make part work, don't change it !
    const bool m_bSingleViewMode;
    const bool m_bReadOnly;

    //
    // KTextEditor::Document stuff
    //
public:
    KTextEditor::View *createView(QWidget *parent, KTextEditor::MainWindow *mainWindow = nullptr) override;

    QList<KTextEditor::View *> views() const override
    {
        return m_views;
    }

    KTextEditor::View *activeView() const
    {
        return m_activeView;
    }

private:
    KTextEditor::View *m_activeView = nullptr;

    //
    // KTextEditor::EditInterface stuff
    //
public Q_SLOTS:
    bool setText(const QString &) override;
    bool setText(const QStringList &text) override;
    bool clear() override;

    bool insertText(KTextEditor::Cursor position, const QString &s, bool block = false) override;
    bool insertText(KTextEditor::Cursor position, const QStringList &text, bool block = false) override;

    bool insertLine(int line, const QString &s) override;
    bool insertLines(int line, const QStringList &s) override;

    bool removeText(KTextEditor::Range range, bool block = false) override;
    bool removeLine(int line) override;

    bool replaceText(KTextEditor::Range range, const QString &s, bool block = false) override;

    // unhide method...
    bool replaceText(KTextEditor::Range r, const QStringList &l, bool b) override
    {
        return KTextEditor::Document::replaceText(r, l, b);
    }

public:
    bool isEditingTransactionRunning() const override;
    QString text(KTextEditor::Range range, bool blockwise = false) const override;
    QStringList textLines(KTextEditor::Range range, bool block = false) const override;
    QString text() const override;
    QString line(int line) const override;
    QChar characterAt(KTextEditor::Cursor position) const override;
    QString wordAt(KTextEditor::Cursor cursor) const override;
    KTextEditor::Range wordRangeAt(KTextEditor::Cursor cursor) const override;
    bool isValidTextPosition(KTextEditor::Cursor cursor) const override;
    int lines() const override;
    bool isLineModified(int line) const override;
    bool isLineSaved(int line) const override;
    bool isLineTouched(int line) const override;
    KTextEditor::Cursor documentEnd() const override;
    qsizetype totalCharacters() const override;
    int lineLength(int line) const override;
    qsizetype cursorToOffset(KTextEditor::Cursor c) const override;
    KTextEditor::Cursor offsetToCursor(qsizetype offset) const override;

Q_SIGNALS:
    void charactersSemiInteractivelyInserted(KTextEditor::Cursor position, const QString &text);

    /**
     * The \p document emits this signal whenever text was inserted.  The
     * insertion occurred at range.start(), and new text now occupies up to
     * range.end().
     * \param document document which emitted this signal
     * \param range range that the newly inserted text occupies
     * \see insertText(), insertLine()
     */
    void textInsertedRange(KTextEditor::Document *document, KTextEditor::Range range);

    /**
     * The \p document emits this signal whenever \p range was removed, i.e.
     * text was removed.
     * \param document document which emitted this signal
     * \param range range that the removed text previously occupied
     * \param oldText the text that has been removed
     * \see removeText(), removeLine(), clear()
     */
    void textRemoved(KTextEditor::Document *document, KTextEditor::Range range, const QString &oldText);

public:
    // BEGIN editStart/editEnd (start, end, undo, cursor update, view update)
    /**
     * Enclose editor actions with @p editStart() and @p editEnd() to group
     * them.
     */
    bool editStart();

    /**
     * End a editor operation.
     * @see editStart()
     */
    bool editEnd();

    void pushEditState();
    void popEditState();

    // END editStart/editEnd

    void inputMethodStart();
    void inputMethodEnd();

    // BEGIN LINE BASED INSERT/REMOVE STUFF (editStart() and editEnd() included)
    /**
     * Add a string in the given line/column
     * @param line line number
     * @param col column
     * @param s string to be inserted
     * @return true on success
     */
    bool editInsertText(int line, int col, const QString &s, bool notify = true);

    /**
     * Remove a string in the given line/column
     * @param line line number
     * @param col column
     * @param len length of text to be removed
     * @return true on success
     */
    bool editRemoveText(int line, int col, int len);

    /**
     * Mark @p line as @p autowrapped. This is necessary if static word warp is
     * enabled, because we have to know whether to insert a new line or add the
     * wrapped words to the following line.
     * @param line line number
     * @param autowrapped autowrapped?
     * @return true on success
     */
    bool editMarkLineAutoWrapped(int line, bool autowrapped);

    /**
     * Wrap @p line. If @p newLine is true, ignore the textline's flag
     * KateTextLine::flagAutoWrapped and force a new line. Whether a new line
     * was needed/added you can grab with @p newLineAdded.
     * @param line line number
     * @param col column
     * @param newLine if true, force a new line
     * @param newLineAdded return value is true, if new line was added (may be 0)
     * @return true on success
     */
    bool editWrapLine(int line, int col, bool newLine = true, bool *newLineAdded = nullptr, bool notify = true);

    /**
     * Unwrap @p line. If @p removeLine is true, we force to join the lines. If
     * @p removeLine is true, @p length is ignored (eg not needed).
     * @param line line number
     * @param removeLine if true, force to remove the next line
     * @return true on success
     */
    bool editUnWrapLine(int line, bool removeLine = true, int length = 0);

    /**
     * Insert a string at the given line.
     * @param line line number
     * @param s string to insert
     * @return true on success
     */
    bool editInsertLine(int line, const QString &s, bool notify = true);

    /**
     * Remove a line
     * @param line line number
     * @return true on success
     */
    bool editRemoveLine(int line);

    bool editRemoveLines(int from, int to);

    /**
     * Warp a line
     * @param startLine line to begin wrapping
     * @param endLine line to stop wrapping
     * @return true on success
     */
    bool wrapText(int startLine, int endLine);

    /**
     * Wrap lines touched by the selection with respect of existing paragraphs.
     * To do so will the paragraph prior to the wrap joined as one single line
     * which cause an almost perfect wrapped paragraph as long as there are no
     * unneeded spaces exist or some formatting like this comment block.
     * Without any selection the current line is wrapped.
     * Empty lines around each paragraph are untouched.
     * @param first line to begin wrapping
     * @param last line to stop wrapping
     * @return true on success
     */
    bool wrapParagraph(int first, int last);
    // END LINE BASED INSERT/REMOVE STUFF

Q_SIGNALS:
    /**
     * Emitted when text from @p line was wrapped at position pos onto line @p nextLine.
     */
    void editLineWrapped(int line, int col, int len);

    /**
     * Emitted each time text from @p nextLine was upwrapped onto @p line.
     */
    void editLineUnWrapped(int line, int col);

public:
    bool isEditRunning() const;

    void setUndoMergeAllEdits(bool merge);

    enum EditingPositionKind { Previous, Next };

    /**
     *Returns the next or previous position cursor in this document from the stack depending on the argument passed.
     *@return cursor invalid if m_editingStack empty
     */
    KTextEditor::Cursor lastEditingPosition(EditingPositionKind nextOrPrevious, KTextEditor::Cursor);

private:
    int editSessionNumber = 0;
    QStack<int> editStateStack;
    bool editIsRunning = false;
    bool m_undoMergeAllEdits = false;
    KTextEditor::Cursor m_editLastChangeStartCursor = KTextEditor::Cursor::invalid();
    QStack<std::shared_ptr<KTextEditor::MovingCursor>> m_editingStack;
    int m_editingStackPosition = -1;

    //
    // KTextEditor::UndoInterface stuff
    //
public Q_SLOTS:
    void undo();
    void redo();

    /**
     * Removes all the elements in m_editingStack of the respective document.
     */
    void clearEditingPosStack();

    /**
     * Saves the editing positions into the stack.
     * If the consecutive editings happens in the same line, then remove
     * the previous and add the new one with updated column no.
     */
    void saveEditingPositions(const KTextEditor::Cursor cursor);

public:
    uint undoCount() const;
    uint redoCount() const;

    KateUndoManager *undoManager()
    {
        return m_undoManager;
    }

protected:
    KateUndoManager *const m_undoManager;

Q_SIGNALS:
    void undoChanged();

public:
    QList<KTextEditor::Range> searchText(KTextEditor::Range range, const QString &pattern, const KTextEditor::SearchOptions options) const;

private:
    /**
     * Return a widget suitable to be used as a dialog parent.
     */
    KTEXTEDITOR_NO_EXPORT
    QWidget *dialogParent();

    /**
     * Wrapper around QFileDialog::getSaveFileUrl, will use proper dialogParent
     * and try it's best to find a good directory as start
     * @param dialogTitle dialog title string
     * @return url to save to or empty url if aborted
     */
    KTEXTEDITOR_NO_EXPORT
    QUrl getSaveFileUrl(const QString &dialogTitle);

    /*
     * Access to the mode/highlighting subsystem
     */
public:
    /**
     * @copydoc KTextEditor::Document::defaultStyleAt()
     */
    KSyntaxHighlighting::Theme::TextStyle defaultStyleAt(KTextEditor::Cursor position) const override;

    /**
     * Return the name of the currently used mode
     * \return name of the used mode
     */
    QString mode() const override;

    /**
     * Return the name of the currently used mode
     * \return name of the used mode
     */
    QString highlightingMode() const override;

    /**
     * Return a list of the names of all possible modes
     * \return list of mode names
     */
    QStringList modes() const override;

    /**
     * Return a list of the names of all possible modes
     * \return list of mode names
     */
    QStringList highlightingModes() const override;

    /**
     * Set the current mode of the document by giving its name
     * \param name name of the mode to use for this document
     * \return \e true on success, otherwise \e false
     */
    bool setMode(const QString &name) override;

    /**
     * Set the current mode of the document by giving its name
     * \param name name of the mode to use for this document
     * \return \e true on success, otherwise \e false
     */
    bool setHighlightingMode(const QString &name) override;
    /**
     * Returns the name of the section for a highlight given its @p index in the highlight
     * list (as returned by highlightModes()).
     * You can use this function to build a tree of the highlight names, organized in sections.
     * \param index in the highlight list for which to find the section name.
     */
    QString highlightingModeSection(int index) const override;

    /**
     * Returns the name of the section for a mode given its @p index in the highlight
     * list (as returned by modes()).
     * You can use this function to build a tree of the mode names, organized in sections.
     * \param index index in the highlight list for which to find the section name.
     */
    QString modeSection(int index) const override;

    /*
     * Helpers....
     */
public:
    void bufferHlChanged();

    /**
     * allow to mark, that we changed hl on user wish and should not reset it
     * atm used for the user visible menu to select highlightings
     */
    void setDontChangeHlOnSave();

    /**
     * Set that the BOM marker is forced via the tool menu
     */
    void bomSetByUser();

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

    //
    // KTextEditor::MarkInterface
    //
public Q_SLOTS:
    void setMark(int line, uint markType) override;
    void clearMark(int line) override;

    void addMark(int line, uint markType) override;
    void removeMark(int line, uint markType) override;

    void clearMarks() override;

    void requestMarkTooltip(int line, QPoint position);

    /// Returns true if the click on the mark should not be further processed
    bool handleMarkClick(int line);

    /// Returns true if the context-menu event should not further be processed
    bool handleMarkContextMenu(int line, QPoint position);

    void setMarkDescription(Document::MarkTypes, const QString &) override;

    void setEditableMarks(uint markMask) override;
    void setMarkIcon(Document::MarkTypes markType, const QIcon &icon) override;

public:
    uint mark(int line) override;
    const QHash<int, KTextEditor::Mark *> &marks() override;
    QString markDescription(Document::MarkTypes) const override;
    virtual QColor markColor(Document::MarkTypes) const;
    uint editableMarks() const override;
    QIcon markIcon(Document::MarkTypes markType) const override;

private:
    QHash<int, KTextEditor::Mark *> m_marks;
    QHash<int, QIcon> m_markIcons; // QPixmap or QIcon, KF6: remove QPixmap support
    QHash<int, QString> m_markDescriptions;
    uint m_editableMarks = markType01;

    //
    // KTextEditor::PrintInterface
    //
public Q_SLOTS:
    bool print() override;
    void printPreview() override;

    //
    // KTextEditor::DocumentInfoInterface ( ### unfinished )
    //
public:
    /**
     * Tries to detect mime-type based on file name and content of buffer.
     *
     * @return the name of the mimetype for the document.
     */
    QString mimeType() override;

    //
    // once was KTextEditor::VariableInterface
    //
public:
    /**
     * Returns the value for the variable @p name.
     * If the Document does not have a variable called @p name,
     * an empty QString() is returned.
     *
     * // TODO KF6: expose in KTextEditor::Document?
     *
     * @param name variable to query
     * @return value of the variable @p name
     * @see setVariable()
     */
    virtual QString variable(const QString &name) const;

    /**
     * Set the variable @p name to @p value. Setting and changing a variable
     * has immediate effect on the Document. For instance, setting the variable
     * @e indent-mode to @e cstyle will immediately cause the Document to load
     * the C Style indenter.
     *
     * // TODO KF6: expose in KTextEditor::Document?
     *
     * @param name the variable name
     * @param value the value to be set
     * @see variable()
     */
    virtual void setVariable(const QString &name, const QString &value);

private:
    std::map<QString, QString> m_storedVariables;

    //
    // MovingInterface API
    //
public:
    /**
     * Create a new moving cursor for this document.
     * @param position position of the moving cursor to create
     * @param insertBehavior insertion behavior
     * @return new moving cursor for the document
     */
    KTextEditor::MovingCursor *newMovingCursor(KTextEditor::Cursor position,
                                               KTextEditor::MovingCursor::InsertBehavior insertBehavior = KTextEditor::MovingCursor::MoveOnInsert) override;

    /**
     * Create a new moving range for this document.
     * @param range range of the moving range to create
     * @param insertBehaviors insertion behaviors
     * @param emptyBehavior behavior on becoming empty
     * @return new moving range for the document
     */
    KTextEditor::MovingRange *newMovingRange(KTextEditor::Range range,
                                             KTextEditor::MovingRange::InsertBehaviors insertBehaviors = KTextEditor::MovingRange::DoNotExpand,
                                             KTextEditor::MovingRange::EmptyBehavior emptyBehavior = KTextEditor::MovingRange::AllowEmpty) override;

    /**
     * Current revision
     * @return current revision
     */
    qint64 revision() const override;

    /**
     * Last revision the buffer got successful saved
     * @return last revision buffer got saved, -1 if none
     */
    qint64 lastSavedRevision() const override;

    /**
     * Lock a revision, this will keep it around until released again.
     * But all revisions will always be cleared on buffer clear() (and therefor load())
     * @param revision revision to lock
     */
    void lockRevision(qint64 revision) override;

    /**
     * Release a revision.
     * @param revision revision to release
     */
    void unlockRevision(qint64 revision) override;

    /**
     * Transform a cursor from one revision to an other.
     * @param cursor cursor to transform
     * @param insertBehavior behavior of this cursor on insert of text at its position
     * @param fromRevision from this revision we want to transform
     * @param toRevision to this revision we want to transform, default of -1 is current revision
     */
    void transformCursor(KTextEditor::Cursor &cursor,
                         KTextEditor::MovingCursor::InsertBehavior insertBehavior,
                         qint64 fromRevision,
                         qint64 toRevision = -1) override;

    /**
     * Transform a cursor from one revision to an other.
     * @param line line number of the cursor to transform
     * @param column column number of the cursor to transform
     * @param insertBehavior behavior of this cursor on insert of text at its position
     * @param fromRevision from this revision we want to transform
     * @param toRevision to this revision we want to transform, default of -1 is current revision
     */
    void
    transformCursor(int &line, int &column, KTextEditor::MovingCursor::InsertBehavior insertBehavior, qint64 fromRevision, qint64 toRevision = -1) override;

    /**
     * Transform a range from one revision to an other.
     * @param range range to transform
     * @param insertBehaviors behavior of this range on insert of text at its position
     * @param emptyBehavior behavior on becoming empty
     * @param fromRevision from this revision we want to transform
     * @param toRevision to this revision we want to transform, default of -1 is current revision
     */
    void transformRange(KTextEditor::Range &range,
                        KTextEditor::MovingRange::InsertBehaviors insertBehaviors,
                        KTextEditor::MovingRange::EmptyBehavior emptyBehavior,
                        qint64 fromRevision,
                        qint64 toRevision = -1) override;

    //
    // Annotation Interface
    //
public:
    void setAnnotationModel(KTextEditor::AnnotationModel *model) override;
    KTextEditor::AnnotationModel *annotationModel() const override;

Q_SIGNALS:
    void annotationModelChanged(KTextEditor::AnnotationModel *, KTextEditor::AnnotationModel *);

private:
    KTextEditor::AnnotationModel *m_annotationModel = nullptr;

    //
    // KParts::ReadWrite stuff
    //
public:
    /**
     * open the file obtained by the kparts framework
     * the framework abstracts the loading of remote files
     * @return success
     */
    bool openFile() override;

    /**
     * save the file obtained by the kparts framework
     * the framework abstracts the uploading of remote files
     * @return success
     */
    bool saveFile() override;

    void setReadWrite(bool rw = true) override;

    void setModified(bool m) override;

    bool isAutoReload();
    KToggleAction *autoReloadToggleAction()
    {
        return m_autoReloadMode;
    };
    void delayAutoReload();

private Q_SLOTS:
    void autoReloadToggled(bool b);

private:
    KTEXTEDITOR_NO_EXPORT
    void activateDirWatch(const QString &useFileName = QString());
    KTEXTEDITOR_NO_EXPORT
    void deactivateDirWatch();

    QString m_dirWatchFile;

    /**
     * Make backup copy during saveFile, if configured that way.
     * @return success? else saveFile should return false and not write the file
     */
    KTEXTEDITOR_NO_EXPORT
    bool createBackupFile();

public:
    /**
     * Type chars in a view.
     * Characters are filtered in KateViewInternal::isAcceptableInput() before calling typeChars.
     *
     * @param view view that received the input
     * @param chars characters to type
     */
    void typeChars(KTextEditor::ViewPrivate *view, QString chars);

    /**
     * gets the last line number (lines() - 1)
     */
    inline int lastLine() const
    {
        return lines() - 1;
    }

    // Repaint all of all of the views
    void repaintViews(bool paintOnlyDirty = true);

    KateHighlighting *highlight() const;

public Q_SLOTS:
    void tagLines(KTextEditor::LineRange lineRange);
    void tagLine(int line);

private Q_SLOTS:
    void internalHlChanged();

public:
    void addView(KTextEditor::View *);
    /** removes the view from the list of views. The view is *not* deleted.
     * That's your job. Or, easier, just delete the view in the first place.
     * It will remove itself. TODO: this could be converted to a private slot
     * connected to the view's destroyed() signal. It is not currently called
     * anywhere except from the KTextEditor::ViewPrivate destructor.
     */
    void removeView(KTextEditor::View *);
    void setActiveView(KTextEditor::View *);

    bool ownedView(KTextEditor::ViewPrivate *);

    int toVirtualColumn(int line, int column) const;
    int toVirtualColumn(const KTextEditor::Cursor) const;
    int fromVirtualColumn(int line, int column) const;
    int fromVirtualColumn(const KTextEditor::Cursor) const;

    enum NewLineIndent { Indent, NoIndent };
    enum NewLinePos { Normal, Above, Below };

    void newLine(KTextEditor::ViewPrivate *view, NewLineIndent indent = NewLineIndent::Indent, NewLinePos newLinePos = Normal); // Changes input
    void backspace(KTextEditor::ViewPrivate *view);
    void del(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor);
    void transpose(const KTextEditor::Cursor);
    void swapTextRanges(KTextEditor::Range firstWord, KTextEditor::Range secondWord);
    bool multiPaste(KTextEditor::ViewPrivate *view, const QStringList &texts);
    void paste(KTextEditor::ViewPrivate *view, const QString &text);

public:
    enum CommentType {
        UnComment = -1,
        ToggleComment = 0,
        Comment = 1,
    };

private:
    // Helper function for use with multiple cursors
    KTEXTEDITOR_NO_EXPORT
    KTextEditor::Cursor backspaceAtCursor(KTextEditor::ViewPrivate *v, KTextEditor::Cursor c);
    void commentSelection(KTextEditor::Range selection, KTextEditor::Cursor c, bool blockSelect, CommentType changeType);
    // exported for katedocument_test
    KTEXTEDITOR_NO_EXPORT
    bool skipAutoBrace(QChar closingBracket, KTextEditor::Cursor pos);

public:
    void indent(KTextEditor::Range range, int change);
    void comment(KTextEditor::ViewPrivate *view, uint line, uint column, CommentType change);
    void align(KTextEditor::ViewPrivate *view, KTextEditor::Range range);
    void alignOn(KTextEditor::Range range, const QString &pattern, bool blockwise);
    void insertTab(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor);

    enum TextTransform { Uppercase, Lowercase, Capitalize };

    /**
      Handling uppercase, lowercase and capitalize for the view.

      If there is a selection, that is transformed, otherwise for uppercase or
      lowercase the character right of the cursor is transformed, for capitalize
      the word under the cursor is transformed.
    */
    void transform(KTextEditor::ViewPrivate *view, KTextEditor::Cursor, TextTransform);
    /**
      Unwrap a range of lines.
    */
    void joinLines(uint first, uint last);

private:
    KTEXTEDITOR_NO_EXPORT
    void transformCursorOrRange(KTextEditor::ViewPrivate *view, const KTextEditor::Cursor, KTextEditor::Range selection, TextTransform);

    KTEXTEDITOR_NO_EXPORT
    bool removeStringFromBeginning(int line, const QString &str);
    KTEXTEDITOR_NO_EXPORT
    bool removeStringFromEnd(int line, const QString &str);

    /**
      Expand tabs to spaces in typed text, if enabled.
      @param cursorPos The current cursor position for the inserted characters.
      @param str The typed characters to expand.
    */
    KTEXTEDITOR_NO_EXPORT
    QString eventuallyReplaceTabs(const KTextEditor::Cursor cursorPos, const QString &str) const;

    /**
      Find the position (line and col) of the next char
      that is not a space. If found line and col point to the found character.
      Otherwise they have both the value -1.
      @param line Line of the character which is examined first.
      @param col Column of the character which is examined first.
      @return True if the specified or a following character is not a space
               Otherwise false.
    */
    KTEXTEDITOR_NO_EXPORT
    bool nextNonSpaceCharPos(int &line, int &col);

    /**
      Find the position (line and col) of the previous char
      that is not a space. If found line and col point to the found character.
      Otherwise they have both the value -1.
      @return True if the specified or a preceding character is not a space.
               Otherwise false.
    */
    KTEXTEDITOR_NO_EXPORT
    bool previousNonSpaceCharPos(int &line, int &col);

    /**
     * Sets a comment marker as defined by the language providing the attribute
     * @p attrib on the line @p line
     */
    KTEXTEDITOR_NO_EXPORT
    void addStartLineCommentToSingleLine(int line, int attrib = 0);
    /**
     * Removes a comment marker as defined by the language providing the attribute
     * @p attrib on the line @p line
     */
    KTEXTEDITOR_NO_EXPORT
    bool removeStartLineCommentFromSingleLine(int line, int attrib = 0);

    /**
     * @see addStartLineCommentToSingleLine.
     */
    KTEXTEDITOR_NO_EXPORT
    void addStartStopCommentToSingleLine(int line, int attrib = 0);
    /**
     *@see removeStartLineCommentFromSingleLine.
     */
    KTEXTEDITOR_NO_EXPORT
    bool removeStartStopCommentFromSingleLine(int line, int attrib = 0);
    /**
     *@see removeStartLineCommentFromSingleLine.
     */
    KTEXTEDITOR_NO_EXPORT
    bool removeStartStopCommentFromRegion(const KTextEditor::Cursor start, const KTextEditor::Cursor end, int attrib = 0);

    /**
     * Add a comment marker as defined by the language providing the attribute
     * @p attrib to each line in the selection.
     */
    KTEXTEDITOR_NO_EXPORT
    void addStartStopCommentToSelection(KTextEditor::Range, bool blockSelection, int attrib = 0);
    /**
     * @see addStartStopCommentToSelection.
     */
    KTEXTEDITOR_NO_EXPORT
    void addStartLineCommentToSelection(KTextEditor::Range, int attrib = 0);

    /**
     * Removes comment markers relevant to the language providing
     * the attribute @p attrib from each line in the selection.
     *
     * @return whether the operation succeeded.
     */
    KTEXTEDITOR_NO_EXPORT
    bool removeStartStopCommentFromSelection(KTextEditor::Range, int attrib = 0);
    /**
     * @see removeStartStopCommentFromSelection.
     */
    KTEXTEDITOR_NO_EXPORT
    bool removeStartLineCommentFromSelection(KTextEditor::Range, int attrib, bool toggleComment);

public:
    KTextEditor::Range findMatchingBracket(const KTextEditor::Cursor start, int maxLines);

public:
    QString documentName() const override
    {
        return m_docName;
    }

private:
    KTEXTEDITOR_NO_EXPORT
    void updateDocName();
    KTEXTEDITOR_NO_EXPORT
    static void uniquifyDocNames(const std::vector<KTextEditor::DocumentPrivate *> &docs);

public:
    /**
     * @return whether the document is modified on disk since last saved
     */
    bool isModifiedOnDisc()
    {
        return m_modOnHd;
    }

    void setModifiedOnDisk(ModifiedOnDiskReason reason) override;

    void setModifiedOnDiskWarning(bool on) override;

public Q_SLOTS:
    /**
     * Ask the user what to do, if the file has been modified on disk.
     * Reimplemented from KTextEditor::Document.
     */
    virtual void slotModifiedOnDisk(KTextEditor::View *v = nullptr);

    /**
     * Reloads the current document from disk if possible
     */
    bool documentReload() override;

    bool documentSave() override;
    bool documentSaveAs() override;
    bool documentSaveAsWithEncoding(const QString &encoding);
    void documentSaveCopyAs();

    bool save() override;

public:
    bool saveAs(const QUrl &url) override;

private:
    // helper to handle the embedded notification for externally modified files
    QPointer<KateModOnHdPrompt> m_modOnHdHandler;

private Q_SLOTS:
    void onModOnHdSaveAs();
    void onModOnHdClose();
    void onModOnHdReload();
    void onModOnHdAutoReload();
    void onModOnHdIgnore();

public:
    bool setEncoding(const QString &e) override;
    QString encoding() const override;

public Q_SLOTS:
    void setWordWrap(bool on);
    void setWordWrapAt(uint col);

public:
    bool wordWrap() const;
    uint wordWrapAt() const;

public Q_SLOTS:
    void setPageUpDownMovesCursor(bool on);

public:
    bool pageUpDownMovesCursor() const;

    // code folding
public:
    /**
     * Same as plainKateTextLine(), except that it is made sure
     * the line is highlighted.
     */
    Kate::TextLine kateTextLine(int i);

    //! @copydoc KateBuffer::plainLine()
    Kate::TextLine plainKateTextLine(int i);

Q_SIGNALS:
    void aboutToRemoveText(KTextEditor::Range);

private Q_SLOTS:
    void slotModOnHdDirty(const QString &path);
    void slotModOnHdCreated(const QString &path);
    void slotModOnHdDeleted(const QString &path);
    void slotDelayedHandleModOnHd();

private:
    /**
     * Create a git compatible sha1 checksum of the file, if it is a local file.
     * The result can be accessed through KateBuffer::digest().
     *
     * @return whether the operation was attempted and succeeded.
     */
    bool createDigest();
    // exported for katedocument_test

    /**
     * create a string for the modonhd warnings, giving the reason.
     */
    KTEXTEDITOR_NO_EXPORT
    QString reasonedMOHString() const;

    /**
     * Removes all trailing whitespace in the document and add new line at eof
     * if configured that way.
     */
    KTEXTEDITOR_NO_EXPORT
    void removeTrailingSpacesAndAddNewLineAtEof();

public:
    /**
     * This function doesn't check for config and is
     * available for use all the time via an action
     */
    void removeAllTrailingSpaces();

    /**
     * Returns a git compatible sha1 checksum of this document on disk.
     * @return checksum for this document on disk
     */
    QByteArray checksum() const override;

    /**
     * @return false if @p newType is an invalid mode.
     */
    bool updateFileType(const QString &newType, bool user = false);

    QString fileType() const
    {
        return m_fileType;
    }

    /**
     * Get access to buffer of this document.
     * Is needed to create cursors and ranges for example.
     * @return document buffer
     */
    KateBuffer &buffer()
    {
        return *m_buffer;
    }

    /**
     * set indentation mode by user
     * this will remember that a user did set it and will avoid reset on save
     */
    void rememberUserDidSetIndentationMode()
    {
        m_indenterSetByUser = true;
    }

    /**
     * User did set encoding for next reload => enforce it!
     */
    void userSetEncodingForNextReload()
    {
        m_userSetEncodingForNextReload = true;
    }

    //
    // REALLY internal data ;)
    //
private:
    // text buffer
    KateBuffer *const m_buffer;

    // indenter
    KateAutoIndent *const m_indenter;

    bool m_hlSetByUser = false;
    bool m_bomSetByUser = false;
    bool m_indenterSetByUser = false;
    bool m_userSetEncodingForNextReload = false;

    bool m_modOnHd = false;
    KToggleAction *m_autoReloadMode;
    QTimer m_autoReloadThrottle;
    ModifiedOnDiskReason m_modOnHdReason = OnDiskUnmodified;
    ModifiedOnDiskReason m_prevModOnHdReason = OnDiskUnmodified;

    QString m_docName;
    int m_docNameNumber = 0;

    // file type !!!
    QString m_fileType;
    bool m_fileTypeSetByUser = false;

    /**
     * document is still reloading a file
     */
    bool m_reloading = false;

public Q_SLOTS:
    void slotQueryClose_save(bool *handled, bool *abortClosing);

public:
    bool queryClose() override;

    /**
     * Configuration
     */
public:
    KateDocumentConfig *config()
    {
        return m_config.get();
    }
    KateDocumentConfig *config() const
    {
        return m_config.get();
    }

    void updateConfig();

private:
    KTEXTEDITOR_NO_EXPORT
    void makeAttribs(bool needInvalidate = true);

    std::unique_ptr<KateDocumentConfig> const m_config;

    /**
     * Variable Reader
     * TODO add register functionality/ktexteditor interface
     */
private:
    /**
     * read dir config file
     */
    KTEXTEDITOR_NO_EXPORT
    void readDirConfig();

    /**
      Reads all the variables in the document.
      Called when opening/saving a document
      Returns true if variables were read
    */
    bool readVariables(bool onlyViewAndRenderer = false);
    // exported for katedocument_test

    /**
      Reads and applies the variables in a single line
      TODO registered variables gets saved in a [map]
    */
    void readVariableLine(const QString &t, bool onlyViewAndRenderer = false);
    // exported for katedocument_test
    /**
      Sets a view variable in all the views.
    */
    KTEXTEDITOR_NO_EXPORT
    void setViewVariable(const QString &var, const QString &val);
    /**
      @return weather a string value could be converted
      to a bool value as supported.
      The value is put in *result.
    */
    KTEXTEDITOR_NO_EXPORT
    static bool checkBoolValue(QString value, bool *result);
    /**
      @return weather a string value could be converted
      to a integer value.
      The value is put in *result.
    */
    KTEXTEDITOR_NO_EXPORT
    static bool checkIntValue(const QString &value, int *result);
    /**
      Feeds value into @p col using QColor::fromString() and returns
      whether the color is valid
    */
    KTEXTEDITOR_NO_EXPORT
    static bool checkColorValue(const QString &value, QColor &col);

    bool m_fileChangedDialogsActivated = false;

    //
    // KTextEditor::ConfigInterface
    //
public:
    QStringList configKeys() const override;
    QVariant configValue(const QString &key) override;
    void setConfigValue(const QString &key, const QVariant &value) override;

    //
    // KTextEditor::RecoveryInterface
    //
public:
    bool isDataRecoveryAvailable() const override;
    void recoverData() override;
    void discardDataRecovery() override;

    //
    // Highlighting information
    //
public:
    QStringList embeddedHighlightingModes() const override;
    QString highlightingModeAt(KTextEditor::Cursor position) override;

    //
    // BEGIN: KTextEditor::MessageInterface
    //
public:
    bool postMessage(KTextEditor::Message *message) override;

public Q_SLOTS:
    void messageDestroyed(KTextEditor::Message *message);

private:
    QHash<KTextEditor::Message *, QList<std::shared_ptr<QAction>>> m_messageHash;
    // END KTextEditor::MessageInterface

public:
    QString defaultDictionary() const;
    QList<QPair<KTextEditor::MovingRange *, QString>> dictionaryRanges() const;
    bool isOnTheFlySpellCheckingEnabled() const;
    KateOnTheFlyChecker *onTheFlySpellChecker() const
    {
        return m_onTheFlyChecker;
    }

    QString dictionaryForMisspelledRange(KTextEditor::Range range) const;
    void clearMisspellingForWord(const QString &word);

public Q_SLOTS:
    void clearDictionaryRanges();
    void setDictionary(const QString &dict, KTextEditor::Range range, bool blockmode);
    void setDictionary(const QString &dict, KTextEditor::Range range);
    void setDefaultDictionary(const QString &dict);
    void onTheFlySpellCheckingEnabled(bool enable);
    void refreshOnTheFlyCheck(KTextEditor::Range range = KTextEditor::Range::invalid());

Q_SIGNALS:
    void dictionaryRangesPresent(bool yesNo);
    void defaultDictionaryChanged(KTextEditor::DocumentPrivate *document);

public:
    bool containsCharacterEncoding(KTextEditor::Range range);

    typedef QList<QPair<int, int>> OffsetList;

    static int computePositionWrtOffsets(const OffsetList &offsetList, int pos);

    /**
     * The first OffsetList is from decoded to encoded, and the second OffsetList from
     * encoded to decoded.
     **/
    QString decodeCharacters(KTextEditor::Range range,
                             KTextEditor::DocumentPrivate::OffsetList &decToEncOffsetList,
                             KTextEditor::DocumentPrivate::OffsetList &encToDecOffsetList);
    void replaceCharactersByEncoding(KTextEditor::Range range);

protected:
    KateOnTheFlyChecker *m_onTheFlyChecker = nullptr;
    QString m_defaultDictionary;
    QList<QPair<KTextEditor::MovingRange *, QString>> m_dictionaryRanges;

    // from KTextEditor::MovingRangeFeedback
    void rangeInvalid(KTextEditor::MovingRange *movingRange) override;
    void rangeEmpty(KTextEditor::MovingRange *movingRange) override;

    void deleteDictionaryRange(KTextEditor::MovingRange *movingRange);

private:
    Kate::SwapFile *m_swapfile;

public:
    Kate::SwapFile *swapFile();

    // helpers for scripting and codefolding
    KSyntaxHighlighting::Theme::TextStyle defStyleNum(int line, int column);
    bool isComment(int line, int column);

public:
    /**
     * Find the next modified/saved line, starting at @p startLine. If @p down
     * is \e true, the search is performed downwards, otherwise upwards.
     * @return the touched line in the requested search direction, or -1 if not found
     */
    int findTouchedLine(int startLine, bool down);

private Q_SLOTS:
    /**
     * watch for all started io jobs to remember if file is perhaps loading atm
     * @param job started job
     */
    void slotStarted(KIO::Job *job);
    void slotCompleted();
    void slotCanceled();

    /**
     * trigger display of loading message, after 1000 ms
     */
    void slotTriggerLoadingMessage();

    /**
     * Abort loading
     */
    void slotAbortLoading();

    void slotUrlChanged(const QUrl &url);

private:
    /**
     * different possible states
     */
    enum DocumentStates {
        /**
         * Idle
         */
        DocumentIdle,

        /**
         * Loading
         */
        DocumentLoading,

        /**
         * Saving
         */
        DocumentSaving,

        /**
         * Pre Saving As, this is between ::saveAs is called and ::save
         */
        DocumentPreSavingAs,

        /**
         * Saving As
         */
        DocumentSavingAs
    };

    /**
     * current state
     */
    DocumentStates m_documentState = DocumentIdle;

    /**
     * read-write state before loading started
     */
    bool m_readWriteStateBeforeLoading = false;

    /**
     * if the document is untitled
     */
    bool m_isUntitled = true;
    /**
     * loading job, we want to cancel with cancel in the loading message
     */
    QPointer<KJob> m_loadingJob;

    /**
     * message to show during loading
     */
    QPointer<KTextEditor::Message> m_loadingMessage;

    /**
     * Was there any open error on last file loading?
     */
    bool m_openingError = false;

public:
    /**
     * reads the line length limit from config, if it is not overridden
     */
    int lineLengthLimit() const;

public Q_SLOTS:
    void openWithLineLengthLimitOverride();

private:
    /**
     * timer for delayed handling of mod on hd
     */
    QTimer m_modOnHdTimer;

private:
    /**
     * currently active template handler; there can be only one
     */
    QPointer<KateTemplateHandler> m_activeTemplateHandler;

private:
    /**
     * current autobrace range
     */
    std::unique_ptr<KTextEditor::MovingRange> m_currentAutobraceRange;
    /**
     * current autobrace closing character (e.g. ']')
     */
    QChar m_currentAutobraceClosingChar;

private Q_SLOTS:
    void checkCursorForAutobrace(KTextEditor::View *view, const KTextEditor::Cursor newPos);

public:
    void setActiveTemplateHandler(KateTemplateHandler *handler);

Q_SIGNALS:
    void loaded(KTextEditor::DocumentPrivate *document);

private:
    QList<KTextEditor::View *> m_views;
    QTimer m_autoSaveTimer;
};

#endif
