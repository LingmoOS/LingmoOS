// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VECTORQUERY_P_H
#define VECTORQUERY_P_H

#include "vectorquery.h"
#include "searcher/semantic/fileresultshandler.h"

namespace GrandSearch {

class VectorQueryPrivate
{
public:
    explicit VectorQueryPrivate(VectorQuery *qq);
    bool timeToPush() const;
    double matchedWeight(qreal distance);
public:
    FileResultsHandler *m_handler = nullptr;
    SemanticParser *m_parser = nullptr;
    QString m_query;
    int m_count = 0;
    QTime m_time;
    int m_lastPush = 0;
private:
    VectorQuery *q;
};

}
#endif // VECTORQUERY_P_H
