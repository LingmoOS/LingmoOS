// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef FULLTEXTSEARCHER_H
#define FULLTEXTSEARCHER_H

#include <LuceneHeaders.h>

#include <QElapsedTimer>
#include <QMutex>

#include "dtksearch_global.h"
#include "searcher/abstractsearcher.h"

DSEARCH_BEGIN_NAMESPACE

class FullTextSearcher : public AbstractSearcher
{
    Q_OBJECT

public:
    enum TaskType {
        Create,
        Update
    };

    enum IndexType {
        AddIndex,
        UpdateIndex,
        DeleteIndex
    };
    Q_ENUM(IndexType)

    explicit FullTextSearcher(const QVariantMap &options, QObject *parent = nullptr);

    void search() override;
    void stop() override;
    bool hasItem() const override;
    QStringList takeAll() override;

    bool createIndex(const QString &path);
    bool updateIndex(const QString &path);

private:
    Lucene::IndexWriterPtr newIndexWriter(bool create = false);
    Lucene::IndexReaderPtr newIndexReader();
    Lucene::DocumentPtr fileDocument(const QString &file);

    void doSearch(const QString &path, const QString &m_keyword);
    void doIndexTask(const Lucene::IndexReaderPtr &reader, const Lucene::IndexWriterPtr &writer, const QString &path, TaskType type);
    void indexDocuments(const Lucene::IndexWriterPtr &writer, const QString &file, IndexType type);
    bool checkUpdate(const Lucene::IndexReaderPtr &reader, const QString &file, IndexType &type);

    QString dealKeyword(const QString &m_keyword);
    void tryNotify();

private:
    QStringList allResults;
    mutable QMutex mutex;
    QMap<QString, QString> bindPathTable;
    QAtomicInt status = AbstractSearcher::Ready;

    QElapsedTimer notifyTimer;
    int lastEmit = 0;
};

DSEARCH_END_NAMESPACE

#endif   // FULLTEXTSEARCHER_H
