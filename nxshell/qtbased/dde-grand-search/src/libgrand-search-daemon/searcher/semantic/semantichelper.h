// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICHELPER_H
#define SEMANTICHELPER_H

#include "global/matcheditem.h"

#include <QStringList>

namespace GrandSearch {

struct SemanticEntity
{
    QStringList types;
    QStringList keys;
    QList<QPair<qint64, qint64>> times;
};

typedef bool (*PushItemCallBack)(const MatchedItemMap &items, void *pdata);

class SemanticHelper
{
public:
    static bool entityFromJson(const QString &json, SemanticEntity &out);
    static QString analyzeServiceName();
    static QString vectorServiceName();
    static bool isMatchTime(qint64 time, const QList<QPair<qint64, qint64>> &intervals);
    static QStringList typeTosuffix(const QStringList &type);
private:
    SemanticHelper();
};

}

#endif // SEMANTICHELPER_H
