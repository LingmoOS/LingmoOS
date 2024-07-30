/*
    SPDX-FileCopyrightText: 2001 Christoph Cullmann <cullmann@kde.org>

    Documentation:
    SPDX-FileCopyrightText: 2005 Dominik Haumann <dhdev@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_VIEW_H
#define KTEXTEDITOR_VIEW_H

#include <ktexteditor_export.h>

// gui merging
#include <KSyntaxHighlighting/Theme>
#include <KXMLGUIClient>
#include <ktexteditor/codecompletionmodel.h>

// widget
#include <QSet>
#include <QWidget>

class QMenu;
class QScrollBar;
class KConfigGroup;

namespace KSyntaxHighlighting
{
class Theme;
}

namespace KTextEditor
{
class Document;
class MainWindow;
class ViewPrivate;
class Attribute;
class AttributeBlock;
class Range;
class Cursor;
class AnnotationModel;
class AbstractAnnotationItemDelegate;
class InlineNoteProvider;
class TextHintProvider;
class CodeCompletionModel;

/**
 * \class View view.h <KTextEditor/View>
 *
 * \brief A text widget with KXMLGUIClient that represents a Document.
 *
 * Topics:
 *  - \ref view_intro
 *  - \ref view_hook_into_gui
 *  - \ref view_selection
 *  - \ref view_cursors
 *  - \ref view_mouse_tracking
 *  - \ref view_modes
 *  - \ref view_config
 *  - \ref view_annoview
 *  - \ref view_inlinenote
 *  - \ref view_texthint
 *  - \ref view_compiface
 *
 * \section view_intro Introduction
 *
 * The View class represents a single view of a KTextEditor::Document,
 * get the document on which the view operates with document().
 * A view provides both the graphical representation of the text and the
 * KXMLGUIClient for the actions. The view itself does not provide
 * text manipulation, use the methods from the Document instead. The only
 * method to insert text is insertText(), which inserts the given text
 * at the current cursor position and emits the signal textInserted().
 *
 * Usually a view is created by using Document::createView().
 * Furthermore a view can have a context menu. Set it with setContextMenu()
 * and get it with contextMenu().
 *
 * \section view_hook_into_gui Merging the View's GUI
 *
 * A View is derived from the class KXMLGUIClient, so its GUI elements (like
 * menu entries and toolbar items) can be merged into the application's GUI
 * (or into a KXMLGUIFactory) by calling
 * \code
 * // view is of type KTextEditor::View*
 * mainWindow()->guiFactory()->addClient( view );
 * \endcode
 * You can add only one view as client, so if you have several views, you first
 * have to remove the current view, and then add the new one, like this
 * \code
 * mainWindow()->guiFactory()->removeClient( currentView );
 * mainWindow()->guiFactory()->addClient( newView );
 * \endcode
 *
 * \section view_selection Text Selection
 *
 * As the view is a graphical text editor it provides \e normal and \e block
 * text selection. You can check with selection() whether a selection exists.
 * removeSelection() will remove the selection without removing the text,
 * whereas removeSelectionText() also removes both, the selection and the
 * selected text. Use selectionText() to get the selected text and
 * setSelection() to specify the selected text range. The signal
 * selectionChanged() is emitted whenever the selection changed.
 *
 * \section view_cursors Cursor Positions
 *
 * A view has one Cursor which represents a line/column tuple. Two different
 * kinds of cursor positions are supported: first is the \e real cursor
 * position where a \e tab character only counts one character. Second is the
 * \e virtual cursor position, where a \e tab character counts as many
 * spaces as defined. Get the real position with cursorPosition() and the
 * virtual position with cursorPositionVirtual(). Set the real cursor
 * position with setCursorPosition(). The signal cursorPositionChanged() is
 * emitted whenever the cursor position changed.
 *
 * Screen coordinates of the current text cursor position in pixels are obtained
 * through cursorPositionCoordinates(). Further conversion of screen pixel
 * coordinates and text cursor positions are provided by cursorToCoordinate()
 * and coordinatesToCursor().
 *
 * \section view_mouse_tracking Mouse Tracking
 *
 * It is possible to get notified via the signal mousePositionChanged() for
 * mouse move events, if mouseTrackingEnabled() returns \e true. Mouse tracking
 * can be turned on/off by calling setMouseTrackingEnabled(). If an editor
 * implementation does not support mouse tracking, mouseTrackingEnabled() will
 * always return \e false.
 *
 * \section view_modes Input/View Modes
 *
 * A view supports several input modes. Common input modes are
 * \e NormalInputMode and \e ViInputMode. Which input modes the editor supports depends on the
 * implementation. The getter viewInputMode() returns enum \InputMode representing the current mode.
 *
 * Input modes can have their own view modes. In case of default \e NormalInputMode those are
 * \e NormalModeInsert and \e NormalModeOverwrite. You can use viewMode() getter to obtain those.
 *
 * For viewMode() and viewInputMode() there are also variants with \e Human suffix, which
 * returns the human readable representation (i18n) usable for displaying in user interface.
 *
 * Whenever the input/view mode changes the signals
 * viewInputModeChanged()/viewModeChanged() are emitted.
 *
 * \section view_config View Config
 * Config provides methods to access and modify the low level config information for a given
 * View
 * KTextEditor::View has support for the following keys:
 *  - line-numbers [bool], show/hide line numbers
 *  - icon-bar [bool], show/hide icon bar
 *  - folding-bar [bool], show/hide the folding bar
 *  - folding-preview [bool], enable/disable folding preview when mouse hovers
 *    on folded region
 *  - dynamic-word-wrap [bool], enable/disable dynamic word wrap
 *  - background-color [QColor], read/set the default background color
 *  - selection-color [QColor], read/set the default color for selections
 *  - search-highlight-color [QColor], read/set the background color for search
 *  - replace-highlight-color [QColor], read/set the background color for replaces
 *  - default-mark-type [uint], read/set the default mark type
 *  - allow-mark-menu [bool], enable/disable the menu shown when right clicking
 *    on the left gutter. When disabled, click on the gutter will always set
 *    or clear the mark of default type.
 *  - icon-border-color [QColor] read/set the icon border color (on the left,
 *    with the line numbers)
 *  - folding-marker-color [QColor] read/set folding marker colors (in the icon border)
 *  - line-number-color [QColor] read/set line number colors (in the icon border)
 *  - current-line-number-color [QColor] read/set current line number color (in the icon border)
 *  - modification-markers [bool] read/set whether the modification markers are shown
 *  - word-count [bool] enable/disable the counting of words and characters in the statusbar
 *  - line-count [bool] show/hide the total number of lines in the status bar (@since 5.66)
 *  - scrollbar-minimap [bool] enable/disable scrollbar minimap
 *  - scrollbar-preview [bool] enable/disable scrollbar text preview on hover
 *  - font [QFont] change the font
 *  - theme [QString] change the theme
 *  - word-completion-minimal-word-length [int] minimal word length to trigger word completion
 *  - enter-to-insert-completion [bool] enable/disable whether pressing enter inserts completion
 *
 * You can retrieve the value of a config key using configValue() and set the value
 * for a config key using setConfigValue().
 *
 * \section view_annoview Annotation Interface
 *
 * The Annotation Interface allows to do these things:
 * - (1) show/hide the annotation border along with the possibility to add actions
 *       into its context menu.
 * - (2) set a separate AnnotationModel for the View: Note that this interface
 *       inherits the AnnotationInterface.
 * - (3) set a custom AbstractAnnotationItemDelegate for the View.
 *
 * For a more detailed explanation about whether you want to set a custom
 * delegate for rendering the annotations, read the detailed documentation about the
 * AbstractAnnotationItemDelegate.
 *
 * \section view_inlinenote Inline Notes
 *
 * The inline notes interface provides a way to render arbitrary things in
 * the text. The text layout of the line is adapted to create space for the
 * note. Possible applications include showing a name of a function parameter
 * in a function call or rendering a square with a color preview next to CSS
 * color property.
 *
 * \image html inlinenote.png "Inline note showing a CSS color preview"
 *
 * To register as inline note provider, call registerInlineNoteProvider() with
 * an instance that inherits InlineNoteProvider. Finally, make sure you remove
 * your inline note provider by calling unregisterInlineNoteProvider().
 *
 * \section view_texthint Introduction
 *
 * The text hint interface provides a way to show tool tips for text located
 * under the mouse. Possible applications include showing a value of a variable
 * when debugging an application, or showing a complete path of an include
 * directive.
 *
 * \image html texthint.png "Text hint showing the contents of a variable"
 *
 * To register as text hint provider, call registerTextHintProvider() with an
 * instance that inherits TextHintProvider. Finally, make sure you remove your
 * text hint provider by calling unregisterTextHintProvider().
 *
 * Text hints are shown after the user hovers with the mouse for a delay of
 * textHintDelay() milliseconds over the same word. To change the delay, call
 * setTextHintDelay().
 *
 * \section view_compiface Completion Interface
 *
 * The Completion Interface is designed to provide code completion
 * functionality for a KTextEditor::View. This interface provides the basic
 * mechanisms to display a list of completions, update this list according
 * to user input, and allow the user to select a completion.
 *
 * Essentially, this provides an item view for the available completions. In
 * order to use this interface, you will need to implement a
 * CodeCompletionModel that generates the relevant completions given the
 * current input.
 *
 * More information about interfaces for the view can be found in
 * \ref kte_group_view_extensions.
 *
 * \see KTextEditor::Document, KXMLGUIClient
 * \author Christoph Cullmann \<cullmann@kde.org\>
 */
