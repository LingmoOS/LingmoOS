/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kviewstatemaintainerbase.h"

#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QPointer>

class KViewStateMaintainerBasePrivate
{
    KViewStateMaintainerBasePrivate(KViewStateMaintainerBase *qq)
        : q_ptr(qq)
    {
    }

    Q_DECLARE_PUBLIC(KViewStateMaintainerBase)
    KViewStateMaintainerBase *const q_ptr;

    void slotModelAboutToBeReset();
    void slotModelReset();

    QPointer<QAbstractItemView> m_view;
    QMetaObject::Connection m_viewModelAboutToBeResetConnection;
    QMetaObject::Connection m_viewModelResetConnection;

    QPointer<QItemSelectionModel> m_selectionModel;
    QMetaObject::Connection m_selectionModelAboutToBeResetConnection;
    QMetaObject::Connection m_selectionModelResetConnection;
};

void KViewStateMaintainerBasePrivate::slotModelAboutToBeReset()
{
    Q_Q(KViewStateMaintainerBase);
    q->saveState();
}

void KViewStateMaintainerBasePrivate::slotModelReset()
{
    Q_Q(KViewStateMaintainerBase);
    q->restoreState();
}

KViewStateMaintainerBase::KViewStateMaintainerBase(QObject *parent)
    : QObject(parent)
    , d_ptr(new KViewStateMaintainerBasePrivate(this))
{
}

KViewStateMaintainerBase::~KViewStateMaintainerBase() = default;

QItemSelectionModel *KViewStateMaintainerBase::selectionModel() const
{
    Q_D(const KViewStateMaintainerBase);
    if (d->m_selectionModel.isNull()) {
        return nullptr;
    }
    return d->m_selectionModel.data();
}

void KViewStateMaintainerBase::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_D(KViewStateMaintainerBase);
    if (selectionModel == d->m_selectionModel) {
        return;
    }

    d->m_selectionModel = selectionModel;

    if (d->m_view && d->m_view->model()) {
        disconnect(d->m_selectionModelAboutToBeResetConnection);
        disconnect(d->m_selectionModelResetConnection);
    }

    d->m_viewModelAboutToBeResetConnection = connect(d->m_selectionModel->model(), &QAbstractItemModel::modelAboutToBeReset, this, [d]() {
        d->slotModelAboutToBeReset();
    });
    d->m_viewModelResetConnection = connect(d->m_selectionModel->model(), &QAbstractItemModel::modelReset, this, [d]() {
        d->slotModelReset();
    });
}

QAbstractItemView *KViewStateMaintainerBase::view() const
{
    Q_D(const KViewStateMaintainerBase);
    if (d->m_view.isNull()) {
        return nullptr;
    }
    return d->m_view.data();
}

void KViewStateMaintainerBase::setView(QAbstractItemView *view)
{
    Q_D(KViewStateMaintainerBase);
    if (view == d->m_view) {
        return;
    }

    d->m_view = view;

    if (d->m_selectionModel && d->m_selectionModel->model()) {
        disconnect(d->m_viewModelAboutToBeResetConnection);
        disconnect(d->m_viewModelResetConnection);
    }

    if (d->m_view && d->m_view->model()) {
        d->m_selectionModelAboutToBeResetConnection = connect(d->m_view->model(), &QAbstractItemModel::modelAboutToBeReset, this, [d]() {
            d->slotModelAboutToBeReset();
        });
        d->m_selectionModelResetConnection = connect(d->m_view->model(), &QAbstractItemModel::modelReset, this, [d]() {
            d->slotModelReset();
        });
    }
}

#include "moc_kviewstatemaintainerbase.cpp"
