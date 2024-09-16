// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHMODEWIDGET_H
#define SEARCHMODEWIDGET_H

#include "appgridview.h"
#include "appslistmodel.h"
#include "appitemdelegate.h"

#include <DIconButton>

#include <QWidget>
#include <QLabel>
#include <QBoxLayout>

DWIDGET_USE_NAMESPACE

class SearchModeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchModeWidget(QWidget *parent = Q_NULLPTR);
    ~SearchModeWidget() Q_DECL_OVERRIDE;

    void initUi();
    void initTitle();
    void initAppView();
    void initConnection();
    void setItemDelegate(AppItemDelegate *delegate);

    void setSearchModel(QSortFilterProxyModel *model);
    void updateTitleContent();
    void updateTitlePos(bool alignCenter);
    void addSpacerItem(QBoxLayout *layout = nullptr);
    void selectFirstItem() const;

    AppGridView *getNativeView() const;

signals:
    void connectViewEvent(AppGridView* pView);

private slots:
    void onLayoutChanged();

private:
    QWidget *m_nativeWidget;
    QWidget *m_outsideWidget;
    QWidget *m_emptyWidget;

    QLabel *m_outsideLabel;

    AppGridView *m_nativeView;
    AppGridView *m_outsideView;

    AppsListModel *m_nativeModel;
    AppsListModel *m_outsideModel;

    DIconButton *m_iconButton;
    QLabel *m_emptyText;
};


#endif
