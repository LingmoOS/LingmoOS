// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontrolofferintegration.h"

WlrDataControlOfferIntegration::WlrDataControlOfferIntegration(::zwlr_data_control_offer_v1 *object)
    : QObject()
    , QtWayland::zwlr_data_control_offer_v1(object)
{

}

WlrDataControlOfferIntegration::~WlrDataControlOfferIntegration()
{
    if (object() && zwlr_data_control_offer_v1_get_version(object()) >= ZWLR_DATA_CONTROL_OFFER_V1_DESTROY_SINCE_VERSION) {
        zwlr_data_control_offer_v1_destroy(object());
    }
}
