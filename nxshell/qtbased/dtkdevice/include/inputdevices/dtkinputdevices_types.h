// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKINPUTDEVICES_TYPES_H
#define DTKINPUTDEVICES_TYPES_H
#include "dtkdevice_global.h"
#include <QObject>
#include <QString>

DDEVICE_BEGIN_NAMESPACE

enum class DeviceType { Mouse, TrackPoint, TouchPad, Tablet, Keyboard, Generic };

struct DeviceInfo
{
    quint32 id;
    QString name;
    DeviceType type;

    friend bool operator==(const DeviceInfo &lhs, const DeviceInfo &rhs);
    friend QDebug operator<<(QDebug debug, const DeviceInfo &info);
};

extern const DeviceInfo DeviceInfoInitializer;

enum class AccelerationProfile { Flat, Adaptive, Na };

enum class Key { KeyUp, KeyDown };

enum class KeyAction { LeftClick, MiddleClick, RightClick, PageUp, PageDown };

enum class ScrollMethod { NoScroll, ScrollTwoFinger, ScrollEdge, ScrollOnButtonDown };

class LIBDTKDEVICESHARED_EXPORT EnumWrapper : public QObject
{
    Q_OBJECT
public:
    enum ErrorCode { InvalidCall };
    Q_ENUM(ErrorCode)
};

QDebug operator<<(QDebug debug, DeviceType type);
QDebug operator<<(QDebug debug, AccelerationProfile profile);

using ErrorCode = EnumWrapper::ErrorCode;

DDEVICE_END_NAMESPACE
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::DeviceType)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::DeviceInfo)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::AccelerationProfile)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::Key)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::KeyAction)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::ScrollMethod)
Q_DECLARE_METATYPE(DTK_DEVICE_NAMESPACE::ErrorCode)
#endif  // DTKINPUTDEVICES_TYPES_H
