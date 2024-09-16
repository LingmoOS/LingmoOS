// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "featurequery_p.h"
#include "utils/specialtools.h"
#include "searcher/semantic/semantichelper.h"
#include "global/searchhelper.h"
#include "global/builtinsearch.h"

using namespace GrandSearch;

Q_DECLARE_METATYPE(FeatureLibEngine::QueryConditons)

FeatureQueryPrivate::FeatureQueryPrivate(FeatureQuery *qq) : q(qq)
{

}

bool FeatureQueryPrivate::processResult(const QString &file, const QSet<QString> &match, void *pdata)
{
    FeatureQueryPrivate::Context *ctx = static_cast<FeatureQueryPrivate::Context *>(pdata);
    Q_ASSERT(ctx);

    auto d = ctx->query->d;
    // 检查是否需要推送数据与中断
    if (d->timeToPush()) {
        auto it = std::move(d->m_results);
        bool ret = ctx->callBack(it, ctx->callBackData);
        // 有数据放入，更新时间
        if (!it.isEmpty())
            d->m_lastPush = d->m_time.elapsed();

        // 中断
        if (!ret)
            return false;
    }

    // 过滤文管设置的隐藏文件
    QHash<QString, QSet<QString>> hiddenFilters;
    if (SpecialTools::isHiddenFile(file, hiddenFilters, QDir::homePath()))
        return true;

    d->m_count++;
    if (d->m_handler) {
        d->m_handler->appendTo(file, d->m_results);
        d->m_handler->setItemWeight(file, d->m_handler->itemWeight(file) + d->matchedWeight(match));
        if (d->m_handler->isResultLimit() || d->m_count >= 100)
            return false;
    } else {
        auto item = FileSearchUtils::packItem(file, GRANDSEARCH_CLASS_GENERALFILE_SEMANTIC);
        d->m_results[GRANDSEARCH_GROUP_FILE_INFERENCE].append(item);
    }

    return true;
}

FeatureLibEngine::QueryConditons FeatureQueryPrivate::translateConditons()
{
    FeatureLibEngine::QueryConditons cond;
    if (m_entity.keys.isEmpty())
        return cond;

    // 图片
    if (m_entity.types.contains(PICTURE_GROUP)) {
        FeatureLibEngine::QueryConditons tmp;
        QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(PICTURE_GROUP);
        tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, suffix));
        tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
        tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Text, m_entity.keys));

        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(tmp)));
    }

    // 音乐
    if (m_entity.types.contains(AUDIO_GROUP)) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

        FeatureLibEngine::QueryConditons tmp;
        QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(AUDIO_GROUP);
        tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, suffix));

        {
            FeatureLibEngine::QueryConditons subTmp;
            subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Author, m_entity.keys));
            subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));
            subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Album, m_entity.keys));

            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(subTmp)));
        }

        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(tmp)));
    }

    if (cond.isEmpty())
        return cond;

#if 0
    // 视频
    if (m_entity.types.contains(VIDEO_GROUP)) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

        QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(VIDEO_GROUP);
        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, suffix));
    }

    // 文档
    if (m_entity.types.contains(DOCUMENT_GROUP)) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

        QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(DOCUMENT_GROUP);
        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, suffix));
    }
#endif

    // 修改时间
    if (!m_entity.times.isEmpty()) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::ModifiedTime, QVariant::fromValue(m_entity.times)));
    }

    return cond;
}

bool FeatureQueryPrivate::timeToPush() const
{
    return (m_time.elapsed() - m_lastPush) > 100;
}

double FeatureQueryPrivate::matchedWeight(const QSet<QString> &back)
{
    double w = 0;
    auto keys = m_entity.keys;
    for (const QString &str : back) {
        if (keys.isEmpty())
            break;
        for (auto it = keys.begin(); it != keys.end();) {
            if (str.contains(*it, Qt::CaseInsensitive)) {
                it = keys.erase(it);
                w += 15;
            } else {
                ++it;
            }
        }
    }
    return w;
}

FeatureQuery::FeatureQuery(QObject *parent)
    : QObject(parent)
    , d(new FeatureQueryPrivate(this))
{

}

FeatureQuery::~FeatureQuery()
{
    delete d;
    d = nullptr;
}

void FeatureQuery::run(void *ptr, PushItemCallBack callBack, void *pdata)
{
    qDebug() << "query by feature library";
    Q_ASSERT(callBack);

    FeatureQuery *self = static_cast<FeatureQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;

    FeatureLibEngine engine;
    if (!engine.init(d->indexStorePath()))
        return;

    auto cond = d->translateConditons();
    if (cond.isEmpty()) {
        qInfo() << "no valid condition to query.";
        return;
    }

    QString path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

    FeatureQueryPrivate::Context ctx;
    ctx.callBack = callBack;
    ctx.callBackData = pdata;
    ctx.query = self;

    d->m_time.start();
    engine.query(path, cond, &FeatureQueryPrivate::processResult, &ctx);

    callBack(d->m_results, pdata);

    qDebug() << "feature is finished spend:" << d->m_time.elapsed() << "found:" << d->m_count;;
}

void FeatureQuery::setEntity(const SemanticEntity &entity)
{
    d->m_entity = entity;
}

void FeatureQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}
