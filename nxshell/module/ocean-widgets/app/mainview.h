// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "global.h"
#include "widgetmanager.h"
#include "instancemodel.h"

#include <DBlurEffectWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <dflowlayout.h>

WIDGETS_FRAME_BEGIN_NAMESPACE
class WidgetStore;
class EditModePanel;
class DisplayModePanel;
class AnimationViewContainer;
class GeometryHandler;
class Appearancehandler;
DWIDGET_USE_NAMESPACE

class MainView : public QWidget
{
    Q_OBJECT

public:
    explicit MainView (WidgetManager *manager, QWidget *parent = nullptr);
    virtual ~MainView() override;

    enum Mode{
        Display,
        Edit
    };

    void init();

    Mode displayMode() const;

public Q_SLOTS:
    void showView();
    void hideView();

    void updateGeometry(const QRect &rect);

    void switchToEditMode();
    void switchToDisplayMode();

    void removePlugin(const PluginId &pluginId);
    void addPlugin(const PluginPath &pluginPath);
Q_SIGNALS:
    void displayModeChanged();

private:
    int expectedWidth() const;
private Q_SLOTS:
    void refreshShownView();
private:
    WidgetManager *m_manager = nullptr;
    WidgetStore *m_storeView;
    EditModePanel *m_editModeView;
    DisplayModePanel *m_displayModeView;
    InstanceModel *m_instanceModel = nullptr;
    QHBoxLayout *m_layout = nullptr;
    Mode m_mode;

    AnimationViewContainer *m_animationContainer;
    GeometryHandler *m_geometryHandler;
    Appearancehandler *m_appearancehandler = nullptr;

    QTimer *m_trickTimer = nullptr;

};
WIDGETS_FRAME_END_NAMESPACE
