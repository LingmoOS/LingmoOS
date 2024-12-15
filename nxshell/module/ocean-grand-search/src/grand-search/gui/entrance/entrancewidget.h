// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRANCEWIDGET_H
#define ENTRANCEWIDGET_H

#include "global/matcheditem.h"

#include <QFrame>
#include <QScopedPointer>

namespace GrandSearch {

class EntranceWidgetPrivate;
class EntranceWidget : public QFrame
{
    Q_OBJECT
public:
    explicit EntranceWidget(QWidget *parent = nullptr);
    ~EntranceWidget() override;

    void showLabelAppIcon(bool visible);
protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnections();

public slots:
    // 切换选择搜索结果时，应用图标发生改变
    void onAppIconChanged(const QString &searchGroupName, const MatchedItem &item);

signals:
    void searchTextChanged(const QString &txt);
    void sigSelectNextItem();
    void sigSelectPreviousItem();
    void sigHandleItem();
    void sigCloseWindow();

private:
    QScopedPointer<EntranceWidgetPrivate> d_p;
};

}

#endif // ENTRANCEWIDGET_H
