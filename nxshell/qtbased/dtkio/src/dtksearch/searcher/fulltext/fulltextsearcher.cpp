// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <dirent.h>
#include <docparser.h>

#include <QDir>
#include <QMetaEnum>
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>

#include <FileUtils.h>
#include <QueryWrapperFilter.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>

#include "fulltextsearcher.h"
#include "3rdparty/fulltext/chineseanalyzer.h"
#include "utils.h"

using namespace Lucene;
DSEARCH_BEGIN_NAMESPACE

FullTextSearcher::FullTextSearcher(const QVariantMap &options, QObject *parent)
    : AbstractSearcher(options, parent)
{
    m_indexPath += "/fulltext";
    bindPathTable = Utils::fstabBindInfo();
}

void FullTextSearcher::search()
{
    if (!status.testAndSetRelease(Ready, Runing))
        return;

    const QString &key = dealKeyword(m_keyword);
    if (m_searchPath.isEmpty() || key.isEmpty()) {
        status.storeRelease(Completed);
        return;
    }

    updateIndex(m_searchPath);
    doSearch(m_searchPath, key);

    if (status.testAndSetRelease(Runing, Completed)) {
        // check results and send
        if (hasItem())
            emit unearthed(this);
    }
}

void FullTextSearcher::stop()
{
    status.storeRelease(Terminated);
}

bool FullTextSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QStringList FullTextSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

bool FullTextSearcher::createIndex(const QString &path)
{
    QDir dir;
    if (!dir.exists(path)) {
        qWarning() << "Source directory doesn't exist: " << path;
        status.storeRelease(AbstractSearcher::Completed);
        return false;
    }

    if (!dir.exists(m_indexPath)) {
        if (!dir.mkpath(m_indexPath)) {
            qWarning() << "Unable to create directory: " << m_indexPath;
            status.storeRelease(AbstractSearcher::Completed);
            return false;
        }
    }

    try {
        // record spending
        QElapsedTimer timer;
        timer.start();
        IndexWriterPtr writer = newIndexWriter(true);
        qDebug() << "Indexing to directory: " << m_indexPath;
        writer->deleteAll();
        doIndexTask(nullptr, writer, path, Create);
        writer->optimize();
        writer->close();

        qInfo() << "create index spending: " << timer.elapsed();
        status.storeRelease(AbstractSearcher::Completed);
        return true;
    } catch (const LuceneException &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString(e.what());
    } catch (...) {
        qWarning() << "Error: " << __FUNCTION__;
    }

    status.storeRelease(AbstractSearcher::Completed);
    return false;
}

bool FullTextSearcher::updateIndex(const QString &path)
{
    bool indexExists = IndexReader::indexExists(FSDirectory::open(m_indexPath.toStdWString()));
    if (!indexExists)
        return createIndex(path);

    try {
        IndexReaderPtr reader = newIndexReader();
        IndexWriterPtr writer = newIndexWriter();

        doIndexTask(reader, writer, path, Update);

        writer->close();
        reader->close();

        return true;
    } catch (const LuceneException &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString(e.what());
    } catch (...) {
        qWarning() << "Error: " << __FUNCTION__;
    }

    return false;
}

Lucene::IndexWriterPtr Dtk::Search::FullTextSearcher::newIndexWriter(bool create)
{
    return newLucene<IndexWriter>(FSDirectory::open(m_indexPath.toStdWString()),
                                  newLucene<ChineseAnalyzer>(),
                                  create,
                                  IndexWriter::MaxFieldLengthLIMITED);
}

Lucene::IndexReaderPtr FullTextSearcher::newIndexReader()
{
    return IndexReader::open(FSDirectory::open(m_indexPath.toStdWString()), true);
}

