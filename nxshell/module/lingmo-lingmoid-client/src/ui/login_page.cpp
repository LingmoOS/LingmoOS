// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login_page.h"

void WebUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    for (auto k : header.keys()) {
        info.setHttpHeader(k, header[k]);
    }
}

