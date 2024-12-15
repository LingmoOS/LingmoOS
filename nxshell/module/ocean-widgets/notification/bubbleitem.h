// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
class CicleIconButton;

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

    void setHasFocus(bool focus);
protected:
    void paintEvent(QPaintEvent *event) override;

protected:
    int m_hoverAlpha = 0;
    int m_unHoverAlpha = 0;
    int m_topRedius = 0;
    int m_bottomRedius = 0;
    bool m_hasFocus = false;
};

class BubbleBase : public QWidget
{
    Q_OBJECT
public:
    explicit BubbleBase(QWidget *parent = nullptr, EntityPtr entity = nullptr);

    virtual void setParentModel(NotifyModel *model);
    virtual void setParentView(NotifyListView *view);
    virtual QList<QPointer<QWidget>> bubbleElements() const;
    void updateTabOrder();

public slots:
    void showSettingsMenu();
    void toggleAppTopping();
    void showNotificationModuleOfControlCenter();

protected:
    NotifyModel *m_model = nullptr;
    NotifyListView *m_view = nullptr;
    QString m_appName;

    CicleIconButton *m_settingBtn = nullptr;
    CicleIconButton *m_closeButton = nullptr;
};

/*!
 * \~chinese \class ItemWidget
 * \~chinese \brief 通知中心中的气泡窗口类
 */
class BubbleItem : public BubbleBase
{
    Q_OBJECT
public:
    BubbleItem(QWidget *parent = nullptr, EntityPtr entity = nullptr);
    ~BubbleItem() override;
    virtual void setParentModel(NotifyModel *model) override;
    virtual void setParentView(NotifyListView *view) override;
    const QPixmap converToPixmap(const QDBusArgument &value);
    void setAlpha(int alpha);
    QList<QPointer<QWidget>> bubbleElements() const override;
    int indexRow();
    EntityPtr getEntity() { return m_entity; }
    // 通知列表在显示之前会获取所有item的sizeHint，bubbleItem中的控件字体有一些是不一样的，大小也不同
    // 所以需要根据实际的情况去获取每个控件的在当前字体字号情况下的高度，求和组成BubbleItem的高度
    static int bubbleItemHeight();

Q_SIGNALS:
    void bubbleRemove();
    void focusStateChanged(bool focus);

public Q_SLOTS:
    void onFocusStateChanged(bool focus);
    void onCloseBubble();
    void onRefreshTime();
    void setOverlapWidget(bool isOverlap);
    bool isOverlapWidget() { return m_isOverlapWidget; }

protected:
    void mousePressEvent(QMouseEvent *event) override;       //获取当前鼠标点击的位置
    void mouseReleaseEvent(QMouseEvent *event) override;     //判断鼠标松开的位置是否与点击的位置相同,相同移除通知
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    bool realHasFocus() const;

private:
    void initUI();          //初始化UI界面
    void initContent();     //初始化信号槽连接
    void refreshTheme();    //刷新主题

private:
    EntityPtr m_entity;

    //controls
    AlphaWidget *m_bgWidget = nullptr;
    AlphaWidget *m_titleWidget = nullptr;
    AlphaWidget *m_bodyWidget = nullptr;
    DLabel *m_appNameLabel = nullptr;
    AppBodyLabel *m_appTimeLabel = nullptr;

    AppIcon *m_icon = nullptr;
    AppBody *m_body = nullptr;
    ActionButton *m_actionButton = nullptr;
    QWidget *m_currentElement = nullptr;

    bool m_showContent = true;
    QString m_defaultAction;
    QPoint m_pressPoint;
    bool m_isOverlapWidget = false;
    QString m_actionId;
};

#endif // BUBBLEITEM_H