DocumentPtr FullTextSearcher::fileDocument(const QString &file)
{
    QString contents = DocParser::convertFile(file.toStdString()).c_str();
    if (contents.isEmpty())
        return nullptr;

    DocumentPtr doc = newLucene<Document>();
    // file path
    doc->add(newLucene<Field>(L"path", file.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file last modified time
    QFileInfo info(file);
    QDateTime lastModified = info.lastModified();
    QString modifyTime = QString::number(lastModified.toSecsSinceEpoch());
    doc->add(newLucene<Field>(L"modified", modifyTime.toStdWString(), Field::STORE_YES, Field::INDEX_NOT_ANALYZED));

    // file contents
    doc->add(newLucene<Field>(L"contents", contents.toStdWString(), Field::STORE_YES, Field::INDEX_ANALYZED));

    return doc;
}

void FullTextSearcher::doSearch(const QString &path, const QString &keyword)
{
    qInfo() << "search path: " << path << " keyword: " << keyword;
    notifyTimer.start();

    bool hasTransform = false;
    QString searchPath = Utils::convertTo(path, Utils::DirType);
    if (searchPath != path)
        hasTransform = true;

    try {
        IndexWriterPtr writer = newIndexWriter();
        IndexReaderPtr reader = newIndexReader();
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        AnalyzerPtr analyzer = newLucene<ChineseAnalyzer>();
        QueryParserPtr parser = newLucene<QueryParser>(LuceneVersion::LUCENE_CURRENT, L"contents", analyzer);
        // set the first `*` can be matched
        parser->setAllowLeadingWildcard(true);
        QueryPtr query = parser->parse(keyword.toStdWString());

        // create query filter
        String filterPath = searchPath.endsWith("/") ? (searchPath + "*").toStdWString() : (searchPath + "/*").toStdWString();
        FilterPtr filter = newLucene<QueryWrapperFilter>(newLucene<WildcardQuery>(newLucene<Term>(L"path", filterPath)));

        // search
        TopDocsPtr topDocs = searcher->search(query, filter, static_cast<int32_t>(m_maxResult));
        Collection<ScoreDocPtr> scoreDocs = topDocs->scoreDocs;

        QHash<QString, QSet<QString>> hiddenFileHash;
        for (auto scoreDoc : scoreDocs) {
            // stop
            if (status.loadAcquire() != AbstractSearcher::Runing)
                return;

            DocumentPtr doc = searcher->doc(scoreDoc->doc);
            String resultPath = doc->get(L"path");

            if (!resultPath.empty()) {
                QFileInfo info(QString::fromStdWString(resultPath));
                // delete invalid index
                if (!info.exists()) {
                    indexDocuments(writer, info.absoluteFilePath(), DeleteIndex);
                    continue;
                }
                QDateTime lastModified = info.lastModified();
                const QString &modifyTime = QString::number(lastModified.toSecsSinceEpoch());
                const String &storeTime = doc->get(L"modified");
                if (modifyTime.toStdWString() != storeTime) {
                    continue;
                } else {
                    if (hasTransform)
                        resultPath.replace(0, static_cast<unsigned long>(searchPath.length()), path.toStdWString());

                    if (!m_filterFunc || !m_filterFunc(StringUtils::toUTF8(resultPath).c_str())) {
                        QMutexLocker lk(&mutex);
                        allResults.append(StringUtils::toUTF8(resultPath).c_str());
                    }

                    // notify
                    tryNotify();
                }
            }
        }

        reader->close();
        writer->close();
    } catch (const LuceneException &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString::fromStdWString(e.getError());
    } catch (const std::exception &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString(e.what());
    } catch (...) {
        qWarning() << "Error: " << __FUNCTION__;
    }
}

void FullTextSearcher::doIndexTask(const IndexReaderPtr &reader, const IndexWriterPtr &writer, const QString &path, FullTextSearcher::TaskType type)
{
    if (status.loadAcquire() != AbstractSearcher::Runing)
        return;

    // filter some folders
    static QRegularExpression reg(Utils::kNotSupportDirectories);
    if (bindPathTable.contains(path) || (reg.match(path).hasMatch() && !path.startsWith("/run/user")))
        return;

    // limit file name length and level
    if (path.size() > FILENAME_MAX - 1 || path.count('/') > 20)
        return;

    const std::string tmp = path.toStdString();
    const char *filePath = tmp.c_str();
    DIR *dir = nullptr;
    if (!(dir = opendir(filePath))) {
        qWarning() << "can not open: " << path;
        return;
    }

    struct dirent *dent = nullptr;
    char fn[FILENAME_MAX] = { 0 };
    strcpy(fn, filePath);
    size_t len = strlen(filePath);
    if (strcmp(filePath, "/"))
        fn[len++] = '/';

    // traverse
    while ((dent = readdir(dir)) && status.loadAcquire() == AbstractSearcher::Runing) {
        if (dent->d_name[0] == '.' || !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
            continue;

        struct stat st;
        strncpy(fn + len, dent->d_name, FILENAME_MAX - len);
        if (lstat(fn, &st) == -1)
            continue;

        const bool is_dir = S_ISDIR(st.st_mode);
        if (is_dir) {
            doIndexTask(reader, writer, fn, type);
        } else {
            QFileInfo info(fn);
            QString suffix = info.suffix();
            static QRegularExpression suffixRegExp(Utils::kSupportFiles);
            if (suffixRegExp.match(suffix).hasMatch()) {
                switch (type) {
                case Create:
                    indexDocuments(writer, fn, AddIndex);
                    break;
                case Update:
                    IndexType type;
                    if (checkUpdate(reader, fn, type)) {
                        indexDocuments(writer, fn, type);
                    }
                    break;
                }
            }
        }
    }

    if (dir)
        closedir(dir);
}

void FullTextSearcher::indexDocuments(const IndexWriterPtr &writer, const QString &file, FullTextSearcher::IndexType type)
{
    try {
        switch (type) {
        case AddIndex: {
            qDebug() << "Adding [" << file << "]";
            auto doc = fileDocument(file);
            if (doc)
                writer->addDocument(doc);
            break;
        }
        case UpdateIndex: {
            qDebug() << "Update file: [" << file << "]";
            auto doc = fileDocument(file);
            if (doc) {
                TermPtr term = newLucene<Term>(L"path", file.toStdWString());
                writer->updateDocument(term, doc);
            }
            break;
        }
        case DeleteIndex: {
            qDebug() << "Delete file: [" << file << "]";
            TermPtr term = newLucene<Term>(L"path", file.toStdWString());
            writer->deleteDocuments(term);
            break;
        }
        }
    } catch (const LuceneException &e) {
        QMetaEnum enumType = QMetaEnum::fromType<IndexType>();
        qWarning() << QString::fromStdWString(e.getError())
                   << " type: " << enumType.valueToKey(type)
                   << " file: " << file;
    } catch (const std::exception &e) {
        QMetaEnum enumType = QMetaEnum::fromType<IndexType>();
        qWarning() << QString(e.what())
                   << " type: " << enumType.valueToKey(type)
                   << " file: " << file;
    } catch (...) {
        qWarning() << "Error: " << __FUNCTION__ << file;
    }
}

bool FullTextSearcher::checkUpdate(const IndexReaderPtr &reader, const QString &file, FullTextSearcher::IndexType &type)
{
    try {
        SearcherPtr searcher = newLucene<IndexSearcher>(reader);
        TermQueryPtr query = newLucene<TermQuery>(newLucene<Term>(L"path", file.toStdWString()));

        // 文件路径为唯一值，所以搜索一个结果就行了
        TopDocsPtr topDocs = searcher->search(query, 1);
        int32_t numTotalHits = topDocs->totalHits;
        if (numTotalHits == 0) {
            type = AddIndex;
            return true;
        } else {
            DocumentPtr doc = searcher->doc(topDocs->scoreDocs[0]->doc);
            QFileInfo info(file);
            QDateTime lastModified = info.lastModified();
            const QString &modifyTime = QString::number(lastModified.toSecsSinceEpoch());
            const String &storeTime = doc->get(L"modified");

            if (modifyTime.toStdWString() != storeTime) {
                type = UpdateIndex;
                return true;
            }
        }
    } catch (const LuceneException &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString::fromStdWString(e.getError()) << " file: " << file;
    } catch (const std::exception &e) {
        qWarning() << "Error: " << __FUNCTION__ << QString(e.what()) << " file: " << file;
    } catch (...) {
        qWarning() << "Error: " << __FUNCTION__ << " file: " << file;
    }

    return false;
}

QString FullTextSearcher::dealKeyword(const QString &keyword)
{
    static QRegularExpression cnReg("^[\u4e00-\u9fa5]");
    static QRegularExpression enReg("^[A-Za-z]+$");
    static QRegularExpression numReg("^[0-9]$");

    QString oldType = "cn", currType = "cn";
    QString newStr;
    for (auto c : keyword) {
        if (cnReg.match(c).hasMatch()) {
            currType = "cn";
        } else if (enReg.match(c).hasMatch()) {
            currType = "en";
        } else if (numReg.match(c).hasMatch()) {
            currType = "digit";
        } else {
            newStr += ' ';
            currType = "symbol";
            continue;
        }

        newStr += c;
        if (oldType == "symbol") {
            oldType = currType;
            continue;
        }

        if (oldType != currType) {
            oldType = currType;
            newStr.insert(newStr.length() - 1, " ");
        }
    }

    return newStr.trimmed();
}

void FullTextSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > Utils::kEmitInterval) {
        lastEmit = cur;
        qDebug() << "unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

DSEARCH_END_NAMESPACE
