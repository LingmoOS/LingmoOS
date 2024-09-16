// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtkinputdevices_converters.h"
#include <QMap>

DDEVICE_BEGIN_NAMESPACE

QString deviceTypeToString(DeviceType type)
{
    static const QMap<DeviceType, QString> &typeMap = {{DeviceType::Generic, "Generic"},
                                                       {DeviceType::Keyboard, "Keyboard"},
                                                       {DeviceType::Mouse, "Mouse"},
                                                       {DeviceType::Tablet, "Tablet"},
                                                       {DeviceType::TouchPad, "TouchPad"},
                                                       {DeviceType::TrackPoint, "TrackPoint"}};
    return typeMap[type];
}
QString keyActionToString(KeyAction action)
{
    static const QMap<KeyAction, QString> map{{KeyAction::LeftClick, "LeftClick"},
                                              {KeyAction::MiddleClick, "MiddleClick"},
                                              {KeyAction::RightClick, "RightClick"},
                                              {KeyAction::PageUp, "PageUp"},
                                              {KeyAction::PageDown, "PageDown"}};
    return map[action];
}

KeyAction stringToKeyAction(const QString &action)
{
    static const QMap<QString, KeyAction> map{{"LeftClick", KeyAction::LeftClick},
                                              {"MiddleClick", KeyAction::MiddleClick},
                                              {"RightClick", KeyAction::RightClick},
                                              {"PageUp", KeyAction::PageUp},
                                              {"PageDown", KeyAction::PageDown}};
    return map[action];
}

QString profileToString(AccelerationProfile profile)

{
    static const QMap<AccelerationProfile, QString> profileMap = {
        {AccelerationProfile::Adaptive, "adaptive"}, {AccelerationProfile::Flat, "flat"}, {AccelerationProfile::Na, "na"}};
    return profileMap[profile];
}

DDEVICE_END_NAMESPACE
