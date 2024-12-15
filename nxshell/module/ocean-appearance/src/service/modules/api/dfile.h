// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILE_H
#define DFILE_H

#include <QString>
#include <vector>

class DFile
{
public:
    explicit DFile();
    static bool isAbs(QString file);
    static bool isExisted(QString file);
    static QString dir(QString file);
    static QString base(QString file);
    static std::vector<QString> glob(const QString& pattern);
};
#endif // DFILE_H
