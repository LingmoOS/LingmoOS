/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QObject>
#include <QWaylandClientExtension>
#include <qwayland-kde-external-brightness-v1.h>

class DisplayBrightness;
class ExternalBrightnessController;

class ExternalBrightnessControl : public QObject, private QtWayland::kde_external_brightness_device_v1
{
    Q_OBJECT
public:
    explicit ExternalBrightnessControl(ExternalBrightnessController *controller, DisplayBrightness *display);
    ~ExternalBrightnessControl() override;

private:
    void kde_external_brightness_device_v1_requested_brightness(uint32_t value) override;

    DisplayBrightness *const m_display;
};

class ExternalBrightnessController : public QWaylandClientExtensionTemplate<ExternalBrightnessController, &QtWayland::kde_external_brightness_v1::destroy>,
                                     public QtWayland::kde_external_brightness_v1
{
public:
    explicit ExternalBrightnessController();

    void setDisplays(const QList<DisplayBrightness *> &displays);

private:
    std::unordered_map<DisplayBrightness *, std::unique_ptr<ExternalBrightnessControl>> m_waylandObjects;
};
