/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kactionselector.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QToolButton>
#include <QVBoxLayout>

class KActionSelectorPrivate
{
public:
    KActionSelectorPrivate(KActionSelector *qq)
        : q(qq)
    {
    }

    KActionSelector *const q = nullptr;
    QListWidget *availableListWidget = nullptr;
    QListWidget *selectedListWidget = nullptr;
    QToolButton *btnAdd = nullptr;
    QToolButton *btnRemove = nullptr;
    QToolButton *btnUp = nullptr;
    QToolButton *btnDown = nullptr;
    QLabel *lAvailable = nullptr;
    QLabel *lSelected = nullptr;
    bool moveOnDoubleClick : 1;
    bool keyboardEnabled : 1;
    bool showUpDownButtons : 1;
    QString addIcon, removeIcon, upIcon, downIcon;
    KActionSelector::InsertionPolicy availableInsertionPolicy, selectedInsertionPolicy;

    /**
      Move item @p item to the other listbox
     */
    void moveItem(QListWidgetItem *item);

    /**
      loads the icons for the move buttons.
     */
    void loadIcons();

    /**
      @return the index to insert an item into listbox @p lb,
     given InsertionPolicy @p policy.

     Note that if policy is Sorted, this will return -1.
     Sort the listbox after inserting the item in that case.
     */
    int insertionIndex(QListWidget *lb, KActionSelector::InsertionPolicy policy);

    /**
     @return the index of the first selected item in listbox @p lb.
     If no item is selected, it will return -1.
     */
    int selectedRowIndex(QListWidget *lb);

    void buttonAddClicked();
    void buttonRemoveClicked();
    void buttonUpClicked();
    void buttonDownClicked();
    void itemDoubleClicked(QListWidgetItem *item);
    void slotCurrentChanged(QListWidgetItem *)
    {
        q->setButtonsEnabled();
    }
};

// BEGIN Constructor/destructor

KActionSelector::KActionSelector(QWidget *parent)
    : QWidget(parent)
    , d(new KActionSelectorPrivate(this))
{
    d->moveOnDoubleClick = true;
    d->keyboardEnabled = true;
    d->addIcon = QLatin1String(QApplication::isRightToLeft() ? "go-previous" : "go-next");
    d->removeIcon = QLatin1String(QApplication::isRightToLeft() ? "go-next" : "go-previous");
    d->upIcon = QStringLiteral("go-up");
    d->downIcon = QStringLiteral("go-down");
    d->availableInsertionPolicy = Sorted;
    d->selectedInsertionPolicy = BelowCurrent;
    d->showUpDownButtons = true;

    QHBoxLayout *lo = new QHBoxLayout(this);
    lo->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *loAv = new QVBoxLayout();
    lo->addLayout(loAv);
    d->lAvailable = new QLabel(tr("&Available:", "@label:listbox"), this);
    loAv->addWidget(d->lAvailable);
    d->availableListWidget = new QListWidget(this);
    loAv->addWidget(d->availableListWidget);
    d->lAvailable->setBuddy(d->availableListWidget);

    QVBoxLayout *loHBtns = new QVBoxLayout();
    lo->addLayout(loHBtns);
    loHBtns->addStretch(1);
    d->btnAdd = new QToolButton(this);
    loHBtns->addWidget(d->btnAdd);
    d->btnRemove = new QToolButton(this);
    loHBtns->addWidget(d->btnRemove);
    loHBtns->addStretch(1);

    QVBoxLayout *loS = new QVBoxLayout();
    lo->addLayout(loS);
    d->lSelected = new QLabel(tr("&Selected:", "@label:listbox"), this);
    loS->addWidget(d->lSelected);
    d->selectedListWidget = new QListWidget(this);
    loS->addWidget(d->selectedListWidget);
    d->lSelected->setBuddy(d->selectedListWidget);

    QVBoxLayout *loVBtns = new QVBoxLayout();
    lo->addLayout(loVBtns);
    loVBtns->addStretch(1);
    d->btnUp = new QToolButton(this);
    d->btnUp->setAutoRepeat(true);
    loVBtns->addWidget(d->btnUp);
    d->btnDown = new QToolButton(this);
    d->btnDown->setAutoRepeat(true);
    loVBtns->addWidget(d->btnDown);
    loVBtns->addStretch(1);

    d->loadIcons();

    connect(d->btnAdd, &QToolButton::clicked, this, [this]() {
        d->buttonAddClicked();
    });
    connect(d->btnRemove, &QToolButton::clicked, this, [this]() {
        d->buttonRemoveClicked();
    });
    connect(d->btnUp, &QToolButton::clicked, this, [this]() {
        d->buttonUpClicked();
    });
    connect(d->btnDown, &QToolButton::clicked, this, [this]() {
        d->buttonDownClicked();
    });
    connect(d->availableListWidget, &QListWidget::itemDoubleClicked, this, [this] (QListWidgetItem *item) { d->itemDoubleClicked(item); });
    connect(d->selectedListWidget, &QListWidget::itemDoubleClicked, this, [this] (QListWidgetItem *item) { d->itemDoubleClicked(item); });
    connect(d->availableListWidget, &QListWidget::itemSelectionChanged, this, &KActionSelector::setButtonsEnabled);
    connect(d->selectedListWidget, &QListWidget::itemSelectionChanged, this, &KActionSelector::setButtonsEnabled);

    d->availableListWidget->installEventFilter(this);
    d->selectedListWidget->installEventFilter(this);
    setButtonsEnabled();
}

