// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace oceanuiV25 {
class OceanUIObject;

class SearchModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SearchModel(QObject *parent = nullptr);

    enum OceanUISearchRole { SearchUrlRole = Qt::UserRole + 300, SearchPlainTextRole, SearchIsBeginRole, SearchTextRole, SearchWeightRole, SearchDataRole };

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    void addSearchData(OceanUIObject *obj, const QString &text, const QString &url);
    void removeSearchData(const OceanUIObject *obj, const QString &text);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

} // namespace oceanuiV25
#endif // SEARCHMODEL_H
