// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDDATAOFFER_H
#define QWAYLANDDATAOFFER_H

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

#include <QtGui/private/qinternalmimedata_p.h>

#include <QtWaylandClient/private/qtwaylandclientglobal_p.h>
#include <QtWaylandClient/private/qwayland-wayland.h>

QT_REQUIRE_CONFIG(wayland_datadevice);

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandMimeData;

class Q_WAYLAND_CLIENT_EXPORT QWaylandDataOffer : public QtWayland::wl_data_offer
{
public:
    explicit QWaylandDataOffer(QWaylandDisplay *display, struct ::wl_data_offer *offer);
    ~QWaylandDataOffer() override;

    QString firstFormat() const;

    QMimeData *mimeData();

protected:
    void data_offer_offer(const QString &mime_type) override;

private:
    QScopedPointer<QWaylandMimeData> m_mimeData;
};


class QWaylandMimeData : public QInternalMimeData {
public:
    explicit QWaylandMimeData(QWaylandDataOffer *dataOffer, QWaylandDisplay *display);
    ~QWaylandMimeData() override;

    void appendFormat(const QString &mimeType);

protected:
    bool hasFormat_sys(const QString &mimeType) const override;
    QStringList formats_sys() const override;
    QVariant retrieveData_sys(const QString &mimeType, QVariant::Type type) const override;

private:
    int readData(int fd, QByteArray &data) const;

    mutable QWaylandDataOffer *m_dataOffer = nullptr;
    QWaylandDisplay *m_display = nullptr;
    mutable QStringList m_types;
    mutable QHash<QString, QByteArray> m_data;
};

}

QT_END_NAMESPACE
#endif
