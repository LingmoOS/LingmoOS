/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2006 Albert Astals Cid <aacid@kde.org>
    SPDX-FileCopyrightText: 2006 Clarence Dang <dang@kde.org>
    SPDX-FileCopyrightText: 2006 Michel Hermier <michel.hermier@gmail.com>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kselectaction.h"
#include "kselectaction_p.h"

#include "loggingcategory.h"

#include <QActionEvent>
#include <QEvent>
#include <QMenu>
#include <QStandardItem>
#include <QToolBar>

// QAction::setText("Hi") and then KPopupAccelManager exec'ing, causes
// QAction::text() to return "&Hi" :(  Comboboxes don't have accels and
// display ampersands literally.
static QString DropAmpersands(const QString &text)
{
    QString label = text;

    int p = label.indexOf(QLatin1Char('&'));
    while (p >= 0 && p < label.length() - 1) {
        if (label[p + 1].isLetterOrNumber() // Valid accelerator.
            || label[p + 1] == QLatin1Char('&')) { // Escaped accelerator marker.
            label.remove(p, 1);
        }

        p = label.indexOf(QLatin1Char('&'), p + 1);
    }

    return label;
}

KSelectAction::KSelectAction(QObject *parent)
    : KSelectAction(*new KSelectActionPrivate(this), parent)
{
}

KSelectAction::KSelectAction(const QString &text, QObject *parent)
    : KSelectAction(*new KSelectActionPrivate(this), parent)
{
    setText(text);
}

KSelectAction::KSelectAction(const QIcon &icon, const QString &text, QObject *parent)
    : KSelectAction(*new KSelectActionPrivate(this), parent)
{
    setIcon(icon);
    setText(text);
}

KSelectAction::KSelectAction(KSelectActionPrivate &dd, QObject *parent)
    : QWidgetAction(parent)
    , d_ptr(&dd)
{
    Q_D(KSelectAction);
    d->init();
}

KSelectAction::~KSelectAction()
{
    menu()->deleteLater();
}

void KSelectActionPrivate::init()
{
    QObject::connect(q_ptr->selectableActionGroup(), &QActionGroup::triggered, q_ptr, &KSelectAction::slotActionTriggered);
    QObject::connect(q_ptr, &QAction::toggled, q_ptr, &KSelectAction::slotToggled);
    q_ptr->setMenu(new QMenu());
    q_ptr->setEnabled(false);
}

QActionGroup *KSelectAction::selectableActionGroup() const
{
    Q_D(const KSelectAction);
    return d->m_actionGroup;
}

QList<QAction *> KSelectAction::actions() const
{
    return selectableActionGroup()->actions();
}

QAction *KSelectAction::currentAction() const
{
    return selectableActionGroup()->checkedAction();
}

int KSelectAction::currentItem() const
{
    return selectableActionGroup()->actions().indexOf(currentAction());
}

QString KSelectAction::currentText() const
{
    if (QAction *a = currentAction()) {
        return ::DropAmpersands(a->text());
    }

    return QString();
}

bool KSelectAction::setCurrentAction(QAction *action)
{
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::setCurrentAction(" << action << ")";
    if (action) {
        if (actions().contains(action)) {
            if (action->isVisible() && action->isEnabled() && action->isCheckable()) {
                action->setChecked(true);
                if (isCheckable()) {
                    setChecked(true);
                }
                return true;
            } else {
                qCWarning(KWidgetsAddonsLog) << "Action does not have the correct properties to be current:" << action->text();
            }
        } else {
            qCWarning(KWidgetsAddonsLog) << "Action does not belong to group:" << action->text();
        }
        return false;
    }

    if (currentAction()) {
        currentAction()->setChecked(false);
    }

    return false;
}

bool KSelectAction::setCurrentItem(int index)
{
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::setCurrentIndex(" << index << ")";
    return setCurrentAction(action(index));
}

QAction *KSelectAction::action(int index) const
{
    if (index >= 0 && index < selectableActionGroup()->actions().count()) {
        return selectableActionGroup()->actions().at(index);
    }

    return nullptr;
}

