// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICETRACKPOINT_P_H
#define DINPUTDEVICETRACKPOINT_P_H
#include "dinputdevicepointer_p.h"
#include "dinputdevicetrackpoint.h"
#include "trackpointinterface.h"
DDEVICE_BEGIN_NAMESPACE
class LIBDTKDEVICESHARED_EXPORT DInputDeviceTrackPointPrivate : public DInputDevicePointerPrivate
{
public:
    DInputDeviceTrackPointPrivate(DInputDeviceTrackPoint *q);
    ~DInputDeviceTrackPointPrivate() override;

private:
    TrackPointInterface *m_trackPointInter;
    D_DECLARE_PUBLIC(DInputDeviceTrackPoint)
};

DDEVICE_END_NAMESPACE

#endif  // DINPUTDEVICETRACKPOINT_P_H
