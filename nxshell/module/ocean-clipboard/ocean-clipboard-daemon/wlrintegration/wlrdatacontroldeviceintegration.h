// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-wlr-data-control-unstable-v1.h"
#include <QObject>
#include <QMutex>

class WlrDataControlOfferIntegration;

class WlrDataControlDeviceIntegration : public QObject, public QtWayland::zwlr_data_control_device_v1
{
    Q_OBJECT
public:
    explicit WlrDataControlDeviceIntegration(::zwlr_data_control_device_v1 *object);
    ~WlrDataControlDeviceIntegration() override;

protected:
    void zwlr_data_control_device_v1_data_offer(struct ::zwlr_data_control_offer_v1 *id) override;
    void zwlr_data_control_device_v1_selection(struct ::zwlr_data_control_offer_v1 *id) override;
    void zwlr_data_control_device_v1_finished() override;
    void zwlr_data_control_device_v1_primary_selection(struct ::zwlr_data_control_offer_v1 *id) override;

Q_SIGNALS:
    void newSelection(WlrDataControlOfferIntegration *offer);
    void finished();

private:
    QStringList m_mimeTypeBuffer;
    QMutex m_mimeTypeBufferLock;
    WlrDataControlOfferIntegration *m_offerObjectBuffer;
};
