/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>
    SPDX-FileCopyrightText: 2016 Ableton AG <info@ableton.com>
    SPDX-FileContributor: Stephen Kelly <stephen.kelly@ableton.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kmodelindexproxymapper.h"
#include "kitemmodels_debug.h"

#include <QAbstractItemModel>
#include <QAbstractProxyModel>
#include <QItemSelectionModel>
#include <QPointer>

class KModelIndexProxyMapperPrivate
{
    KModelIndexProxyMapperPrivate(const QAbstractItemModel *leftModel, const QAbstractItemModel *rightModel, KModelIndexProxyMapper *qq)
        : q_ptr(qq)
        , m_leftModel(leftModel)
        , m_rightModel(rightModel)
        , mConnected(false)
    {
        createProxyChain();
    }

    void createProxyChain();
    void checkConnected();
    void setConnected(bool connected);

    bool assertSelectionValid(const QItemSelection &selection) const
    {
        for (const QItemSelectionRange &range : selection) {
            if (!range.isValid()) {
                qCDebug(KITEMMODELS_LOG) << selection << m_leftModel << m_rightModel << m_proxyChainDown << m_proxyChainUp;
            }
            Q_ASSERT(range.isValid());
        }
        return true;
    }

    Q_DECLARE_PUBLIC(KModelIndexProxyMapper)
    KModelIndexProxyMapper *const q_ptr;

    QList<QPointer<const QAbstractProxyModel>> m_proxyChainUp;
    QList<QPointer<const QAbstractProxyModel>> m_proxyChainDown;

    QPointer<const QAbstractItemModel> m_leftModel;
    QPointer<const QAbstractItemModel> m_rightModel;

    bool mConnected;
};

/*

  The idea here is that <tt>this</tt> selection model and proxySelectionModel might be in different parts of the
  proxy chain. We need to build up to two chains of proxy models to create mappings between them.

  Example 1:

     Root model
          |
        /    \
    Proxy 1   Proxy 3
       |       |
    Proxy 2   Proxy 4

  Need Proxy 1 and Proxy 2 in one chain, and Proxy 3 and 4 in the other.

  Example 2:

     Root model
          |
        Proxy 1
          |
        Proxy 2
        /     \
    Proxy 3   Proxy 6
       |       |
    Proxy 4   Proxy 7
       |
    Proxy 5

  We first build the chain from 1 to 5, then start building the chain from 7 to 1. We stop when we find that proxy 2 is
  already in the first chain.

  Stephen Kelly, 30 March 2010.
*/

void KModelIndexProxyMapperPrivate::createProxyChain()
{
    for (auto p : std::as_const(m_proxyChainUp)) {
        p->disconnect(q_ptr);
    }
    for (auto p : std::as_const(m_proxyChainDown)) {
        p->disconnect(q_ptr);
    }
    m_proxyChainUp.clear();
    m_proxyChainDown.clear();
    QPointer<const QAbstractItemModel> targetModel = m_rightModel;

    QList<QPointer<const QAbstractProxyModel>> proxyChainDown;
    QPointer<const QAbstractProxyModel> selectionTargetProxyModel = qobject_cast<const QAbstractProxyModel *>(targetModel);
    while (selectionTargetProxyModel) {
        proxyChainDown.prepend(selectionTargetProxyModel);
        QObject::connect(selectionTargetProxyModel.data(), &QAbstractProxyModel::sourceModelChanged, q_ptr, [this] {
            createProxyChain();
        });

        selectionTargetProxyModel = qobject_cast<const QAbstractProxyModel *>(selectionTargetProxyModel->sourceModel());

        if (selectionTargetProxyModel == m_leftModel) {
            m_proxyChainDown = proxyChainDown;
            checkConnected();
            return;
        }
    }

    QPointer<const QAbstractItemModel> sourceModel = m_leftModel;
    QPointer<const QAbstractProxyModel> sourceProxyModel = qobject_cast<const QAbstractProxyModel *>(sourceModel);

    while (sourceProxyModel) {
        m_proxyChainUp.append(sourceProxyModel);
        QObject::connect(sourceProxyModel.data(), &QAbstractProxyModel::sourceModelChanged, q_ptr, [this] {
            createProxyChain();
        });

        sourceProxyModel = qobject_cast<const QAbstractProxyModel *>(sourceProxyModel->sourceModel());

        const int targetIndex = proxyChainDown.indexOf(sourceProxyModel);

        if (targetIndex != -1) {
            m_proxyChainDown = proxyChainDown.mid(targetIndex + 1, proxyChainDown.size());
            checkConnected();
            return;
        }
    }
    m_proxyChainDown = proxyChainDown;
    checkConnected();
}

void KModelIndexProxyMapperPrivate::checkConnected()
{
    auto konamiRight = m_proxyChainUp.isEmpty() ? m_leftModel : m_proxyChainUp.last()->sourceModel();
    auto konamiLeft = m_proxyChainDown.isEmpty() ? m_rightModel : m_proxyChainDown.first()->sourceModel();
    setConnected(konamiLeft && (konamiLeft == konamiRight));
}

