/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Lingmo/Theme>
#include <LingmoQuick/PopupLingmoWindow>
#include <LingmoQuick/SharedQmlEngine>

#include "panelview.h"

#include <QJSValue>
#include <QPointer>
#include <QQmlListProperty>
#include <QQuickItem>
#include <QQuickView>
#include <QStandardItemModel>
#include <lingmoquick/lingmowindow.h>
#include <qevent.h>

class PanelView;

namespace Lingmo
{
class Containment;
}

class PanelRulerView : public LingmoQuick::LingmoWindow
{
    Q_OBJECT

public:
    PanelRulerView(Lingmo::Containment *interface, PanelView *panelView, PanelConfigView *mainConfigView);
    ~PanelRulerView() override;

    void syncPanelLocation();

protected:
    void showEvent(QShowEvent *ev) override;
    void focusOutEvent(QFocusEvent *ev) override;

private:
    Lingmo::Containment *m_containment;
    PanelView *m_panelView;
    PanelConfigView *m_mainConfigView;
    LayerShellQt::Window *m_layerWindow = nullptr;
};

class PanelConfigView : public LingmoQuick::PopupLingmoWindow
{
    Q_OBJECT
    Q_PROPERTY(PanelRulerView *panelRulerView READ panelRulerView CONSTANT)
    Q_PROPERTY(QRect geometry READ geometry NOTIFY geometryChanged)

public:
    PanelConfigView(Lingmo::Containment *interface, PanelView *panelView);
    ~PanelConfigView() override;

    void init();

    PanelRulerView *panelRulerView();
    QRect geometry() const;

    Q_INVOKABLE QScreen *screenFromWindow(QWindow *window) const;

    friend class PanelRulerView;

protected:
    void keyPressEvent(QKeyEvent *ev) override;
    void showEvent(QShowEvent *ev) override;
    void hideEvent(QHideEvent *ev) override;
    void focusInEvent(QFocusEvent *ev) override;
    void moveEvent(QMoveEvent *ev) override;
    void resizeEvent(QResizeEvent *ev) override;

public Q_SLOTS:
    void showAddWidgetDialog();
    void addPanelSpacer();

protected Q_SLOTS:
    void syncGeometry();

Q_SIGNALS:
    void geometryChanged(const QRect &geometry);

private:
    void focusVisibilityCheck(QWindow *focusWindow);

    Lingmo::Containment *m_containment;
    QPointer<PanelView> m_panelView;
    QPointer<QWindow> m_focusWindow;
    std::unique_ptr<PanelRulerView> m_panelRulerView;
    KSvg::FrameSvg::EnabledBorders m_enabledBorders = KSvg::FrameSvg::AllBorders;
    Lingmo::Theme m_theme;
    QTimer m_screenSyncTimer;
    std::unique_ptr<LingmoQuick::SharedQmlEngine> m_sharedQmlEngine;
};