class KTEXTEDITOR_EXPORT View : public QWidget, public KXMLGUIClient
{
    Q_OBJECT

protected:
    /**
     * Constructor.
     *
     * Create a view attached to the widget \p parent.
     *
     * Pass it the internal implementation to store a d-pointer.
     *
     * \param impl d-pointer to use
     * \param parent parent widget
     * \see Document::createView()
     */
    View(ViewPrivate *impl, QWidget *parent);

public:
    /**
     * Virtual destructor.
     */
    ~View() override;

    /*
     * Accessor for the document
     */
public:
    /**
     * Get the view's \e document, that means the view is a view of the
     * returned document.
     * \return the view's document
     */
    virtual Document *document() const = 0;

    /*
     * General information about this view
     */
public:
    /**
     * Possible input modes.
     * These correspond to various modes the text editor might be in.
     */
    enum InputMode {
        NormalInputMode = 0, /**< Normal Mode. */
        ViInputMode = 1 /**< Vi mode. The view will behave like the editor vi(m) */
    };

    /**
     * Possible view modes
     * These correspond to various modes the text editor might be in.
     */
    enum ViewMode {
        NormalModeInsert = 0, /**< Insert mode. Characters will be added. */
        NormalModeOverwrite = 1, /**< Overwrite mode. Characters will be replaced. */

