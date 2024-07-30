/*
    SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCHECKABLEPROXYMODEL_H
#define KCHECKABLEPROXYMODEL_H

#include "kitemmodels_export.h"

#include <QIdentityProxyModel>
#include <QItemSelection>

#include <memory>

class KCheckableProxyModelPrivate;

/**
 * @class KCheckableProxyModel kcheckableproxymodel.h KCheckableProxyModel
 *
 * @brief Adds a checkable capability to a source model
 *
 * Items is standard Qt views such as QTreeView and QListView can have a
 * checkable capability and draw checkboxes. Adding such a capability
 * requires specific implementations of the data() and flags() virtual methods.
 * This class implements those methods generically so that it is not necessary to
 * implement them in your model.
 *
 * This can be combined with a KSelectionProxyModel showing the items currently selected
 *
 * @code
 *
 *   QItemSelectionModel *checkModel = new QItemSelectionModel(rootModel, this);
 *   KCheckableProxyModel *checkable = new KCheckableProxyModel(this);
 *   checkable->setSourceModel(rootModel);
 *   checkable->setSelectionModel(checkModel);
 *
 *   QTreeView *tree1 = new QTreeView(vSplitter);
 *   tree1->setModel(checkable);
 *   tree1->expandAll();
 *
 *   KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(checkModel, this);
 *   selectionProxy->setFilterBehavior(KSelectionProxyModel::ExactSelection);
 *   selectionProxy->setSourceModel(rootModel);
 *
 *   QTreeView *tree2 = new QTreeView(vSplitter);
 *   tree2->setModel(selectionProxy);
 * @endcode
 *
 * @image html kcheckableproxymodel.png "A KCheckableProxyModel and KSelectionProxyModel showing checked items"
 *
 * @since 4.6
 * @author Stephen Kelly <steveire@gmail.com>
 */
class KITEMMODELS_EXPORT KCheckableProxyModel : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit KCheckableProxyModel(QObject *parent = nullptr);
    ~KCheckableProxyModel() override;

    void setSelectionModel(QItemSelectionModel *itemSelectionModel);
    QItemSelectionModel *selectionModel() const;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    /// Expose following role: "checkState" => Qt::CheckStateRole
    QHash<int, QByteArray> roleNames() const override;

protected:
    virtual bool select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command);

private:
    Q_DECLARE_PRIVATE(KCheckableProxyModel)
    std::unique_ptr<KCheckableProxyModelPrivate> const d_ptr;

    Q_PRIVATE_SLOT(d_func(), void selectionChanged(const QItemSelection &, const QItemSelection &))
};

#endif
