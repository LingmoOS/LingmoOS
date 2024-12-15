// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NotifyCenterWidget_H
#define NotifyCenterWidget_H

#include "notifywidget.h"
#include "notification/constants.h"

#include <DBlurEffectWidget>
#include <DLabel>
#include <DWindowManagerHelper>
#include <DRegionMonitor>

#include <QTime>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class CicleIconButton;
class TransparentButton;
class AbstractPersistence;
class QVariantAnimation;
class QPropertyAnimation;
class QSequentialAnimationGroup;
class IconButton;
class QPushButton;
class QStackedLayout;
/*!
 * \~chinese \class NotifyCenterWidget
 * \~chinese \brief 继承于DBlurEffectWidget,DBlurEffectWidget是DTK库中的类,继承自QWidget.
 * \~chinese 通知中心主窗口类
 */
class NotifyCenterWidget : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit NotifyCenterWidget(AbstractPersistence *database, QWidget* parent = nullptr);

private:
    void initUI();              //初始化主界面
    void initConnections();     //初始化信号槽连接
    void refreshTheme();        //系统主题改变时,刷新主题

private Q_SLOTS:
    void CompositeChanged();   //用来设置是否开启窗口特效
    void updateDisplayOfRemainingNotification();
    void updateTabFocus();
    void expandNotificationFolding();
    void expandNotificationFoldingImpl(const bool refreshData);
    void collapesNotificationFolding();
    void collapesNotificationFoldingImpl(const bool refreshData);
    void showSettingMenu();
    void showNotificationModuleOfControlCenter();
    void updateClearButtonVisible();
    void updateToggleNotificationFoldingButtonVisible();

private:
    inline bool hasAppNotification() const;

    QWidget *m_headWidget;
    NotifyWidget *m_notifyWidget;
    DLabel *title_label = nullptr;
    CicleIconButton *m_clearButton = nullptr;
    TransparentButton *m_expandRemaining = nullptr;
    DLabel *m_noNotifyLabel = nullptr;
    QStackedLayout *m_bottomTipLayout = nullptr;
    CicleIconButton *m_settingBtn = nullptr;
    CicleIconButton *m_toggleNotificationFolding = nullptr;
    bool m_isCollapesNotificationFolding = true;
    QRect m_notifyRect;
    QRect m_dockRect;
    DWindowManagerHelper *m_wmHelper;
    bool m_hasComposite = false;
    QTimer *m_refreshTimer = nullptr;
    QTime m_tickTime;
    qreal m_scale = 1;
};

#endif // NotifyCenterWidget_H
