// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef COMMON_H
#define COMMON_H
#include <unistd.h>
#include <pwd.h>

#include <QString>
#include <QVariantMap>
#include <QMetaType>

enum UserType {
    UserTypeStandard = 0,
    UserTypeAdmin,
    UserTypeUdcp,

    UserTypeInvalid,
};

UserType checkUserIsAdmin();
bool checkAuth(const QString &actionId, const int &pid);

#endif // COMMON_H
