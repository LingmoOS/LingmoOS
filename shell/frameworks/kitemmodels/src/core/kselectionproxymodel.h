/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSELECTIONPROXYMODEL_H
#define KSELECTIONPROXYMODEL_H

#include <QAbstractProxyModel>
#include <QItemSelectionModel>

#include "kitemmodels_export.h"

#include <memory>

class KSelectionProxyModelPrivate;

/**
  @class KSelectionProxyModel kselectionproxymodel.h KSelectionProxyModel

  @brief A Proxy Model which presents a subset of its source model to observers.

  The KSelectionProxyModel is most useful as a convenience for displaying the selection in one view in
  another view. The selectionModel of the initial view is used to create a proxied model which is filtered
  based on the configuration of this class.

  For example, when a user clicks a mail folder in one view in an email application, the contained emails
  should be displayed in another view.

  This takes away the need for the developer to handle the selection between the views, including all the
  mapToSource, mapFromSource and setRootIndex calls.

  @code
  MyModel *sourceModel = new MyModel(this);
  QTreeView *leftView = new QTreeView(this);
  leftView->setModel(sourceModel);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(leftView->selectionModel(), this);
  selectionProxy->setSourceModel(sourceModel);

  QTreeView *rightView = new QTreeView(this);
  rightView->setModel(selectionProxy);
  @endcode

  \image html selectionproxymodelsimpleselection.png "A Selection in one view creating a model for use with another view."

  The KSelectionProxyModel can handle complex selections.

  \image html selectionproxymodelmultipleselection.png "Non-contiguous selection creating a new simple model in a second view."

  The contents of the secondary view depends on the selection in the primary view, and the configuration of the proxy model.
  See KSelectionProxyModel::setFilterBehavior for the different possible configurations.

  For example, if the filterBehavior is SubTrees, selecting another item in an already selected subtree has no effect.

  \image html selectionproxymodelmultipleselection-withdescendant.png "Selecting an item and its descendant."

  See the test application in tests/proxymodeltestapp to try out the valid configurations.

  \image html kselectionproxymodel-testapp.png "KSelectionProxyModel test application"

  Obviously, the KSelectionProxyModel may be used in a view, or further processed with other proxy models.
  See KAddressBook and AkonadiConsole in kdepim for examples which use a further KDescendantsProxyModel
  and QSortFilterProxyModel on top of a KSelectionProxyModel.

  Additionally, this class can be used to programmatically choose some items from the source model to display in the view. For example,
  this is how the Favourite Folder View in KMail works, and is also used in unit testing.

  See also: https://doc.qt.io/qt-5/model-view-programming.html#proxy-models

  @since 4.4
  @author Stephen Kelly <steveire@gmail.com>

*/
class KITEMMODELS_EXPORT KSelectionProxyModel : public QAbstractProxyModel
{
    Q_OBJECT
    Q_PROPERTY(FilterBehavior filterBehavior READ filterBehavior WRITE setFilterBehavior NOTIFY filterBehaviorChanged)
    Q_PROPERTY(QItemSelectionModel *selectionModel READ selectionModel WRITE setSelectionModel NOTIFY selectionModelChanged)
public:
    /**
    ctor.

    @p selectionModel The selection model used to filter what is presented by the proxy.
    */
    // KF6: Remove the selectionModel from the constructor here.
    explicit KSelectionProxyModel(QItemSelectionModel *selectionModel, QObject *parent = nullptr);

    /**
     Default constructor. Allow the creation of a KSelectionProxyModel in QML
     code. QML will assign a parent after construction.
     */
    // KF6: Remove in favor of the constructor above.
    explicit KSelectionProxyModel();

    /**
    dtor
    */
    ~KSelectionProxyModel() override;

    /**
    reimp.
    */
    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QItemSelectionModel *selectionModel() const;
    void setSelectionModel(QItemSelectionModel *selectionModel);

    enum FilterBehavior {
        SubTrees,
        SubTreeRoots,
        SubTreesWithoutRoots,
        ExactSelection,
        ChildrenOfExactSelection,
        InvalidBehavior,
    };
    Q_ENUM(FilterBehavior)

