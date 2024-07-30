/*
 * Copyright (C) 2015 Weng Xuetian <wengxt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "wacomtabletservice.h"
#include "dbustabletinterface.h"
#include "screenrotation.h"
#include <Plasma5Support/ServiceJob>
#include <QProcess>

using namespace Wacom;

class WacomTabletJob : public Plasma::ServiceJob
{
    Q_OBJECT
public:
    WacomTabletJob(const QString &destination, const QString &operation, const QMap<QString, QVariant> &parameters, QObject *parent = nullptr)
        : ServiceJob(destination, operation, parameters, parent)
    {
    }

    virtual void start() override
    {
        if (!DBusTabletInterface::instance().isValid()) {
            return;
        }

        const auto op = operationName();
        if (op == QLatin1String("RunKCM")) {
            QProcess::startDetached(QLatin1String("kcmshell6"), QStringList() << QLatin1String("wacomtablet"));
            return;
        }

        const auto param = parameters();

        if (!param.contains(QLatin1String("tabletId"))) {
            return;
        }

        const auto tabletId = param[QLatin1String("tabletId")].toString();

        if (op == QLatin1String("SetProfile")) {
            if (param.contains(QLatin1String("profile"))) {
                DBusTabletInterface::instance().setProfile(tabletId, param[QLatin1String("profile")].toString());
            }
        } else if (op == QLatin1String("SetStylusMode")) {
            if (param.contains(QLatin1String("mode"))) {
                DBusTabletInterface::instance().setProperty(tabletId, DeviceType::Stylus.key(), Property::Mode.key(), param[QLatin1String("mode")].toString());
                DBusTabletInterface::instance().setProperty(tabletId, DeviceType::Eraser.key(), Property::Mode.key(), param[QLatin1String("mode")].toString());
            }
        } else if (op == QLatin1String("SetRotation")) {
            if (param.contains(QLatin1String("rotation"))) {
                const auto rotationName = param[QLatin1String("rotation")].toString();

                const ScreenRotation *rotation = nullptr;
                if (rotationName == QLatin1String("none")) {
                    rotation = &ScreenRotation::NONE;
                } else if (rotationName == QLatin1String("cw")) {
                    rotation = &ScreenRotation::CW;
                } else if (rotationName == QLatin1String("ccw")) {
                    rotation = &ScreenRotation::CCW;
                } else if (rotationName == QLatin1String("half")) {
                    rotation = &ScreenRotation::HALF;
                }

                if (rotation) {
                    DBusTabletInterface::instance().setProperty(tabletId, DeviceType::Stylus.key(), Property::Rotate.key(), rotation->key());
                    DBusTabletInterface::instance().setProperty(tabletId, DeviceType::Eraser.key(), Property::Rotate.key(), rotation->key());
                    DBusTabletInterface::instance().setProperty(tabletId, DeviceType::Touch.key(), Property::Rotate.key(), rotation->key());
                }
            }
        } else if (op == QLatin1String("SetTouchMode")) {
            if (param.contains(QLatin1String("enable"))) {
                DBusTabletInterface::instance().setProperty(tabletId,
                                                            DeviceType::Touch.key(),
                                                            Property::Touch.key(),
                                                            param[QLatin1String("enable")].toBool() ? QLatin1String("on") : QLatin1String("off"));
            }
        }
    }
};

WacomTabletService::WacomTabletService(const QString &destination, QObject *parent)
    : Service(parent)
{
    setName(QLatin1String("wacomtablet"));
    setObjectName(destination);
    setDestination(destination);
    setOperationEnabled(QLatin1String("SetProfile"), true);
    setOperationEnabled(QLatin1String("SetStylusMode"), true);
    setOperationEnabled(QLatin1String("SetRotation"), true);
    setOperationEnabled(QLatin1String("SetTouchMode"), true);
}

WacomTabletService::~WacomTabletService()
{
}

Plasma::ServiceJob *WacomTabletService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new WacomTabletJob(destination(), operation, parameters, this);
}

#include "wacomtabletservice.moc"

#include "moc_wacomtabletservice.cpp"
