// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDSEARCHER_P_H
#define EXTENDSEARCHER_P_H

#include "extendsearcher.h"

namespace GrandSearch {

class ExtendSearcherPrivate : public QObject
{
    Q_OBJECT
    friend class ExtendSearcher;
public:
    ExtendSearcherPrivate(ExtendSearcher *parent);
private:
    QString m_name;
    QString m_version;
    ExtendSearcher::Activatable m_activatable = ExtendSearcher::Inactivatable;
    QString m_service;    //dbus服务
    QString m_address;    //dbus地址
    QString m_interface;  //dbus接口
};

}

#endif // EXTENDSEARCHER_P_H
