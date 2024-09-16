// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "treelandwindow.h"
#include "abstractwindowmonitor.h"
#include "qwayland-treeland-foreign-toplevel-manager-v1.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QTimer>

#include <QtWaylandClient/QWaylandClientExtension>

namespace dock {
class ForeignToplevelManager : public QWaylandClientExtensionTemplate<ForeignToplevelManager>, public QtWayland::ztreeland_foreign_toplevel_manager_v1
{
    Q_OBJECT
public:
    explicit ForeignToplevelManager(TreeLandWindowMonitor* monitor);

Q_SIGNALS:
    void newForeignToplevelHandle(ForeignToplevelHandle *handle);

protected:
    void ztreeland_foreign_toplevel_manager_v1_toplevel(struct ::ztreeland_foreign_toplevel_handle_v1 *toplevel) override;

private:
    TreeLandWindowMonitor* m_monitor;
};

class TreeLandDockPreviewContext : public QWaylandClientExtensionTemplate<TreeLandDockPreviewContext>, public QtWayland::treeland_dock_preview_context_v1
{
    Q_OBJECT

public:
    explicit TreeLandDockPreviewContext(struct ::treeland_dock_preview_context_v1 *);
    ~TreeLandDockPreviewContext();
    void showWindowsPreview(QByteArray windowsId, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction);
    void hideWindowsPreview();

Q_SIGNALS:
    void entered();
    void exited();

protected:
    virtual void treeland_dock_preview_context_v1_enter() override;
    virtual void treeland_dock_preview_context_v1_leave() override;
    friend class TreeLandWindowMonitor;

private:
    bool m_isPreviewEntered;
    bool m_isDockMouseAreaEnter;
    QTimer* m_hideTimer;
};

class TreeLandWindowMonitor : public AbstractWindowMonitor
{
    Q_OBJECT

public:
    explicit TreeLandWindowMonitor(QObject* parent = nullptr);
    virtual void start() override;
    virtual void stop() override;
    virtual void clear() override;

    virtual QPointer<AbstractWindow> getWindowByWindowId(ulong windowId) override;

    virtual void presentWindows(QList<uint32_t> windows) override;
    virtual void showItemPreview(const QPointer<AppItem> &item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction) override;
    virtual void hideItemPreview() override;

private Q_SLOTS:
    friend class ForeignToplevelManager;

    void handleForeignToplevelHandleAdded();
    void handleForeignToplevelHandleRemoved();

private:
    QHash<ulong, QSharedPointer<TreeLandWindow>> m_windows;
    QScopedPointer<ForeignToplevelManager> m_foreignToplevelManager;
    QScopedPointer<TreeLandDockPreviewContext> m_dockPreview;

};
}
