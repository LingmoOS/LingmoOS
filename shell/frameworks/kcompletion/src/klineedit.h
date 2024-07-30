/*
    This file is part of the KDE libraries

    This class was originally inspired by Torben Weis'
    fileentry.cpp for KFM II.

    SPDX-FileCopyrightText: 1997 Sven Radej <sven.radej@iname.com>
    SPDX-FileCopyrightText: 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>

    Completely re-designed:
    SPDX-FileCopyrightText: 2000, 2001 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLINEEDIT_H
#define KLINEEDIT_H

#include <kcompletion.h>
#include <kcompletion_export.h>
#include <kcompletionbase.h>

#include <QLineEdit>
#include <memory>

class QAction;
class QMenu;
class KCompletionBox;
class QUrl;
class KLineEditPrivate;

/**
 * @class KLineEdit klineedit.h KLineEdit
 *
 * An enhanced QLineEdit widget for inputting text.
 *
 * \b Detail \n
 *
 * This widget inherits from QLineEdit and implements the following
 * additional functionalities:
 * @li a completion object that provides both automatic and manual text
 * completion as well as multiple match iteration features
 * @li configurable key-bindings to activate these features
 * @li a popup-menu item that can be used to allow the user to set text
 * completion modes on the fly based on their preference
 *
 * To support these features KLineEdit also emits a few more additional
 * signals:
 * @li @c completion(const QString &): this signal can be connected to
 * a slot that will assist the user in filling out the remaining text
 * @li @c textRotation(KeyBindingType): this signal is intended to be
 * used to iterate through the list of all possible matches whenever
 * there is more than one match for the entered text
 * @li @c returnKeyPressed(const QString &): this signal provides the
 * current text in the widget as its argument whenever appropriate (this
 * is in addition to the @c QLineEdit::returnPressed() signal which KLineEdit
 * inherits from QLineEdit).
 *
 * This widget by default creates a completion object when you invoke
 * the @c completionObject(bool) member function for the first time or
 * use @c setCompletionObject(KCompletion *, bool) to assign your own
 * completion object. Additionally, to make this widget more functional,
 * KLineEdit will by default handle the text rotation and completion
 * events internally when a completion object is created through either one
 * of the methods mentioned above. If you do not need this functionality,
 * simply use @c KCompletionBase::setHandleSignals(bool) or set the boolean
 * parameter in the above functions to false.
 *
 * The default key-bindings for completion and rotation is determined
 * from the global settings in KStandardShortcut. These values, however,
 * can be overridden locally by invoking @c KCompletionBase::setKeyBinding().
 * The values can easily be reverted back to the default setting, by simply
 * calling @c useGlobalSettings(). An alternate method would be to default
 * individual key-bindings by using setKeyBinding() with the default
 * second argument.
 *
 * If @c EchoMode for this widget is set to something other than @c QLineEdit::Normal,
 * the completion mode will always be defaulted to CompletionNone.
 * This is done purposefully to guard against protected entries, such as
 * passwords, being cached in KCompletion's list. Hence, if the @c EchoMode
 * is not @c QLineEdit::Normal, the completion mode is automatically disabled.
 *
 * A read-only KLineEdit will have the same background color as a disabled
 * KLineEdit, but its foreground color will be the one used for the read-write
 * mode. This differs from QLineEdit's implementation and is done to give visual
 * distinction between the three different modes: disabled, read-only, and read-write.
 *
 * @b Usage
 *
 * To enable the basic completion feature:
 *
 * @code
 * KLineEdit *edit = new KLineEdit(this);
 * KCompletion *comp = edit->completionObject();
 * // Connect to the Return pressed signal - optional
 * connect(edit, &KLineEdit::returnKeyPressed, comp, [this](const QString &text) { addItem(text); });
 * @endcode
 *
 * To use a customized completion object or your own completion object:
 *
 * @code
 * KLineEdit *edit = new KLineEdit(this);
 * KUrlCompletion *comp = new KUrlCompletion();
 * edit->setCompletionObject(comp);
 * // Connect to the return pressed signal - optional
 * connect(edit, &KLineEdit::returnKeyPressed, comp, [this](const QString &text) { addItem(text); });
 * @endcode
 *
 * Note if you specify your own completion object you have to either delete
 * it when you don't need it anymore, or you can tell KLineEdit to delete it
 * for you:
 * @code
 * edit->setAutoDeleteCompletionObject(true);
 * @endcode
 *
 * <b>Miscellaneous function calls :</b>\n
 *
 * @code
 * // Tell the widget to not handle completion and iteration automatically
 * edit->setHandleSignals(false);
 *
 * // Set your own key-bindings for a text completion mode
 * edit->setKeyBinding(KCompletionBase::TextCompletion, Qt::End);
 *
 * // Hide the context (popup) menu
 * edit->setContextMenuPolicy(Qt::NoContextMenu);
 *
 * // Default the key-bindings back to the default system settings
 * edit->useGlobalKeyBindings();
 * @endcode
 *
 * @image html klineedit.png "KLineEdit widgets with clear-button"
 *
 * @author Dawit Alemayehu <adawit@kde.org>
 */

