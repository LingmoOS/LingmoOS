// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORDERWIDGET_H
#define ORDERWIDGET_H

#include <DToolButton>

#include "attributewidget.h"

DWIDGET_USE_NAMESPACE

class Page;
class DrawBoard;
class PageScene;

class OrderWidget : public AttributeWgt
{
    Q_OBJECT
public:
    OrderWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);
public slots:
    void updateAlignButtonStatus();
    void updateLayerButtonStatus();
    void updateButtonsStatus();
private:
    void initUi();
    void initConnect();
    DToolButton *createIconButton(const QString &icon, const QString &toolTip, const QString &accesibleName);
    void doAlignment(Qt::AlignmentFlag align);
private:
    DToolButton *m_oneLayerUp = nullptr;      //向上一层
    DToolButton *m_oneLayerDown = nullptr;    //向下一层
    DToolButton *m_bringToFront = nullptr;    //置于最顶层
    DToolButton *m_sendToback = nullptr;      //置于最底层

    DToolButton *m_LeftAlign = nullptr;     //左对齐
    DToolButton *m_HCenterAlign = nullptr;  //水平居中对齐
    DToolButton *m_RightAlign = nullptr;    //右对齐
    DToolButton *m_TopAlign = nullptr;      //顶对齐
    DToolButton *m_VCenterAlign = nullptr;  //垂直居中对齐
    DToolButton *m_BottomAlign = nullptr;   //底对齐

    DToolButton *m_HEqulSpaceAlign = nullptr; //水平等间距对齐
    DToolButton *m_VEqulSpaceAlign = nullptr; //垂直等间距对齐
    DrawBoard   *m_drawBoard = nullptr;
    Page        *m_currentPage = nullptr;
    PageScene   *m_currentScene = nullptr;
};

#endif // ORDERWIDGET_H
