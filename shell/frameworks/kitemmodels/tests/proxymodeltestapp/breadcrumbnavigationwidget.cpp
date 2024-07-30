/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "breadcrumbnavigationwidget.h"

#include "dynamictreemodel.h"
#include "dynamictreewidget.h"

#include <QHBoxLayout>
#include <QListView>
#include <QSplitter>
#include <QTreeView>

#include "kbreadcrumbselectionmodel.h"
#include "kselectionproxymodel.h"

#define SON(object) object->setObjectName(QStringLiteral(#object))

CurrentItemLabel::CurrentItemLabel(QAbstractItemModel *model, QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
    , m_model(model)
{
    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), SLOT(dataChanged(QModelIndex, QModelIndex)));
    connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), SLOT(rowsInserted(QModelIndex, int, int)));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(rowsRemoved(QModelIndex, int, int)));
    connect(model, SIGNAL(modelReset()), SLOT(modelReset()));

    if (!m_model->hasChildren()) {
        setText(QStringLiteral("No selection"));
    }
}

void CurrentItemLabel::dataChanged(const QModelIndex &, const QModelIndex &)
{
    setText(m_model->index(0, 0).data().toString());
}

void CurrentItemLabel::rowsInserted(const QModelIndex &, int, int)
{
    setText(m_model->index(0, 0).data().toString());
}

void CurrentItemLabel::rowsRemoved(const QModelIndex &, int, int)
{
    if (!m_model->hasChildren()) {
        setText(QStringLiteral("No selection"));
        return;
    }
    setText(m_model->index(0, 0).data().toString());
}

void CurrentItemLabel::modelReset()
{
    if (!m_model->hasChildren()) {
        setText(QStringLiteral("No selection"));
    }
    setText(m_model->index(0, 0).data().toString());
}

KBreadcrumbNavigationProxyModel::KBreadcrumbNavigationProxyModel(QItemSelectionModel *selectionModel, QObject *parent)
    : KSelectionProxyModel(selectionModel, parent)
{
}

QVariant KBreadcrumbNavigationProxyModel::data(const QModelIndex &index, int role) const
{
    if (rowCount() > 2 && index.row() == 0 && role == Qt::DisplayRole) {
        QModelIndex sourceIndex = mapToSource(index);
        QStringList dataList;
        while (sourceIndex.isValid()) {
            dataList.prepend(sourceIndex.data().toString());
            sourceIndex = sourceIndex.parent();
        }
        return dataList.join(QLatin1String(" > "));
    }
    return KSelectionProxyModel::data(index, role);
}

void KBreadcrumbNavigationProxyModel::setShowHiddenAscendantData(bool showHiddenAscendantData)
{
    m_showHiddenAscendantData = showHiddenAscendantData;
}

bool KBreadcrumbNavigationProxyModel::showHiddenAscendantData() const
{
    return m_showHiddenAscendantData;
}

KNavigatingProxyModel::KNavigatingProxyModel(QItemSelectionModel *selectionModel, QObject *parent)
    : KSelectionProxyModel(selectionModel, parent)
    , m_selectionModel(selectionModel)
{
}

void KNavigatingProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(navigationSelectionChanged(QItemSelection, QItemSelection)));

    KSelectionProxyModel::setSourceModel(sourceModel);
    updateNavigation();
}

void KNavigatingProxyModel::navigationSelectionChanged(const QItemSelection &, const QItemSelection &)
{
    updateNavigation();
}

void KNavigatingProxyModel::updateNavigation()
{
    if (!sourceModel()) {
        return;
    }

    if (m_selectionModel->selection().isEmpty()) {
        setFilterBehavior(KSelectionProxyModel::ExactSelection);
        QModelIndex top = sourceModel()->index(0, 0);
        QModelIndex bottom = sourceModel()->index(sourceModel()->rowCount() - 1, 0);

        disconnect(m_selectionModel,
                   SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                   this,
                   SLOT(navigationSelectionChanged(QItemSelection, QItemSelection)));
        m_selectionModel->select(QItemSelection(top, bottom), QItemSelectionModel::Select);
        connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(navigationSelectionChanged(QItemSelection, QItemSelection)));
    } else if (filterBehavior() != KSelectionProxyModel::ChildrenOfExactSelection) {
        setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
    }
}

void KNavigatingProxyModel::modelReset()
{
    updateNavigation();
}

QVariant KNavigatingProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole && sourceModel()->hasChildren(mapToSource(index))) {
        return QString("+ " + KSelectionProxyModel::data(index, role).toString());
    }
    return KSelectionProxyModel::data(index, role);
}

KForwardingItemSelectionModel::KForwardingItemSelectionModel(QAbstractItemModel *model, QItemSelectionModel *selectionModel, QObject *parent)
    : QItemSelectionModel(model, parent)
    , m_selectionModel(selectionModel)
    , m_direction(Forward)
{
    Q_ASSERT(model == selectionModel->model());
    connect(selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(navigationSelectionChanged(QItemSelection, QItemSelection)));
}

