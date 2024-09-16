// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUBBLEITEM_H
#define BUBBLEITEM_H

#include <DWidget>
#include <DLabel>

#include <QDBusArgument>

#include "notification/constants.h"

class NotificationEntity;
class NotifyModel;
class AppIcon;
class AppBody;
class Button;
class ActionButton;
class IconButton;
class NotifyModel;
class NotifyListView;
class AppBodyLabel;

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
DWIDGET_END_NAMESPACE

DWIDGET_USE_NAMESPACE
/*!
 * \~chinese \class AlphaWidget
 * \~chinese \brief 绘制一个无边框,带有透明度的窗口
 */
class AlphaWidget : public DWidget
{
    Q_OBJECT

public:
    explicit AlphaWidget(QWidget *parent = nullptr);

    //设置和获取窗口属性的接口
    void setAlpha(int alpha) { m_hoverAlpha = alpha; m_unHoverAlpha = alpha; update(); }
    void setRadius(int top, int bottom) { m_topRedius = top; m_bottomRedius = bottom; }

    int hoverAlpha() {return m_hoverAlpha;}
    void setHoverAlpha(int alpha) {m_hoverAlpha = alpha; update();}

    int unHoverAlpha() {return m_unHoverAlpha;}
    void setUnHoverAlpha(int alpha) {m_unHoverAlpha = alpha; update();}

    void setHasFocus(bool focus) { m_hasFocus = focus; }
protected:
    void paintEvent(QPaintEvent *event) override;

protected:
    int m_hoverAlpha = 0;
    int m_unHoverAlpha = 0;
    int m_topRedius = 0;
    int m_bottomRedius = 0;
    bool m_hasFocus = false;
};

/*!
 * \~chinese \class ItemWidget
 * \~chinese \brief 通知中心中的气泡窗口类
 */
class BubbleItem : public QWidget
{
    Q_OBJECT
public:
    BubbleItem(QWidget *parent = nullptr, EntityPtr entity = nullptr);
    ~BubbleItem() override;
    void setParentModel(NotifyModel *model);
    void setParentView(NotifyListView *view);
    const QPixmap converToPixmap(const QDBusArgument &value);
    void setAlpha(int alpha);
    QList<QPointer<QWidget>> bubbleElements();
    int indexRow();
    void setHasFocus(bool focus);
    EntityPtr getEntity() { return m_entity; }
    // 通知列表在显示之前会获取所有item的sizeHint，bubbleItem中的控件字体有一些是不一样的，大小也不同
    // 所以需要根据实际的情况去获取每个控件的在当前字体字号情况下的高度，求和组成BubbleItem的高度
    static int bubbleItemHeight();

Q_SIGNALS:
    void havorStateChanged(bool);
    void bubbleRemove();

public Q_SLOTS:
    void onHavorStateChanged(bool hover);
    void onCloseBubble();
    void onRefreshTime();
    void setOverlapWidget(bool isOverlap);
    bool isOverlapWidget() { return m_isOverlapWidget; }

protected:
    void mousePressEvent(QMouseEvent *event) override;       //获取当前鼠标点击的位置
    void mouseReleaseEvent(QMouseEvent *event) override;     //判断鼠标松开的位置是否与点击的位置相同,相同移除通知
    void keyPressEvent(QKeyEvent *event) override;
    void enterEvent(QEvent *event) override;                 //鼠标移动到通知窗口上,隐藏时间显示关闭按钮
    void leaveEvent(QEvent *event) override;                 //鼠标移出通知窗口,隐藏关闭按钮显示时间
    void focusInEvent(QFocusEvent *event) override;          //当焦点移入或移出时背景发生变化
    void focusOutEvent(QFocusEvent *event) override;

private:
    void initUI();          //初始化UI界面
    void initContent();     //初始化信号槽连接
    void refreshTheme();    //刷新主题

private:
    EntityPtr m_entity;
    NotifyModel *m_model = nullptr;
    NotifyListView *m_view = nullptr;

    //controls
    AlphaWidget *m_bgWidget = nullptr;
    AlphaWidget *m_titleWidget = nullptr;
    AlphaWidget *m_bodyWidget = nullptr;
    DLabel *m_appNameLabel = nullptr;
    AppBodyLabel *m_appTimeLabel = nullptr;

    AppIcon *m_icon = nullptr;
    AppBody *m_body = nullptr;
    ActionButton *m_actionButton = nullptr;
    DIconButton *m_closeButton = nullptr;
    QWidget *m_currentElement = nullptr;

    bool m_showContent = true;
    QString m_defaultAction;
    QPoint m_pressPoint;
    bool m_isOverlapWidget = false;
    QString m_actionId;
};

#endif // BUBBLEITEM_H
