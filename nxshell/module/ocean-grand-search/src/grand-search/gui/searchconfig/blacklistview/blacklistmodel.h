// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLACKLISTMODEL_H
#define BLACKLISTMODEL_H

#include <QStandardItemModel>

#define DATAROLE Qt::UserRole+1

namespace GrandSearch {

class BlackListModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit BlackListModel(int rows, int columns, QObject *parent = Q_NULLPTR);
    ~BlackListModel() override;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;

};
}
#endif // BLACKLISTMODEL_H
