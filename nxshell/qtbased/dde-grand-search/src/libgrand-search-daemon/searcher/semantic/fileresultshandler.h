// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILERESULTSHANDLER_H
#define FILERESULTSHANDLER_H

#include "searcher/file/filesearchutils.h"

#include <QSet>

namespace GrandSearch {

class FileResultsHandler
{
public:
    explicit FileResultsHandler();
    bool appendTo(const QString &file, MatchedItemMap &container);
    bool isResultLimit() const;
    inline double itemWeight(const QString &file) const{
        QReadLocker lk(&m_lock);
        return m_resultWeight.value(file, 0);
    }

    inline void setItemWeight(const QString &file, double w) {
        QWriteLocker lk(&m_lock);
        m_resultWeight[file] = w;
    }

    inline QHash<QString, double> allItemWeight() const {
        QReadLocker lk(&m_lock);
        return m_resultWeight;
    }

    inline int resultCount() const {
        QReadLocker lk(&m_lock);
        return m_tmpSearchResults.size();
    }

    void appendExtra(const QString &file, const QString &key, const QVariant &value);
    inline QHash<QString, QVariantHash> allItemExtra() const {
        QReadLocker lk(&m_lock);
        return m_resultExtra;
    }
private:
    void initConfig();
protected:
    mutable QReadWriteLock m_lock;
    QSet<QString> m_tmpSearchResults;     // 存储所有的搜索结果，用于去重
    QHash<QString, double> m_resultWeight;
    QHash<QString, QVariantHash> m_resultExtra;
    QHash<FileSearchUtils::Group, quint32> m_resultCountHash; // 记录各类型文件搜索结果数量
};

}

#endif // FILERESULTSHANDLER_H