QAction *KSelectAction::action(const QString &text, Qt::CaseSensitivity cs) const
{
    QString compare;
    if (cs == Qt::CaseSensitive) {
        compare = text;
    } else {
        compare = text.toLower();
    }

    const auto selectableActions = selectableActionGroup()->actions();
    for (QAction *action : selectableActions) {
        const QString text = ::DropAmpersands(action->text());
        if (cs == Qt::CaseSensitive) {
            if (text == compare) {
                return action;
            }

        } else if (cs == Qt::CaseInsensitive) {
            if (text.toLower() == compare) {
                return action;
            }
        }
    }

    return nullptr;
}

bool KSelectAction::setCurrentAction(const QString &text, Qt::CaseSensitivity cs)
{
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::setCurrentAction(" << text << ",cs=" << cs << ")";
    return setCurrentAction(action(text, cs));
}

void KSelectAction::setComboWidth(int width)
{
    Q_D(KSelectAction);
    if (width < 0) {
        return;
    }

    d->m_comboWidth = width;

    for (QComboBox *box : std::as_const(d->m_comboBoxes)) {
        box->setMaximumWidth(d->m_comboWidth);
    }

    Q_EMIT changed();
}

void KSelectAction::setMaxComboViewCount(int n)
{
    Q_D(KSelectAction);
    d->m_maxComboViewCount = n;

    for (QComboBox *box : std::as_const(d->m_comboBoxes)) {
        if (d->m_maxComboViewCount != -1) {
            box->setMaxVisibleItems(d->m_maxComboViewCount);
        } else
        // hardcoded qt default
        {
            box->setMaxVisibleItems(10);
        }
    }

    Q_EMIT changed();
}

void KSelectAction::addAction(QAction *action)
{
    insertAction(nullptr, action);
}

QAction *KSelectAction::addAction(const QString &text)
{
    Q_D(KSelectAction);
    QAction *newAction = new QAction(parent());
    newAction->setText(text);
    newAction->setCheckable(true);
    newAction->setProperty("isShortcutConfigurable", false);

    if (!d->m_menuAccelsEnabled) {
        newAction->setText(text);
        newAction->setShortcut(QKeySequence());
    }

    addAction(newAction);
    return newAction;
}

QAction *KSelectAction::addAction(const QIcon &icon, const QString &text)
{
    QAction *newAction = addAction(text);
    newAction->setIcon(icon);
    return newAction;
}

QAction *KSelectAction::removeAction(QAction *action)
{
    Q_D(KSelectAction);
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::removeAction(" << action << ")";
    // int index = selectableActionGroup()->actions().indexOf(action);
    // qCDebug(KWidgetsAddonsLog) << "\tindex=" << index;

    // Removes the action from the group and sets its parent to null.
    d->m_actionGroup->removeAction(action);

    // Disable when no action is in the group
    bool hasActions = selectableActionGroup()->actions().isEmpty();
    setEnabled(!hasActions);

    for (QToolButton *button : std::as_const(d->m_buttons)) {
        button->setEnabled(!hasActions);
        button->removeAction(action);
    }

    for (QComboBox *comboBox : std::as_const(d->m_comboBoxes)) {
        comboBox->setEnabled(!hasActions);
        comboBox->removeAction(action);
    }

    menu()->removeAction(action);

    return action;
}

void KSelectAction::insertAction(QAction *before, QAction *action)
{
    Q_D(KSelectAction);
    action->setActionGroup(selectableActionGroup());

    // Re-Enable when an action is added
    setEnabled(true);

    // Keep in sync with createToolBarWidget()
    for (QToolButton *button : std::as_const(d->m_buttons)) {
        button->setEnabled(true);
        button->insertAction(before, action);
    }

    for (QComboBox *comboBox : std::as_const(d->m_comboBoxes)) {
        comboBox->setEnabled(true);
        comboBox->insertAction(before, action);
    }

    menu()->insertAction(before, action);
}

