// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDebug>
#include <QDirIterator>

#include "iteratorsearcher.h"
#include "utils.h"

DSEARCH_BEGIN_NAMESPACE

IteratorSearcher::IteratorSearcher(const QVariantMap &options, QObject *parent)
    : AbstractSearcher(options, parent)
{
    searchPathList << m_searchPath;
    m_keyword = Utils::checkWildcardAndToRegularExpression(m_keyword);
    regex = QRegularExpression(m_keyword, QRegularExpression::CaseInsensitiveOption);
}

IteratorSearcher::~IteratorSearcher()
{
}

void IteratorSearcher::search()
{
    if (!status.testAndSetRelease(Ready, Runing))
        return;

    notifyTimer.start();
    doSearch();

    if (status.testAndSetRelease(Runing, Completed)) {
        // send results
        if (hasItem())
            emit unearthed(this);
    }
}

void IteratorSearcher::stop()
{
    status.storeRelease(Terminated);
}

bool IteratorSearcher::hasItem() const
{
    QMutexLocker lk(&mutex);
    return !allResults.isEmpty();
}

QStringList IteratorSearcher::takeAll()
{
    QMutexLocker lk(&mutex);
    return std::move(allResults);
}

void IteratorSearcher::tryNotify()
{
    int cur = notifyTimer.elapsed();
    if (hasItem() && (cur - lastEmit) > Utils::kEmitInterval) {
        lastEmit = cur;
        qDebug() << "IteratorSearcher unearthed, current spend:" << cur;
        emit unearthed(this);
    }
}

void IteratorSearcher::doSearch()
{
    forever {
        if (searchPathList.isEmpty() || resultCount >= m_maxResult || status.loadAcquire() != Runing)
            return;

        const auto &path = searchPathList.takeAt(0);
        QRegularExpression reg(Utils::kNotSupportDirectories);
        // The filter directory can be searched only when
        // searching in the filter directory
        if (!reg.match(m_searchPath).hasMatch() && reg.match(path).hasMatch())
            continue;

        QDirIterator iterator(path, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
        while (iterator.hasNext() && resultCount < m_maxResult) {
            if (status.loadAcquire() != Runing)
                return;

            iterator.next();
            auto info = iterator.fileInfo();
            if (!info.exists())
                continue;

            // add dir path to 'searchPathList'
            if (info.isDir() && !info.isSymLink()) {
                const auto &dirPath = info.absoluteFilePath();
                if (!searchPathList.contains(dirPath) || !dirPath.startsWith("/sys/"))
                    searchPathList << dirPath;
            }

            const auto &filename = info.fileName();
            QRegularExpressionMatch match = regex.match(filename);
            bool hasMatched = match.hasMatch();

            if (!hasMatched && m_flags.testFlag(SearchFlag::PinYin)) {
                const auto &pinyinName = Utils::Chinese2Pinyin(filename);
                match = regex.match(pinyinName);
                hasMatched = match.hasMatch();
            }

            if (hasMatched) {
                const auto &filePath = info.absoluteFilePath();
                if (!m_filterFunc || !m_filterFunc(filePath)) {
                    ++resultCount;
                    QMutexLocker lk(&mutex);
                    allResults << filePath;
                }

                // notify
                tryNotify();
            }
        }
    }
}

DSEARCH_END_NAMESPACE
