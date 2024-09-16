// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "panel.h"
#include "private/panel_p.h"
#include "containmentitem.h"

#include <QLoggingCategory>

#include <QDir>
#include <QQmlContext>
#include <DIconTheme>

DS_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(dsLog)

DPanel::DPanel(QObject *parent)
    : DContainment(*new DPanelPrivate(this), parent)
{
    QObject::connect(this, &DPanel::rootObjectChanged, this, [this]() {

        D_D(DPanel);
        d->ensurePopupWindow();
        d->ensureToolTipWindow();
        d->ensureMenuWindow();
    });
}

DPanel::~DPanel()
{

}

QQuickWindow *DPanel::window() const
{
    D_DC(DPanel);
    return qobject_cast<QQuickWindow *>(d->m_rootObject);
}

bool DPanel::load()
{
    D_D(DPanel);
    return DContainment::load();
}

bool DPanel::init()
{
    D_D(DPanel);
    d->initDciSearchPaths();

    return DContainment::init();
}

DPanel *DPanel::qmlAttachedProperties(QObject *object)
{
    auto applet = qobject_cast<DApplet *>(DContainmentItem::qmlAttachedProperties(object));
    while (applet) {
        if (auto panel = qobject_cast<DPanel *>(applet)) {
            return panel;
        }
        applet = applet->parentApplet();
    }
    return nullptr;
}

QQuickWindow *DPanel::popupWindow() const
{
    D_DC(DPanel);
    return d->m_popupWindow;
}

QQuickWindow *DPanel::toolTipWindow() const
{
    D_DC(DPanel);
    return d->m_toolTipWindow;
}

QQuickWindow *DPanel::menuWindow() const
{
    D_DC(DPanel);
    return d->m_menuWindow;
}

void DPanelPrivate::initDciSearchPaths()
{
    D_Q(DPanel);
    DGUI_USE_NAMESPACE;
    auto dciPaths = DIconTheme::dciThemeSearchPaths();
    QList<DApplet *> list = m_applets;
    list.append(q);
    for (const auto &item : list) {
        QDir root(item->pluginMetaData().pluginDir());
        if (root.exists("icons")) {
            dciPaths.push_back(root.absoluteFilePath("icons"));
        }
    }
    DIconTheme::setDciThemeSearchPaths(dciPaths);
}

void DPanelPrivate::ensurePopupWindow() const
{
    if (m_popupWindow)
        return;
    D_QC(DPanel);
    if (!q->window()) {
        qCWarning(dsLog) << "Failed to create PopupWindow because TransientParent window is empty.";
        return;
    }

    auto object = DQmlEngine::createObject(QUrl("qrc:/ddeshell/qml/PanelPopupWindow.qml"));
     if (!object)
         return;
     const_cast<DPanelPrivate *>(this)->m_popupWindow = qobject_cast<QQuickWindow *>(object);
     if (m_popupWindow) {
         qCDebug(dsLog) << "Create PopupWidow successfully.";
         m_popupWindow->setObjectName("PanelPopupWidow");
         m_popupWindow->setTransientParent(q->window());
         Q_EMIT const_cast<DPanel *>(q)->popupWindowChanged();
     }
}

void DPanelPrivate::ensureToolTipWindow() const
{
    if (m_toolTipWindow)
        return;
    D_QC(DPanel);
    if (!q->window()) {
        qCWarning(dsLog) << "Failed to create ToolTipWindow because TransientParent window is empty.";
        return;
    }

    auto object = DQmlEngine::createObject(QUrl("qrc:/ddeshell/qml/PanelToolTipWindow.qml"));
    if (!object)
        return;
    const_cast<DPanelPrivate *>(this)->m_toolTipWindow = qobject_cast<QQuickWindow *>(object);
    if (m_toolTipWindow) {
        qCDebug(dsLog) << "Create ToolTipWindow successfully.";
        m_toolTipWindow->setObjectName("PanelToolTipWidow");
        m_toolTipWindow->setTransientParent(q->window());
        Q_EMIT const_cast<DPanel *>(q)->toolTipWindowChanged();
    }
}

void DPanelPrivate::ensureMenuWindow() const
{
    if (m_menuWindow)
        return;
    D_QC(DPanel);
    if (!q->window()) {
        qCWarning(dsLog) << "Failed to create MenuWindow because TransientParent window is empty.";
        return;
    }

    auto object = DQmlEngine::createObject(QUrl("qrc:/ddeshell/qml/PanelMenuWindow.qml"));
    if (!object)
        return;
    const_cast<DPanelPrivate *>(this)->m_menuWindow = qobject_cast<QQuickWindow *>(object);
    if (m_menuWindow) {
        qCDebug(dsLog) << "Create MenuWindow successfully.";
        m_menuWindow->setObjectName("PanelMenuWidow");
        m_menuWindow->setTransientParent(q->window());
        QObject::connect(m_menuWindow, &QWindow::visibleChanged, m_popupWindow, [this] (bool arg) {
            if (arg) {
                if (m_popupWindow && m_popupWindow->isVisible())
                    m_popupWindow->close();
            }
        });
        Q_EMIT const_cast<DPanel *>(q)->menuWindowChanged();
    }
}

DS_END_NAMESPACE