void KSelectAction::slotActionTriggered(QAction *action)
{
    // cache values so we don't need access to members in the action
    // after we've done an emit()
    const QString text = ::DropAmpersands(action->text());
    const int index = selectableActionGroup()->actions().indexOf(action);
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::slotActionTriggered(" << action << ") text=" << text
    //          << " index=" << index  << " emitting triggered()" << endl;

    if (isCheckable()) { // if this is subsidiary of other KSelectAction-derived class
        trigger(); // then imitate usual QAction behaviour so that other submenus (and their items) become unchecked
    }

    Q_EMIT actionTriggered(action);
    Q_EMIT indexTriggered(index);
    Q_EMIT textTriggered(text);
}

QStringList KSelectAction::items() const
{
    Q_D(const KSelectAction);
    QStringList ret;

    const auto actions = d->m_actionGroup->actions();
    ret.reserve(actions.size());
    for (QAction *action : actions) {
        ret << ::DropAmpersands(action->text());
    }

    return ret;
}

void KSelectAction::changeItem(int index, const QString &text)
{
    Q_D(KSelectAction);
    if (index < 0 || index >= actions().count()) {
        qCWarning(KWidgetsAddonsLog) << "KSelectAction::changeItem Index out of scope";
        return;
    }

    actions()[index]->setText(d->makeMenuText(text));
}

void KSelectAction::setItems(const QStringList &lst)
{
    Q_D(KSelectAction);
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::setItems(" << lst << ")";

    clear();

    for (const QString &string : lst) {
        if (!string.isEmpty()) {
            addAction(string);
        } else {
            QAction *action = new QAction(this);
            action->setSeparator(true);
            addAction(action);
        }
    }

    // Disable if empty and not editable
    setEnabled(lst.count() > 0 || d->m_edit);
}

int KSelectAction::comboWidth() const
{
    Q_D(const KSelectAction);
    return d->m_comboWidth;
}

void KSelectAction::clear()
{
    Q_D(KSelectAction);
    // qCDebug(KWidgetsAddonsLog) << "KSelectAction::clear()";

    // we need to delete the actions later since we may get a call to clear()
    // from a method called due to a triggered(...) signal
    const QList<QAction *> actions = d->m_actionGroup->actions();
    for (int i = 0; i < actions.count(); ++i) {
        // deleteLater() only removes us from the actions() list (among
        // other things) on the next entry into the event loop.  Until then,
        // e.g. action() and setCurrentItem() will be working on items
        // that are supposed to have been deleted.  So detach the action to
        // prevent this from happening.
        removeAction(actions[i]);

        actions[i]->deleteLater();
    }
}

void KSelectAction::removeAllActions()
{
    Q_D(KSelectAction);
    while (d->m_actionGroup->actions().count()) {
        removeAction(d->m_actionGroup->actions().first());
    }
}

void KSelectAction::setEditable(bool edit)
{
    Q_D(KSelectAction);
    d->m_edit = edit;

    for (QComboBox *comboBox : std::as_const(d->m_comboBoxes)) {
        comboBox->setEditable(edit);
    }

    Q_EMIT changed();
}

bool KSelectAction::isEditable() const
{
    Q_D(const KSelectAction);
    return d->m_edit;
}

void KSelectAction::slotToggled(bool checked)
{
    // if (checked && selectableActionGroup()->checkedAction())
    if (!checked && currentAction()) { // other's submenu item has been selected
        currentAction()->setChecked(false);
    }
}

KSelectAction::ToolBarMode KSelectAction::toolBarMode() const
{
    Q_D(const KSelectAction);
    return d->m_toolBarMode;
}

void KSelectAction::setToolBarMode(ToolBarMode mode)
{
    Q_D(KSelectAction);
    d->m_toolBarMode = mode;
}

QToolButton::ToolButtonPopupMode KSelectAction::toolButtonPopupMode() const
{
    Q_D(const KSelectAction);
    return d->m_toolButtonPopupMode;
}

