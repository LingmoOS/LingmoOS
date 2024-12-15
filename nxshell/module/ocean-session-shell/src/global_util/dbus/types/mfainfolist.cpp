// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mfainfolist.h"

bool MFAInfo::operator==(const MFAInfo &info) const
{
    return AuthType == info.AuthType && Priority == info.Priority
           && InputType == info.InputType && Required == info.Required;
}

QDebug operator<<(QDebug argument, const MFAInfo &info)
{
    argument << info.AuthType << "," << info.Priority << "," << info.InputType << "," << info.Required;
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const MFAInfo &info)
{
    argument.beginStructure();
    argument << info.AuthType << info.Priority << info.InputType << info.Required;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, MFAInfo &info)
{
    argument.beginStructure();
    argument >> info.AuthType >> info.Priority >> info.InputType >> info.Required;
    argument.endStructure();
    return argument;
}

void registerMFAInfoMetaType()
{
    qRegisterMetaType<MFAInfo>("MFAInfo");
    qDBusRegisterMetaType<MFAInfo>();
}

void registerMFAInfoListMetaType()
{
    registerMFAInfoMetaType();
    qRegisterMetaType<MFAInfoList>("MFAInfoList");
    qDBusRegisterMetaType<MFAInfoList>();
}
