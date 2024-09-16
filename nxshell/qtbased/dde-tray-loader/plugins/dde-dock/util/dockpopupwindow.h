// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCKPOPUPWINDOW_H
#define DOCKPOPUPWINDOW_H

#include <DRegionMonitor>
#include <DWindowManagerHelper>
#include <DPlatformWindowHandle>

#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
#include <DBlurEffectWithBorderWidget>
#else
#include <DBlurEffectWidget>
#endif

#include <QTimer>
#include <QPointer>

#include <com_deepin_dde_daemon_dock.h>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

using PopupDBusDock = com::deepin::dde::daemon::Dock;

#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
class DockPopupWindow : public DBlurEffectWithBorderWidget
#else
class DockPopupWindow : public DBlurEffectWidget
#endif
{
    Q_OBJECT

public:
    explicit DockPopupWindow(QWidget *parent = 0, bool noFocus = true, bool tellWmDock = false);
    ~DockPopupWindow();
    bool isModel() const;
    void setContent(QWidget *content);
    QWidget *getContent() const { return m_lastWidget; }
    void blockMousePressEvent(bool block);
    void setPopupRadius(int radius);
    void setEnableSystemMove(bool enable);

public slots:
    void show(const QPoint &pos, const bool model = false);
    void show(const int x, const int y);
    void hide();
    void updateBackgroundColor();
    void updateWindowOpacity();

signals:
    void accept() const;
    // 在把专业版的仓库降级到debian的stable时, dock出现了一个奇怪的问题:
    // 在plugins/tray/system-trays/systemtrayitem.cpp中的showPopupWindow函数中
    // 无法连接到上面这个信号: "accept", qt给出一个运行时警告提示找不到信号
    // 目前的解决方案就是在下面增加了这个信号
    void unusedSignal();

protected:
    void showEvent(QShowEvent *e) override;
    void enterEvent(QEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;
    QPoint scalePoint(QPoint point);

private slots:
    void onGlobMousePress(const QPoint &mousePos, const int flag);
    void compositeChanged();
    void ensureRaised();

private:
    bool m_model;
    bool m_blockPressEvent;
    QPoint m_lastPoint;
    DRegionMonitor *m_regionInter;
    QTimer *m_blockPressEventTimer;

    QPointer<QWidget> m_lastWidget;
    DPlatformWindowHandle m_platformWindowHandle;
    QColor m_innerBorderColor;

    PopupDBusDock *m_dockInter;
};

#endif // DOCKPOPUPWINDOW_H
