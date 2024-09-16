// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netaddress.h"
#include "co/log.h"

namespace zrpc_ns {

NetAddress::NetAddress(const char *ip, uint16 port, char *key, char *crt) {
    if ((ip && *ip)) {
        strcpy(m_ip, ip);
    } else {
        strcpy(m_ip, "0.0.0.0");
    }
    if ((key && *key) && (crt && *crt)) {
      strcpy(m_ssl_key, key);
      strcpy(m_ssl_crt, crt);
      m_ssl = true;
    } else {
      m_ssl = false;
    }

    m_port = port;
}

NetAddress::NetAddress(const char *ip, uint16 port, bool ssl) {
    if ((ip && *ip)) {
        strcpy(m_ip, ip);
    } else {
        strcpy(m_ip, "0.0.0.0");
    }

    m_port = port;
    m_ssl = ssl;
}

} // namespace zrpc_ns
