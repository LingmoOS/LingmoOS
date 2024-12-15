// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDCLIENTGLOBAL_H
#define QWAYLANDCLIENTGLOBAL_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGui/qtguiglobal.h>
#include <QtWaylandClient/qtwaylandclient-config.h>

QT_BEGIN_NAMESPACE

#if !defined(Q_WAYLAND_CLIENT_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_CLIENT_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_CLIENT_EXPORT
#  endif
#endif

QT_END_NAMESPACE

#endif //QWAYLANDCLIENTGLOBAL_H

