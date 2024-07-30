/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2000, 2001, 2002, 2003, 2004 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONBOX_H
#define KCOMPLETIONBOX_H

#include "kcompletion_export.h"
#include <QListWidget>
#include <memory>

class KCompletionBoxPrivate;
class QEvent;

/**
 * @class KCompletionBox kcompletionbox.h KCompletionBox
 *
 * @short A helper widget for "completion-widgets" (KLineEdit, KComboBox))
 *
 * A little utility class for "completion-widgets", like KLineEdit or
 * KComboBox. KCompletionBox is a listbox, displayed as a rectangle without
 * any window decoration, usually directly under the lineedit or combobox.
 * It is filled with all possible matches for a completion, so the user
 * can select the one he wants.
 *
 * It is used when KCompletion::CompletionMode == CompletionPopup or CompletionPopupAuto.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KCOMPLETION_EXPORT KCompletionBox : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(bool isTabHandling READ isTabHandling WRITE setTabHandling)
    Q_PROPERTY(QString cancelledText READ cancelledText WRITE setCancelledText)
    Q_PROPERTY(bool activateOnSelect READ activateOnSelect WRITE setActivateOnSelect)

public:
    /**
     * Constructs a KCompletionBox.
     *
     * The parent widget is used to give the focus back when pressing the
     * up-button on the very first item.
     */
    explicit KCompletionBox(QWidget *parent = nullptr);

    /**
     * Destroys the box
     */
    ~KCompletionBox() override;

    QSize sizeHint() const override;

    /**
     * @returns true if selecting an item results in the emission of the selected() signal.
     */
    bool activateOnSelect() const;

    /**
     * Returns a list of all items currently in the box.
     */
    QStringList items() const;

    /**
     * @returns true if this widget is handling Tab-key events to traverse the
     * items in the dropdown list, otherwise false.
     *
     * Default is true.
     *
     * @see setTabHandling
     */
    bool isTabHandling() const;

    /**
     * @returns the text set via setCancelledText() or QString().
     */
    QString cancelledText() const;

public Q_SLOTS:
    /**
     * Inserts @p items into the box. Does not clear the items before.
     * @p index determines at which position @p items will be inserted.
     * (defaults to appending them at the end)
     */
    void insertItems(const QStringList &items, int index = -1);

    /**
     * Clears the box and inserts @p items.
     */
    void setItems(const QStringList &items);

    /**
     * Adjusts the size of the box to fit the width of the parent given in the
     * constructor and pops it up at the most appropriate place, relative to
     * the parent.
     *
     * Depending on the screensize and the position of the parent, this may
     * be a different place, however the default is to pop it up and the
     * lower left corner of the parent.
     *
     * Make sure to hide() the box when appropriate.
     */
    virtual void popup();

    /**
     * Makes this widget (when visible) capture Tab-key events to traverse the
     * items in the dropdown list (Tab goes down, Shift+Tab goes up).
     *
     * On by default, but should be turned off when used in combination with KUrlCompletion.
     * When off, KLineEdit handles Tab itself, making it select the current item from the completion box,
     * which is particularly useful when using KUrlCompletion.
     *
     * @see isTabHandling
     */
    void setTabHandling(bool enable);

    /**
     * Sets the text to be emitted if the user chooses not to
     * pick from the available matches.
     *
     * If the cancelled text is not set through this function, the
     * userCancelled signal will not be emitted.
     *
     * @see userCancelled( const QString& )
     * @param text the text to be emitted if the user cancels this box
     */
    void setCancelledText(const QString &text);

    /**
     * Set whether or not the selected signal should be emitted when an
     * item is selected. By default the selected() signal is emitted.
     *
     * @param doEmit false if the signal should not be emitted.
     */
    void setActivateOnSelect(bool doEmit);

    /**
     * Moves the selection one line down or select the first item if nothing is selected yet.
     */
    void down();

    /**
     * Moves the selection one line up or select the first item if nothing is selected yet.
     */
    void up();

    /**
     * Moves the selection one page down.
     */
    void pageDown();

    /**
     * Moves the selection one page up.
     */
    void pageUp();

    /**
     * Moves the selection up to the first item.
     */
    void home();

    /**
     * Moves the selection down to the last item.
     */
    void end();

    /**
     * Reimplemented for internal reasons. API is unaffected.
     * Call it only if you really need it (i.e. the widget was hidden before) to have better performance.
     */
    void setVisible(bool visible) override;

Q_SIGNALS:
    /**
     * Emitted when an item is selected, @p text is the text of the selected item.
     *
     * @since 5.81
     */
    void textActivated(const QString &text);

    /**
     * Emitted whenever the user chooses to ignore the available
     * selections and closes this box.
     */
    void userCancelled(const QString &);

protected:
    /**
     * This calculates the size of the dropdown and the relative position of the top
     * left corner with respect to the parent widget. This matches the geometry and position
     * normally used by K/QComboBox when used with one.
     */
    QRect calculateGeometry() const;

    /**
     * This properly resizes and repositions the listbox.
     *
     * @since 5.0
     */
    void resizeAndReposition();

    /**
     * Reimplemented from QListWidget to get events from the viewport (to hide
     * this widget on mouse-click, Escape-presses, etc.
     */
    bool eventFilter(QObject *, QEvent *) override;

    /**
     * The preferred global coordinate at which the completion box's top left corner
     * should be positioned.
     */
    virtual QPoint globalPositionHint() const;

protected Q_SLOTS:
    /**
     * Called when an item is activated. Emits KCompletionBox::textActivated(const QString &) with the item text.
     *
     * @note For releases <= 5.81, this slot emitted KCompletionBox::activated(const QString &) with the item text.
     */
    virtual void slotActivated(QListWidgetItem *);

private:
    std::unique_ptr<KCompletionBoxPrivate> const d;
};

#endif // KCOMPLETIONBOX_H