void KSelectAction::setToolButtonPopupMode(QToolButton::ToolButtonPopupMode mode)
{
    Q_D(KSelectAction);
    d->m_toolButtonPopupMode = mode;
}

void KSelectActionPrivate::comboBoxDeleted(QComboBox *combo)
{
    m_comboBoxes.removeAll(combo);
}

void KSelectActionPrivate::comboBoxCurrentIndexChanged(int index)
{
    Q_Q(KSelectAction);
    // qCDebug(KWidgetsAddonsLog) << "KSelectActionPrivate::comboBoxCurrentIndexChanged(" << index << ")";

    QComboBox *triggeringCombo = qobject_cast<QComboBox *>(q->sender());

    QAction *a = q->action(index);
    // qCDebug(KWidgetsAddonsLog) << "\ta=" << a;
    if (a) {
        // qCDebug(KWidgetsAddonsLog) << "\t\tsetting as current action";
        a->trigger();

    } else if (q->isEditable() && triggeringCombo && triggeringCombo->count() > 0 && index == triggeringCombo->count() - 1) {
        // User must have added a new item by typing and pressing enter.
        const QString newItemText = triggeringCombo->currentText();
        // qCDebug(KWidgetsAddonsLog) << "\t\tuser typed new item '" << newItemText << "'";

        // Only 1 combobox contains this and it's not a proper action.
        bool blocked = triggeringCombo->blockSignals(true);
        triggeringCombo->removeItem(index);
        triggeringCombo->blockSignals(blocked);

        QAction *newAction = q->addAction(newItemText);

        newAction->trigger();
    } else {
        if (q->selectableActionGroup()->checkedAction()) {
            q->selectableActionGroup()->checkedAction()->setChecked(false);
        }
    }
}

// TODO: DropAmpersands() certainly makes sure this doesn't work.  But I don't
// think it did anyway esp. in the presence KCheckAccelerator - Clarence.
void KSelectAction::setMenuAccelsEnabled(bool b)
{
    Q_D(KSelectAction);
    d->m_menuAccelsEnabled = b;
}

bool KSelectAction::menuAccelsEnabled() const
{
    Q_D(const KSelectAction);
    return d->m_menuAccelsEnabled;
}

QWidget *KSelectAction::createWidget(QWidget *parent)
{
    Q_D(KSelectAction);
    QMenu *menu = qobject_cast<QMenu *>(parent);
    if (menu) { // If used in a menu want to return 0 and use only the text, not a widget
        return nullptr;
    }
    ToolBarMode mode = toolBarMode();
    QToolBar *toolBar = qobject_cast<QToolBar *>(parent);
    if (!toolBar && mode != ComboBoxMode) { // we can return a combobox just fine.
        return nullptr;
    }
    switch (mode) {
    case MenuMode: {
        QToolButton *button = new QToolButton(toolBar);
        button->setToolTip(toolTip());
        button->setWhatsThis(whatsThis());
        button->setStatusTip(statusTip());
        button->setAutoRaise(true);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIconSize(toolBar->iconSize());
        button->setToolButtonStyle(toolBar->toolButtonStyle());
        QObject::connect(toolBar, &QToolBar::iconSizeChanged, button, &QAbstractButton::setIconSize);
        QObject::connect(toolBar, &QToolBar::toolButtonStyleChanged, button, &QToolButton::setToolButtonStyle);
        button->setDefaultAction(this);
        QObject::connect(button, &QToolButton::triggered, toolBar, &QToolBar::actionTriggered);

        button->setPopupMode(toolButtonPopupMode());

        button->addActions(selectableActionGroup()->actions());

        d->m_buttons.append(button);
        return button;
    }

    case ComboBoxMode: {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->installEventFilter(this);

        if (d->m_maxComboViewCount != -1) {
            comboBox->setMaxVisibleItems(d->m_maxComboViewCount);
        }

        if (d->m_comboWidth > 0) {
            comboBox->setMaximumWidth(d->m_comboWidth);
        }

        comboBox->setEditable(isEditable());
        comboBox->setToolTip(toolTip());
        comboBox->setWhatsThis(whatsThis());
        comboBox->setStatusTip(statusTip());

        const auto selectableActions = selectableActionGroup()->actions();
        for (QAction *action : selectableActions) {
            comboBox->addAction(action);
        }

        if (selectableActions.isEmpty()) {
            comboBox->setEnabled(false);
        }

        connect(comboBox, &QComboBox::destroyed, this, [d, comboBox]() {
            d->comboBoxDeleted(comboBox);
        });

        connect(comboBox, &QComboBox::currentIndexChanged, this, [d](int value) {
            d->comboBoxCurrentIndexChanged(value);
        });

        d->m_comboBoxes.append(comboBox);

        return comboBox;
    }
    }

    return nullptr;
}