        ViModeNormal = 10,
        ViModeInsert = 11,
        ViModeVisual = 12,
        ViModeVisualLine = 13,
        ViModeVisualBlock = 14,
        ViModeReplace = 15
    };

    /**
     * Possible line types
     * \since 5.33
     */
    enum LineType {
        RealLine = 0, /**< Real line */
        VisibleLine = 1 /**< Visible line. Line that is not folded. */
    };
    /**
     * Get the current view mode/state.
     * This can be used to detect the view's current mode. For
     * example \NormalInputMode, \ViInputMode or whatever other input modes are
     * supported. \see viewModeHuman() for translated version.
     * \return current view mode/state
     * \see viewModeChanged()
     */
    virtual ViewMode viewMode() const = 0;

    /**
     * Get the current view mode state.
     * This can be used to visually indicate the view's current mode, for
     * example \e INSERT mode, \e OVERWRITE mode or \e COMMAND mode - or
     * whatever other edit modes are supported. The string should be
     * translated (i18n), as this is a user aimed representation of the view
     * state, which should be shown in the GUI, for example in the status bar.
     * This string may be rich-text.
     * \return Human-readable version of the view mode state
     * \see viewModeChanged()
     */
    virtual QString viewModeHuman() const = 0;

    /**
     * Set the view's new input mode.
     * \param inputMode new InputMode value
     * \see viewInputMode()
     * @since 5.54
     */
    virtual void setViewInputMode(InputMode inputMode) = 0;

    /**
     * Get the view's current input mode.
     * The current mode can be \NormalInputMode and \ViInputMode.
     * For human translated version \see viewInputModeHuman.
     *
     * \return the current input mode of this view
     * \see viewInputModeChanged()
     */
    virtual InputMode viewInputMode() const = 0;

    /**
     * Get the view's current input mode in human readable form.
     * The string should be translated (i18n). For id like version \see viewInputMode
     *
     * \return the current input mode of this view in human readable form
     */
    virtual QString viewInputModeHuman() const = 0;

    /**
     * Get the view's main window, if any
     * \return the view's main window, will always return at least some non-nullptr dummy interface
     */
    virtual KTextEditor::MainWindow *mainWindow() const = 0;

    /*
     * SIGNALS
     * following signals should be emitted by the editor view
     */
Q_SIGNALS:
    /**
     * This signal is emitted whenever the \p view gets the focus.
     * \param view view which gets focus
     * \see focusOut()
     */
    void focusIn(KTextEditor::View *view);

    /**
     * This signal is emitted whenever the \p view loses the focus.
     * \param view view which lost focus
     * \see focusIn()
     */
    void focusOut(KTextEditor::View *view);

    /**
     * This signal is emitted whenever the view mode of \p view changes.
     * \param view the view which changed its mode
     * \param mode new view mode
     * \see viewMode()
     */
    void viewModeChanged(KTextEditor::View *view, KTextEditor::View::ViewMode mode);

    /**
     * This signal is emitted whenever the \p view's input \p mode changes.
     * \param view view which changed its input mode
     * \param mode new input mode
     * \see viewInputMode()
     */
    void viewInputModeChanged(KTextEditor::View *view, KTextEditor::View::InputMode mode);

    /**
     * This signal is emitted from \p view whenever the users inserts \p text
     * at \p position, that means the user typed/pasted text.
     * \param view view in which the text was inserted
     * \param position position where the text was inserted
     * \param text the text the user has typed into the editor
     * \see insertText()
     */
    void textInserted(KTextEditor::View *view, KTextEditor::Cursor position, const QString &text);

    /*
     * Context menu handling
     */
public:
    /**
     * Set a context menu for this view to \p menu.
     *
     * \note any previously assigned menu is not deleted.  If you are finished
     *       with the previous menu, you may delete it.
     *
     * \warning Use this with care! Plugin xml gui clients are not merged
     *          into this menu!
     * \warning !!!!!! DON'T USE THIS FUNCTION, UNLESS YOU ARE SURE YOU DON'T WANT PLUGINS TO WORK !!!!!!
     *
     * \param menu new context menu object for this view
     * \see contextMenu()
     */
    virtual void setContextMenu(QMenu *menu) = 0;