KActionSelector::~KActionSelector() = default;

// END Constructor/destroctor

// BEGIN Public Methods

QListWidget *KActionSelector::availableListWidget() const
{
    return d->availableListWidget;
}

QListWidget *KActionSelector::selectedListWidget() const
{
    return d->selectedListWidget;
}

void KActionSelector::setButtonIcon(const QString &icon, MoveButton button)
{
    switch (button) {
    case ButtonAdd:
        d->addIcon = icon;
        d->btnAdd->setIcon(QIcon::fromTheme(icon));
        break;
    case ButtonRemove:
        d->removeIcon = icon;
        d->btnRemove->setIcon(QIcon::fromTheme(icon));
        break;
    case ButtonUp:
        d->upIcon = icon;
        d->btnUp->setIcon(QIcon::fromTheme(icon));
        break;
    case ButtonDown:
        d->downIcon = icon;
        d->btnDown->setIcon(QIcon::fromTheme(icon));
        break;
    default:
        //     qCDebug(KWidgetsAddonsLog)<<"KActionSelector::setButtonIcon: DAINBREAD!";
        break;
    }
}

void KActionSelector::setButtonIconSet(const QIcon &iconset, MoveButton button)
{
    switch (button) {
    case ButtonAdd:
        d->btnAdd->setIcon(iconset);
        break;
    case ButtonRemove:
        d->btnRemove->setIcon(iconset);
        break;
    case ButtonUp:
        d->btnUp->setIcon(iconset);
        break;
    case ButtonDown:
        d->btnDown->setIcon(iconset);
        break;
    default:
        //     qCDebug(KWidgetsAddonsLog)<<"KActionSelector::setButtonIconSet: DAINBREAD!";
        break;
    }
}

void KActionSelector::setButtonTooltip(const QString &tip, MoveButton button)
{
    switch (button) {
    case ButtonAdd:
        d->btnAdd->setText(tip);
        d->btnAdd->setToolTip(tip);
        break;
    case ButtonRemove:
        d->btnRemove->setText(tip);
        d->btnRemove->setToolTip(tip);
        break;
    case ButtonUp:
        d->btnUp->setText(tip);
        d->btnUp->setToolTip(tip);
        break;
    case ButtonDown:
        d->btnDown->setText(tip);
        d->btnDown->setToolTip(tip);
        break;
    default:
        //     qCDebug(KWidgetsAddonsLog)<<"KActionSelector::setButtonToolTip: DAINBREAD!";
        break;
    }
}

void KActionSelector::setButtonWhatsThis(const QString &text, MoveButton button)
{
    switch (button) {
    case ButtonAdd:
        d->btnAdd->setWhatsThis(text);
        break;
    case ButtonRemove:
        d->btnRemove->setWhatsThis(text);
        break;
    case ButtonUp:
        d->btnUp->setWhatsThis(text);
        break;
    case ButtonDown:
        d->btnDown->setWhatsThis(text);
        break;
    default:
        //     qCDebug(KWidgetsAddonsLog)<<"KActionSelector::setButtonWhatsThis: DAINBREAD!";
        break;
    }
}

// END Public Methods

