// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSINFOMODEL_H
#define PROCESSINFOMODEL_H

#include "processinfomanager.h"

#include <QAbstractItemModel>
#include <QPointer>

#define COLUMN_ICON     0
#define COLUMN_NAME     1
#define COLUMN_MEM      2
#define COLUMN_FREE_BTN 3
#define COLUMN_NUMS     4

QPixmap appIcon(const int size, const QString &desktop);

class ProcessInfoModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ProcessInfoModel(QObject *parent = nullptr);

    enum ProcessInfoRole
    {
        TextAlignmentRole = Qt::TextAlignmentRole,
        IconRole = Qt::DecorationRole,
        DisplayRole = Qt::DisplayRole,

        StateRole = Qt::UserRole,
        PidListRole,
        TabListRole,
    };

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &child) const;

public slots:
    void clearPressed();
    void startRefreshData() { m_processInfos->startRefreshData(); }
    void stopRefreshData() { m_processInfos->stopRefreshData(); }

private:
    QPointer<ProcessInfoManager> m_processInfos;
    QModelIndex m_pressedIndex;
};

#endif // PROCESSINFOMODEL_H