    /**
     * Get the context menu for this view. The return value can be NULL
     * if no context menu object was set and kxmlgui is not initialized yet.
     * If there is no user set menu, the kxmlgui menu is returned. Do not delete this menu, if
     * if it is the xmlgui menu.
     * \return context menu object
     * \see setContextMenu()
     */
    virtual QMenu *contextMenu() const = 0;

    /**
     * Populate \a menu with default text editor actions.  If \a menu is
     * null, a menu will be created with the view as its parent.
     *
     * \note to use this menu, you will next need to call setContextMenu(),
     *       as this does not assign the new context menu.
     *
     * \warning This contains only basic options from the editor component
     *          (katepart). Plugins are \p not merged/integrated into it!
     *          If you want to be a better citizen and take full advantage
     *          of KTextEditor plugins do something like:
     * \code
     * KXMLGUIClient* client = view;
     * // search parent XmlGuiClient
     * while (client->parentClient()) {
     *   client = client->parentClient();
     * }
     *
     * if (client->factory()) {
     *   const QList<QWidget*> menuContainers = client->factory()->containers("menu");
     *   for (QWidget *w : menuContainers) {
     *     if (w->objectName() == "ktexteditor_popup") {
     *       // do something with the menu (ie adding an onshow handler)
     *       break;
     *     }
     *   }
     * }
     * \endcode
     * \warning or simply use the aboutToShow, aboutToHide signals !!!!!
     *
     * \param menu the menu to be populated, or null to create a new menu.
     * \return the menu, whether created or passed initially
     */
    virtual QMenu *defaultContextMenu(QMenu *menu = nullptr) const = 0;

Q_SIGNALS:
    /**
     * Signal which is emitted immediately prior to showing the current
     * context \a menu.
     */
    void contextMenuAboutToShow(KTextEditor::View *view, QMenu *menu);

    /*
     * Cursor handling
     */
public:
    /**
     * Set the view's new cursor to \p position. A \e TAB character
     * is handled as only on character.
     * \param position new cursor position
     * \return \e true on success, otherwise \e false
     * \see cursorPosition()
     */
    virtual bool setCursorPosition(Cursor position) = 0;

    /**
     * Set the view's new cursors to \p positions. A \e TAB character
     * is handled as only on character.
     *
     * This allows to create multiple cursors in this view.
     *
     * The first passed position will be used for the primary cursor
     * just like if you would call \ref setCursorPosition.
     *
     * \param positions new cursor positions
     * \see cursorPositions()
     *
     * @since 5.95
     */
    void setCursorPositions(const QList<KTextEditor::Cursor> &positions);

    /**
     * Get the view's current cursor position. A \e TAB character is
     * handled as only one character.
     * \return current cursor position
     * \see setCursorPosition()
     */
    virtual Cursor cursorPosition() const = 0;

    /**
     * Get the view's current cursor positions. A \e TAB character is
     * handled as only one character.
     *
     * The returned vector contains the primary cursor as first element.
     *
     * @since 5.95
     *
     * \return all currently existing cursors
     */
    QList<KTextEditor::Cursor> cursorPositions() const;

    /**
     * Get the current \e virtual cursor position, \e virtual means the
     * tabulator character (TAB) counts \e multiple characters, as configured
     * by the user (e.g. one TAB is 8 spaces). The virtual cursor
     * position provides access to the user visible values of the current
     * cursor position.
     *
     * \return virtual cursor position
     * \see cursorPosition()
     */
    virtual Cursor cursorPositionVirtual() const = 0;

    /**
     * Get the screen coordinates (x, y) of the supplied \a cursor relative
     * to the view widget in pixels. Thus, (0, 0) represents the top left hand
     * of the view widget.
     *
     * To map pixel coordinates to a Cursor position (the reverse transformation)
     * use coordinatesToCursor().
     *
     * \param cursor cursor to determine coordinate for.
     * \return cursor screen coordinates relative to the view widget
     *
     * \see cursorPositionCoordinates(), coordinatesToCursor()
     */
    virtual QPoint cursorToCoordinate(KTextEditor::Cursor cursor) const = 0;

    /**
     * Get the screen coordinates (x, y) of the cursor position in pixels.
     * The returned coordinates are relative to the View such that (0, 0)
     * represents tht top-left corner of the View.
     *
     * If global screen coordinates are required, e.g. for showing a QToolTip,
     * convert the view coordinates to global screen coordinates as follows:
     * \code
     * QPoint viewCoordinates = view->cursorPositionCoordinates();
     * QPoint globalCoorinates = view->mapToGlobal(viewCoordinates);
     * \endcode
     * \return cursor screen coordinates
     * \see cursorToCoordinate(), coordinatesToCursor()
     */
    virtual QPoint cursorPositionCoordinates() const = 0;

