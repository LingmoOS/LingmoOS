// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPATIBLE_DEFINES_H
#define COMPATIBLE_DEFINES_H

#include <QString>
#include <QSharedPointer>

namespace Compatible {

struct CompPkgInfo
{
    using Ptr = QSharedPointer<CompPkgInfo>;

    QString filePath;  // for local file
    QString name;
    QString version;
    QString arch;          // architecture
    QString rootfs;        // rootfs name, which installed this package
    QString targetRootfs;  // target rootfs, prepare for install pacakge to specific rootfs

    inline bool installed() const { return !rootfs.isEmpty(); }
};

struct RootfsInfo
{
    using Ptr = QSharedPointer<RootfsInfo>;

    int prioriy{0};
    QString name;
    QString osName;
};

}  // namespace Compatible

#endif  // COMPATIBLE_DEFINES_H
