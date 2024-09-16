// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDDND_H
#define QWAYLANDDND_H

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

#include <qpa/qplatformdrag.h>
#include <QtGui/private/qsimpledrag_p.h>

#include <QtGui/QDrag>
#include <QtCore/QMimeData>

#include <QtWaylandClient/qtwaylandclientglobal.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
#if QT_CONFIG(draganddrop)
class Q_WAYLAND_CLIENT_EXPORT QWaylandDrag : public QBasicDrag
{
public:
    QWaylandDrag(QWaylandDisplay *display);
    ~QWaylandDrag() override;

    void updateTarget(const QString &mimeType);
    void setResponse(const QPlatformDragQtResponse &response);
    void finishDrag(const QPlatformDropQtResponse &response);

protected:
    void startDrag() override;
    void cancel() override;
    void move(const QPoint &globalPos) override;
    void drop(const QPoint &globalPos) override;
    void endDrag() override;


private:
    QWaylandDisplay *m_display = nullptr;
};
#endif
}

QT_END_NAMESPACE

#endif // QWAYLANDDND_H
