/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kviewstateserializer.h"

#include <QAbstractScrollArea>
#include <QPointer>
#include <QScrollBar>
#include <QTimer>
#include <QTreeView>

class KViewStateSerializerPrivate
{
public:
    KViewStateSerializerPrivate(KViewStateSerializer *qq)
        : q_ptr(qq)
        , m_treeView(nullptr)
        , m_view(nullptr)
        , m_selectionModel(nullptr)
        , m_scrollArea(nullptr)
        , m_horizontalScrollBarValue(-1)
        , m_verticalScrollBarValue(-1)
    {
    }

    Q_DECLARE_PUBLIC(KViewStateSerializer)
    KViewStateSerializer *const q_ptr;

    QStringList getExpandedItems(const QModelIndex &index) const;

    void listenToPendingChanges();
    void processPendingChanges();

    inline void restoreScrollBarState()
    {
        if (!m_scrollArea || !m_scrollArea->horizontalScrollBar() || !m_scrollArea->verticalScrollBar()) {
            return;
        }
        if (m_horizontalScrollBarValue >= 0 && m_horizontalScrollBarValue <= m_scrollArea->horizontalScrollBar()->maximum()) {
            m_scrollArea->horizontalScrollBar()->setValue(m_horizontalScrollBarValue);
            m_horizontalScrollBarValue = -1;
        }
        if (m_verticalScrollBarValue >= 0 && m_verticalScrollBarValue <= m_scrollArea->verticalScrollBar()->maximum()) {
            m_scrollArea->verticalScrollBar()->setValue(m_verticalScrollBarValue);
            m_verticalScrollBarValue = -1;
        }
    }

    void restoreSelection();
    void restoreCurrentItem();
    void restoreExpanded();

    inline bool hasPendingChanges() const
    {
        return !m_pendingCurrent.isEmpty() || !m_pendingExpansions.isEmpty() || !m_pendingSelections.isEmpty();
    }

    const QAbstractItemModel *getModel()
    {
        if (m_selectionModel && m_selectionModel->model()) {
            return m_selectionModel->model();
        } else if (m_view && m_view->model()) {
            return m_view->model();
        }
        return nullptr;
    }

    void rowsInserted(const QModelIndex & /*index*/, int /*start*/, int /*end*/)
    {
        Q_Q(KViewStateSerializer);
        processPendingChanges();

        if (!hasPendingChanges()) {
            q->disconnect(m_rowsInsertedConnection);
            q->deleteLater();
        }
    }

    QTreeView *m_treeView;
    QAbstractItemView *m_view;
    QItemSelectionModel *m_selectionModel;
    QPointer<QAbstractScrollArea> m_scrollArea;

    int m_horizontalScrollBarValue;
    int m_verticalScrollBarValue;
    QSet<QString> m_pendingSelections;
    QSet<QString> m_pendingExpansions;
    QString m_pendingCurrent;
    QMetaObject::Connection m_rowsInsertedConnection;
};

KViewStateSerializer::KViewStateSerializer(QObject *parent)
    : QObject(nullptr)
    , d_ptr(new KViewStateSerializerPrivate(this))
{
    Q_UNUSED(parent);
    qRegisterMetaType<QModelIndex>("QModelIndex");
}

KViewStateSerializer::~KViewStateSerializer() = default;

void KViewStateSerializer::setView(QAbstractItemView *view)
{
    Q_D(KViewStateSerializer);
    d->m_scrollArea = view;
    if (view) {
        d->m_selectionModel = view->selectionModel();
        d->m_treeView = qobject_cast<QTreeView *>(view);
    } else {
        d->m_selectionModel = nullptr;
        d->m_treeView = nullptr;
    }
    d->m_view = view;
}

QAbstractItemView *KViewStateSerializer::view() const
{
    Q_D(const KViewStateSerializer);
    return d->m_view;
}

QItemSelectionModel *KViewStateSerializer::selectionModel() const
{
    Q_D(const KViewStateSerializer);
    return d->m_selectionModel;
}

void KViewStateSerializer::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_D(KViewStateSerializer);
    d->m_selectionModel = selectionModel;
}

void KViewStateSerializerPrivate::listenToPendingChanges()
{
    Q_Q(KViewStateSerializer);
    // watch the model for stuff coming in delayed
    if (hasPendingChanges()) {
        const QAbstractItemModel *model = getModel();
        if (model) {
            q->disconnect(m_rowsInsertedConnection);
            m_rowsInsertedConnection = q->connect(model, &QAbstractItemModel::rowsInserted, q, [this](const QModelIndex &parent, int first, int last) {
                rowsInserted(parent, first, last);
            });
            return;
        } else {
            q->deleteLater();
        }
    } else {
        q->deleteLater();
    }
}

void KViewStateSerializerPrivate::processPendingChanges()
{
    Q_Q(KViewStateSerializer);

    q->restoreCurrentItem(m_pendingCurrent);
    q->restoreSelection(m_pendingSelections.values());
    q->restoreExpanded(m_pendingExpansions.values());
    q->restoreScrollState(m_verticalScrollBarValue, m_horizontalScrollBarValue);
}