class KCOMPLETION_EXPORT KLineEdit : public QLineEdit, public KCompletionBase // krazy:exclude=qclasses
{
    friend class KComboBox;
    friend class KLineEditStyle;

    Q_OBJECT
    Q_DECLARE_PRIVATE(KLineEdit)
    Q_PROPERTY(bool trapEnterKeyEvent READ trapReturnKey WRITE setTrapReturnKey)
    Q_PROPERTY(bool squeezedTextEnabled READ isSqueezedTextEnabled WRITE setSqueezedTextEnabled)

public:
    /**
     * Constructs a KLineEdit object with a default text, a parent,
     * and a name.
     *
     * @param string Text to be shown in the edit widget.
     * @param parent The parent widget of the line edit.
     */
    explicit KLineEdit(const QString &string, QWidget *parent = nullptr);

    /**
     * Constructs a line edit
     * @param parent The parent widget of the line edit.
     */
    explicit KLineEdit(QWidget *parent = nullptr);

    /**
     *  Destructor.
     */
    ~KLineEdit() override;

    /**
     * Sets @p url into the lineedit. It uses QUrl::toDisplayString() so
     * that the url is properly decoded for displaying.
     */
    void setUrl(const QUrl &url);

    /**
     * Reimplemented from KCompletionBase for internal reasons.
     *
     * This function is re-implemented in order to make sure that
     * the EchoMode is acceptable before we set the completion mode.
     *
     * See KCompletionBase::setCompletionMode
     */
    void setCompletionMode(KCompletion::CompletionMode mode) override;

    /**
     * Disables completion modes by making them non-checkable.
     *
     * The context menu allows to change the completion mode.
     * This method allows to disable some modes.
     */
    void setCompletionModeDisabled(KCompletion::CompletionMode mode, bool disable = true);

    /**
     * Enables/Disables handling of URL drops. If enabled and the user
     * drops an URL, the decoded URL will be inserted. Otherwise the default
     * behavior of QLineEdit is used, which inserts the encoded URL.
     * Call setUrlDropsEnabled(false) if you need dropEvent to be called in a KLineEdit subclass.
     *
     * @param enable If @c true, insert decoded URLs
     */
    void setUrlDropsEnabled(bool enable); // KF6: remove it and don't create LineEditUrlDropEventFilter by default.

    /**
     * Returns @c true when decoded URL drops are enabled
     */
    bool urlDropsEnabled() const;

    /**
     * By default, KLineEdit recognizes @c Key_Return and @c Key_Enter and emits
     * the returnPressed() signals, but it also lets the event pass,
     * for example causing a dialog's default-button to be called.
     *
     * Call this method with @p trap = @c true to make @c KLineEdit stop these
     * events. The signals will still be emitted of course.
     *
     * @see trapReturnKey()
     */
    void setTrapReturnKey(bool trap);

    /**
     * @returns @c true if keyevents of @c Key_Return or
     * @c Key_Enter will be stopped or if they will be propagated.
     *
     * @see setTrapReturnKey ()
     */
    bool trapReturnKey() const;

    /**
     * This method will create a completion-box if none is there, yet.
     *
     * @param create Set this to false if you don't want the box to be created
     *               i.e. to test if it is available.
     * @returns the completion-box, that is used in completion mode
     * CompletionPopup.
     */
    virtual KCompletionBox *completionBox(bool create = true);

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    void setCompletionObject(KCompletion *, bool handle = true) override;

    /**
     * Reimplemented for internal reasons, the API is not affected.
     */
    virtual void copy() const;

    /**
     * Enable text squeezing whenever the supplied text is too long.
     * Only works for "read-only" mode.
     *
     * Note that once text squeezing is enabled, QLineEdit::text()
     * and QLineEdit::displayText() return the squeezed text. If
     * you want the original text, use @ref originalText.
     *
     * @see QLineEdit
     */
    void setSqueezedTextEnabled(bool enable);

    /**
     * Returns true if text squeezing is enabled.
     * This is only valid when the widget is in read-only mode.
     */
    bool isSqueezedTextEnabled() const;

    /**
     * Returns the original text if text squeezing is enabled.
     * If the widget is not in "read-only" mode, this function
     * returns the same thing as QLineEdit::text().
     *
     * @see QLineEdit
     */
    QString originalText() const;

    /**
     * Returns the text as given by the user (i.e. not autocompleted)
     * if the widget has autocompletion disabled, this function
     * returns the same as QLineEdit::text().
     * @since 4.2.2
     */
    QString userText() const;

    /**
     * Set the completion-box to be used in completion mode
     * CompletionPopup.
     * This will do nothing if a completion-box already exists.
     *
     * @param box The KCompletionBox to set
     */
    void setCompletionBox(KCompletionBox *box);

    /**
     * @return the size used by the clear button
     * @since 4.1
     */
    QSize clearButtonUsedSize() const;

