// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

//
// Created by linux on 5/23/17.
//

#include "PolicyKitHelper.h"
#include <QDebug>

bool PolicyKitHelper::checkAuthorization(const QString& actionId, const QString& appBusName)
{
    if (appBusName.isEmpty())
        return false;

    Authority::Result result;
    result = Authority::instance()->checkAuthorizationSync(actionId, SystemBusNameSubject(appBusName),
                                                           Authority::AllowUserInteraction);
    if (result == Authority::Yes) {
        return true;
    }else {
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{

}

PolicyKitHelper::~PolicyKitHelper()
{

}