KForwardingItemSelectionModel::KForwardingItemSelectionModel(QAbstractItemModel *model,
                                                             QItemSelectionModel *selectionModel,
                                                             Direction direction,
                                                             QObject *parent)
    : QItemSelectionModel(model, parent)
    , m_selectionModel(selectionModel)
    , m_direction(direction)
{
    Q_ASSERT(model == selectionModel->model());
    if (m_direction == Forward) {
        connect(selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(navigationSelectionChanged(QItemSelection, QItemSelection)));
    }
}

void KForwardingItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    if (m_direction == Reverse) {
        m_selectionModel->select(index, command);
    } else {
        QItemSelectionModel::select(index, command);
    }
}

void KForwardingItemSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if (m_direction == Reverse) {
        m_selectionModel->select(selection, command);
    } else {
        QItemSelectionModel::select(selection, command);
    }
}

void KForwardingItemSelectionModel::navigationSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    select(selected, ClearAndSelect);
}

BreadcrumbNavigationWidget::BreadcrumbNavigationWidget(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f)
{
    DynamicTreeModel *rootModel = new DynamicTreeModel(this);
    QSplitter *splitter = new QSplitter(this);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(splitter);

    DynamicTreeWidget *dynamicTree = new DynamicTreeWidget(rootModel, splitter);
    dynamicTree->treeView()->setSelectionMode(QAbstractItemView::SingleSelection);
    dynamicTree->setInitialTree(
        QLatin1String("- 1"
                      "- - 2"
                      "- - 2"
                      "- - - 3"
                      "- - - - 4"
                      "- - - - - 5"
                      "- - 2"
                      "- 6"
                      "- 6"
                      "- 6"
                      "- - 7"
                      "- - - 8"
                      "- - - 8"
                      "- - - - 9"
                      "- - - - - 10"
                      "- - - 8"
                      "- - - 8"
                      "- - 8"
                      "- 16"
                      "- - 17"
                      "- - - 18"
                      "- - - - 19"
                      "- - - - - 20"));

    QList<QItemSelectionModel *> selectionModelList;

    QSplitter *vSplitter = new QSplitter(Qt::Vertical, splitter);

    QItemSelectionModel *rootSelectionModel = new QItemSelectionModel(rootModel, this);
    SON(rootSelectionModel);

    dynamicTree->treeView()->setSelectionModel(rootSelectionModel);

    KBreadcrumbSelectionModel *breadcrumbOnlyProxySelector2 =
        new KBreadcrumbSelectionModel(rootSelectionModel, KBreadcrumbSelectionModel::MakeBreadcrumbSelectionInOther, this);
    SON(breadcrumbOnlyProxySelector2);
    breadcrumbOnlyProxySelector2->setActualSelectionIncluded(false);

    KBreadcrumbNavigationProxyModel *breadcrumbNavigationModel = new KBreadcrumbNavigationProxyModel(breadcrumbOnlyProxySelector2, this);
    SON(breadcrumbNavigationModel);
    breadcrumbNavigationModel->setSourceModel(rootModel);
    breadcrumbNavigationModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);

    QListView *breadcrumbView = new QListView(vSplitter);
    //   SON(breadcrumbNavigationModel);
    breadcrumbView->setModel(breadcrumbNavigationModel);

    // This shouldn't operate on rootSelectionModel. It should operate on oneway instead?
    KLinkItemSelectionModel *breadcrumbViewSelectionModel = new KLinkItemSelectionModel(breadcrumbNavigationModel, rootSelectionModel, this);
    SON(breadcrumbViewSelectionModel);

    KForwardingItemSelectionModel *oneway2 =
        new KForwardingItemSelectionModel(breadcrumbNavigationModel, breadcrumbViewSelectionModel, KForwardingItemSelectionModel::Reverse);
    SON(oneway2);

    breadcrumbView->setSelectionModel(oneway2);

    KSelectionProxyModel *currentItemSelectionModel = new KSelectionProxyModel(rootSelectionModel, this);
    currentItemSelectionModel->setFilterBehavior(KSelectionProxyModel::ExactSelection);
    currentItemSelectionModel->setSourceModel(rootModel);
    SON(currentItemSelectionModel);

    new CurrentItemLabel(currentItemSelectionModel, vSplitter);

    QListView *selectionView = new QListView(vSplitter);

    // Need a one-way connection from rootSelectionModel to rootSelectionModel2

    KForwardingItemSelectionModel *oneway = new KForwardingItemSelectionModel(rootModel, rootSelectionModel);

    KNavigatingProxyModel *navigatingProxyModel = new KNavigatingProxyModel(oneway, this);
    SON(navigatingProxyModel);
    navigatingProxyModel->setSourceModel(rootModel);
    selectionView->setModel(navigatingProxyModel);

    KLinkItemSelectionModel *selectedChildrenSelectionModel = new KLinkItemSelectionModel(navigatingProxyModel, rootSelectionModel, this);

    selectionView->setSelectionModel(selectedChildrenSelectionModel);
}

#include "moc_breadcrumbnavigationwidget.cpp"
