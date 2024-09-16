// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file 这个文件是播放进度条相关
 */
#ifndef _DMR_SLIDER_H
#define _DMR_SLIDER_H 

#include <QtWidgets>
#include <DSlider>
#include <DImageButton>
DWIDGET_USE_NAMESPACE
namespace dmr {
/**
 * @brief The DMRSlider class
 * 实现播放进度条
 */
class DMRSlider: public DSlider {
    Q_OBJECT
public:
    /**
     * @brief DMRSlider 构造函数
     * @param parent 父窗口
     */
    explicit DMRSlider(QWidget *parent = 0);
    /**
     * @brief ~DMRSlider 析构函数
     */
    virtual ~DMRSlider();
    /**
     * @brief setEnableIndication
     * @param on
     */
    void setEnableIndication(bool on);

    //workaround
    /**
     * @brief forceLeave 离开范围调用
     */
    void forceLeave();

signals:
    /**
     * @brief hoverChanged 悬停变更信号
     */
    void hoverChanged(int);
    /**
     * @brief leave 鼠标离开信号
     */
    void leave();
    /**
     * @brief enter 鼠标进入信号
     */
    void enter();
    /**
      * @brief 功能不支持信号
      */
    void sigUnsupported();
protected:
    /**
     * @brief onValueChanged 进度条值改变槽函数
     * @param v 值
     */
    void onValueChanged(const QVariant& v);
    /**
     * @brief onAnimationStopped 动画结束槽函数
     */
    void onAnimationStopped();

protected:
    /**
     * @brief mouseReleaseEvent 鼠标释放事件函数
     * @param pMouseEvent
     */
    void mouseReleaseEvent(QMouseEvent *pMouseEvent) override;
    /**
     * @brief mousePressEvent 鼠标按下事件函数
     * @param pMouseEvent 鼠标按下事件
     */
    void mousePressEvent(QMouseEvent *pMouseEvent) override;
    /**
     * @brief mouseMoveEvent 鼠标移动事件函数
     * @param pMouseEvent 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *pMouseEvent) override;
    /**
     * @brief leaveEvent 鼠标离开事件函数
     * @param pEvent 事件
     */
    void leaveEvent(QEvent *pEvent) override;
    /**
     * @brief enterEvent 鼠标进入事件函数
     * @param pEvent 事件
     */
    void enterEvent(QEvent *pEvent) override;
    /**
     * @brief wheelEvent 鼠标滚轮事件函数
     * @param pWheelEvent 鼠标滚轮事件
     */
    void wheelEvent(QWheelEvent *pWheelEvent) override;
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    void paintEvent(QPaintEvent *pPaintEvent) override;

    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

    bool event(QEvent* pEvent) override;

private:
    /**
     * @brief position2progress 像素点到进度条位置转换
     * @param p 像素点
     * @return 进度条位置
     */
    int position2progress(const QPoint& p);
    /**
     * @brief startAnimation
     * @param reverse
     */
    void startAnimation(bool reverse);

    bool m_bDown;                             ///鼠标是否按下标志位
    bool m_bIndicatorEnabled;                 ///进入后设置显示状态
    bool m_bShowIndicator;                    ///是否显示当前位置
    int m_nLastHoverValue;                    ///上次悬停的位置
    QPoint m_indicatorPos;                    ///鼠标悬停的位置
    QColor m_indicatorColor;                  ///鼠标悬停活动色
};

}

#endif /* ifndef _DMR_SLIDER_H */
