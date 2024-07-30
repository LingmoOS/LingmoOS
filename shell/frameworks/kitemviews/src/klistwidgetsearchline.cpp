/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 Scott Wheeler <wheeler@kde.org>
    SPDX-FileCopyrightText: 2004 Gustavo Sverzut Barbieri <gsbarbieri@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klistwidgetsearchline.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QListWidget>
#include <QTimer>

class KListWidgetSearchLinePrivate
{
public:
    KListWidgetSearchLinePrivate(KListWidgetSearchLine *parent)
        : q(parent)
    {
    }

    void _k_listWidgetDeleted();
    void _k_queueSearch(const QString &);
    void _k_activateSearch();
    void _k_rowsInserted(const QModelIndex &, int, int);
    void _k_dataChanged(const QModelIndex &, const QModelIndex &);

    void init(QListWidget *listWidget = nullptr);
    void updateHiddenState(int start, int end);

    KListWidgetSearchLine *const q;
    QListWidget *listWidget = nullptr;
    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;
    bool activeSearch = false;
    QString search;
    int queuedSearches = 0;
};

/******************************************************************************
 * Public Methods                                                             *
 *****************************************************************************/
KListWidgetSearchLine::KListWidgetSearchLine(QWidget *parent, QListWidget *listWidget)
    : QLineEdit(parent)
    , d(new KListWidgetSearchLinePrivate(this))

{
    d->init(listWidget);
}

KListWidgetSearchLine::~KListWidgetSearchLine()
{
    clear(); // returning items back to listWidget
}

Qt::CaseSensitivity KListWidgetSearchLine::caseSensitive() const
{
    return d->caseSensitivity;
}

QListWidget *KListWidgetSearchLine::listWidget() const
{
    return d->listWidget;
}

/******************************************************************************
 * Public Slots                                                               *
 *****************************************************************************/
void KListWidgetSearchLine::updateSearch(const QString &s)
{
    d->search = s.isNull() ? text() : s;
    if (d->listWidget) {
        d->updateHiddenState(0, d->listWidget->count() - 1);
    }
}

void KListWidgetSearchLine::clear()
{
    // Show items back to QListWidget
    if (d->listWidget != nullptr) {
        for (int i = 0; i < d->listWidget->count(); ++i) {
            d->listWidget->item(i)->setHidden(false);
        }
    }

    d->search = QString();
    d->queuedSearches = 0;
    QLineEdit::clear();
}

void KListWidgetSearchLine::setCaseSensitivity(Qt::CaseSensitivity cs)
{
    d->caseSensitivity = cs;
}

void KListWidgetSearchLine::setListWidget(QListWidget *lw)
{
    if (d->listWidget != nullptr) {
        disconnect(d->listWidget, SIGNAL(destroyed()), this, SLOT(_k_listWidgetDeleted()));
        d->listWidget->model()->disconnect(this);
    }

    d->listWidget = lw;

    if (lw != nullptr) {
        // clang-format off
        connect(d->listWidget, SIGNAL(destroyed()), this, SLOT(_k_listWidgetDeleted()));
        connect(d->listWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(_k_rowsInserted(QModelIndex,int,int)));
        connect(d->listWidget->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(_k_dataChanged(QModelIndex,QModelIndex)));
        // clang-format on
        setEnabled(true);
    } else {
        setEnabled(false);
    }
}

/******************************************************************************
 * Protected Methods                                                          *
 *****************************************************************************/
bool KListWidgetSearchLine::itemMatches(const QListWidgetItem *item, const QString &s) const
{
    if (s.isEmpty()) {
        return true;
    }

    if (item == nullptr) {
        return false;
    }

    return (item->text().indexOf(s, 0, caseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive) >= 0);
}

void KListWidgetSearchLinePrivate::init(QListWidget *_listWidget)
{
    listWidget = _listWidget;

    QObject::connect(q, SIGNAL(textChanged(QString)), q, SLOT(_k_queueSearch(QString)));

    if (listWidget != nullptr) {
        // clang-format off
        QObject::connect(listWidget, SIGNAL(destroyed()), q, SLOT(_k_listWidgetDeleted()));
        QObject::connect(listWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), q, SLOT(_k_rowsInserted(QModelIndex,int,int)));
        QObject::connect(listWidget->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), q, SLOT(_k_dataChanged(QModelIndex,QModelIndex)));
        // clang-format on
        q->setEnabled(true);
    } else {
        q->setEnabled(false);
    }
    q->setClearButtonEnabled(true);
}

void KListWidgetSearchLinePrivate::updateHiddenState(int start, int end)
{
    if (!listWidget) {
        return;
    }

    QListWidgetItem *currentItem = listWidget->currentItem();

    // Remove Non-Matching items
    for (int index = start; index <= end; ++index) {
        QListWidgetItem *item = listWidget->item(index);
        if (!q->itemMatches(item, search)) {
            item->setHidden(true);

            if (item == currentItem) {
                currentItem = nullptr; // It's not in listWidget anymore.
            }
        } else if (item->isHidden()) {
            item->setHidden(false);
        }
    }

    if (listWidget->isSortingEnabled()) {
        listWidget->sortItems();
    }

    if (currentItem != nullptr) {
        listWidget->scrollToItem(currentItem);
    }
}

bool KListWidgetSearchLine::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::MoveToNextLine) || keyEvent->matches(QKeySequence::SelectNextLine)
            || keyEvent->matches(QKeySequence::MoveToPreviousLine) || keyEvent->matches(QKeySequence::SelectPreviousLine)
            || keyEvent->matches(QKeySequence::MoveToNextPage) || keyEvent->matches(QKeySequence::SelectNextPage)
            || keyEvent->matches(QKeySequence::MoveToPreviousPage) || keyEvent->matches(QKeySequence::SelectPreviousPage)) {
            if (d->listWidget) {
                QApplication::sendEvent(d->listWidget, event);
                return true;
            }
        } else if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            if (d->listWidget) {
                QApplication::sendEvent(d->listWidget, event);
                return true;
            }
        }
    }
    return QLineEdit::event(event);
}
/******************************************************************************
 * Protected Slots                                                            *
 *****************************************************************************/
void KListWidgetSearchLinePrivate::_k_queueSearch(const QString &s)
{
    queuedSearches++;
    search = s;
    QTimer::singleShot(200, q, SLOT(_k_activateSearch()));
}

void KListWidgetSearchLinePrivate::_k_activateSearch()
{
    queuedSearches--;

    if (queuedSearches <= 0) {
        q->updateSearch(search);
        queuedSearches = 0;
    }
}

/******************************************************************************
 * KListWidgetSearchLinePrivate Slots                                                              *
 *****************************************************************************/
void KListWidgetSearchLinePrivate::_k_listWidgetDeleted()
{
    listWidget = nullptr;
    q->setEnabled(false);
}

void KListWidgetSearchLinePrivate::_k_rowsInserted(const QModelIndex &parent, int start, int end)
{
    if (parent.isValid()) {
        return;
    }

    updateHiddenState(start, end);
}

void KListWidgetSearchLinePrivate::_k_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft.parent().isValid()) {
        return;
    }

    updateHiddenState(topLeft.row(), bottomRight.row());
}

#include "moc_klistwidgetsearchline.cpp"