// BEGIN Properties

bool KActionSelector::moveOnDoubleClick() const
{
    return d->moveOnDoubleClick;
}

void KActionSelector::setMoveOnDoubleClick(bool b)
{
    d->moveOnDoubleClick = b;
}

bool KActionSelector::keyboardEnabled() const
{
    return d->keyboardEnabled;
}

void KActionSelector::setKeyboardEnabled(bool b)
{
    d->keyboardEnabled = b;
}

QString KActionSelector::availableLabel() const
{
    return d->lAvailable->text();
}

void KActionSelector::setAvailableLabel(const QString &text)
{
    d->lAvailable->setText(text);
}

QString KActionSelector::selectedLabel() const
{
    return d->lSelected->text();
}

void KActionSelector::setSelectedLabel(const QString &text)
{
    d->lSelected->setText(text);
}

KActionSelector::InsertionPolicy KActionSelector::availableInsertionPolicy() const
{
    return d->availableInsertionPolicy;
}

void KActionSelector::setAvailableInsertionPolicy(InsertionPolicy p)
{
    d->availableInsertionPolicy = p;
}

KActionSelector::InsertionPolicy KActionSelector::selectedInsertionPolicy() const
{
    return d->selectedInsertionPolicy;
}

void KActionSelector::setSelectedInsertionPolicy(InsertionPolicy p)
{
    d->selectedInsertionPolicy = p;
}

bool KActionSelector::showUpDownButtons() const
{
    return d->showUpDownButtons;
}

void KActionSelector::setShowUpDownButtons(bool show)
{
    d->showUpDownButtons = show;
    if (show) {
        d->btnUp->show();
        d->btnDown->show();
    } else {
        d->btnUp->hide();
        d->btnDown->hide();
    }
}

// END Properties

// BEGIN Public Slots

void KActionSelector::setButtonsEnabled()
{
    d->btnAdd->setEnabled(d->selectedRowIndex(d->availableListWidget) > -1);
    d->btnRemove->setEnabled(d->selectedRowIndex(d->selectedListWidget) > -1);
    d->btnUp->setEnabled(d->selectedRowIndex(d->selectedListWidget) > 0);
    d->btnDown->setEnabled(d->selectedRowIndex(d->selectedListWidget) > -1 //
                           && d->selectedRowIndex(d->selectedListWidget) < d->selectedListWidget->count() - 1);
}

// END Public Slots

// BEGIN Protected
void KActionSelector::keyPressEvent(QKeyEvent *e)
{
    if (!d->keyboardEnabled) {
        return;
    }
    if ((e->modifiers() & Qt::ControlModifier)) {
        switch (e->key()) {
        case Qt::Key_Right:
            d->buttonAddClicked();
            break;
        case Qt::Key_Left:
            d->buttonRemoveClicked();
            break;
        case Qt::Key_Up:
            d->buttonUpClicked();
            break;
        case Qt::Key_Down:
            d->buttonDownClicked();
            break;
        default:
            e->ignore();
            return;
        }
    }
}

bool KActionSelector::eventFilter(QObject *o, QEvent *e)
{
    if (d->keyboardEnabled && e->type() == QEvent::KeyPress) {
        if ((((QKeyEvent *)e)->modifiers() & Qt::ControlModifier)) {
            switch (((QKeyEvent *)e)->key()) {
            case Qt::Key_Right:
                d->buttonAddClicked();
                break;
            case Qt::Key_Left:
                d->buttonRemoveClicked();
                break;
            case Qt::Key_Up:
                d->buttonUpClicked();
                break;
            case Qt::Key_Down:
                d->buttonDownClicked();
                break;
            default:
                return QWidget::eventFilter(o, e);
            }
            return true;
        } else if (QListWidget *lb = qobject_cast<QListWidget *>(o)) {
            switch (((QKeyEvent *)e)->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
                int index = lb->currentRow();
                if (index < 0) {
                    break;
                }
                d->moveItem(lb->item(index));
                return true;
            }
        }
    }
    return QWidget::eventFilter(o, e);
}

// END Protected

// BEGIN Private Slots

