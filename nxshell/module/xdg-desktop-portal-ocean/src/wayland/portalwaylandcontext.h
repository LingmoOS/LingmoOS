// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "protocols/screencopy.h"
#include "protocols/treelandcapture.h"

#include <QDBusContext>
#include <private/qwaylanddisplay_p.h>

class PortalWaylandContext : public QObject, public QDBusContext
{
    Q_OBJECT

public:
    PortalWaylandContext(QObject *parent = nullptr);
    inline QPointer<ScreenCopyManager> screenCopyManager() { return m_screenCopyManager; }
    inline QPointer<TreeLandCaptureManager> treelandCaptureManager()  { return m_treelandCaptureManager; }

private:
    ScreenCopyManager *m_screenCopyManager;
    TreeLandCaptureManager *m_treelandCaptureManager;
};
