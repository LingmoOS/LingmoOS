// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "abstractwindow.h"

#include <cstdint>

#include <QObject>


namespace dock {
class AppItem;
class AbstractWindowMonitor : public QObject
{
    Q_OBJECT

public:
    AbstractWindowMonitor(QObject* parent = nullptr): QObject(parent){};
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void clear() = 0;

    virtual QPointer<AbstractWindow> getWindowByWindowId(ulong windowId) = 0;

    virtual void presentWindows(QList<uint32_t> windowsId) = 0;

    virtual void showItemPreview(const QPointer<AppItem>& item, QObject* relativePositionItem, int32_t previewXoffset, int32_t previewYoffset, uint32_t direction) = 0;
    virtual void hideItemPreview() = 0;

Q_SIGNALS:
    void windowAdded(QPointer<AbstractWindow> window);
    void WindowMonitorShutdown();
};
}
