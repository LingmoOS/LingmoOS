// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCROLLSHOTTIP_H
#define SCROLLSHOTTIP_H

#include <DWidget>
#include <DCommandLinkButton>
#include <DLabel>
#include <DIconButton>

#include <QtDBus/QtDBus>
#include <QHBoxLayout>
#include <QTimer>

DWIDGET_USE_NAMESPACE

enum TipType {
    StartScrollShotTip,  //开始滚动截图前的提示
    ErrorScrollShotTip,  //滚动截图出现错误的提示
    EndScrollShotTip,     //滚动截图到底部出现的提示
    QuickScrollShotTip,    //滚动速度过快出现的提示
    MaxLengthScrollShotTip,  //滚动截图拼接已到达最大长度
    InvalidAreaShotTip      //无效区域,点击调整捕捉区域
};

/**
 * @brief 滚动截图的提示
 */
class ScrollShotTip : public DWidget
{

public:
    static const int NUMBER_PADDING_Y;
    static const int TIP_HEIGHT;

    explicit ScrollShotTip(DWidget *parent = 0);
    ~ScrollShotTip();

    Q_OBJECT

public:

    /**
     * @brief 根据提示的类型选取相应的提示方法
     * @param tipText
     */
    void showTip(TipType tipType);

    /**
     * @brief 获取当前的提示类型
     * @return
     */
    TipType getTipType();

    /**
     * @brief 设置滚动截图当前屏幕的背景
     * @param backgroundPixmap
     */
    void setBackgroundPixmap(QPixmap &backgroundPixmap);

signals:
    /**
     * @brief 打开截图录屏帮助文档并定位到滚动截图
     */
    void openScrollShotHelp();

    /**
     * @brief 调整捕捉区域
     */
    void adjustCaptureArea();

protected:
    /**
     * @brief 开始滚动截图前的提示
     */
    void showStartScrollShotTip();

    /**
     * @brief 滚动截图出现错误的提示
     */
    void showErrorScrollShotTip();

    /**
     * @brief 滚动截图到底部出现的提示
     */
    void showEndScrollShotTip();

    /**
     * @brief 图像拼接长度限制
     */
    void showMaxScrollShotTip();

    /**
     * @brief 显示提示滚动速度过快
     */
    void showQuickScrollShotTip();

    /**
     * @brief 显示无效区域,调整捕捉区域提示
     */
    void showInvalidAreaShotTip();

    void paintEvent(QPaintEvent *event);

    /**
     * @brief getTooltipBackground 先获取模糊背景图
     * @return
     */
    QPixmap getTooltipBackground();
    /**
     * @brief paintRect  画模糊背景图
     * @param painter
     * @param blurPixmap  矩形框位置的模糊图形
     */
    void paintRect(QPainter &painter, QPixmap &blurPixmap, int radius);

    /**
     * @brief 画提示的背景
     * @param painter
     * @param rect
     * @param textColor
     * @param opacity
     */
    void drawTooltipBackground(QPainter &painter, QRect rect, QString textColor, qreal opacity, int radius);


private:
    //警告图片
    DIconButton *m_warmingIconButton;
    //提示的内容
    QString m_tipText;

    DLabel *m_tipTextLable;
    /**
     * @brief 背景图片
     */
    QPixmap m_backgroundPixmap;


    /**
     * @brief 提示的类型
     * 参见 TipType 定义
     */
    TipType m_tipType = TipType::StartScrollShotTip;

    /**
     * @brief 帮助文字按钮
     */
    DCommandLinkButton *m_scrollShotHelp;

    /**
     * @brief 调整捕捉区域文字按钮
     */
    DCommandLinkButton *m_scrollShotAdjust;
};

#endif // SCROLLSHOTTIP_H
