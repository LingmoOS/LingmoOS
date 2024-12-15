// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WLRDATACONTROLCLIPBOARDINTERFACE_H
#define WLRDATACONTROLCLIPBOARDINTERFACE_H

#include <QtConcurrent>
#include <QSharedPointer>
#include <QSemaphore>

#include <memory>

#include "wlrdatacontrolmanagerintegration.h"
#include "wlrdatacontroldeviceintegration.h"
#include "wlrdatacontrolsourceintegration.h"

class WlrDataControlClipboardInterface : public QObject
{
    Q_OBJECT
public:
    explicit WlrDataControlClipboardInterface(QObject *parent = nullptr);

    // R/W interfaces
    const QMimeData *mimeData() const;
    void setMimeData(QMimeData *mimeData);

protected:
    bool managerReady() { return m_dcManager && m_dcManager->isActive(); }
    void refreshDataControlSourceDevice();

    void takeoverClipboardDataSource();

    void abortLastRead();

protected slots:
    void onActiveChanged() { refreshDataControlSourceDevice(); }
    void onDataControlDeviceFinished();

    // Read clipboard, Stage 2
    void onNewSelection(WlrDataControlOfferIntegration *offer);
    // Read clipboard, read task finished
    void onReadTaskFinished();

    // Write clipboard, Stage 2
    void onSourceSend(QString mimeType, int fd);
    // Write clipboard (abort signal from compositor)
    void onSourceCancelled();

signals:
    // Notifies daemon the clipboard content changed
    void dataChanged();

private:
    // Wayland objects
    std::unique_ptr<WlrDataControlManagerIntegration> m_dcManager;
    std::unique_ptr<WlrDataControlSourceIntegration> m_dcSource;
    std::unique_ptr<WlrDataControlDeviceIntegration> m_dcDevice;

    // Clipboard reading synchronization (used to cancelling reading)
    QSemaphore m_readLock;
    QFuture<std::unique_ptr<QMimeData>> m_readTaskFuture;
    QFutureWatcher<std::unique_ptr<QMimeData>> m_readTaskWatcher;

    // Clipboard write doesn't need a synchronization mechanism

    // Clipboard content read from / written to clipboard
    std::unique_ptr<QMimeData> m_mimeData;
};

#endif // WLRDATACONTROLCLIPBOARDINTERFACE_H
