// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vectorquery_p.h"
#include "utils/specialtools.h"
#include "semanticparser/semanticparser.h"
#include "global/builtinsearch.h"

#include <QJsonDocument>

using namespace GrandSearch;

VectorQueryPrivate::VectorQueryPrivate(VectorQuery *qq) : q(qq)
{

}

bool VectorQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

double VectorQueryPrivate::matchedWeight(qreal distance)
{
    if (distance < 0 || distance >= 2)
        return 0;

    return 80 * (1 - distance / 2.0);
}

VectorQuery::VectorQuery(QObject *parent)
    : QObject(parent)
    , d(new VectorQueryPrivate(this))
{

}

VectorQuery::~VectorQuery()
{
    delete d;
    d = nullptr;
}

void VectorQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    qDebug() << "query by vector index";
    Q_ASSERT(callBack);

    VectorQuery *self = static_cast<VectorQuery *>(ptr);
    Q_ASSERT(self);

    VectorQueryPrivate *d = self->d;
    d->m_time.start();

    auto vr = d->m_parser->vectorSearch(d->m_query);
    //qDebug() << "vector search result" << vr;

    QHash<QString, QSet<QString>> filter;
    QSet<QString> duped; // the result contaions duplicate files.

    QVariantHash varRet = QJsonDocument::fromJson(vr.toUtf8()).object().toVariantHash();
    auto jitems = varRet["result"].value<QVariantList>();

    MatchedItemMap items;
    for (auto varIt = jitems.begin(); varIt != jitems.end(); ++varIt) {
        // 检查是否需要推送数据与中断
        if (d->timeToPush()) {
            auto it = std::move(items);
            bool ret = callBack(it, pdata);
            // 有数据放入，更新时间
            if (!it.isEmpty())
                d->m_lastPush = d->m_time.elapsed();

            // 中断
            if (!ret)
                return;
        }

        QVariantHash hIt = varIt->value<QVariantHash>();
        QString absoluteFilePath = hIt["source"].toString();
        QString matchedInfo = hIt["content"].toString();
        qreal distance = hIt.value("distance", -1).toDouble();
        if (absoluteFilePath.isEmpty() || duped.contains(absoluteFilePath))
            continue;

        duped.insert(absoluteFilePath);

        // 过滤文管设置的隐藏文件
        if (!QFileInfo::exists(absoluteFilePath) ||
                SpecialTools::isHiddenFile(absoluteFilePath, filter, QDir::homePath()))
            continue;

        d->m_count++;
        if (d->m_handler) {
            d->m_handler->appendTo(absoluteFilePath, items);
            d->m_handler->setItemWeight(absoluteFilePath, d->m_handler->itemWeight(absoluteFilePath) + d->matchedWeight(distance));
            // 匹配的上下文
            if (!matchedInfo.isEmpty())
                d->m_handler->appendExtra(absoluteFilePath, GRANDSEARCH_PROPERTY_ITEM_MATCHEDCONTEXT, matchedInfo);
            if (d->m_handler->isResultLimit() || d->m_count >= 100)
                break;
        } else {
            auto item = FileSearchUtils::packItem(absoluteFilePath, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
            items[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
        }
    }

    callBack(items, pdata);
    return;
}

void VectorQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}

void VectorQuery::setParser(SemanticParser *paser)
{
    d->m_parser = paser;
}

void VectorQuery::setQuery(const QString &query)
{
    d->m_query = query;
}

