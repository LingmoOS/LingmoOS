// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appitem.h"
#include "treelandwindow.h"
#include "abstractwindow.h"
#include "treelandwindowmonitor.h"
#include "abstractwindowmonitor.h"

#include <QPointer>
#include <QIODevice>
#include <QVarLengthArray>

#include <QtWaylandClient/private/qwaylandwindow_p.h>

#include <algorithm>
#include <cstdint>
#include <iterator>


namespace dock {
ForeignToplevelManager::ForeignToplevelManager(TreeLandWindowMonitor* monitor)
    : QWaylandClientExtensionTemplate<ForeignToplevelManager>(1)
    , m_monitor(monitor)
{
}

void ForeignToplevelManager::ztreeland_foreign_toplevel_manager_v1_toplevel(struct ::ztreeland_foreign_toplevel_handle_v1 *toplevel)
{
    ForeignToplevelHandle* handle = new ForeignToplevelHandle(toplevel);
    connect(handle, &ForeignToplevelHandle::handlerIsReady, m_monitor, &TreeLandWindowMonitor::handleForeignToplevelHandleAdded, Qt::UniqueConnection);
}

TreeLandDockPreviewContext::TreeLandDockPreviewContext(struct ::treeland_dock_preview_context_v1 *context)
    : QWaylandClientExtensionTemplate<TreeLandDockPreviewContext>(1)
    , m_hideTimer(new QTimer(this))
    , m_isPreviewEntered(false)
    , m_isDockMouseAreaEnter(false)
{
    init(context);

    m_hideTimer->setSingleShot(true);
    m_hideTimer->setInterval(800);

    connect(m_hideTimer, &QTimer::timeout, this, [this](){
        if (!m_isDockMouseAreaEnter && !m_isPreviewEntered) {
            close();
        }
    }, Qt::QueuedConnection);
}

TreeLandDockPreviewContext::~TreeLandDockPreviewContext()
{
    destroy();
}

void TreeLandDockPreviewContext::showWindowsPreview(QByteArray windowsId, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction)
{
    m_isDockMouseAreaEnter = true;
    show(windowsId, previewXoffset, previewYoffset, direction);
}

void TreeLandDockPreviewContext::hideWindowsPreview()
{
    m_isDockMouseAreaEnter = false;
    m_hideTimer->start();
}

void TreeLandDockPreviewContext::treeland_dock_preview_context_v1_enter()
{
    m_isPreviewEntered = true;
}

void TreeLandDockPreviewContext::treeland_dock_preview_context_v1_leave()
{
    m_isPreviewEntered = false;
    m_hideTimer->start();
}

TreeLandWindowMonitor::TreeLandWindowMonitor(QObject* parent)
    :AbstractWindowMonitor(parent)
{
}

void TreeLandWindowMonitor::start()
{
    m_foreignToplevelManager.reset(new ForeignToplevelManager(this));
    connect(m_foreignToplevelManager.get(), &ForeignToplevelManager::newForeignToplevelHandle, this, &TreeLandWindowMonitor::handleForeignToplevelHandleAdded);
    connect(m_foreignToplevelManager.get(), &ForeignToplevelManager::activeChanged, this, [this]{
        if (!m_foreignToplevelManager->isActive()) {
            clear();
        }
    });
}

void TreeLandWindowMonitor::stop()
{
    m_foreignToplevelManager.reset(nullptr);
}


void TreeLandWindowMonitor::clear()
{
    m_windows.clear();
    m_dockPreview.reset(nullptr);
}

QPointer<AbstractWindow> TreeLandWindowMonitor::getWindowByWindowId(ulong windowId)
{
    return m_windows.value(windowId).get();
}

void TreeLandWindowMonitor::presentWindows(QList<uint32_t> windows)
{

}

void TreeLandWindowMonitor::showItemPreview(const QPointer<AppItem> &item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction)
{
    if (m_dockPreview.isNull()) {
        auto window = qobject_cast<QWindow*>(relativePositionItem);
        if (!window) return;
        auto waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow*>(window->handle());
        if (!waylandWindow) return;

        auto context = m_foreignToplevelManager->get_dock_preview_context(waylandWindow->wlSurface());
        m_dockPreview.reset(new TreeLandDockPreviewContext(context));
    }

    auto windows = item->getAppendWindows();
    m_dockPreview->m_isDockMouseAreaEnter = true;
    if (windows.length() != 0) {
        QVarLengthArray array = QVarLengthArray<uint32_t>();
        std::transform(windows.begin(), windows.end(), std::back_inserter(array), [](const QPointer<AbstractWindow>& window){
            return window->id();
        });
        
        QByteArray byteArray;
        int size = array.size() * sizeof(uint32_t);
        byteArray.resize(size);
        memcpy(byteArray.data(), array.constData(), size);
        m_dockPreview->showWindowsPreview(byteArray, previewXoffset, previewYoffset, direction);
    } else {
        m_dockPreview->show_tooltip(item->name(), previewXoffset, previewYoffset, direction);
    }
}

void TreeLandWindowMonitor::hideItemPreview()
{
    if (m_dockPreview.isNull()) return;
    m_dockPreview->hideWindowsPreview();
}

void TreeLandWindowMonitor::handleForeignToplevelHandleAdded()
{
    auto handle = qobject_cast<ForeignToplevelHandle*>(sender());
    if (!handle) {
        return;
    }

    auto id = handle->id();
    auto window = m_windows.value(id, nullptr);
    connect(handle, &ForeignToplevelHandle::handlerIsDeleted,this, &TreeLandWindowMonitor::handleForeignToplevelHandleRemoved, Qt::UniqueConnection);

    if (!window) {
        window = QSharedPointer<TreeLandWindow>(new TreeLandWindow(id));
        m_windows.insert(id, window);
    }

    window->setForeignToplevelHandle(handle);

    if (window->isReady())
        Q_EMIT AbstractWindowMonitor::windowAdded(static_cast<QPointer<AbstractWindow>>(window.get()));
}

void TreeLandWindowMonitor::handleForeignToplevelHandleRemoved()
{
    auto handle = qobject_cast<ForeignToplevelHandle*>(sender());
    if (!handle) {
        return;
    }

    auto id = handle->id();
    auto window = m_windows.value(id, nullptr);

    if (window) {
        m_windows.remove(id);
    }
}
}

