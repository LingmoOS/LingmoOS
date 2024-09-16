// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <QListView>
/*!
 * \~chinese \class ListView
 * \~chinese \brief osd的视图类
 */
class ListView : public QListView
{
    Q_OBJECT
public:
    explicit ListView(QWidget *parent = 0);

Q_SIGNALS:
    void currentIndexChanged(const QModelIndex &index);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
};

#endif // LISTVIEW_H
