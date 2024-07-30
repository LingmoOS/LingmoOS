/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000, 2001 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KHistoryComboBoxBOX_H
#define KHistoryComboBoxBOX_H

#include <kcombobox.h>
#include <kcompletion_export.h>

#include <functional>

class KHistoryComboBoxPrivate;

/**
 * @class KHistoryComboBox khistorycombobox.h KHistoryComboBox
 *
 * @short A combobox for offering a history and completion
 *
 * A combobox which implements a history like a unix shell. You can navigate
 * through all the items by using the Up or Down arrows (configurable of
 * course). Additionally, weighted completion is available. So you should
 * load and save the completion list to preserve the weighting between
 * sessions.
 *
 * KHistoryComboBox obeys the HISTCONTROL environment variable to determine
 * whether duplicates in the history should be tolerated in
 * addToHistory() or not. During construction of KHistoryComboBox,
 * duplicates will be disabled when HISTCONTROL is set to "ignoredups" or
 * "ignoreboth". Otherwise, duplicates are enabled by default.
 *
 * \image html khistorycombobox.png "KHistoryComboBox widget"
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class KCOMPLETION_EXPORT KHistoryComboBox : public KComboBox
{
    Q_OBJECT

    Q_PROPERTY(QStringList historyItems READ historyItems WRITE setHistoryItems)

public:
    /**
     * Constructs a "read-write" combobox. A read-only history combobox
     * doesn't make much sense, so it is only available as read-write.
     * Completion will be used automatically for the items in the combo.
     *
     * The insertion-policy is set to NoInsert, you have to add the items
     * yourself via the slot addToHistory. If you want every item added,
     * use
     *
     * \code
     * connect( combo, SIGNAL( activated( const QString& )),
     *          combo, SLOT( addToHistory( const QString& )));
     * \endcode
     *
     * Use QComboBox::setMaxCount() to limit the history.
     *
     * @p parent the parent object of this widget.
     */
    explicit KHistoryComboBox(QWidget *parent = nullptr);

    /**
     * Same as the previous constructor, but additionally has the option
     * to specify whether you want to let KHistoryComboBox handle completion
     * or not. If set to @c true, KHistoryComboBox will sync the completion to the
     * contents of the combobox.
     */
    explicit KHistoryComboBox(bool useCompletion, QWidget *parent = nullptr);

    /**
     * Destructs the combo and the completion-object
     */
    ~KHistoryComboBox() override;

    /**
     * Inserts @p items into the combobox. @p items might get
     * truncated if it is longer than maxCount()
     *
     * @see historyItems
     */
    void setHistoryItems(const QStringList &items);

    /**
     * Inserts @p items into the combobox. @p items might get
     * truncated if it is longer than maxCount()
     *
     * Set @c setCompletionList to true, if you don't have a list of
     * completions. This tells KHistoryComboBox to use all the items for the
     * completion object as well.
     * You won't have the benefit of weighted completion though, so normally
     * you should do something like
     * \code
     * KConfigGroup config(KSharedConfig::openConfig(), "somegroup");
     *
     * // load the history and completion list after creating the history combo
     * QStringList list;
     * list = config.readEntry("Completion list", QStringList());
     * combo->completionObject()->setItems(list);
     * list = config.readEntry("History list", QStringList());
     * combo->setHistoryItems(list);
     *
     * [...]
     *
     * // save the history and completion list when the history combo is
     * // destroyed
     * QStringList list;
     * KConfigGroup config(KSharedConfig::openConfig(), "somegroup");
     * list = combo->completionObject()->items();
     * config.writeEntry("Completion list", list);
     * list = combo->historyItems();
     * config.writeEntry("History list", list);
     * \endcode
     *
     * Be sure to use different names for saving with KConfig if you have more
     * than one KHistoryComboBox.
     *
     * @note When @c setCompletionList is true, the items are inserted into the
     * KCompletion object with mode KCompletion::Insertion and the mode is set
     * to KCompletion::Weighted afterwards.
     *
     * @see historyItems
     * @see KComboBox::completionObject
     * @see KCompletion::setItems
     * @see KCompletion::items
     */
    void setHistoryItems(const QStringList &items, bool setCompletionList);

    /**
     * Returns the list of history items. Empty, when this is not a read-write
     * combobox.
     *
     * @see setHistoryItems
     */
    QStringList historyItems() const;

    /**
     * Removes all items named @p item.
     *
     * @return @c true if at least one item was removed.
     *
     * @see addToHistory
     */
    bool removeFromHistory(const QString &item);

    /**
     * Sets an icon provider, so that items in the combobox can have an icon.
     * The provider is a function that takes a QString and returns a QIcon
     * @since 5.66
     */
    void setIconProvider(std::function<QIcon(const QString &)> providerFunction);

    using QComboBox::insertItems;

public Q_SLOTS:
    /**
     * Adds an item to the end of the history list and to the completion list.
     * If maxCount() is reached, the first item of the list will be
     * removed.
     *
     * If the last inserted item is the same as @p item, it will not be
     * inserted again.
     *
     * If duplicatesEnabled() is false, any equal existing item will be
     * removed before @p item is added.
     *
     * @note By using this method and not the Q and KComboBox insertItem()
     * methods, you make sure that the combobox stays in sync with the
     * completion. It would be annoying if completion would give an item
     * not in the combobox, and vice versa.
     *
     * @see removeFromHistory
     * @see QComboBox::setDuplicatesEnabled
     */
    void addToHistory(const QString &item);

    /**
     * Clears the history and the completion list.
     */
    void clearHistory();

    /**
     * Resets the current position of the up/down history. Call this
     * when you manually call setCurrentItem() or clearEdit().
     */
    void reset();

Q_SIGNALS:
    /**
     * Emitted when the history was cleared by the entry in the popup menu.
     */
    void cleared();

protected:
    /**
     * Handling key-events, the shortcuts to rotate the items.
     */
    void keyPressEvent(QKeyEvent *) override;

    /**
     * Handling wheel-events, to rotate the items.
     */
    void wheelEvent(QWheelEvent *ev) override;

    /**
     * Inserts @p items into the combo, honoring setIconProvider()
     * Does not update the completionObject.
     *
     * @note duplicatesEnabled() is not honored here.
     *
     * Called from setHistoryItems()
     */
    void insertItems(const QStringList &items);

    /**
     * @returns if we can modify the completion object or not.
     */
    bool useCompletion() const;

private:
    Q_DECLARE_PRIVATE(KHistoryComboBox)

    Q_DISABLE_COPY(KHistoryComboBox)
};

#endif
