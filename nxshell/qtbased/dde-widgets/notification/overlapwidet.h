// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OverLapWidet_H
#define OverLapWidet_H

#include "bubbleitem.h"
#include "notification/constants.h"

#include <DWidget>

DWIDGET_USE_NAMESPACE

class NotifyListView;
class NotifyModel;

class HalfRoundedRectWidget : public AlphaWidget
{
    Q_OBJECT

public:
    explicit HalfRoundedRectWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class OverLapWidet : public DWidget
{
    Q_OBJECT

public:
    OverLapWidet(NotifyModel *model,EntityPtr ptr,QWidget * parent = nullptr);

public:
    void setParentView(NotifyListView *view);
    BubbleItem *getFaceItem() { return m_faceBubbleItem; }
    QList<QPointer<QWidget>> bubbleElements();
    void expandAppGroup();

private:
    void initOverlap();
    void hideOverlapBubble();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override; //鼠标点击时展开重叠的气泡
    void keyPressEvent(QKeyEvent *event) override;

private:
    EntityPtr m_entify;
    NotifyModel *m_model;
    NotifyListView *m_view;
    BubbleItem *m_faceBubbleItem;
};

#endif // OverLapWidet_H
