// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUBBLETITLEWIDGET_H
#define BUBBLETITLEWIDGET_H
#include "notification/constants.h"
#include "notifymodel.h"
#include "bubbleitem.h"
#include <DWidget>
#include <DIconButton>
#include <DLabel>

DWIDGET_USE_NAMESPACE

class NotifyListView;

class BubbleTitleWidget : public DWidget
{
    Q_OBJECT
public:
    BubbleTitleWidget(NotifyModel *model = nullptr, EntityPtr entity = nullptr, QWidget *parent = nullptr);
    void setParentView(NotifyListView *view);

public:
    void setIndexRow(int row);
    int indexRow() { return m_indexRow; }
    QList<QPointer<QWidget>> bubbleElements();
    static int bubbleTitleWidgetHeight();

protected:
    void enterEvent(QEvent *event) override;        //鼠标移动到窗口上,显示气泡组的关闭按钮
    void leaveEvent(QEvent *event) override;        //鼠标移出窗口,气泡组的关闭按钮不显示
    void focusInEvent(QFocusEvent *event) override;          //当焦点移入或移出时背景发生变化
    void focusOutEvent(QFocusEvent *event) override;

private:
    DLabel *m_titleLabel;
    DIconButton *m_closeButton;
    NotifyModel *m_model;
    int m_indexRow = 0;
    NotifyListView *m_view = nullptr;
};

#endif // BUBBLETITLEWIDGET_H
