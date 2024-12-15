// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "colortoolwidget.h"
#include "shottoolwidget.h"
#include "toolbutton.h"

#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>
#include <DVerticalLine>

#include <QPainter>
#include <QEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE
class MainWindow;
class SideBarWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit SideBarWidget(MainWindow *pmainwindow, DWidget *parent = nullptr);
    ~SideBarWidget() Q_DECL_OVERRIDE;

    void changeShotToolWidget(const QString &func);
    /**
     * @brief getSideBarWidth:获取二级工具栏的宽度
     * @param func:选择的形状
     * @return 二级工具栏宽度
     */
    int getSideBarWidth(const QString &func);
signals:
    void changeArrowAndLineEvent(int line);
    void closeSideBar();
protected:
    /**
     * @brief 初始化侧边栏界面
     */
    void initSideBarWidget();

    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    DVerticalLine *m_seperator;
    ColorToolWidget *m_colorTool;
    ShotToolWidget *m_shotTool;
    bool  m_expanded;
    MainWindow *m_pMainWindow = nullptr;
};

class SideBar : public DLabel
{
    Q_OBJECT
public:
    explicit SideBar(DWidget *parent = nullptr);
    ~SideBar() Q_DECL_OVERRIDE;

    void changeShotToolFunc(const QString &func);

    /**
     * @brief getSideBarWidth:获取二级工具栏的宽度
     * @param func:选择的形状
     * @return 二级工具栏宽度
     */
    int getSideBarWidth(const QString &func);

    /**
     * @brief isDraged 工具栏是否已经被拖动
     * @return
     */
    bool isDraged();

    /**
     * @brief isPressed 是否在此工具栏按下鼠标左键
     * @return
     */
    bool isPressed();
signals:
    void heightChanged();
    void buttonChecked(QString shape);
    void updateColor(QColor color);
    void requestSaveScreenshot();
    void shapePressed(QString tool);
    void closed();
    void changeArrowAndLineToMain(int line);
public slots:
    //bool isButtonChecked();
    //void setExpand(bool expand, QString shapeType);
    void showAt(QPoint pos);
    void initSideBar(MainWindow *pmainWindow);
    //void setColorFunc(const QString &func);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    SideBarWidget *m_sidebarWidget;

    bool m_expanded;

    /**
     * @brief 工具栏是否已经被拖动
     */
    bool m_isDrag = false;

    /**
     * @brief 鼠标左键是否按下
     */
    bool m_isPress = false;

    QPoint m_mouseStartPoint;
    QPoint m_windowStartPoint;
    MainWindow *m_pMainWindow = nullptr;

};

#endif // SIDEBAR_H
