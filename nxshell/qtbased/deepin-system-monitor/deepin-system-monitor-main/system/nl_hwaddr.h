// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NL_HWADDR_H
#define NL_HWADDR_H
#include <QByteArray>
#include <QString>
using namespace std;

class NLHWAddr
{
public:
    explicit NLHWAddr(const QByteArray &ifname);

public:
    QByteArray conn_type() const;
    bool is_virtual() const;
    int sa_family() const;
private:
    void initData();
    QByteArray m_conn_type;
    QByteArray m_ifname;
    int m_sa_family;
    bool  m_is_virtual;
};

inline bool NLHWAddr::is_virtual() const
{
    return m_is_virtual;
}

inline int NLHWAddr::sa_family() const
{
    return m_sa_family;
}

inline QByteArray NLHWAddr::conn_type() const
{
    return m_conn_type;
}


#endif // NL_HWADDR_H