    /**
     * Do completion now. This is called automatically when typing a key for instance.
     * Emits completion() and/or calls makeCompletion(), depending on
     * emitSignals and handleSignals.
     *
     * @since 4.2.1
     */
    void doCompletion(const QString &text);

Q_SIGNALS:
    /**
     * Emitted whenever the completion box is activated.
     */
    void completionBoxActivated(const QString &);

    /**
     * Emitted when the user presses the Return or Enter key.
     *
     * The argument is the current text. Note that this signal is @em not emitted
     * if the widget's @c EchoMode is set to QLineEdit::EchoMode.
     *
     * @since 5.81
     */
    void returnKeyPressed(const QString &text);

    /**
     * Emitted when the completion key is pressed.
     *
     * Please note that this signal is @em not emitted if the
     * completion mode is set to @c CompletionNone or @c EchoMode is
     * @em normal.
     */
    void completion(const QString &);

    /**
     * Emitted when the shortcut for substring completion is pressed.
     */
    void substringCompletion(const QString &);

    /**
     * Emitted when the text rotation key-bindings are pressed.
     *
     * The argument indicates which key-binding was pressed.
     * In KLineEdit's case this can be either one of two values:
     * PrevCompletionMatch or NextCompletionMatch. See
     * KCompletionBase::setKeyBinding for details.
     *
     * Note that this signal is @em not emitted if the completion
     * mode is set to @c CompletionNone or @c echoMode() is @em not  normal.
     */
    void textRotation(KCompletionBase::KeyBindingType);

    /**
     * Emitted when the user changed the completion mode by using the
     * popupmenu.
     */
    void completionModeChanged(KCompletion::CompletionMode);

    /**
     * Emitted before the context menu is displayed.
     *
     * The signal allows you to add your own entries into the
     * the context menu that is created on demand.
     *
     * NOTE: Do not store the pointer to the QMenu
     * provided through since it is created and deleted
     * on demand.
     *
     * @param contextMenu the context menu about to be displayed
     */
    void aboutToShowContextMenu(QMenu *contextMenu);

    /**
     * Emitted when the user clicked on the clear button
     */
    void clearButtonClicked();

public Q_SLOTS:

    /**
     * Sets the lineedit to read-only. Similar to QLineEdit::setReadOnly
     * but also takes care of the background color, and the clear button.
     */
    virtual void setReadOnly(bool);

    /**
     * Iterates through all possible matches of the completed text or
     * the history list.
     *
     * This function simply iterates over all possible matches in case
     * multiple matches are found as a result of a text completion request.
     * It will have no effect if only a single match is found.
     *
     * @param type The key-binding invoked.
     */
    void rotateText(KCompletionBase::KeyBindingType type);

    /**
     * See KCompletionBase::setCompletedText.
     */
    void setCompletedText(const QString &) override;

    /**
     * Same as the above function except it allows you to temporarily
     * turn off text completion in CompletionPopupAuto mode.
     *
     *
     * @param items list of completion matches to be shown in the completion box.
     * @param autoSuggest true if you want automatic text completion (suggestion) enabled.
     */
    void setCompletedItems(const QStringList &items, bool autoSuggest = true) override;

    /**
     * Squeezes @p text into the line edit.
     * This can only be used with read-only line-edits.
     */
    void setSqueezedText(const QString &text);

    /**
     * Reimplemented to enable text squeezing. API is not affected.
     */
    virtual void setText(const QString &);

protected Q_SLOTS:

    /**
     * Completes the remaining text with a matching one from
     * a given list.
     */
    virtual void makeCompletion(const QString &);

    /**
     * Resets the current displayed text.
     * Call this function to revert a text completion if the user
     * cancels the request. Mostly applies to popup completions.
     */
    void userCancelled(const QString &cancelText);

protected:
    /**
     * Reimplemented for internal reasons. API not affected.
     */
    bool event(QEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::resizeEvent().
     */
    void resizeEvent(QResizeEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::keyPressEvent().
     */
    void keyPressEvent(QKeyEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::mousePressEvent().
     */
    void mousePressEvent(QMouseEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::mouseReleaseEvent().
     */
    void mouseReleaseEvent(QMouseEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QWidget::mouseDoubleClickEvent().
     */
    void mouseDoubleClickEvent(QMouseEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::contextMenuEvent().
     */
    void contextMenuEvent(QContextMenuEvent *) override;

    /**
     * Reimplemented for internal reasons. API not affected.
     *
     * See QLineEdit::createStandardContextMenu().
     */
    QMenu *createStandardContextMenu();

    /**
     * This function simply sets the lineedit text and
     * highlights the text appropriately if the boolean
     * value is set to true.
     *
     * @param text
     * @param marked
     */
    virtual void setCompletedText(const QString & /*text*/, bool /*marked*/);

    /**
     * Sets the widget in userSelection mode or in automatic completion
     * selection mode. This changes the colors of selections.
     */
    void setUserSelection(bool userSelection);

    /**
     * Whether in current state text should be auto-suggested
     */
    bool autoSuggest() const;

    void paintEvent(QPaintEvent *ev) override;

private:
    std::unique_ptr<KLineEditPrivate> const d_ptr;
};

#endif
