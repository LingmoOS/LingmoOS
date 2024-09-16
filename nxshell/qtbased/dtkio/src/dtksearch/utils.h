// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QMap>

#include "dtksearch_global.h"

DSEARCH_BEGIN_NAMESPACE

namespace Utils {
inline constexpr char kNotSupportDirectories[] = "^/(boot|dev|proc|sys|run|lib|usr).*$";
inline constexpr char kSupportFiles[] = "(rtf)|(odt)|(ods)|(odp)|(odg)|(docx)|(xlsx)|(pptx)|(ppsx)|(md)|"
                                        "(xls)|(xlsb)|(doc)|(dot)|(wps)|(ppt)|(pps)|(txt)|(pdf)|(dps)";
inline constexpr int kEmitInterval = 50;

enum ConvertType {
    DirType,
    DeviceType
};

QMap<QString, QString> &fstabBindInfo();
QString convertTo(const QString &path, ConvertType type);

QString checkWildcardAndToRegularExpression(const QString &pattern);
QString wildcardToRegularExpression(const QString &pattern);
inline QString anchoredPattern(const QString &expression)
{
    return QLatin1String("\\A(?:")
            + expression
            + QLatin1String(")\\z");
}

QHash<uint, QString> &dictData();
QString Chinese2Pinyin(const QString& words);

}   // namespace Utils

DSEARCH_END_NAMESPACE

#endif   // UTILS_H
