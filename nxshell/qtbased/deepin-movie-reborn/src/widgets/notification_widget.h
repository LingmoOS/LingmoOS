// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 *@file 这个文件是实现影院左上角信息弹窗
 */
#ifndef _DMR_NOTIFICATION_WIDGET_H
#define _DMR_NOTIFICATION_WIDGET_H 

#include <QtWidgets>

#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>
#include <DLabel>

#include "compositing_manager.h"

DWIDGET_USE_NAMESPACE

namespace dmr {
/**
 * @brief The NotificationWidget class
 * 这个类是影院左上角弹窗的类
 */
class NotificationWidget: public QFrame {
    Q_OBJECT
public:
    enum MessageAnchor {
        ANCHOR_NONE = 0,
        ANCHOR_BOTTOM,
        ANCHOR_NORTH_WEST
    };
    /**
     * @brief NotificationWidget 构造函数
     * @param parent 父窗口
     */
    explicit NotificationWidget(QWidget *parent = 0);
    /**
     * @brief setAnchor 设置信息弹窗位置
     * @param messageAnchor 位置
     */
    void setAnchor(MessageAnchor messageAnchor) { m_pAnchor = messageAnchor; }
    /**
     * @brief setAnchorDistance 同步控件高度
     * @param v 控件高度
     */
    void setAnchorDistance(int v) { m_nAnchorDist = v; }
    /**
     * @brief setAnchorPoint 设置控件位置
     * @param p 位置点
     */
    void setAnchorPoint(const QPoint& p) { m_anchorPoint = p; }

    void setWM(bool isWM) { m_bIsWM = isWM; }

public slots:
    /**
     * @brief popup 显示函数
     * @param msg 传入信息内容
     * @param flag 是否自动隐藏，默认为是
     */
    void popup(const QString& msg, bool flag = true);
    /**
     * @brief updateWithMessage 更新显示信息
     * @param newMsg 显示信息
     * @param flag 是否自动隐藏，默认为是
     */
    void updateWithMessage(const QString& newMsg, bool flag = true);
    /**
     * @brief syncPosition 同步控件位置
     */
    void syncPosition();
    /**
     * @brief syncPosition 同步控件位置
     * @param rect 控件的位置
     */
    void syncPosition(QRect rect);
protected:
    /**
     * @brief showEvent 重载显示事件函数
     * @param pShowEvent 显示事件
     */
    void showEvent(QShowEvent *pShowEvent) override;
    /**
     * @brief resizeEvent 重载大小变化事件函数
     * @param pResizeEvent 大小变化事件
     */
    void resizeEvent(QResizeEvent *pResizeEvent) override;
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    void paintEvent(QPaintEvent* pPaintEvent) override;

private:
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

private:
    QWidget *m_pMainWindow;       ///主窗口
    DLabel *m_pMsgLabel;          ///文本信息label控件
    QLabel *m_pIconLabel;         ///图标Label控件
    QTimer *m_pTimer;             ///消失定时器
    QFrame *m_pFrame;             ///
    QHBoxLayout *m_pMainLayout;   ///窗口主布局
    MessageAnchor m_pAnchor;      ///控件位置
    int m_nAnchorDist;            ///
    QPoint m_anchorPoint;         ///控件位置点
    bool m_bIsWheel;              ///
    bool m_bIsWM;
};

}

#endif /* ifndef _DMR_NOTIFICATION_WIDGET_H */
