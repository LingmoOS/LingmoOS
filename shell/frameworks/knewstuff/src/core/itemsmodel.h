/*
    knewstuff3/ui/itemsmodel.h.
    SPDX-FileCopyrightText: 2008 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KNEWSTUFF3_ITEMSMODEL_P_H
#define KNEWSTUFF3_ITEMSMODEL_P_H

#include <QAbstractListModel>
#include <memory>

#include "entry.h"
#include "knewstuffcore_export.h"

class KJob;

namespace KNSCore
{
class EngineBase;
class ItemsModelPrivate;

class KNEWSTUFFCORE_EXPORT ItemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ItemsModel(EngineBase *engine, QObject *parent = nullptr);
    ~ItemsModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * The row of the entry passed to the function, or -1 if the entry is not contained
     * within the model.
     * @since 5.63
     */
    int row(const Entry &entry) const;

    void addEntry(const Entry &entry);
    void removeEntry(const Entry &entry);

    bool hasPreviewImages() const;
    bool hasWebService() const;

Q_SIGNALS:
    void jobStarted(KJob *, const QString &label);
    void loadPreview(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type);

public Q_SLOTS:
    void slotEntryChanged(const KNSCore::Entry &entry);
    void slotEntriesLoaded(const KNSCore::Entry::List &entries);
    void clearEntries();
    void slotEntryPreviewLoaded(const KNSCore::Entry &entry, KNSCore::Entry::PreviewType type);

private:
    const std::unique_ptr<ItemsModelPrivate> d;
};

} // end KNS namespace

#endif
