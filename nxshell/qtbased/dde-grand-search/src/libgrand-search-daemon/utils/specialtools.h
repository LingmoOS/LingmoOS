// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SPECIALTOOLS_H
#define SPECIALTOOLS_H

#include <QtCore>

namespace GrandSearch {

class SpecialTools
{
public:
    static bool splitCommand(const QString &cmd, QString &program, QStringList &args);
    static QMimeType getMimeType(const QFileInfo &file);
    static QString getJsonString(QJsonObject *json, const QString &key);
    static QJsonArray getJsonArray(QJsonObject *json, const QString &key);
    static bool isHiddenFile(const QString &fileName, QHash<QString, QSet<QString> > &filters, const QString &pathPrefix = "/");
};

}

#endif // SPECIALTOOLS_H