    /**
     * Get the text-cursor in the document from the screen coordinates,
     * relative to the view widget.
     *
     * To map a cursor to pixel coordinates (the reverse transformation)
     * use cursorToCoordinate().
     *
     * \param coord coordinates relative to the view widget
     * \return cursor in the View, that points onto the character under
     *         the given coordinate. May be KTextEditor::Cursor::invalid().
     */
    virtual KTextEditor::Cursor coordinatesToCursor(const QPoint &coord) const = 0;

    /*
     * SIGNALS
     * following signals should be emitted by the editor view
     * if the cursor position changes
     */
Q_SIGNALS:
    /**
     * This signal is emitted whenever the \p view's cursor position changed.
     * \param view view which emitted the signal
     * \param newPosition new position of the cursor (Kate will pass the real
     *        cursor position, not the virtual)
     * \see cursorPosition(), cursorPositionVirtual()
     */
    void cursorPositionChanged(KTextEditor::View *view, KTextEditor::Cursor newPosition);

    /**
     * This signal should be emitted whenever the \p view is scrolled vertically.
     * \param view view which emitted the signal
     * \param newPos the new scroll position
     */
    void verticalScrollPositionChanged(KTextEditor::View *view, KTextEditor::Cursor newPos);

    /**
     * This signal should be emitted whenever the \p view is scrolled horizontally.
     * \param view view which emitted the signal
     */
    void horizontalScrollPositionChanged(KTextEditor::View *view);
    /*
     * Mouse position
     */
public:
    /**
     * Check, whether mouse tracking is enabled.
     *
     * Mouse tracking is required to have the signal mousePositionChanged()
     * emitted.
     * \return \e true, if mouse tracking is enabled, otherwise \e false
     * \see setMouseTrackingEnabled(), mousePositionChanged()
     */
    virtual bool mouseTrackingEnabled() const = 0;

    /**
     * Try to enable or disable mouse tracking according to \p enable.
     * The return value contains the state of mouse tracking \e after the
     * request. Mouse tracking is required to have the mousePositionChanged()
     * signal emitted.
     *
     * \note Implementation Notes: An implementation is not forced to support
     *       this, and should always return \e false if it does not have
     *       support.
     *
     * \param enable if \e true, try to enable mouse tracking, otherwise disable
     *        it.
     * \return the current state of mouse tracking
     * \see mouseTrackingEnabled(), mousePositionChanged()
     */
    virtual bool setMouseTrackingEnabled(bool enable) = 0;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the position of the mouse changes over
     * this \a view. If the mouse moves off the view, an invalid cursor position
     * should be emitted, i.e. Cursor::invalid().
     * \note If mouseTrackingEnabled() returns \e false, this signal is never
     *       emitted.
     * \param view view which emitted the signal
     * \param newPosition new position of the mouse or Cursor::invalid(), if the
     *        mouse moved out of the \p view.
     * \see mouseTrackingEnabled()
     */
    void mousePositionChanged(KTextEditor::View *view, KTextEditor::Cursor newPosition);

    /*
     * Selection methods.
     * This deals with text selection and copy&paste
     */
public:
    /**
     * Set the view's selection to the range \p selection.
     * The old selection will be discarded.
     * \param range the range of the new selection
     * \return \e true on success, otherwise \e false (e.g. when the cursor
     *         range is invalid)
     * \see selectionRange(), selection()
     */
    virtual bool setSelection(Range range) = 0;

    /**
     * Set the view's selection to the range \p selection.
     * The old selection will be discarded.
     * \param ranges the ranges of the new selections
     * \see selectionRanges(), selection()
     *
     * @since 5.95
     */
    void setSelections(const QList<KTextEditor::Range> &ranges);

    /**
     * Query the view whether it has selected text, i.e. whether a selection
     * exists.
     * \return \e true if a text selection exists, otherwise \e false
     * \see setSelection(), selectionRange()
     */
    virtual bool selection() const = 0;

    /**
     * Get the range occupied by the current selection.
     * \return selection range, valid only if a selection currently exists.
     * \see setSelection()
     */
    virtual Range selectionRange() const = 0;

    /**
     * Get the ranges occupied by the current selections.
     * \return selection ranges, valid only if a selection currently exists.
     * \see setSelections()
     *
     * @since 5.95
     */
    QList<KTextEditor::Range> selectionRanges() const;

    /**
     * Get the view's selected text.
     * \return the selected text
     * \see setSelection()
     */
    virtual QString selectionText() const = 0;

    /**
     * Remove the view's current selection, \e without deleting the selected
     * text.
     * \return \e true on success, otherwise \e false
     * \see removeSelectionText()
     */
    virtual bool removeSelection() = 0;

    /**
     * Remove the view's current selection \e including the selected text.
     * \return \e true on success, otherwise \e false
     * \see removeSelection()
     */
    virtual bool removeSelectionText() = 0;

    /*
     * Blockselection stuff
     */
public:
    /**
     * Set block selection mode to state \p on.
     * \param on if \e true, block selection mode is turned on, otherwise off
     * \return \e true on success, otherwise \e false
     * \see blockSelection()
     */
    virtual bool setBlockSelection(bool on) = 0;

