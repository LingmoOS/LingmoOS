// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SLIDEWIDGET_H
#define SLIDEWIDGET_H

#include <DWidget>
#include <DSpinner>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE
class DocSheet;
class QTimer;
class SlidePlayWidget;
class QPropertyAnimation;

/**
 * @brief The SlideWidget class
 * 幻灯片控件
 */
class SlideWidget : public DWidget
{
    Q_OBJECT
public:
    explicit SlideWidget(DocSheet *docsheet);

    ~SlideWidget() override;

public slots:
    /**
     * @brief setWidgetState
     * 设置控件状态
     * @param full
     */
    void setWidgetState(bool full);

    /**
     * @brief handleKeyPressEvent
     * 按键处理事件
     * @param sKey
     */
    void handleKeyPressEvent(const QString &sKey);

    /**
     * @brief onImageAniFinished
     * 单页幻灯片播放结束
     */
    void onImageAniFinished();

private:
    /**
     * @brief initControl
     * 初始化控件
     */
    void initControl();

    /**
     * @brief initImageControl
     * 初始化图片数据
     */
    void initImageControl();

    /**
     * @brief playImage
     * 播放幻灯片
     */
    void playImage();

    /**
     * @brief drawImage
     * 绘制幻灯片内容
     * @param srcImage
     * @return
     */
    QPixmap drawImage(const QPixmap &srcImage);

private slots:
    /**
     * @brief onParentDestroyed
     * 父窗口已销毁
     */
    void onParentDestroyed();

    /**
     * @brief onPreBtnClicked
     * 响应上一页按钮点击
     */
    void onPreBtnClicked();

    /**
     * @brief onPlayBtnClicked
     * 响应播放按钮点击
     */
    void onPlayBtnClicked();

    /**
     * @brief onNextBtnClicked
     * 响应下一页按钮点击
     */
    void onNextBtnClicked();

    /**
     * @brief onExitBtnClicked
     * 响应退出按钮点击
     */
    void onExitBtnClicked();

    /**
     * @brief onImagevalueChanged
     * 幻灯片播放过程变换
     * @param variant
     */
    void onImagevalueChanged(const QVariant &variant);

    /**
     * @brief onImageShowTimeOut
     * 单页幻灯片显示超时
     */
    void onImageShowTimeOut();

    /**
     * @brief onFetchImage
     * 获取指定页数图片
     * @param index
     */
    void onFetchImage(int index);

    /**
     * @brief onUpdatePageImage
     * 更新对应页码图片
     * @param pageIndex
     */
    void onUpdatePageImage(int pageIndex);

protected:
    /**
     * @brief paintEvent
     * 绘制事件
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief mouseMoveEvent
     * 鼠标移动事件
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief mousePressEvent
     * 鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    int m_curPageIndex = 0;
    bool m_canRestart = false;
    bool m_parentIsDestroyed = false;
    QPixmap m_lpixmap;
    QPixmap m_rpixmap;

    Qt::WindowStates m_nOldState = Qt::WindowNoState;
    DocSheet *m_docSheet;
    SlidePlayWidget *m_slidePlayWidget;
    DSpinner *m_loadSpinner;

    int m_offset = 0;
    int m_preIndex = 0;
    bool m_blefttoright = true; //从左向右移动
    QTimer *m_imageTimer;
    QPropertyAnimation *m_imageAnimation;
};

#endif // SLIDEWIDGET_H
