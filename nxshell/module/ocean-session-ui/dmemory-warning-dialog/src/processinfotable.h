// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSINFOTABLE_H
#define PROCESSINFOTABLE_H

#include <QTreeView>

class ProcessInfoView : public QTreeView
{
    Q_OBJECT

public:
    explicit ProcessInfoView(QWidget *parent = nullptr);

    void setModel(QAbstractItemModel *model);

protected:
    void mouseReleaseEvent(QMouseEvent *e);
    void drawRow(QPainter *painter,
                 const QStyleOptionViewItem &options,
                 const QModelIndex &index) const;
};

#endif // PROCESSINFOTABLE_H
