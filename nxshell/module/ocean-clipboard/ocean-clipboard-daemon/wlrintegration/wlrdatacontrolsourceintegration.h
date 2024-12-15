// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-wlr-data-control-unstable-v1.h"
#include <QObject>

class WlrDataControlSourceIntegration : public QObject, public QtWayland::zwlr_data_control_source_v1
{
    Q_OBJECT
public:
    explicit WlrDataControlSourceIntegration(::zwlr_data_control_source_v1 *object);
    ~WlrDataControlSourceIntegration() override;

protected:
    void zwlr_data_control_source_v1_send(const QString &mime_type, int32_t fd) override;
    void zwlr_data_control_source_v1_cancelled() override;

Q_SIGNALS:
    void send(QString mimeType, int fd);
    void cancelled();
};
