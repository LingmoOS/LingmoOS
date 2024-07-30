/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KREPARENTINGPROXYMODEL_H
#define KREPARENTINGPROXYMODEL_H

#include <QAbstractProxyModel>

class KReparentingProxyModelPrivate;

/**
  @brief Restructures a source model, changing the parents of items.

  Subclasses can change the structure of a source model by reimplementing
  the isDescendantOf method.

  For example, if the source model is a list,

  @verbatim
  0
  - A
  - B
  - C
  - D
  - E
  @endverbatim

  It could be converted to a tree by an implementation something like:

  @code
  bool MyReparentingModel::isDescedantOf(const QModelIndex& ancestor, const QModelIndex& descendant ) const
  {
     return (
             (ancestor.data().toString() == "A" && descendant.data().toString() == "B")
          || (ancestor.data().toString() == "A" && descendant.data().toString() == "C")
          || (ancestor.data().toString() == "B" && descendant.data().toString() == "C")
          || (ancestor.data().toString() == "A" && descendant.data().toString() == "D")
          )
    ? true : KReparentingProxyModel::isDescendantOf(ancestor, descendant);
  }
  @endcode

  to get this result:

  @verbatim
  0
  - A
  - - B
  - - - C
  - - D
  - E
  @endverbatim

  Note that the implementation returns true for a query if "C" is a descendant of "A".
  The implementation must return the correct value for all of its descendants, not only its direct parent.
  The actual location to insert the descendant in the tree is determined internally by a binary find algorithm.

  The KReparentingProxyModel performs movement of items in the left-right directions, but not the up-down directions.

  \image html reparenting1.png "KReparentingProxyModel moving rows left to right"

  \image html reparenting2.png "KReparentingProxyModel can not move items both left-right and up-down"

  Reordering the rows in a model is the domain of QSortFilterProxyModel. An intermediate QSortFilterProxyModel
  can be used to achieve the desired result.

  \image html reparenting3.png "KReparentingProxyModel and QSortFilterProxyModel working in concert to move items both left-right and up-down"

  @code
    QAbstractItemModel *model = getModel();

    QSortFilterProxyModel *sorter = getSorter();
    sorter->setSourceModel(model);

    KReparentingProxyModel *reparenter = getReparenter();
    reparenter->setSourceModel(sorter);

    QTreeView *view = getView();
    view->setModel(reparenter);
  @endcode

*/
class KReparentingProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    KReparentingProxyModel(QObject *parent = nullptr);

    ~KReparentingProxyModel() override;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    /**
      Reimplement this to return whether @p descendant is a descendant of @p ancestor.
    */
    virtual bool isDescendantOf(const QModelIndex &ancestor, const QModelIndex &descendant) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    Qt::DropActions supportedDropActions() const override;

protected:
    void beginChangeRule();
    void endChangeRule();

private:
    Q_DECLARE_PRIVATE(KReparentingProxyModel)
    //@cond PRIVATE
    KReparentingProxyModelPrivate *d_ptr;

    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeInserted(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsInserted(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeRemoved(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsRemoved(const QModelIndex &, int, int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int))
    Q_PRIVATE_SLOT(d_func(), void sourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int))
    Q_PRIVATE_SLOT(d_func(), void sourceModelAboutToBeReset())
    Q_PRIVATE_SLOT(d_func(), void sourceModelReset())
    Q_PRIVATE_SLOT(d_func(), void sourceLayoutAboutToBeChanged())
    Q_PRIVATE_SLOT(d_func(), void sourceLayoutChanged())
    Q_PRIVATE_SLOT(d_func(), void sourceDataChanged(const QModelIndex &, const QModelIndex &))

    //@endcond
};

#endif
