/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_OUTPUTDEVICEMODE_V2_H
#define WAYLAND_OUTPUTDEVICEMODE_V2_H

#include <QObject>
#include <QSize>

#include <DWayland/Client/kwaylandclient_export.h>

struct kde_output_device_mode_v2;

namespace KWayland
{
namespace Client
{
class KWAYLANDCLIENT_EXPORT DeviceModeV2 : public QObject
{
    Q_OBJECT
public:
    DeviceModeV2(QObject *parent, kde_output_device_mode_v2 *m);

    ~DeviceModeV2();

    int refreshRate() const;
    QSize size() const;
    bool preferred() const;

    bool operator==(const DeviceModeV2 &other);

    operator kde_output_device_mode_v2 *();
    operator kde_output_device_mode_v2 *() const;

    static DeviceModeV2 *get(kde_output_device_mode_v2 *object);

    class Private;
    QScopedPointer<Private> d;

Q_SIGNALS:
    void removed();
};

}
}

#endif // WAYLAND_OUTPUTDEVICEMODE_V2_H
