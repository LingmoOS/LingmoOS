// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UAB_DEFINES_H
#define UAB_DEFINES_H

#include <QString>
#include <QStringList>
#include <QSharedPointer>

namespace Uab {

struct UabPkgInfo
{
    using Ptr = QSharedPointer<UabPkgInfo>;

    QString filePath;  // absolute file path
    QString id;        // package id, e.g.: com.deepin.package
    QString appName;   // display name
    QString version;
    QStringList architecture;  // uab package may support multiple arch
    QString shortDescription;
    QString channel;
};

};  // namespace Uab

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
Q_CORE_EXPORT QDebug operator<<(QDebug, const Uab::UabPkgInfo &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const Uab::UabPkgInfo::Ptr &);
#endif  // QT_NO_DEBUG_STREAM
QT_END_NAMESPACE

#endif  // UAB_DEFINES_H