    /**
     * Get the status of the selection mode. \e true indicates that block
     * selection mode is on. If this is \e true, selections applied via the
     * SelectionInterface are handled as block selections and the Copy&Paste
     * functions work on rectangular blocks of text rather than normal.
     * \return \e true, if block selection mode is enabled, otherwise \e false
     * \see setBlockSelection()
     */
    virtual bool blockSelection() const = 0;

    /*
     * SIGNALS
     * following signals should be emitted by the editor view for selection
     * handling.
     */
Q_SIGNALS:
    /**
     * This signal is emitted whenever the \p view's selection changes.
     * \note If the mode switches from block selection to normal selection
     *       or vice versa this signal should also be emitted.
     * \param view view in which the selection changed
     * \see selection(), selectionRange(), selectionText()
     */
    void selectionChanged(KTextEditor::View *view);

public:
    /**
     * This is a convenience function which inserts \p text at the view's
     * current cursor position. You do not necessarily need to reimplement
     * it, except you want to do some special things.
     * \param text Text to be inserted
     * \return \e true on success of insertion, otherwise \e false
     * \see textInserted()
     */
    virtual bool insertText(const QString &text);

    /**
     * Insert a template into the document. The template can have editable fields
     * which can be filled by the user. You can create editable fields
     * with ${fieldname}; multiple fields with the same name will have their
     * contents synchronized automatically, and only the first one is editable
     * in this case.
     * Fields can have a default value specified by writing ${fieldname=default}.
     * Note that `default' is a JavaScript expression and strings need to be quoted.
     * You can also provide a piece of JavaScript for more complex logic.
     * To create a field which provides text based on a JS function call and the values
     * of the other, editable fields, use the ${func()} syntax. func() must be a callable
     * object defined in @p script. You can pass arguments to the function by just
     * writing any constant expression or a field name.
     * \param insertPosition where to insert the template
     * \param templateString template to insert using the above syntax
     * \param script script with functions which can be used in @p templateScript
     * \return true on success, false if insertion failed (e.g. read-only mode)
     */
    bool insertTemplate(KTextEditor::Cursor insertPosition, const QString &templateString, const QString &script = QString());

    /**
     * Scroll view to cursor.
     *
     * \param cursor the cursor position to scroll to.
     *
     * \since 5.33
     */
    void setScrollPosition(KTextEditor::Cursor cursor);

    /**
     * Horizontally scroll view to position.
     *
     * \param x the pixel position to scroll to.
     *
     * \since 5.33
     */
    void setHorizontalScrollPosition(int x);

    /**
     * Get the cursor corresponding to the maximum position
     * the view can vertically scroll to.
     *
     * \return cursor position of the maximum vertical scroll position.
     *
     * \since 5.33
     */
    KTextEditor::Cursor maxScrollPosition() const;

    /**
     * Get the first displayed line in the view.
     *
     * \note If code is folded, many hundred lines can be
     * between firstDisplayedLine() and lastDisplayedLine().
     *
     * \param lineType if RealLine (the default), it returns the real line number
     * accounting for folded regions. In that case it walks over all folded
     * regions
     * O(n) for n == number of folded ranges
     * \return the first displayed line
     *
     * \see lastDisplayedLine()
     * \since 5.33
     */
    int firstDisplayedLine(LineType lineType = RealLine) const;

    /**
     * Get the last displayed line in the view.
     *
     * \note If code is folded, many hundred lines can be
     * between firstDisplayedLine() and lastDisplayedLine().
     *
     * \param lineType if RealLine (the default), it returns the real line number
     * accounting for folded regions. In that case it walks over all folded
     * regions
     * O(n) for n == number of folded ranges
     * \return the last displayed line
     *
     * \see firstDisplayedLine()
     * \since 5.33
     */
    int lastDisplayedLine(LineType lineType = RealLine) const;

    /**
     * Get the view's text area rectangle excluding border, scrollbars, etc.
     *
     * \return the view's text area rectangle
     *
     * \since 5.33
     */
    QRect textAreaRect() const;

    /**
     * \return The vertical scrollbar of this view
     * \since 6.0
     */
    virtual QScrollBar *verticalScrollBar() const = 0;

    /**
     * \return The horizontal scrollbar of this view
     * \since 6.0
     */
    virtual QScrollBar *horizontalScrollBar() const = 0;

public:
    /**
     * Print the document. This should result in showing the print dialog.
     *
     * @returns true if document was printed
     */
    virtual bool print() = 0;

    /**
     * Shows the print preview dialog/
     */
    virtual void printPreview() = 0;

    /**
     * Is the status bar enabled?
     *
     * @return status bar enabled?
     */
    bool isStatusBarEnabled() const;