QStringList KViewStateSerializerPrivate::getExpandedItems(const QModelIndex &index) const
{
    Q_Q(const KViewStateSerializer);

    QStringList expansion;
    for (int i = 0; i < m_treeView->model()->rowCount(index); ++i) {
        const QModelIndex child = m_treeView->model()->index(i, 0, index);

        // http://bugreports.qt.nokia.com/browse/QTBUG-18039
        if (m_treeView->model()->hasChildren(child)) {
            if (m_treeView->isExpanded(child)) {
                expansion << q->indexToConfigString(child);
            }
            expansion << getExpandedItems(child);
        }
    }
    return expansion;
}

void KViewStateSerializerPrivate::restoreCurrentItem()
{
    Q_Q(KViewStateSerializer);

    QModelIndex currentIndex = q->indexFromConfigString(m_selectionModel->model(), m_pendingCurrent);
    if (currentIndex.isValid()) {
        if (m_treeView) {
            m_treeView->setCurrentIndex(currentIndex);
        } else {
            m_selectionModel->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
        }
        m_pendingCurrent.clear();
    }
}

void KViewStateSerializer::restoreCurrentItem(const QString &indexString)
{
    Q_D(KViewStateSerializer);
    if (!d->m_selectionModel || !d->m_selectionModel->model()) {
        return;
    }

    if (indexString.isEmpty()) {
        return;
    }
    d->m_pendingCurrent = indexString;
    d->restoreCurrentItem();

    if (d->hasPendingChanges()) {
        d->listenToPendingChanges();
    }
}

void KViewStateSerializerPrivate::restoreExpanded()
{
    Q_Q(KViewStateSerializer);

    QSet<QString>::iterator it = m_pendingExpansions.begin();
    for (; it != m_pendingExpansions.end();) {
        QModelIndex idx = q->indexFromConfigString(m_treeView->model(), *it);
        if (idx.isValid()) {
            m_treeView->expand(idx);
            it = m_pendingExpansions.erase(it);
        } else {
            ++it;
        }
    }
}

void KViewStateSerializer::restoreExpanded(const QStringList &indexStrings)
{
    Q_D(KViewStateSerializer);
    if (!d->m_treeView || !d->m_treeView->model()) {
        return;
    }

    if (indexStrings.isEmpty()) {
        return;
    }

    d->m_pendingExpansions.unite(QSet<QString>(indexStrings.begin(), indexStrings.end()));

    d->restoreExpanded();
    if (d->hasPendingChanges()) {
        d->listenToPendingChanges();
    }
}

void KViewStateSerializer::restoreScrollState(int verticalScoll, int horizontalScroll)
{
    Q_D(KViewStateSerializer);

    if (!d->m_scrollArea) {
        return;
    }

    d->m_verticalScrollBarValue = verticalScoll;
    d->m_horizontalScrollBarValue = horizontalScroll;

    QTimer::singleShot(0, this, [d]() {
        d->restoreScrollBarState();
    });
}

void KViewStateSerializerPrivate::restoreSelection()
{
    Q_Q(KViewStateSerializer);

    QSet<QString>::iterator it = m_pendingSelections.begin();
    for (; it != m_pendingSelections.end();) {
        QModelIndex idx = q->indexFromConfigString(m_selectionModel->model(), *it);
        if (idx.isValid()) {
            m_selectionModel->select(idx, QItemSelectionModel::Select);
            it = m_pendingSelections.erase(it);
        } else {
            ++it;
        }
    }
}

void KViewStateSerializer::restoreSelection(const QStringList &indexStrings)
{
    Q_D(KViewStateSerializer);

    if (!d->m_selectionModel || !d->m_selectionModel->model()) {
        return;
    }

    if (indexStrings.isEmpty()) {
        return;
    }

    d->m_pendingSelections.unite(QSet<QString>(indexStrings.begin(), indexStrings.end()));

    d->restoreSelection();
    if (d->hasPendingChanges()) {
        d->listenToPendingChanges();
    }
}

QString KViewStateSerializer::currentIndexKey() const
{
    Q_D(const KViewStateSerializer);
    if (!d->m_selectionModel) {
        return QString();
    }
    return indexToConfigString(d->m_selectionModel->currentIndex());
}

QStringList KViewStateSerializer::expansionKeys() const
{
    Q_D(const KViewStateSerializer);
    if (!d->m_treeView || !d->m_treeView->model()) {
        return QStringList();
    }

    return d->getExpandedItems(QModelIndex());
}

QStringList KViewStateSerializer::selectionKeys() const
{
    Q_D(const KViewStateSerializer);
    if (!d->m_selectionModel) {
        return QStringList();
    }

    const QModelIndexList selectedIndexes = d->m_selectionModel->selectedRows();
    QStringList selection;
    selection.reserve(selectedIndexes.count());
    for (const QModelIndex &index : selectedIndexes) {
        selection << indexToConfigString(index);
    }

    return selection;
}

QPair<int, int> KViewStateSerializer::scrollState() const
{
    Q_D(const KViewStateSerializer);
    return qMakePair(d->m_scrollArea->verticalScrollBar()->value(), d->m_scrollArea->horizontalScrollBar()->value());
}

void KViewStateSerializer::restoreState()
{
    Q_D(KViewStateSerializer);
    // Delete myself if not finished after 60 seconds
    QTimer::singleShot(60000, this, &KViewStateSerializer::deleteLater);

    d->processPendingChanges();
    if (d->hasPendingChanges()) {
        d->listenToPendingChanges();
    }
}

#include "moc_kviewstateserializer.cpp"
