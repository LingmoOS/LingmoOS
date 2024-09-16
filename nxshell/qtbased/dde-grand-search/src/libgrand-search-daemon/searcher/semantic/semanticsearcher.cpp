// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticsearcher_p.h"
#include "global/builtinsearch.h"
#include "semanticworker.h"
#include "semantichelper.h"
#include "configuration/configer.h"
#include "tools/semanticparser/semanticparser.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

using namespace GrandSearch;

SemanticSearcherPrivate::SemanticSearcherPrivate(SemanticSearcher *parent): q(parent)
{

}

SemanticSearcherPrivate::~SemanticSearcherPrivate()
{

}

SemanticSearcher::SemanticSearcher(QObject *parent)
    : Searcher(parent)
    , d(new SemanticSearcherPrivate(this))
{

}

SemanticSearcher::~SemanticSearcher()
{
    delete d;
    d = nullptr;
}

QString SemanticSearcher::name() const
{
    return GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC;
}

bool SemanticSearcher::isActive() const
{
    auto config = ConfigerIns->group(GRANDSEARCH_SEMANTIC_GROUP);
    Q_ASSERT(config);

    SemanticParser paser;
    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, false)) {
        if (paser.connectToAnalyze(SemanticHelper::analyzeServiceName())) {
            d->m_semantic  = paser.isAnalayzeSupported();
        }
    }

    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true)) {
        if (paser.connectToVector(SemanticHelper::vectorServiceName())) {
            d->m_vector = paser.isVectorSupported();
        }
    }

    return d->m_semantic || d->m_vector;
}

bool SemanticSearcher::activate()
{
    return false;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    auto worker = new SemanticWorker(name());
    worker->setEngineState(d->m_semantic, d->m_vector);
    return worker;
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}
