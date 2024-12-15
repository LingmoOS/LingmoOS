// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "semanticworker_p.h"
#include "global/builtinsearch.h"
#include "semanticparser/semanticparser.h"
#include "dslparser/dslparser.h"
#include "semantichelper.h"
#include "fileresultshandler.h"

#include "database/anythingquery.h"
#include "database/fulltextquery.h"
#include "database/featurequery.h"
#include "database/vectorquery.h"

#include <QDebug>
#include <QtConcurrent>

using namespace GrandSearch;

#define checkRuning() \
if (d->m_status.loadAcquire() != ProxyWorker::Runing) \
    return false

SemanticWorkerPrivate::SemanticWorkerPrivate(SemanticWorker *parent) : q(parent)
{

}

SemanticWorkerPrivate::~SemanticWorkerPrivate()
{

}

//void SemanticWorkerPrivate::tryNotify()
//{
//    int cur = m_time.elapsed();
//    if (q->hasItem() && (cur - m_lastEmit) > 50) {
//        m_lastEmit = cur;
//        qDebug() << "semantic unearthed, current spend:" << cur;
//        emit q->unearthed(q);
//    }
//}

bool SemanticWorkerPrivate::pushItem(const MatchedItemMap &items, void *ptr)
{
    SemanticWorkerPrivate *d =  static_cast<SemanticWorkerPrivate *>(ptr);
    if (d->m_status.loadAcquire() == ProxyWorker::Terminated)
        return false;

    // lock and append item
    //bool hasItem = false;
    {
        QMutexLocker lk(&d->m_mtx);
        for(auto it = items.begin(); it != items.end(); ++it) {
            if (it.value().isEmpty())
                continue;
            d->m_items[it.key()].append(it.value());
        }
        //hasItem = !d->m_items.isEmpty();
    }
// 搜索完再推数据
//    if (hasItem)
//        d->tryNotify();

    return true;
}

void SemanticWorkerPrivate::run(const QueryFunction &func)
{
    func.function(func.object, &SemanticWorkerPrivate::pushItem, func.worker);
}

void SemanticWorkerPrivate::sortItems(MatchedItemMap &items, const QHash<QString, double> &weight)
{
    auto setExt = [](const MatchedItem &t, int w){
        if (t.extra.isValid() &&
            t.extra.toHash().contains(GRANDSEARCH_PROPERTY_ITEM_WEIGHT))
            return;

        auto var = t.extra.toHash();
        var.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, w);
        const_cast<MatchedItem *>(&t)->extra = QVariant::fromValue(var);
    };

    for (auto it = items.begin(); it != items.end(); ++it) {
        MatchedItems &list = it.value();

        if (list.size() == 1) {
            MatchedItem &item = list.first();
            setExt(item, weight.value(item.item, 0));
        } else {
            std::stable_sort(list.begin(), list.end(), [&weight, setExt](const MatchedItem &t1, const MatchedItem &t2) {
                double w1 = weight.value(t1.item, 0.0);
                double w2 = weight.value(t2.item, 0.0);
                setExt(t1, w1);
                setExt(t2, w2);

                if (w1 == w2)
                    return t1.name > t2.name;
                return w1 > w2;
            });
        }
    }
}

void SemanticWorkerPrivate::mergeExtra(MatchedItemMap &items, const QHash<QString, QVariantHash> &extra)
{
    for (auto it = items.begin(); it != items.end(); ++it) {
        MatchedItems &list = it.value();
        for (auto mi = list.begin(); mi != list.end(); ++mi) {
            auto ex = extra.find(mi->item);
            if (ex != extra.end()) {
                QVariantHash vh =  mi->extra.value<QVariantHash>();
                for (auto ev = ex->begin(); ev != ex->end(); ++ev)
                    vh.insert(ev.key(), ev.value());
                mi->extra = QVariant::fromValue(vh);
            }
        }
    }
}

SemanticWorker::SemanticWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent)
    , d(new SemanticWorkerPrivate(this))
{
}

SemanticWorker::~SemanticWorker()
{
    delete d;
    d = nullptr;
}

void SemanticWorker::setContext(const QString &context)
{
    d->m_context = context;
}

bool SemanticWorker::isAsync() const
{
    return false;
}

