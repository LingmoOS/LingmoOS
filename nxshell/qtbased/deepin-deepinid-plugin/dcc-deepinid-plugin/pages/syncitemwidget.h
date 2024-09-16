// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCITEMWIDGET_H
#define SYNCITEMWIDGET_H

#include <QWidget>
#include <dtkwidget_global.h>
#include <QStandardItemModel>

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DLabel;
class DLineEdit;
class DToolButton;
class DGroupBox;
class DFrame;
class DTipLabel;
class DDialog;
class DListView;
class DToolTip;
class DHorizontalLine;
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class SyncItemWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SyncItemWidget(QWidget *parent = nullptr);

    void SetEnable(bool bEnable);

    void UpdateConfigIndex(const QModelIndex &index);

    void UpdateItemIndex(const QModelIndex &index);

    void SetViewModel(QStandardItemModel *configModel, QStandardItemModel *itemModel);
Q_SIGNALS:
    void configClicked(const QModelIndex &index);

    void itemClicked(const QModelIndex &index);
private:
    void InitUI();
private:
    DListView *m_syncConfigView;
    DListView *m_syncItemView;
};

#endif // SYNCITEMWIDGET_H
