// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEFTVIEWSORTFILTER_H
#define LEFTVIEWSORTFILTER_H
#include <QSortFilterProxyModel>
#include <QList>

struct VNoteFolder;
//记事本排序过滤
class LeftViewSortFilter : public QSortFilterProxyModel
{
public:
    explicit LeftViewSortFilter(QObject *parent = nullptr);
    //设置不显示项
    void setBlackFolders(const QList<VNoteFolder *> &folders);

protected:
    //处理排序
    virtual bool lessThan(
        const QModelIndex &source_left,
        const QModelIndex &source_right) const override;
    //数据过滤
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QList<VNoteFolder *> m_blackFolders;
};

#endif // LEFTVIEWSORTFILTER_H
