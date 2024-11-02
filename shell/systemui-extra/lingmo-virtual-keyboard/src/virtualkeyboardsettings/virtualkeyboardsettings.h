/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _VIRTUALKEYBOARDSETTINGS_H_
#define _VIRTUALKEYBOARDSETTINGS_H_

#include <memory>

#include <QGSettings>
#include <QObject>

class VirtualKeyboardSettings : public QObject {
    Q_OBJECT

public:
    static VirtualKeyboardSettings &getInstance();
    void updateFloatButtonAvailability(bool value);
    bool isFloatButtonEnabled() const;
    bool isAnimationEnabled() const;
    float calculateVirtualKeyboardScaleFactor() const;

private:
    VirtualKeyboardSettings();
    ~VirtualKeyboardSettings() override = default;

    Q_DISABLE_COPY(VirtualKeyboardSettings)

    void init();
    void emitFloatButtonAvailabilityChanged();

signals:
    void requestFloatButtonEnabled();
    void requestFloatButtonDisabled();
    void scaleFactorChanged();
    void animationAvailabilityChanged();

private:
    std::unique_ptr<QGSettings> gsettings_;
    const QString gsettingsId_ = "org.lingmo.virtualkeyboard";
    const QString floatButtonEnabledKey_ = "floatButtonEnabled";
    const QString virtualKeyboardScaleFactorKey_ = "virtualKeyboardScaleFactor";
    const QString animationEnabledKey_ = "animationEnabled";
};
#endif