void KSelectAction::deleteWidget(QWidget *widget)
{
    Q_D(KSelectAction);
    if (QToolButton *toolButton = qobject_cast<QToolButton *>(widget)) {
        d->m_buttons.removeAll(toolButton);
    } else if (QComboBox *comboBox = qobject_cast<QComboBox *>(widget)) {
        d->m_comboBoxes.removeAll(comboBox);
    }
    QWidgetAction::deleteWidget(widget);
}

bool KSelectAction::event(QEvent *event)
{
    Q_D(KSelectAction);
    if (event->type() == QEvent::ActionChanged) {
        for (QComboBox *comboBox : std::as_const(d->m_comboBoxes)) {
            comboBox->setToolTip(toolTip());
            comboBox->setWhatsThis(whatsThis());
            comboBox->setStatusTip(statusTip());
        }
        for (QToolButton *toolButton : std::as_const(d->m_buttons)) {
            toolButton->setToolTip(toolTip());
            toolButton->setWhatsThis(whatsThis());
            toolButton->setStatusTip(statusTip());
        }
    }
    return QWidgetAction::event(event);
}

// KSelectAction::eventFilter() is called before action->setChecked()
// invokes the signal to update QActionGroup so KSelectAction::currentItem()
// returns an old value.  There are 3 possibilities, where n actions will
// report QAction::isChecked() where n is:
//
// 0: the checked action was unchecked
// 1: the checked action did not change
// 2: another action was checked but QActionGroup has not been invoked yet
//    to uncheck the one that was checked before
//
// TODO: we might want to cache this since QEvent::ActionChanged is fired
//       often.
static int TrueCurrentItem(KSelectAction *sa)
{
    QAction *curAction = sa->currentAction();
    // qCDebug(KWidgetsAddonsLog) << "\tTrueCurrentItem(" << sa << ") curAction=" << curAction;

    const auto actions = sa->actions();
    int i = 0;
    for (QAction *action : actions) {
        if (action->isChecked()) {
            // qCDebug(KWidgetsAddonsLog) << "\t\taction " << action << " (text=" << action->text () << ") isChecked";

            // 2 actions checked case?
            if (action != curAction) {
                // qCDebug(KWidgetsAddonsLog) << "\t\t\tmust be newly selected one";
                return i;
            }
        }
        ++i;
    }

    // qCDebug(KWidgetsAddonsLog) << "\t\tcurrent action still selected? " << (curAction && curAction->isChecked ());
    // 1 or 0 actions checked case (in that order)?
    return (curAction && curAction->isChecked()) ? sa->actions().indexOf(curAction) : -1;
}

