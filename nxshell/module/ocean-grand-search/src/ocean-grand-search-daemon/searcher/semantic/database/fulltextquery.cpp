// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fulltextquery_p.h"

#include "utils/specialtools.h"
#include "global/searchhelper.h"
#include "global/builtinsearch.h"

using namespace GrandSearch;

FullTextQueryPrivate::FullTextQueryPrivate(FullTextQuery *qq) : q(qq)
{

}

bool FullTextQueryPrivate::processResult(const QString &file, void *pdata, void *ctx)
{
    FullTextQueryPrivate::Context *context = static_cast<FullTextQueryPrivate::Context *>(pdata);
    Q_ASSERT(context);

    auto d = context->query->d;

    // 检查是否需要推送数据与中断
    if (d->timeToPush()) {
        auto it = std::move(d->m_results);
        bool ret = context->callBack(it, context->callBackData);
        // 有数据放入，更新时间
        if (!it.isEmpty())
            d->m_lastPush = d->m_time.elapsed();

        // 中断
        if (!ret)
            return false;
    }

   QFileInfo info(file);
   // 检查后缀
   if (!context->suffix.isEmpty() && !context->suffix.contains(info.suffix(), Qt::CaseInsensitive))
       return true;

   // 检查时间
   if (!SemanticHelper::isMatchTime(info.lastModified().toSecsSinceEpoch(), context->times))
       return true;

   // 过滤文管设置的隐藏文件
   QHash<QString, QSet<QString>> hioceannFilters;
   if (SpecialTools::isHiddenFile(file, hioceannFilters, QDir::homePath()))
       return true;

   d->m_count++;
   if (d->m_handler) {
       d->m_handler->appendTo(file, d->m_results);
       if (ctx && context->eng) {
           auto keys = context->eng->matchedKeys(ctx);
           d->m_handler->setItemWeight(file, d->m_handler->itemWeight(file) + d->matchedWeight(keys));
       }
       if (d->m_handler->isResultLimit() || d->m_count > 100)
           return false;
   } else {
       auto item = FileSearchUtils::packItem(file, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
       d->m_results[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
   }

   return true;
}

bool FullTextQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

double FullTextQueryPrivate::matchedWeight(const QSet<QString> &back)
{
    double w = 0;
    QSet<QString> mergedKeys;
    for (const SemanticEntity &e: m_entity) {
        auto keys = e.keys;
        for (const QString &key : keys)
            mergedKeys.insert(key);
    }

    for (const QString &str : back) {
        if (mergedKeys.isEmpty())
            break;
        for (auto it = mergedKeys.begin(); it != mergedKeys.end();) {
            if (str.contains(*it, Qt::CaseInsensitive)) {
                it = mergedKeys.erase(it);
                w += 10;
            } else {
                ++it;
            }
        }
    }
    return w;
}

FullTextQuery::FullTextQuery(QObject *parent)
    : QObject(parent)
    , d(new FullTextQueryPrivate(this))
{

}

FullTextQuery::~FullTextQuery()
{
    delete d;
    d = nullptr;
}

void FullTextQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    Q_ASSERT(callBack);

    FullTextQuery *self = static_cast<FullTextQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;
    qDebug() << "query by fulltext" << d->m_entity.size();

    FullTextEngine engine;
    if (!engine.init(d->indexStorePath()))
        return;

    d->m_time.start();

    for (const SemanticEntity &entity : d->m_entity) {
        FullTextQueryPrivate::Context context;
        context.eng = &engine;
        context.query = self;
        context.callBack = callBack;
        context.callBackData = pdata;
        context.suffix = SemanticHelper::typeTosuffix(entity.types);
        if (!entity.suffix.isEmpty())
            context.suffix.append(entity.suffix);

        context.times = entity.times;

        QString path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

        engine.query(path, entity.keys, &FullTextQueryPrivate::processResult, &context);
    }

    callBack(d->m_results, pdata);

    qDebug() << "fulltext is finished spend:" << d->m_time.elapsed() << "found:" << d->m_count;
}

void FullTextQuery::setEntity(const QList<GrandSearch::SemanticEntity> &entity)
{
    d->m_entity.clear();
    for (const SemanticEntity &e : entity) {
        if (e.keys.isEmpty())
            continue;

        d->m_entity.append(e);
    }
}

void FullTextQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}