bool SemanticWorker::working(void *context)
{
    if (!d->m_status.testAndSetRelease(Ready, Runing))
        return false;

    // 发送空数据用于提前显示分组
    d->m_items.insert(GRANDSEARCH_GROUP_FILE_INFERENCE, {});
    emit unearthed(this);

    // 等待2秒确认无输入后再执行。
    int count = 10;
    while (count > 0) {
        checkRuning();
        QThread::msleep(200);
        count--;
    }

    // DSL
    d->m_time.start();
    SemanticParser parser;
    QList<SemanticEntity> entityList;
    bool canSemantic = false;

    if (d->m_doSemantic && parser.connectToQueryLang(SemanticHelper::querylangServiceName())) {
        checkRuning();
        // get AI engine output
        QString dslStr = parser.query(d->m_context);
        // 处理语法差异
        dslStr.replace("'", "\"");
        dslStr.replace("WITH META_VALUE", "AND META_VALUE");
        dslStr.replace("DIRECTORY_NAME IS", "PATH IS");
        qDebug() << QString("query(%1) => dsl(%2), spend(%3 ms)").arg(d->m_context).arg(dslStr).arg(d->m_time.elapsed());
        //dslStr = "((DATE >= CURRENT -\"5 minute\") AND (DATE <= \"CURRENT\")) AND (TYPE IS \"document\")";

        // parse DSL
        QList<SemanticWorkerPrivate::QueryFunction> querys;
        FileResultsHandler fileHandler;
        DslParser parser(dslStr, &querys, &fileHandler, d);
        entityList = parser.entityList();
        for (int i = 0; i < entityList.size(); i++) {
            qDebug() << "entityList" << i << entityList[i].toString();
        }
        canSemantic = !entityList.isEmpty();
    }
#if 0
        auto future = QtConcurrent::map(querys, &SemanticWorkerPrivate::run);
        future.waitForFinished();
        if (parser.getMatchedItems().isEmpty()) {
            qInfo() << "semantic worker is finished, total spend: " << d->m_time.elapsed() << " ms found: " << 0;
            return true;
        }

        d->m_items[GRANDSEARCH_GROUP_FILE_INFERENCE].append(parser.getMatchedItems());

        // Keep the top 100 files
        MatchedItemMap top;
        for (auto it = d->m_items.begin(); it != d->m_items.end(); ++it) {
            MatchedItems &items = it.value();
            if (items.size() > 100) {
                MatchedItems tmp;
                for (int i = 0; i < 100; ++i)
                    tmp.append(items[i]);
                top.insert(it.key(), tmp);
            } else {
                top.insert(it.key(), items);
            }
        }

        d->m_items = top;

        checkRuning();

        qInfo() << "semantic worker is finished, total spend: " << d->m_time.elapsed() << " ms found: " << d->m_items.begin().value().size();
        // 发送所有数据
        if (!d->m_items.isEmpty())
            emit unearthed(this);
        return true;
    }
    return false;
#else

    d->m_time.restart();

    QList<SemanticWorkerPrivate::QueryFunction> querys;
    FileResultsHandler fileHandler;

    AnythingQuery anything;
    if (canSemantic) {
        SemanticWorkerPrivate::QueryFunction func = {&anything, &AnythingQuery::run, d};
        querys.append(func);
        anything.setEntity(entityList);
        anything.setFileHandler(&fileHandler);
    }

    FullTextQuery fuletext;
    if (canSemantic && d->m_doFulltext) {
        SemanticWorkerPrivate::QueryFunction func = {&fuletext, &FullTextQuery::run, d};
        querys.append(func);
        fuletext.setEntity(entityList);
        fuletext.setFileHandler(&fileHandler);
    }

    FeatureQuery feature;
    if (canSemantic) {
        SemanticWorkerPrivate::QueryFunction func = {&feature, &FeatureQuery::run, d};
        querys.append(func);
        feature.setEntity(entityList);
        feature.setFileHandler(&fileHandler);
    }

    checkRuning();

    d->m_time.restart();
    auto future = QtConcurrent::map(querys, &SemanticWorkerPrivate::run);
    future.waitForFinished();

    // sort and set weight
    {
        QMutexLocker lk(&d->m_mtx);
        d->sortItems(d->m_items, fileHandler.allItemWeight());
        d->mergeExtra(d->m_items, fileHandler.allItemExtra());

        // Keep the top 100 files
        MatchedItemMap top;
        for (auto it = d->m_items.begin(); it != d->m_items.end(); ++it) {
            MatchedItems &items = it.value();
            if (items.size() > 100) {
                MatchedItems tmp;
                for (int i = 0; i < 100; ++i)
                    tmp.append(items[i]);
                top.insert(it.key(), items);
            } else {
                top.insert(it.key(), items);
            }
        }

        d->m_items = top;
    }

    checkRuning();

    qInfo() << "semantic worker is finished, total spend:" << d->m_time.elapsed() << "ms found:" << fileHandler.resultCount();
    // 发送所有数据
    if (!d->m_items.isEmpty())
        emit unearthed(this);

    return true;
#endif
}

void SemanticWorker::terminate()
{
    d->m_status.storeRelease(Terminated);
}

ProxyWorker::Status SemanticWorker::status()
{
    return static_cast<ProxyWorker::Status>(d->m_status.loadAcquire());
}

bool SemanticWorker::hasItem() const
{
    QMutexLocker lk(&d->m_mtx);
    return !d->m_items.isEmpty();
}

MatchedItemMap SemanticWorker::takeAll()
{
    QMutexLocker lk(&d->m_mtx);
    MatchedItemMap items = std::move(d->m_items);
    lk.unlock();

    return items;
}

void SemanticWorker::setEngineState(bool e, bool v, bool f)
{
    d->m_doSemantic = e;
    d->m_doVector = v;
    d->m_doFulltext = f;
}
