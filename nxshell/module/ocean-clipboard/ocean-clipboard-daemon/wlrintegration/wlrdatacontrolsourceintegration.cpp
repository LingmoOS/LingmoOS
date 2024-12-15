// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontrolsourceintegration.h"

WlrDataControlSourceIntegration::WlrDataControlSourceIntegration(::zwlr_data_control_source_v1 *object)
    : QObject()
    , QtWayland::zwlr_data_control_source_v1(object)
{

}

WlrDataControlSourceIntegration::~WlrDataControlSourceIntegration()
{
    if (object() && zwlr_data_control_source_v1_get_version(object()) >= ZWLR_DATA_CONTROL_SOURCE_V1_DESTROY_SINCE_VERSION) {
        zwlr_data_control_source_v1_destroy(object());
    }
}

void WlrDataControlSourceIntegration::zwlr_data_control_source_v1_send(const QString &mime_type, int32_t fd)
{
    Q_EMIT send(mime_type, fd);
}

void WlrDataControlSourceIntegration::zwlr_data_control_source_v1_cancelled()
{
    Q_EMIT cancelled();
}
