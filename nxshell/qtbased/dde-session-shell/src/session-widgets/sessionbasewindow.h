// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSIONBASEWINDOW_H
#define SESSIONBASEWINDOW_H
#include "fakewindowlayer.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QPointer>
#include <QSpacerItem>

/**
 * @brief The SessionBaseWindow class 提供快捷的内容设置接口的类，方便使用
 */
class SessionBaseWindow : public QFrame
{
    Q_OBJECT
public:
    explicit SessionBaseWindow(QWidget *parent = nullptr);
    virtual void setLeftBottomWidget(QWidget *const widget) final;
    virtual void setCenterBottomWidget(QWidget *const widget) final;
    virtual void setRightBottomWidget(QWidget *const widget) final;
    virtual void setCenterContent(QWidget *const widget, Qt::Alignment align = Qt::AlignCenter, int spacerHeight = 0) final;
    virtual void setTopWidget(QWidget *const widget) final;

    inline QPointer<QWidget> centerWidget() { return m_centerWidget; }
    QSize getCenterContentSize();
    void changeCenterSpaceSize(int w, int h);
    void setTopFrameVisible(bool visible);
    void setBottomFrameVisible(bool visible);
    void showPopup(QPoint globalPos, QWidget *popup);
    inline void showPopup(int globalX, int globalY, QWidget *popup) { showPopup(QPoint(globalX, globalY), popup); }
    void hidePopup();
    void togglePopup(QPoint globalPos, QWidget *popup);

private:
    void initUI();
    int autoScaledSize(const int height) const;
    QMargins getMainLayoutMargins() const;

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    QFrame *m_TopFrame;                 // 上布局容器(按照比例计算，其高度固定为当前屏幕高度的132/1080)
    QFrame *m_centerFrame;              // 中间布局容器(高度 = 屏幕高度 - 上下两个容器的高度)
    QFrame *m_bottomFrame;              // 下布局容器(按照比例计算，其高度固定为当前屏幕高度的132/1080)

    // An overlay layer used to emulate window like popups.
    // When this layer is visible, it will catch all events for its parent SessionBaseWindow.
    FakeWindowLayer *m_fakeWindowLayer;

    QVBoxLayout *m_mainLayout;          // 主布局(垂直布局，将整个屏幕分为上、中、下三部分,其上下的margin为当前高度的33/1080)
    QHBoxLayout *m_topLayout;           // 上布局(水平布局)
    QVBoxLayout *m_centerLayout;        // 中间布局

    // 以3:2:3的比例划分下布局区域
    QHBoxLayout *m_leftBottomLayout;    // 下布局-左侧布局
    QHBoxLayout *m_centerBottomLayout;  // 下布局-中间布局
    QHBoxLayout *m_rightBottomLayout;   // 下布局-右侧布局

    QWidget *m_topWidget;               // 上布局中添加的界面
    QPointer<QWidget> m_centerWidget;   // 中间布局内容

    QWidget *m_leftBottomWidget;        // 下布局-左侧布局内容
    QWidget *m_centerBottomWidget;      // 下布局-中间布局内容
    QWidget *m_rightBottomWidget;       // 下布局-右侧布局内容

    QSpacerItem *m_centerSpacerItem;
};

#endif // SESSIONBASEWINDOW_H