    /**
     * Show/hide the status bar of the view.
     * Per default, the status bar is enabled.
     *
     * @param enable should the status bar be enabled?
     */
    void setStatusBarEnabled(bool enable);

Q_SIGNALS:
    /**
     * This signal is emitted whenever the status bar of \p view is toggled.
     *
     * @param enabled Whether the status bar is currently enabled or not
     */
    void statusBarEnabledChanged(KTextEditor::View *view, bool enabled);

public:
    /**
     * Read session settings from the given \p config.
     *
     * Known flags:
     *  none atm
     *
     * \param config read the session settings from this KConfigGroup
     * \param flags additional flags
     * \see writeSessionConfig()
     */
    virtual void readSessionConfig(const KConfigGroup &config, const QSet<QString> &flags = QSet<QString>()) = 0;

    /**
     * Write session settings to the \p config.
     * See readSessionConfig() for more details.
     *
     * \param config write the session settings to this KConfigGroup
     * \param flags additional flags
     * \see readSessionConfig()
     */
    virtual void writeSessionConfig(KConfigGroup &config, const QSet<QString> &flags = QSet<QString>()) = 0;

public:
    /**
     * Returns the attribute for the default style \p defaultStyle.
     * @param defaultStyle default style to get the attribute for
     * @see KTextEditor::Attribute
     */
    virtual QExplicitlySharedDataPointer<KTextEditor::Attribute> defaultStyleAttribute(KSyntaxHighlighting::Theme::TextStyle defaultStyle) const = 0;

    /**
     * Get the list of AttributeBlocks for a given \p line in the document.
     *
     * \return list of AttributeBlocks for given \p line.
     */
    virtual QList<KTextEditor::AttributeBlock> lineAttributes(int line) = 0;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the current view configuration is changed.
     *
     * \param view the view which's config has changed
     *
     * \since 5.79
     */
    void configChanged(KTextEditor::View *view);

    /**
     * View Config
     */
public:
    /**
     * Get a list of all available keys.
     */
    virtual QStringList configKeys() const = 0;
    /**
     * Get a value for the \p key.
     */
    virtual QVariant configValue(const QString &key) = 0;
    /**
     * Set a the \p key's value to \p value.
     */
    virtual void setConfigValue(const QString &key, const QVariant &value) = 0;

    /**
     * View Annotation Interface
     */
public:
    /**
     * Sets a new \ref AnnotationModel for this document to provide
     * annotation information for each line.
     *
     * \param model the new AnnotationModel
     */
    virtual void setAnnotationModel(AnnotationModel *model) = 0;

    /**
     * returns the currently set \ref AnnotationModel or 0 if there's none
     * set
     * @returns the current \ref AnnotationModel
     */
    virtual AnnotationModel *annotationModel() const = 0;

    /**
     * This function can be used to show or hide the annotation border
     * The annotation border is hidden by default.
     *
     * @param visible if \e true the annotation border is shown, otherwise hidden
     */
    virtual void setAnnotationBorderVisible(bool visible) = 0;

    /**
     * Checks whether the View's annotation border is visible.
     */
    virtual bool isAnnotationBorderVisible() const = 0;

    /**
     * Sets the AbstractAnnotationItemDelegate for this view and the model
     * to provide custom rendering of annotation information for each line.
     * Ownership is not transferred.
     *
     * \param delegate the new AbstractAnnotationItemDelegate, or \c nullptr to reset to the default delegate
     *
     * @since 6.0
     */
    virtual void setAnnotationItemDelegate(KTextEditor::AbstractAnnotationItemDelegate *delegate) = 0;

    /**
     * Returns the currently used AbstractAnnotationItemDelegate
     *
     * @returns the current AbstractAnnotationItemDelegate
     *
     * @since 6.0
     */
    virtual KTextEditor::AbstractAnnotationItemDelegate *annotationItemDelegate() const = 0;

    /**
     * This function can be used to declare whether it is known that the annotation items
     * rendered by the set delegate all have the same size.
     * This enables the view to do some optimizations for performance purposes.
     *
     * By default the value of this property is \c false .
     *
     * @param uniformItemSizes if \c true the annotation items are considered to all have the same size
     *
     * @since 6.0
     */
    virtual void setAnnotationUniformItemSizes(bool uniformItemSizes) = 0;

    /**
     * Checks whether the annotation items all have the same size.
     *
     * @since 6.0
     */
    virtual bool uniformAnnotationItemSizes() const = 0;

Q_SIGNALS:
    /**
     * This signal is emitted before a context menu is shown on the annotation
     * border for the given line and view.
     *
     * \note Kate Part implementation detail: In Kate Part, the menu has an
     *       entry to hide the annotation border.
     *
     * \param view the view that the annotation border belongs to
     * \param menu the context menu that will be shown
     * \param line the annotated line for which the context menu is shown
     */
    void annotationContextMenuAboutToShow(KTextEditor::View *view, QMenu *menu, int line);

    /**
     * This signal is emitted when an entry on the annotation border was activated,
     * for example by clicking or double-clicking it. This follows the KDE wide
     * setting for activation via click or double-clcik
     *
     * \param view the view to which the activated border belongs to
     * \param line the document line that the activated position belongs to
     */
    void annotationActivated(KTextEditor::View *view, int line);

