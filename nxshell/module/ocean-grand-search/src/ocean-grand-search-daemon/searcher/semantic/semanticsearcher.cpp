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
    d->m_semantic = false;
    d->m_vector   = false;
    d->m_fulltext = false;

    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_ANALYSIS, true)) {
        /*if (paser.connectToQueryLang(SemanticHelper::querylangServiceName())) {
            d->m_semantic  = paser.isQueryLangSupported();
        }*/
        d->m_semantic = true;
    }

//    if (config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_VECTOR, true)) {
//        if (paser.connectToVector(SemanticHelper::vectorServiceName())) {
//            d->m_vector = paser.isVectorSupported();
//        }
//    }

    d->m_fulltext = config->value(GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC_FULLTEXT, false);

    return d->m_semantic;
}

bool SemanticSearcher::activate()
{
    return false;
}

ProxyWorker *SemanticSearcher::createWorker() const
{
    auto worker = new SemanticWorker(name());
    worker->setEngineState(d->m_semantic, d->m_vector, d->m_fulltext);
    return worker;
}

bool SemanticSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item)
    qWarning() << "no such action:" << action << ".";
    return false;
}
