// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FORMAT_H
#define FORMAT_H

#include <QObject>
#include <QFile>
#include <QMap>

class FormatPicture{

public:
    static QString getPictureType(QString file);

private:
    static QMap<QString,QString> typeMap;
};


#endif // FORMAT_H
