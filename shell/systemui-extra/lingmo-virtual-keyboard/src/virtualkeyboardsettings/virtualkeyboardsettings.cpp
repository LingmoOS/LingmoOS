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

#include "virtualkeyboardsettings.h"

#include <QByteArray>
#include <QDebug>
#include <QVariant>

VirtualKeyboardSettings::VirtualKeyboardSettings() { init(); }

void VirtualKeyboardSettings::init() {
    if (!QGSettings::isSchemaInstalled(gsettingsId_.toUtf8())) {
        qWarning() << "WARNING : INCORRECT GSETTINGS ID :" << gsettingsId_
                   << "IS NOT INSTALLED!";
        return;
    }

    gsettings_.reset(new QGSettings(gsettingsId_.toUtf8()));

    connect(gsettings_.get(), &QGSettings::changed, this,
            [this](const QString &key) {
                if (!gsettings_->keys().contains(key)) {
                    return;
                }

                if (key == floatButtonEnabledKey_) {
                    emitFloatButtonAvailabilityChanged();
                } else if (key == virtualKeyboardScaleFactorKey_) {
                    emit scaleFactorChanged();
                }
            });
}

void VirtualKeyboardSettings::emitFloatButtonAvailabilityChanged() {
    const bool value = gsettings_->get(floatButtonEnabledKey_).toBool();
    if (value) {
        emit requestFloatButtonEnabled();
    } else {
        emit requestFloatButtonDisabled();
    }
}

void VirtualKeyboardSettings::updateFloatButtonAvailability(const bool value) {
    if (gsettings_ == nullptr) {
        qWarning() << "WARNING : Gsettings Objetc is NULL !";
        return;
    }

    gsettings_->set(floatButtonEnabledKey_, QVariant(value));
}

bool VirtualKeyboardSettings::isFloatButtonEnabled() const {
    if (gsettings_ == nullptr) {
        qWarning() << "WARNING : Gsettings Objetc is NULL !";
        return false;
    }

    return gsettings_->get(floatButtonEnabledKey_).toBool();
}

bool VirtualKeyboardSettings::isAnimationEnabled() const {
    if (gsettings_ == nullptr) {
        qWarning() << "WARNING : Gsettings Objetc is NULL !";
        return false;
    }

    return gsettings_->get(animationEnabledKey_).toBool();
}

float VirtualKeyboardSettings::calculateVirtualKeyboardScaleFactor() const {
    return static_cast<float>(
               gsettings_->get(virtualKeyboardScaleFactorKey_).toInt()) /
           100;
}

VirtualKeyboardSettings &VirtualKeyboardSettings::getInstance() {
    static VirtualKeyboardSettings virtualKeyboardSettings;
    return virtualKeyboardSettings;
}
