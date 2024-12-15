// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIRELESS_H
#define WIRELESS_H
#include <QByteArray>
#include <netlink/route/link.h>
namespace core {
namespace system {

class wireless
{
public:
    wireless();
    explicit wireless(QByteArray ifname);
    ~wireless();
    // 服务器别号
    QByteArray essid();
    unsigned char link_quality();
    unsigned char signal_levle();
    unsigned char noise_level();

    bool is_wireless();
protected:
    bool read_wireless_info();


private:
    bool m_bwireless;
    QByteArray m_ifname;

    QByteArray m_essid;
    unsigned char m_link_quality;
    unsigned char m_signal_levle;
    unsigned char m_noise_level;
};


}
}



#endif // WIRELESS_H
