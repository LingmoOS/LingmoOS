// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOW_H
#define WINDOW_H

#include "itemdelegate.h"
#include "sortproxy.h"
#include <DMenu>
#include <DMenuBar>
#include <DScrollArea>
#include <DWidget>
#include <QHBoxLayout>
#include <QListView>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class Window : public DWidget
{
    Q_OBJECT
public:
    explicit Window(DWidget *parent = nullptr);
    ~Window();

private:
    SortProxy *m_sortProxy = nullptr;
    QStandardItemModel *m_model = nullptr;

    void initItems();

private Q_SLOTS:
    void deviceMountAdd(const QString &devPath, const QString &mountPoint);
    void deviceMountRemove(const QString &devPath);
    void deviceAdd(const QString &devPath);
    void deviceRemove(const QString &devPath);
};

#endif   // WINDOW_H
