// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICPARSER_P_H
#define SEMANTICPARSER_P_H

#include "semanticparser.h"
#include "analyzeserver.h"
#include "vectorindex.h"

using namespace org::deepin::ai::daemon;

namespace GrandSearch {
class SemanticParserPrivate
{
public:
    explicit SemanticParserPrivate(SemanticParser *parent);
    //~SemanticParserPrivate();
public:
    AnalyzeServer *m_analyze = nullptr;
    VectorIndex *m_vector = nullptr;

private:
    SemanticParser *q;
};
}
#endif // SEMANTICPARSER_P_H
