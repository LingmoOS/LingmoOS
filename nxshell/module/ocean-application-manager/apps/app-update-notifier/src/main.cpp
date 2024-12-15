// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/applicationupdatenotifier1service.h"

int main()
{
    ApplicationUpdateNotifier1Service service;
    emit service.ApplicationUpdated();
    return 0;
}
