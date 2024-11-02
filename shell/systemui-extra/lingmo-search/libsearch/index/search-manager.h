/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */
#ifndef SEARCHMANAGER_H
#define SEARCHMANAGER_H

#include <QObject>
#include <xapian.h>
#include <QStandardPaths>
#include <QVector>
#include <QMap>
#include <QQueue>
#include <QPair>
#include <QMutex>
#include <thread>
#include <QRunnable>
#include <QThreadPool>
#include <QFileInfo>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QThread>
#include <QUrl>
/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#include "search-plugin-iface.h"
#include "file-utils.h"
#include "global-settings.h"
#include "chinese-segmentation.h"
#include "common.h"

#define INDEX_PATH (QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/org.lingmo/lingmo-search/index_data").toStdString()
#define CONTENT_INDEX_PATH (QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/org.lingmo/lingmo-search/content_index_data").toStdString()
#define OCR_CONTENT_INDEX_PATH (QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+"/.config/org.lingmo/lingmo-search/ocr_content_index_data").toStdString()
namespace LingmoUISearch {
class FileMatchDecider;
class FileContentMatchDecider;
class OcrMatchDecider;
class LIBSEARCH_EXPORT SearchManager : public QObject {
    friend class FileSearch;
    friend class FileContentSearch;
    friend class OcrSearch;
    friend class DirectSearch;
    friend class FileMatchDecider;
    friend class FileContentMatchDecider;
    friend class OcrMatchDecider;
    Q_OBJECT
public:
    explicit SearchManager(QObject *parent = nullptr);
    ~SearchManager();

    static uint getCurrentIndexCount();

    static size_t uniqueSymbolFile;
    static size_t uniqueSymbolDir;
    static size_t uniqueSymbolContent;
    static size_t uniqueSymbolOcr;
    static QMutex m_mutexFile;
    static QMutex m_mutexDir;
    static QMutex m_mutexContent;
    static QMutex m_mutexOcr;

private:
    static bool isBlocked(QString &path);
    static bool creatResultInfo(LingmoUISearch::SearchPluginIface::ResultInfo &ri, const QString& path);
};

class FileSearch : public QRunnable {
public:
    explicit FileSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, QString keyword, QString value, unsigned slot = 1, int begin = 0, int num = 20);
    ~FileSearch();
protected:
    void run();
private:
    uint keywordSearchFile();
    Xapian::Query creatQueryForFileSearch();
    int getResult(Xapian::MSet &result);

    DataQueue<SearchPluginIface::ResultInfo> *m_search_result = nullptr;
    FileMatchDecider *m_matchDecider;
    QString m_value;
    unsigned m_slot = 1;
    size_t m_uniqueSymbol;
    QString m_keyword;
    int m_begin = 0;
    int m_num = 20;
};

class FileContentSearch : public QRunnable {
public:
    explicit FileContentSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, size_t uniqueSymbol, QString keyword, bool fuzzy,int begin = 0, int num = 20);
    ~FileContentSearch();
protected:
    void run();
private:
    uint keywordSearchContent();
    int getResult(Xapian::MSet &result, std::string &keyWord);

    DataQueue<SearchPluginIface::ResultInfo> *m_search_result = nullptr;
    FileContentMatchDecider *m_matchDecider;
    size_t m_uniqueSymbol;
    QString m_keyword;
    bool m_fuzzy;
    int m_begin = 0;
    int m_num = 20;
};

class DirectSearch : public QRunnable {
public:
    explicit DirectSearch(QString keyword, DataQueue<SearchPluginIface::ResultInfo> *searchResult, QString value, size_t uniqueSymbol);
protected:
    void run();
private:
    void match(const QFileInfo& info);
    QString m_keyword;
    DataQueue<SearchPluginIface::ResultInfo>* m_searchResult = nullptr;
    size_t m_uniqueSymbol;
    QString m_value;
};

class FileMatchDecider : public Xapian::MatchDecider {
public:
    bool operator ()(const Xapian::Document &doc) const;
};
class FileContentMatchDecider : public Xapian::MatchDecider {
public:
    bool operator ()(const Xapian::Document &doc) const;
};

}
#endif // SEARCHMANAGER_H