void KActionSelectorPrivate::buttonAddClicked()
{
    // move all selected items from available to selected listbox
    const QList<QListWidgetItem *> list = availableListWidget->selectedItems();
    for (QListWidgetItem *item : list) {
        availableListWidget->takeItem(availableListWidget->row(item));
        selectedListWidget->insertItem(insertionIndex(selectedListWidget, selectedInsertionPolicy), item);
        selectedListWidget->setCurrentItem(item);
        Q_EMIT q->added(item);
    }
    if (selectedInsertionPolicy == KActionSelector::Sorted) {
        selectedListWidget->sortItems();
    }
    selectedListWidget->setFocus();
}

void KActionSelectorPrivate::buttonRemoveClicked()
{
    // move all selected items from selected to available listbox
    const QList<QListWidgetItem *> list = selectedListWidget->selectedItems();
    for (QListWidgetItem *item : list) {
        selectedListWidget->takeItem(selectedListWidget->row(item));
        availableListWidget->insertItem(insertionIndex(availableListWidget, availableInsertionPolicy), item);
        availableListWidget->setCurrentItem(item);
        Q_EMIT q->removed(item);
    }
    if (availableInsertionPolicy == KActionSelector::Sorted) {
        availableListWidget->sortItems();
    }
    availableListWidget->setFocus();
}

void KActionSelectorPrivate::buttonUpClicked()
{
    int c = selectedRowIndex(selectedListWidget);
    if (c < 1) {
        return;
    }
    QListWidgetItem *item = selectedListWidget->item(c);
    selectedListWidget->takeItem(c);
    selectedListWidget->insertItem(c - 1, item);
    selectedListWidget->setCurrentItem(item);
    Q_EMIT q->movedUp(item);
}

void KActionSelectorPrivate::buttonDownClicked()
{
    int c = selectedRowIndex(selectedListWidget);
    if (c < 0 || c == selectedListWidget->count() - 1) {
        return;
    }
    QListWidgetItem *item = selectedListWidget->item(c);
    selectedListWidget->takeItem(c);
    selectedListWidget->insertItem(c + 1, item);
    selectedListWidget->setCurrentItem(item);
    Q_EMIT q->movedDown(item);
}

void KActionSelectorPrivate::itemDoubleClicked(QListWidgetItem *item)
{
    if (moveOnDoubleClick) {
        moveItem(item);
    }
}

// END Private Slots

// BEGIN Private Methods

void KActionSelectorPrivate::loadIcons()
{
    btnAdd->setIcon(QIcon::fromTheme(addIcon));
    btnRemove->setIcon(QIcon::fromTheme(removeIcon));
    btnUp->setIcon(QIcon::fromTheme(upIcon));
    btnDown->setIcon(QIcon::fromTheme(downIcon));
}

void KActionSelectorPrivate::moveItem(QListWidgetItem *item)
{
    QListWidget *lbFrom = item->listWidget();
    QListWidget *lbTo;
    if (lbFrom == availableListWidget) {
        lbTo = selectedListWidget;
    } else if (lbFrom == selectedListWidget) {
        lbTo = availableListWidget;
    } else { //?! somewhat unlikely...
        return;
    }

    KActionSelector::InsertionPolicy p = (lbTo == availableListWidget) ? availableInsertionPolicy : selectedInsertionPolicy;

    lbFrom->takeItem(lbFrom->row(item));
    lbTo->insertItem(insertionIndex(lbTo, p), item);
    lbTo->setFocus();
    lbTo->setCurrentItem(item);

    if (p == KActionSelector::Sorted) {
        lbTo->sortItems();
    }
    if (lbTo == selectedListWidget) {
        Q_EMIT q->added(item);
    } else {
        Q_EMIT q->removed(item);
    }
}

int KActionSelectorPrivate::insertionIndex(QListWidget *lb, KActionSelector::InsertionPolicy policy)
{
    int index;
    switch (policy) {
    case KActionSelector::BelowCurrent:
        index = lb->currentRow();
        if (index > -1) {
            index += 1;
        }
        break;
    case KActionSelector::AtTop:
        index = 0;
        break;
    case KActionSelector::AtBottom:
        index = lb->count();
        break;
    default:
        index = -1;
    }
    return index;
}

int KActionSelectorPrivate::selectedRowIndex(QListWidget *lb)
{
    QList<QListWidgetItem *> list = lb->selectedItems();
    if (list.isEmpty()) {
        return -1;
    }
    return lb->row(list.at(0));
}

// END Private Methods
#include "moc_kactionselector.cpp"