    /**
     * This signal is emitted when the annotation border is shown or hidden.
     *
     * \param view the view to which the border belongs to
     * \param visible the current visibility state
     */
    void annotationBorderVisibilityChanged(KTextEditor::View *view, bool visible);

    /**
     * Inline Note
     */
public:
    /**
     * Register the inline note provider @p provider.
     *
     * Whenever a line is painted, the @p provider will be queried for notes
     * that should be painted in it. When the provider is about to be
     * destroyed, make sure to call unregisterInlineNoteProvider() to avoid a
     * dangling pointer.
     *
     * @param provider inline note provider
     * @see unregisterInlineNoteProvider(), InlineNoteProvider
     */
    virtual void registerInlineNoteProvider(KTextEditor::InlineNoteProvider *provider) = 0;

    /**
     * Unregister the inline note provider @p provider.
     *
     * @param provider inline note provider to unregister
     * @see registerInlineNoteProvider(), InlineNoteProvider
     */
    virtual void unregisterInlineNoteProvider(KTextEditor::InlineNoteProvider *provider) = 0;

    /**
     * Text Hint
     */
public:
    /**
     * Register the text hint provider \p provider.
     *
     * Whenever the user hovers over text, \p provider will be asked for
     * a text hint. When the provider is about to be destroyed, make
     * sure to call unregisterTextHintProvider() to avoid a dangling pointer.
     *
     * @param provider text hint provider
     * @see unregisterTextHintProvider(), TextHintProvider
     */
    virtual void registerTextHintProvider(KTextEditor::TextHintProvider *provider) = 0;

    /**
     * Unregister the text hint provider \p provider.
     *
     * @param provider text hint provider to unregister
     * @see registerTextHintProvider(), TextHintProvider
     */
    virtual void unregisterTextHintProvider(KTextEditor::TextHintProvider *provider) = 0;

    /**
     * Set the text hint delay to \p delay milliseconds.
     *
     * The delay specifies the time the user needs to hover over the text
     * before the tool tip is shown. Therefore, \p delay should not be
     * too large, a value of 500 milliseconds is recommended and set by
     * default.
     *
     * If \p delay is <= 0, the default delay will be set.
     *
     * \param delay tool tip delay in milliseconds
     */
    virtual void setTextHintDelay(int delay) = 0;

    /**
     * Get the text hint delay in milliseconds.
     * By default, the text hint delay is set to 500 milliseconds.
     * It can be changed by calling \p setTextHintDelay().
     */
    virtual int textHintDelay() const = 0;

    /**
     * Completion
     */
public:
    /**
     * Query whether the code completion box is currently displayed.
     */
    virtual bool isCompletionActive() const = 0;

    /**
     * Invoke code completion over a given range, with a specific \a model.
     */
    virtual void startCompletion(Range word, CodeCompletionModel *model) = 0;

    /**
     * Abort the currently displayed code completion without executing any currently
     * selected completion. This is safe, even when the completion box is not currently
     * active.
     * \see isCompletionActive()
     */
    virtual void abortCompletion() = 0;

    /**
     * Force execution of the currently selected completion, and hide the code completion
     * box.
     */
    virtual void forceCompletion() = 0;

    /**
     * Register a new code completion \p model.
     * \param model new completion model
     * \see unregisterCompletionModel()
     */
    virtual void registerCompletionModel(CodeCompletionModel *model) = 0;

    /**
     * Unregister a code completion \p model.
     * \param model the model that should be unregistered
     * \see registerCompletionModel()
     */
    virtual void unregisterCompletionModel(CodeCompletionModel *model) = 0;

    /**
     * Determine the status of automatic code completion invocation.
     */
    virtual bool isAutomaticInvocationEnabled() const = 0;

    /**
     * Enable or disable automatic code completion invocation.
     */
    virtual void setAutomaticInvocationEnabled(bool enabled = true) = 0;

    /**
     * Invoke code completion over a given range, with specific models and invocation type.
     * \param models list of models to start. If this is an empty list, all registered models are started.
     */
    virtual void startCompletion(const Range &word,
                                 const QList<CodeCompletionModel *> &models = QList<CodeCompletionModel *>(),
                                 KTextEditor::CodeCompletionModel::InvocationType invocationType = KTextEditor::CodeCompletionModel::ManualInvocation) = 0;

    /**
     * Obtain the list of registered code completion models.
     * \returns a list of a models that are currently registered
     * \see registerCompletionModel(CodeCompletionModel*)
     */
    virtual QList<CodeCompletionModel *> codeCompletionModels() const = 0;

public:
    /**
     * Get the current active theme of this view.
     * Might change during runtime, configChanged() will be emitted in that cases.
     *
     * \return current active theme
     *
     * \since 5.79
     */
    KSyntaxHighlighting::Theme theme() const;

private:
    /**
     * private d-pointer, pointing to the internal implementation
     */
    ViewPrivate *const d;
};

}

#endif
