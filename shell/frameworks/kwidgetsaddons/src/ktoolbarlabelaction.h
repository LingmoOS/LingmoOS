/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 Felix Berger <felixberger@beldesign.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBARLABELACTION_H
#define KTOOLBARLABELACTION_H

#include <QWidgetAction>
#include <memory>

#include <kwidgetsaddons_export.h>

/**
 * @class KToolBarLabelAction ktoolbarlabelaction.h KToolBarLabelAction
 *
 * @short Class to display a label in a toolbar.
 *
 * KToolBarLabelAction is a convenience class for displaying a label in a
 * toolbar.
 *
 * It provides easy access to the label's #setBuddy(QAction*) and #buddy()
 * methods and can be used as follows:
 *
 * \code
 *
 * KHistoryComboBox *findCombo = new KHistoryComboBox(true, this);
 *
 * KWidgetAction *action = new KWidgetAction(findCombo, i18n("Find Combo"),
 *                                            Qt::Key_F6, this, SLOT( slotFocus()),
 *                                            actionCollection(), "find_combo");
 *
 * QAction *action = new KToolBarLabelAction(action, i18n("Find "), "find_label");
 * action->setShortcut(Qt::Key_F6);
 * connect(action, &QAction::triggered, this, [this]() { slotFocus(); });
 *
 * \endcode
 *
 * @author Felix Berger <felixberger@beldesign.de>
 */
class KWIDGETSADDONS_EXPORT KToolBarLabelAction : public QWidgetAction
{
    Q_OBJECT

public:
    /**
     * Creates a toolbar label.
     *
     * @param text The label's and the action's text.
     * @param parent This action's parent.
     */
    KToolBarLabelAction(const QString &text, QObject *parent);

    /**
     * Creates a toolbar label setting a buddy for the label.
     *
     * @param buddy The action whose widget which is focused when the label's accelerator is
     * typed.
     * @param text The label's and the action's text.
     * @param parent This action's parent.
     */
    KToolBarLabelAction(QAction *buddy, const QString &text, QObject *parent);

    /**
     * Destroys the toolbar label.
     */
    ~KToolBarLabelAction() override;

    /**
     * Sets the label's buddy to buddy.
     *
     * See QLabel#setBuddy() for details.
     */
    void setBuddy(QAction *buddy);

    /**
     * Returns the label's buddy or a null pointer if no buddy is currently set.
     *
     * See QLabel#buddy() and QLabel#setBuddy() for more information.
     */
    QAction *buddy() const;

    /**
     * Reimplemented from QWidgetAction.
     */
    QWidget *createWidget(QWidget *parent) override;

Q_SIGNALS:
    /**
     * This signal is emitted whenever the text of this action
     * is changed.
     */
    void textChanged(const QString &newText);

protected:
    bool event(QEvent *) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    std::unique_ptr<class KToolBarLabelActionPrivate> const d;
};

#endif
