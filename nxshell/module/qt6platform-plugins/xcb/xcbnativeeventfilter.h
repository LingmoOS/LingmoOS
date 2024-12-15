// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef XCBNATIVEEVENTFILTER_H
#define XCBNATIVEEVENTFILTER_H

#include "global.h"

#include <QAbstractNativeEventFilter>
#include <QClipboard>
#include <QHash>

#include <xcb/xproto.h>

QT_BEGIN_NAMESPACE
class QXcbConnection;
class QInputEvent;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class XcbNativeEventFilter : public QAbstractNativeEventFilter
{
public:
    struct XIDeviceInfos {
        XIDeviceInfos(DeviceType t = UnknowDevice): type(t) {}

        DeviceType type;
    };

    XcbNativeEventFilter(QXcbConnection *connection);

    QClipboard::Mode clipboardModeForAtom(xcb_atom_t a) const;

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) Q_DECL_OVERRIDE;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
#endif

    DeviceType xiEventSource(const QInputEvent *event) const;

private:
    void updateXIDeviceInfoMap();

    QXcbConnection *m_connection;
    uint8_t m_damageFirstEvent;
    QHash<quint16, XIDeviceInfos> xiDeviceInfoMap;
    QPair<quint32, XIDeviceInfos> lastXIEventDeviceInfo;
};

DPP_END_NAMESPACE

#endif // XCBNATIVEEVENTFILTER_H
