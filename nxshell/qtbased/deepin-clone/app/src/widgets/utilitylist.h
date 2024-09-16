// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef UTILITYLIST_H
#define UTILITYLIST_H

#include <QListWidget>

class UtilityListItem;
class UtilityList : public QListWidget
{
    Q_OBJECT
public:
    explicit UtilityList(QWidget *parent = 0);

    void addItem(UtilityListItem *item);
    void addItem(const QIcon &icon, const QString &title, const QString &message, const QSize &iconSize = QSize());

protected:
    void showEvent(QShowEvent *e) Q_DECL_OVERRIDE;
};

#endif // UTILITYLIST_H
