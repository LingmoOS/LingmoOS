// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QStringList>

namespace ddc
{

class AuthorizeRequest
{
public:
    QString clientID;
    QStringList scopes;
    QString callback;
    QString state;

    bool operator==(const AuthorizeRequest &ps) const
    {
      if (this->clientID==ps.clientID)
         return true;
      return false;
    }
};

class AuthorizeResponse
{
public:
    bool success;
    bool networkError;
    QString code;
    QString state;

    AuthorizeRequest req;
};

}
