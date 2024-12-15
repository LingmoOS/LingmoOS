// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QUERIES_H
#define QUERIES_H

#include <QMutex>
#include <QVariant>
#include <QWaitCondition>

struct NewStr {
    QStringList strList;
    QString resultStr;
    int fontHeifht = 0;
};

/**
 * @brief The OverwriteQuery_Result enum   处理文件已存在时的选项
 */
enum OverwriteQuery_Result {
    Result_Cancel = 0,            // 取消
    Result_Skip = 1,              // 跳过
    Result_SkipAll = 2,           // 全部跳过
    Result_Overwrite = 3,         // 替换
    Result_OverwriteAll = 4,      // 全部替换
    Result_Readonly = 5           // 以只读方式打开，例如损坏的分卷包
};

typedef QHash<QString, QVariant> QueryData;

#endif // QUERIES_H
