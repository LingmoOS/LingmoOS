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
    QHash<QString, QSet<QString>> hioceannFilters;
    if (SpecialTools::isHiddenFile(file, hioceannFilters, QDir::homePath()))
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

FeatureLibEngine::QueryConditons FeatureQueryPrivate::translateConditons(const SemanticEntity &entity)
{
    FeatureLibEngine::QueryConditons cond;
    if (entity.keys.isEmpty() && entity.author.isEmpty() && entity.album.isEmpty() && entity.duration.isEmpty() && entity.resolution.isEmpty())
        return cond;

    // 图片
    if (!entity.keys.isEmpty()) {
        const QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(PICTURE_GROUP);
        QStringList picSuffix;
        if (entity.types.contains(PICTURE_GROUP))
            picSuffix = suffix;

        if (!entity.suffix.isEmpty()) {
            picSuffix.clear();
            if (suffix.contains(entity.suffix, Qt::CaseInsensitive))
                picSuffix.append(entity.suffix.toLower());
        }

        if (!picSuffix.isEmpty()) {
            FeatureLibEngine::QueryConditons tmp;
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, picSuffix));
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Text, entity.keys));

            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(tmp)));
        }
    }

    // 音乐
    if (!(entity.keys.isEmpty() && entity.author.isEmpty() && entity.album.isEmpty() && entity.duration.isEmpty())) {
        const QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(AUDIO_GROUP);
        QStringList mscSuffix;
        if (entity.types.contains(AUDIO_GROUP))
            mscSuffix = suffix;

        if (!entity.suffix.isEmpty()) {
            mscSuffix.clear();;
            if (suffix.contains(entity.suffix, Qt::CaseInsensitive))
                mscSuffix.append(entity.suffix.toLower());
        }

        if (!mscSuffix.isEmpty()) {
            if (!cond.isEmpty())
                cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

            FeatureLibEngine::QueryConditons tmp;
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, mscSuffix));

            {
                FeatureLibEngine::QueryConditons subTmp;
                bool isFirst = true;
                if (!entity.author.isEmpty()) {
                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Author, entity.author));
                    isFirst = false;
                }
                if (!entity.album.isEmpty()) {
                    if (!isFirst) {
                        subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
                    }
                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Album, entity.album));
                    isFirst = false;
                }

                if (isFirst && !entity.keys.isEmpty()) {
                    FeatureLibEngine::QueryConditons guess;
                    guess.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Author, entity.keys.first()));
                    guess.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));
                    guess.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Album, entity.keys.first()));

                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(guess)));
                    isFirst = false;
                }

                if (!entity.duration.isEmpty()) {
                    if (!isFirst) {
                        subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
                    }
                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Duration, entity.duration));
                    isFirst = false;
                }

                tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
                tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(subTmp)));
            }

            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(tmp)));
        }
    }

    // 视频
    if (!(entity.duration.isEmpty() && entity.resolution.isEmpty())) {
        const QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(VIDEO_GROUP);
        QStringList vidSuffix;
        if (entity.types.contains(VIDEO_GROUP))
            vidSuffix = suffix;

        if (!entity.suffix.isEmpty()) {
            vidSuffix.clear();;
            if (suffix.contains(entity.suffix, Qt::CaseInsensitive))
                vidSuffix.append(entity.suffix.toLower());
        }

        if (!vidSuffix.isEmpty()) {
            if (!cond.isEmpty())
                cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

            FeatureLibEngine::QueryConditons tmp;
            tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, vidSuffix));

            {
                FeatureLibEngine::QueryConditons subTmp;
                bool isFirst = true;
                if (!entity.duration.isEmpty()) {
                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Duration, entity.duration));
                    isFirst = false;
                }
                if (!entity.resolution.isEmpty()) {
                    if (!isFirst) {
                        subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
                    }
                    subTmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Resolution, entity.resolution));
                    isFirst = false;
                }

                tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
                tmp.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(subTmp)));
            }

            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Composite, QVariant::fromValue(tmp)));
        }
    }

    if (cond.isEmpty())
        return cond;

#if 0
    // 文档
    if (entity.types.contains(DOCUMENT_GROUP)) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::Or));

        QStringList suffix = SearchHelper::instance()->getSuffixByGroupName(DOCUMENT_GROUP);
        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::FileType, suffix));
    }
#endif

    // 修改时间
    if (!entity.times.isEmpty()) {
        if (!cond.isEmpty())
            cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::And));
        cond.append(FeatureLibEngine::makeProperty(FeatureLibEngine::ModifiedTime, QVariant::fromValue(entity.times)));
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
    QSet<QString> mergedKeys;
    for (const SemanticEntity &e: m_entity) {
        auto keys = e.keys;
        for (const QString &key : keys)
            mergedKeys.insert(key);

        if (!e.album.isEmpty())
            mergedKeys.insert(e.album);

        if (!e.author.isEmpty())
            mergedKeys.insert(e.author);
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
    Q_ASSERT(callBack);

    FeatureQuery *self = static_cast<FeatureQuery *>(ptr);
    Q_ASSERT(self);

    auto d = self->d;
    qDebug() << "query by feature library" << d->m_entity.size();

    FeatureLibEngine engine;
    if (!engine.init(d->indexStorePath()))
        return;

    QString path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();

    d->m_time.start();
    for (const SemanticEntity &entity : d->m_entity) {
        auto cond = d->translateConditons(entity);
        if (cond.isEmpty()) {
            qInfo() << "no valid condition to query.";
            continue;
        }

        FeatureQueryPrivate::Context ctx;
        ctx.callBack = callBack;
        ctx.callBackData = pdata;
        ctx.query = self;

        engine.query(path, cond, &FeatureQueryPrivate::processResult, &ctx);
    }

    callBack(d->m_results, pdata);
    qDebug() << "feature is finished spend:" << d->m_time.elapsed() << "found:" << d->m_count;
}

void FeatureQuery::setEntity(const QList<SemanticEntity> &entity)
{
    d->m_entity.clear();
    for (const SemanticEntity &e : entity) {
        if (e.keys.isEmpty() && e.author.isEmpty() && e.album.isEmpty() && e.duration.isEmpty() && e.resolution.isEmpty())
            continue;

        d->m_entity.append(e);
    }
}

void FeatureQuery::setFileHandler(FileResultsHandler *handler)
{
    d->m_handler = handler;
}