    /**
      Set the filter behaviors of this model.
      The filter behaviors of the model govern the content of the model based on the selection of the contained QItemSelectionModel.

      See kdeui/proxymodeltestapp to try out the different proxy model behaviors.

      The most useful behaviors are SubTrees, ExactSelection and ChildrenOfExactSelection.

      The default behavior is SubTrees. This means that this proxy model will contain the roots of the items in the source model.
      Any descendants which are also selected have no additional effect.
      For example if the source model is like:

      @verbatim
      (root)
        - A
        - B
          - C
          - D
            - E
              - F
            - G
        - H
        - I
          - J
          - K
          - L
      @endverbatim

      And A, B, C and D are selected, the proxy will contain:

      @verbatim
      (root)
        - A
        - B
          - C
          - D
            - E
              - F
            - G
      @endverbatim

      That is, selecting 'D' or 'C' if 'B' is also selected has no effect. If 'B' is de-selected, then 'C' amd 'D' become top-level items:

      @verbatim
      (root)
        - A
        - C
        - D
          - E
            - F
          - G
      @endverbatim

      This is the behavior used by KJots when rendering books.

      If the behavior is set to SubTreeRoots, then the children of selected indexes are not part of the model. If 'A', 'B' and 'D' are selected,

      @verbatim
      (root)
        - A
        - B
      @endverbatim

      Note that although 'D' is selected, it is not part of the proxy model, because its parent 'B' is already selected.

      SubTreesWithoutRoots has the effect of not making the selected items part of the model, but making their children part of the model instead. If 'A', 'B'
      and 'I' are selected:

      @verbatim
      (root)
        - C
        - D
          - E
            - F
          - G
        - J
        - K
        - L
      @endverbatim

      Note that 'A' has no children, so selecting it has no outward effect on the model.

      ChildrenOfExactSelection causes the proxy model to contain the children of the selected indexes,but further descendants are omitted.
      Additionally, if descendants of an already selected index are selected, their children are part of the proxy model.
      For example, if 'A', 'B', 'D' and 'I' are selected:

      @verbatim
      (root)
        - C
        - D
        - E
        - G
        - J
        - K
        - L
      @endverbatim

      This would be useful for example if showing containers (for example maildirs) in one view and their items in another. Sub-maildirs would still appear in
      the proxy, but could be filtered out using a QSortfilterProxyModel.

      The ExactSelection behavior causes the selected items to be part of the proxy model, even if their ancestors are already selected, but children of
      selected items are not included.

      Again, if 'A', 'B', 'D' and 'I' are selected:

      @verbatim
      (root)
        - A
        - B
        - D
        - I
      @endverbatim

      This is the behavior used by the Favourite Folder View in KMail.

    */
    void setFilterBehavior(FilterBehavior behavior);
    FilterBehavior filterBehavior() const;

    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

    QItemSelection mapSelectionFromSource(const QItemSelection &selection) const override;
    QItemSelection mapSelectionToSource(const QItemSelection &selection) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    QStringList mimeTypes() const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int, int, const QModelIndex & = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &) const override;
    int columnCount(const QModelIndex & = QModelIndex()) const override;

    virtual QModelIndexList match(const QModelIndex &start,
                                  int role,
                                  const QVariant &value,
                                  int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;

Q_SIGNALS:
    /**
      @internal
      Emitted before @p removeRootIndex, an index in the sourceModel is removed from
      the root selected indexes. This may be unrelated to rows removed from the model,
      depending on configuration.
    */
    void rootIndexAboutToBeRemoved(const QModelIndex &removeRootIndex, QPrivateSignal);

    /**
      @internal
      Emitted when @p newIndex, an index in the sourceModel is added to the root selected
      indexes. This may be unrelated to rows inserted to the model,
      depending on configuration.
    */
    void rootIndexAdded(const QModelIndex &newIndex, QPrivateSignal);

    /**
      @internal
      Emitted before @p selection, a selection in the sourceModel, is removed from
      the root selection.
    */
    void rootSelectionAboutToBeRemoved(const QItemSelection &selection, QPrivateSignal);

    /**
      @internal
      Emitted after @p selection, a selection in the sourceModel, is added to
      the root selection.
    */
    void rootSelectionAdded(const QItemSelection &selection, QPrivateSignal);

    void selectionModelChanged(QPrivateSignal);
    void filterBehaviorChanged(QPrivateSignal);

protected:
    QList<QPersistentModelIndex> sourceRootIndexes() const;

private:
    Q_DECLARE_PRIVATE(KSelectionProxyModel)
    //@cond PRIVATE
    std::unique_ptr<KSelectionProxyModelPrivate> const d_ptr;

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
    Q_PRIVATE_SLOT(d_func(), void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected))
    Q_PRIVATE_SLOT(d_func(), void sourceModelDestroyed())

    //@endcond
};

#endif
