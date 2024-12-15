// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "featurelibengine_p.h"

#include "lucene/chineseanalyzer.h"

// Lucune++ headers
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>

#include <QFileInfo>
#include <QDebug>
#include <QDateTime>

using namespace GrandSearch;
using namespace Lucene;

Q_DECLARE_METATYPE(FeatureLibEngine::QueryConditons)

FeatureLibEnginePrivate::FeatureLibEnginePrivate(FeatureLibEngine *qq) : q(qq)
{

}

int FeatureLibEnginePrivate::conditonsToString(const FeatureLibEngine::QueryConditons &cond, QString &out) const
{
    int mul = 0;
    QString chain;
    for (const FeatureLibEngine::QueryProperty &prop : cond) {
        switch (prop.first) {
        case FeatureLibEngine::And:
            chain.append(" AND ");
            mul++;
            break;
        case FeatureLibEngine::Or:
            chain.append(" OR ");
            mul++;
            break;
        case FeatureLibEngine::Composite: {
            QString tmp;
            if (conditonsToString(prop.second.value<FeatureLibEngine::QueryConditons>(), tmp) > 0)
                tmp = QString("(%0)").arg(tmp);
            chain.append(tmp);
        }
            break;
        case FeatureLibEngine::Path:

            break;
        case FeatureLibEngine::FileType:
            chain.append(packageString("fileType", prop.second.toStringList()));
            break;
        case FeatureLibEngine::CreatedTime:

            break;
        case FeatureLibEngine::ModifiedTime:
            chain.append(packageTime("lastModified", prop.second.value<QList<QPair<qint64, qint64>>>()));
            break;
        case FeatureLibEngine::ReadTime:

            break;
        case FeatureLibEngine::Album:
            chain.append(packageString("Album", prop.second.toStringList()));
            break;
        case FeatureLibEngine::Author:
            chain.append(packageString("Author", prop.second.toStringList()));
            break;
        case FeatureLibEngine::Duration:
            chain.append(packageString("duration", prop.second.toStringList()));
            break;
        case FeatureLibEngine::Resolution:
            chain.append(packageString("resolution", prop.second.toStringList()));
            break;
        case FeatureLibEngine::Text:
            chain.append(packageString("contents", prop.second.toStringList()));
            break;
        default:
            break;
        }
    }

    out.append(chain);
    return mul;
}

QString FeatureLibEnginePrivate::packageString(const QString &key, const QStringList &value) const
{
    Q_ASSERT(!key.isEmpty());
    QString ret;
    const int size = value.size();
    int count = 0;
    for (int i = 0; i < size; ++i) {
        QString str = value.at(i).trimmed();
        if (str.isEmpty())
            continue;

        ++count;
        ret.append(QString("%0:%1").arg(key).arg(str));
        if (i != (size - 1))
            ret.append(" OR ");
    }

    // add () for multiple conditions
    if (count > 1)
        ret = QString("(%0)").arg(ret);

    return ret;
}

QString FeatureLibEnginePrivate::packageTime(const QString &key, const QList<QPair<qint64, qint64> > &value) const
{
    Q_ASSERT(!key.isEmpty());
    QString ret;
    const QString format("yyyyMMddhhmmss");
    const int size = value.size();
    int count = 0;
    for (int i = 0; i < size; ++i) {
        QPair<qint64, qint64> interval = value.at(i);
        if (interval.first < 0)
            interval.first = 0;

        if (interval.second < 0)
            interval.second = QDateTime::currentSecsSinceEpoch();

        ret.append(QString("%0:[%1 TO %2]").arg(key)
                   .arg(QDateTime::fromSecsSinceEpoch(interval.first).toString(format))
                   .arg(QDateTime::fromSecsSinceEpoch(interval.second).toString(format)));
        if (i != (size - 1))
            ret.append(" OR ");
        ++count;
    }
    // add () for multiple conditions
    if (count > 1)
        ret = QString("(%0)").arg(ret);
    return ret;
}

IndexReaderPtr FeatureLibEnginePrivate::createReader(const QString &cache)
{
    return IndexReader::open(FSDirectory::open(cache.toStdWString()), true);
}

FeatureLibEngine::FeatureLibEngine(QObject *parent)
    : QObject(parent)
    , d(new FeatureLibEnginePrivate(this))
{

}

FeatureLibEngine::~FeatureLibEngine()
{
    delete d;
    d = nullptr;
}

bool FeatureLibEngine::init(const QString &cache)
{
    QFileInfo info(cache);
    if (!info.isReadable()) {
        qWarning() << "the cahce file is not readable" << cache;
        return false;
    } else if (d->m_reader) {
        qCritical() << "duplicate initialization";
        return false;
    }
    qDebug() << "feature library cache" << cache;
    d->m_reader = d->createReader(cache);
    return d->m_reader.get() != nullptr;
}

void FeatureLibEngine::query(const QString &searchPath, const QueryConditons &cond, CheckAndPushItem func, void *pdata)
{
    if (d->m_reader.get() == nullptr || func == nullptr || searchPath.isEmpty())
        return;

    QString key;
    d->conditonsToString(cond, key);
    qDebug() << "feature search conditions" << key << "dir" << searchPath;
    if (key.isEmpty())
        return;

    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(d->m_reader);
        AnalyzerPtr analyzer = newLucene<ChineseAnalyzer>();
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);

        QueryPtr query = parser->parse(key.toStdWString());
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));
        TopDocsPtr topDocs = searcher->search(query, filter, 100);
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;

        // for get matched keys
#if 0
        FormatterPtr simple(new KeyFormatter);
        auto format = dynamic_cast<KeyFormatter *>(simple.get());
        QueryScorerPtr score(new QueryScorer(query));
        HighlighterPtr lighter(new Highlighter(simple, score));
        FragmenterPtr frag(new SimpleFragmenter(0));
        lighter->setTextFragmenter(frag);
#endif
        for (auto scoreDoc : scoreDocs) {
            DocumentPtr doc = searcher->doc(scoreDoc->doc);
            String resultPath = doc->get(L"path");
            QString filePath = StringUtils::toUTF8(resultPath).c_str();

            QSet<QString> match;
#if 0
            format->clear();
            lighter->getBestFragments(analyzer, L"contents", doc->get(L"contents"), 50);
            match = format->keys();
#endif
            if (!QFile::exists(filePath))
                continue;
            if (!func(filePath, match, pdata))
                return; // 中断
        }
    } catch (const LuceneException &e) {
        qWarning() << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        qWarning() << QString(e.what());
    } catch (...) {
        qWarning() << "Search failed! unkown error";
    }
}