void KModelIndexProxyMapperPrivate::setConnected(bool connected)
{
    if (mConnected != connected) {
        Q_Q(KModelIndexProxyMapper);
        mConnected = connected;
        Q_EMIT q->isConnectedChanged();
    }
}

KModelIndexProxyMapper::KModelIndexProxyMapper(const QAbstractItemModel *leftModel, const QAbstractItemModel *rightModel, QObject *parent)
    : QObject(parent)
    , d_ptr(new KModelIndexProxyMapperPrivate(leftModel, rightModel, this))
{
}

KModelIndexProxyMapper::~KModelIndexProxyMapper() = default;

QModelIndex KModelIndexProxyMapper::mapLeftToRight(const QModelIndex &index) const
{
    const QItemSelection selection = mapSelectionLeftToRight(QItemSelection(index, index));
    if (selection.isEmpty()) {
        return QModelIndex();
    }

    return selection.indexes().first();
}

QModelIndex KModelIndexProxyMapper::mapRightToLeft(const QModelIndex &index) const
{
    const QItemSelection selection = mapSelectionRightToLeft(QItemSelection(index, index));
    if (selection.isEmpty()) {
        return QModelIndex();
    }

    return selection.indexes().first();
}

QItemSelection KModelIndexProxyMapper::mapSelectionLeftToRight(const QItemSelection &selection) const
{
    Q_D(const KModelIndexProxyMapper);

    if (selection.isEmpty() || !d->mConnected) {
        return QItemSelection();
    }

    if (selection.first().model() != d->m_leftModel) {
        qCDebug(KITEMMODELS_LOG) << "FAIL" << selection.first().model() << d->m_leftModel << d->m_rightModel;
    }
    Q_ASSERT(selection.first().model() == d->m_leftModel);

    QItemSelection seekSelection = selection;
    Q_ASSERT(d->assertSelectionValid(seekSelection));
    QListIterator<QPointer<const QAbstractProxyModel>> iUp(d->m_proxyChainUp);

    while (iUp.hasNext()) {
        const QPointer<const QAbstractProxyModel> proxy = iUp.next();
        if (!proxy) {
            return QItemSelection();
        }

        Q_ASSERT(seekSelection.isEmpty() || seekSelection.first().model() == proxy);
        seekSelection = proxy->mapSelectionToSource(seekSelection);
        Q_ASSERT(seekSelection.isEmpty() || seekSelection.first().model() == proxy->sourceModel());

        Q_ASSERT(d->assertSelectionValid(seekSelection));
    }

    QListIterator<QPointer<const QAbstractProxyModel>> iDown(d->m_proxyChainDown);

    while (iDown.hasNext()) {
        const QPointer<const QAbstractProxyModel> proxy = iDown.next();
        if (!proxy) {
            return QItemSelection();
        }
        Q_ASSERT(seekSelection.isEmpty() || seekSelection.first().model() == proxy->sourceModel());
        seekSelection = proxy->mapSelectionFromSource(seekSelection);
        Q_ASSERT(seekSelection.isEmpty() || seekSelection.first().model() == proxy);

        Q_ASSERT(d->assertSelectionValid(seekSelection));
    }

    Q_ASSERT((!seekSelection.isEmpty() && seekSelection.first().model() == d->m_rightModel) || true);
    return seekSelection;
}

QItemSelection KModelIndexProxyMapper::mapSelectionRightToLeft(const QItemSelection &selection) const
{
    Q_D(const KModelIndexProxyMapper);

    if (selection.isEmpty() || !d->mConnected) {
        return QItemSelection();
    }

    if (selection.first().model() != d->m_rightModel) {
        qCDebug(KITEMMODELS_LOG) << "FAIL" << selection.first().model() << d->m_leftModel << d->m_rightModel;
    }
    Q_ASSERT(selection.first().model() == d->m_rightModel);

    QItemSelection seekSelection = selection;
    Q_ASSERT(d->assertSelectionValid(seekSelection));
    QListIterator<QPointer<const QAbstractProxyModel>> iDown(d->m_proxyChainDown);

    iDown.toBack();
    while (iDown.hasPrevious()) {
        const QPointer<const QAbstractProxyModel> proxy = iDown.previous();
        if (!proxy) {
            return QItemSelection();
        }
        seekSelection = proxy->mapSelectionToSource(seekSelection);

        Q_ASSERT(d->assertSelectionValid(seekSelection));
    }

    QListIterator<QPointer<const QAbstractProxyModel>> iUp(d->m_proxyChainUp);

    iUp.toBack();
    while (iUp.hasPrevious()) {
        const QPointer<const QAbstractProxyModel> proxy = iUp.previous();
        if (!proxy) {
            return QItemSelection();
        }
        seekSelection = proxy->mapSelectionFromSource(seekSelection);

        Q_ASSERT(d->assertSelectionValid(seekSelection));
    }

    Q_ASSERT((!seekSelection.isEmpty() && seekSelection.first().model() == d->m_leftModel) || true);
    return seekSelection;
}

bool KModelIndexProxyMapper::isConnected() const
{
    Q_D(const KModelIndexProxyMapper);
    return d->mConnected;
}

#include "moc_kmodelindexproxymapper.cpp"
