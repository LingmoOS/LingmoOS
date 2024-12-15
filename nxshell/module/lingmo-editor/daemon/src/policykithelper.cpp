// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

bool PolicyKitHelper::checkAuthorization(const QString& actionId, qint64 applicationPid)
{
    Authority::Result result;

    result = Authority::instance()->checkAuthorizationSync(
        actionId, 
        UnixProcessSubject(applicationPid),
        Authority::AllowUserInteraction);
    
    return result == Authority::Yes;
}

PolicyKitHelper::PolicyKitHelper()
{

}

PolicyKitHelper::~PolicyKitHelper()
{

}
