// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEMANTICPARSER_P_H
#define SEMANTICPARSER_P_H

#include "semanticparser.h"
#include "vectorindex.h"
#include "querylang.h"

using namespace org::lingmo::ai::daemon;

namespace GrandSearch {
class SemanticParserPrivate
{
public:
    explicit SemanticParserPrivate(SemanticParser *parent);
    //~SemanticParserPrivate();
public:
    VectorIndex *m_vector = nullptr;
    QueryLang *m_querylang = nullptr;

private:
    SemanticParser *q;
};
}
#endif // SEMANTICPARSER_P_H
