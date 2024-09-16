// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "subtoolbar.h"
#include "subtoolwidget.h"

#include <DLabel>
#include <DBlurEffectWidget>
#include <DImageButton>
#include <DFloatingWidget>
#include <DIconButton>

#include <QPainter>
#include <QEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class MainWindow;
enum SaveAction : unsigned int;
class ToolBarWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit ToolBarWidget(MainWindow *pMainwindow, DWidget *parent = nullptr);
    ~ToolBarWidget() Q_DECL_OVERRIDE;

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled(const bool state);
    /**
     * @brief 设置贴图的激活状态
     */
    void setPinScreenshotsEnable(const bool &state);

    void setOcrScreenshotsEnable(const bool &state);

    void setButEnableOnLockScreen(const bool &state);
    /**
     * @brief 根据当前传入的按钮形状获取当前按钮坐标的x值
     * @param func:当前按钮的形状（目前：矩形、圆形、直线等）
     * @return 当前按钮坐标的x值
     */
    int getFuncSubToolX(QString &func);
signals:
    void buttonChecked(QString shapeType);
    void closed();
    /**
     * @brief 切换截图功能或者录屏功能的信号
     * @param shapeType : "record" or "shot"
     */
    void changeFunctionSignal(QString shapeType);
    void keyBoardCheckedSignal(bool checked);
public slots:
    void setRecordLaunchFromMain(const unsigned int funType);
    void setRecordButtonDisable();
    void setVideoInitFromMain();
    void shapeClickedFromBar(QString shape);
    void setCameraDeviceEnable(bool status);

private:
    DLabel *m_hSeparatorLine;
    /**
     * @brief 截图录屏工具栏子工具栏
     */
    SubToolWidget *m_subTool;
    /**
     * @brief 截图录屏工具栏关闭按钮
     */
    ToolButton *m_closeButton;
    /**
     * @brief 截图录屏工具栏确认按钮
     */
    ToolButton *m_confirmButton;
};

class ToolBar : public DLabel
{
    Q_OBJECT
public:
    explicit ToolBar(DWidget *parent = nullptr);
    ~ToolBar() Q_DECL_OVERRIDE;
    //public接口非slots
    void initToolBar(MainWindow *pmainWindow);

    /**
     * @brief 设置禁止滚动截图
     */
    void setScrollShotDisabled(const bool state);

    /**
     * @brief 设置贴图的激活状态
     */
    void setPinScreenshotsEnable(const bool &state);

    void setOcrScreenshotsEnable(const bool &state);

    void setButEnableOnLockScreen(const bool &state);

    /**
     * @brief 根据当前传入的按钮形状获取当前按钮坐标的x值
     * @param func:当前按钮的形状（目前：矩形、圆形、直线等）
     * @return 当前按钮坐标的x值
     */
    int getFuncSubToolX(QString &func);

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
    void buttonChecked(QString shape);
    void currentFunctionToMain(QString shapeType);
    void shotToolChangedToMain(const QString &func);
public slots:
    void showAt(QPoint pos);
    /**
     * @brief 切换截图功能或者录屏功能
     * @param shapeType : "record" or "shot"
     */
    void currentFunctionMode(QString shapeType);
    void setRecordButtonDisable();
    void setRecordLaunchMode(const unsigned int funType);
    void setVideoButtonInit();
    void shapeClickedFromMain(QString shape);
    void setCameraDeviceEnable(bool status);
protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    ToolBarWidget *m_toolbarWidget;

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
#endif // TOOLBAR_H
