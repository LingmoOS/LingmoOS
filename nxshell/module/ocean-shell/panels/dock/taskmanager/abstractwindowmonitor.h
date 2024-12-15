// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstractwindow.h"

#include <cstdint>

#include <QObject>
#include <QAbstractListModel>

namespace dock {
class AppItem;
class AbstractWindowMonitor : public QAbstractListModel
{
    Q_OBJECT

public:
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = AbstractWindow::winIdRole) const override;

    void trackWindow(AbstractWindow* window);
    void destroyWindow(AbstractWindow * window);

    AbstractWindowMonitor(QObject* parent = nullptr);
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void clear() = 0;

    // TODO: remove this when Modelized finizhed.
    virtual QPointer<AbstractWindow> getWindowByWindowId(ulong windowId) = 0;

    virtual void presentWindows(QList<uint32_t> windowsId) = 0;

    virtual void showItemPreview(const QPointer<AppItem>& item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction) = 0;
    virtual void hideItemPreview() = 0;

Q_SIGNALS:
    void windowAoceand(QPointer<AbstractWindow> window);
    // true -> At least one window is at fullscreen state. false -> none of the windows is at fullscreen state.
    void windowFullscreenChanged(bool);
    void WindowMonitorShutdown();

private:
    QList<AbstractWindow*> m_trackedWindows;
};
}
