/*
    SPDX-FileCopyrightText: 2023 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include "deepin-kwin_export.h"

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QUuid>
#include <QVector>
#include <memory>

struct wl_resource;

namespace KWin
{
class Output;
class OutputMode;
}

namespace KWaylandServer
{

class Display;
class OutputDeviceModeInterface;
class OutputDeviceInterfacePrivate;

/** @class OutputDeviceInterface
 *
 * Represents an output device, the difference to Output is that this output can be disabled,
 * so not currently used to display content.
 *
 * @see OutputManagementInterface
 * @since 5.5
 */
class KWIN_EXPORT OutputDeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit OutputDeviceInterface(Display *display, KWin::Output *handle, QObject *parent = nullptr);
    ~OutputDeviceInterface() override;

    void remove();

    KWin::Output *handle() const;

    bool invalid() const;

    OutputDeviceModeInterface *getMode(int modeId);

    static OutputDeviceInterface *get(wl_resource *native);

private:
    void updatePhysicalSize();
    void updateGlobalPosition();
    void updateManufacturer();
    void updateModel();
    void updateSerialNumber();
    void updateEisaId();
    void updateName();
    void updateScale();
    void updateSubPixel();
    void updateTransform();
    void updateModes();
    void updateCurrentMode();
    void updateEdid();
    void updateEnabled();
    void updateUuid();
    void updateCapabilities();
    void updateOverscan();
    void updateVrrPolicy();
    void updateRgbRange();
    void updateGeometry();
    void updateBrightness();
    void updateCtmValue();
    void updateCurvesChanged();
    void done();

    std::unique_ptr<OutputDeviceInterfacePrivate> d;
};

class OutputDeviceModeInterface : public QObject
{
    Q_OBJECT
public:
    OutputDeviceModeInterface(std::shared_ptr<KWin::OutputMode> handle, int id, QObject *parent = nullptr);
    ~OutputDeviceModeInterface() override;

    enum class ModeFlag : uint {
        Current = 0x1,
        Preferred = 0x2,
    };
    Q_DECLARE_FLAGS(ModeFlags, ModeFlag)

    QSize size() const
    {
        return m_size;
    }

    uint32_t refreshRate() const
    {
        return m_refreshRate;
    }

    ModeFlags flags() const
    {
        return m_flags;
    }

    int modeId() const
    {
        return m_modeId;
    }

    void markCurrent(bool current) {
        if (current) {
            m_flags |= ModeFlag::Current;
        } else {
            m_flags &= ~uint(ModeFlag::Current);
        }
    }

    std::weak_ptr<KWin::OutputMode> m_handle;

private:
    int m_modeId;
    QSize m_size;
    uint32_t m_refreshRate;
    ModeFlags m_flags;
    bool m_preferred = false;
};

}
