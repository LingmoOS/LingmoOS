// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MFAINFO_H
#define MFAINFO_H

#include <QDBusMetaType>
#include <QDebug>

struct MFAInfo {
    int AuthType;
    int Priority;
    int InputType;
    bool Required;

    bool operator==(const MFAInfo &info) const;
};

Q_DECLARE_METATYPE(MFAInfo);
typedef QList<MFAInfo> MFAInfoList;
Q_DECLARE_METATYPE(MFAInfoList);

QDebug operator<<(QDebug argument, const MFAInfo &info);
QDBusArgument &operator<<(QDBusArgument &argument, const MFAInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, MFAInfo &info);

void registerMFAInfoListMetaType();

#endif // MFAINFO_H