bool KSelectAction::eventFilter(QObject *watched, QEvent *event)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(watched);
    if (!comboBox) {
        return false /*propagate event*/;
    }

    // If focus is lost, replace any edited text with the currently selected
    // item.
    if (event->type() == QEvent::FocusOut) {
        QFocusEvent *const e = static_cast<QFocusEvent *>(event);
        // qCDebug(KWidgetsAddonsLog) << "KSelectAction::eventFilter(FocusOut)"
        //  << "    comboBox: ptr=" << comboBox
        //  << " reason=" << e->reason ()
        //  << endl;

        if (e->reason() != Qt::ActiveWindowFocusReason // switch window
            && e->reason() != Qt::PopupFocusReason // menu
            && e->reason() != Qt::OtherFocusReason // inconsistently reproduceable actions...
        ) {
            // qCDebug(KWidgetsAddonsLog) << "\tkilling text";
            comboBox->setEditText(comboBox->itemText(comboBox->currentIndex()));
        }

        return false /*propagate event*/;
    }

    bool blocked = comboBox->blockSignals(true);

    if (event->type() == QEvent::ActionAdded) {
        QActionEvent *const e = static_cast<QActionEvent *>(event);

        const int index = e->before() ? comboBox->findData(QVariant::fromValue(e->before())) : comboBox->count();
        const int newItem = ::TrueCurrentItem(this);
        // qCDebug(KWidgetsAddonsLog) << "KSelectAction::eventFilter(ActionAdded)"
        //          << "    comboBox: ptr=" << comboBox
        //          << " currentItem=" << comboBox->currentIndex ()
        //          << "    add index=" << index
        //          << "    action new: e->before=" << e->before ()
        //          << " ptr=" << e->action ()
        //          << " icon=" << e->action ()->icon ()
        //          << " text=" << e->action ()->text ()
        //          << " currentItem=" << newItem
        //          << endl;
        comboBox->insertItem(index, e->action()->icon(), ::DropAmpersands(e->action()->text()), QVariant::fromValue(e->action()));
        if (QStandardItemModel *model = qobject_cast<QStandardItemModel *>(comboBox->model())) {
            QStandardItem *item = model->item(index);
            item->setEnabled(e->action()->isEnabled());
        }

        // Inserting an item into a combobox can change the current item so
        // make sure the item corresponding to the checked action is selected.
        comboBox->setCurrentIndex(newItem);
    } else if (event->type() == QEvent::ActionChanged) {
        QActionEvent *const e = static_cast<QActionEvent *>(event);

        const int index = comboBox->findData(QVariant::fromValue(e->action()));
        const int newItem = ::TrueCurrentItem(this);
        // qCDebug(KWidgetsAddonsLog) << "KSelectAction::eventFilter(ActionChanged)"
        //          << "    comboBox: ptr=" << comboBox
        //          << " currentItem=" << comboBox->currentIndex ()
        //          << "    changed action's index=" << index
        //          << "    action new: ptr=" << e->action ()
        //          << " icon=" << e->action ()->icon ()
        //          << " text=" << e->action ()->text ()
        //          << " currentItem=" << newItem
        //          << endl;
        comboBox->setItemIcon(index, e->action()->icon());
        comboBox->setItemText(index, ::DropAmpersands(e->action()->text()));
        if (QStandardItemModel *model = qobject_cast<QStandardItemModel *>(comboBox->model())) {
            QStandardItem *item = model->item(index);
            item->setEnabled(e->action()->isEnabled());
        }

        // The checked action may have become unchecked so
        // make sure the item corresponding to the checked action is selected.
        comboBox->setCurrentIndex(newItem);
    } else if (event->type() == QEvent::ActionRemoved) {
        QActionEvent *const e = static_cast<QActionEvent *>(event);

        const int index = comboBox->findData(QVariant::fromValue(e->action()));
        const int newItem = ::TrueCurrentItem(this);
        // qCDebug(KWidgetsAddonsLog) << "KSelectAction::eventFilter(ActionRemoved)"
        //          << "    comboBox: ptr=" << comboBox
        //          << " currentItem=" << comboBox->currentIndex ()
        //          << "    delete action index=" << index
        //          << "    new: currentItem=" << newItem
        //          << endl;
        comboBox->removeItem(index);

        // Removing an item from a combobox can change the current item so
        // make sure the item corresponding to the checked action is selected.
        comboBox->setCurrentIndex(newItem);
    }

    comboBox->blockSignals(blocked);

    return false /*propagate event*/;
}

// END

#include "moc_kselectaction.cpp"
