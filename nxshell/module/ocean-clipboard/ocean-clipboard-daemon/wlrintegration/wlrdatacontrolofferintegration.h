// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

#include <QObject>

#include "qwayland-wlr-data-control-unstable-v1.h"

class WlrDataControlDeviceIntegration;

class WlrDataControlOfferIntegration : public QObject, public QtWayland::zwlr_data_control_offer_v1
{
    Q_OBJECT
public:
    explicit WlrDataControlOfferIntegration(::zwlr_data_control_offer_v1 *object);
    ~WlrDataControlOfferIntegration() override;

    QStringList availableMimeTypes() { return m_mimeTypes; }
    void setAvailableMimeTypes(const QStringList &mimeTypes) { m_mimeTypes = mimeTypes; }

protected:
    void zwlr_data_control_offer_v1_offer(const QString &mime_type) override {
        Q_EMIT advertiseMimeType(mime_type);
    }

private:
    QStringList m_mimeTypes;

Q_SIGNALS:
    void advertiseMimeType(QString mimeType);
};
