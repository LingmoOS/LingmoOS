// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDDATASOURCE_H
#define QWAYLANDDATASOURCE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>

#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QtWaylandClient/private/qtwaylandclientglobal_p.h>

QT_REQUIRE_CONFIG(wayland_datadevice);

QT_BEGIN_NAMESPACE

class QMimeData;

namespace QtWaylandClient {

class QWaylandDataDeviceManager;
class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataSource : public QObject, public QtWayland::wl_data_source
{
    Q_OBJECT
public:
    QWaylandDataSource(QWaylandDataDeviceManager *dataDeviceManager, QMimeData *mimeData);
    ~QWaylandDataSource() override;

    QMimeData *mimeData() const;

Q_SIGNALS:
    void targetChanged(const QString &mime_type);
    void cancelled();

protected:
    void data_source_cancelled() override;
    void data_source_send(const QString &mime_type, int32_t fd) override;
    void data_source_target(const QString &mime_type) override;

private:
    QWaylandDisplay *m_display = nullptr;
    QMimeData *m_mime_data = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDDATASOURCE_H
