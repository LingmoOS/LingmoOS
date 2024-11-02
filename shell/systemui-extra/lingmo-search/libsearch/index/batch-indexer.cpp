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
#include "batch-indexer.h"
#include <QFileInfo>
#include <QElapsedTimer>
#include <QDebug>
#include <malloc.h>
#include <QQueue>
#include <QDateTime>
#include <QMetaEnum>
#include <QJsonArray>

#include "file-utils.h"
#include "basic-indexer.h"
#include "file-indexer-config.h"
#include "file-content-indexer.h"
#include "writable-database.h"
#include "compatible-define.h"
#include "ai-indexer.h"
using namespace LingmoUISearch;
BatchIndexer::BatchIndexer(const QStringList &folders, const QStringList &blackList,
                           QAtomicInt& indexStop,
                           QAtomicInt &contentIndexStop,
                           QAtomicInt &contentIndexOcrStop,
                           QAtomicInt& aiIndexStop,
                           WorkMode mode, Targets target)
    : m_folders(folders),
      m_blackList(blackList),
      m_indexStop(&indexStop),
      m_contentIndexStop(&contentIndexStop),
      m_contentIndexOcrStop(&contentIndexOcrStop),
      m_aiIndexStop(&aiIndexStop),
      m_mode(mode),
      m_target(target)
{
}

void BatchIndexer::run()
{
    QElapsedTimer timer;
    timer.start();
    if((!m_target.testFlag(Target::Basic) || m_indexStop->LOAD)
       && (!m_target.testFlag(Target::Content) || m_contentIndexStop->LOAD)
       && (!m_target.testFlag(Target::Ocr) || m_contentIndexOcrStop->LOAD)
       && (!m_target.testFlag(Target::Ai) || m_aiIndexStop->LOAD)) {
        Q_EMIT done(m_mode, m_target);
        return;
    }

    fetch();

    if(m_target & Target::Basic) {
        Q_EMIT basicIndexStart(m_mode);
        basicIndex();
        Q_EMIT basicIndexDone(m_mode);
    }
    if(m_target & Target::Content) {
        Q_EMIT contentIndexStart(m_mode);
        contentIndex();
        Q_EMIT contentIndexDone(m_mode);
    }
    if(m_target & Target::Ocr) {
        Q_EMIT ocrContentIndexStart(m_mode);
        ocrIndex();
        Q_EMIT ocrContentIndexDone(m_mode);
    }
    if(m_target & Target::Ai) {
        Q_EMIT aiIndexStart(m_mode);
        aiIndex();
        Q_EMIT aiIndexDone(m_mode);
    }
    m_cache.clear();
    malloc_trim(0);

    qDebug() << "BatchIndexer " << QMetaEnum::fromType<Targets>().valueToKeys(m_target) << ": time :" << timer.elapsed() << "milliseconds";
    Q_EMIT done(m_mode, m_target);
}

void BatchIndexer::fetch()
{
    qDebug() << "Now begin fetching files to be indexed...";
    qDebug() << "Index folders:" << m_folders << "blacklist :" << m_blackList;
    QQueue<QString> bfs;
    for(const QString& blockPath : m_blackList) {
        for(const QString& path : m_folders) {
            if(FileUtils::isOrUnder(path, blockPath)) {
                m_folders.removeOne(path);
            }
        }
    }
    m_cache.append(m_folders);
    for(const QString &path : m_folders) {
        bfs.enqueue(path);
    }
    QFileInfoList list;
    QDir dir;
    QStringList tmpList = m_blackList;
    // QDir::Hidden
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    dir.setSorting(QDir::DirsFirst);
    while(!bfs.empty()) {
        dir.setPath(bfs.dequeue());
        list = dir.entryInfoList();
        for(const auto& i : list) {
            bool isBlocked = false;
            for(const QString &path : tmpList) {
                if(i.absoluteFilePath() == path) {
                    isBlocked = true;
                    tmpList.removeOne(path);
                    break;
                }
            }
            if(isBlocked)
                continue;

            if(i.isDir() && (!(i.isSymLink()))) {
                bfs.enqueue(i.absoluteFilePath());
            }
            m_cache.append(i.absoluteFilePath());
        }
    }
    qDebug() << m_cache.size() << "files founded, start index...";
}

void BatchIndexer::basicIndex()
{
    qDebug() << "Begin basic index";
    WritableDatabase basicDb(DataBaseType::Basic);
    if(!basicDb.open()) {
        qWarning() << "Basic db open failed, fail to run basic index!";
        return;
    }
    QStringList filesNeedIndex;
    if(m_mode == WorkMode::Rebuild) {
        basicDb.rebuild();
        if(!basicDb.open()) {
            qWarning() << "basicDb db open failed, fail to run basic index!";
            return;
        }
        filesNeedIndex = m_cache;
        qDebug() <<filesNeedIndex.size() << "files need index.";
    } else if(m_mode == WorkMode::Add) {
        filesNeedIndex = m_cache;
        qDebug() <<filesNeedIndex.size() << "files need index.";
    } else if (m_mode == WorkMode::Update) {
        QFileInfo info;
        QMap<std::string, std::string> indexTimes = basicDb.getIndexTimes();
        qDebug() << indexTimes.size() << "documents recorded";
        for(const QString& path : m_cache) {
            info.setFile(path);
            if(indexTimes.take(FileUtils::makeDocUterm(path)) != info.lastModified().toString("yyyyMMddHHmmsszzz").toStdString()) {
                filesNeedIndex.append(path);
            }
        }
        if(!indexTimes.isEmpty()) {
            qDebug() << indexTimes.size() << "documents need remove.";
            for(const std::string& uniqueTerm : indexTimes.keys()) {
                basicDb.removeDocument(uniqueTerm);
            }
            basicDb.commit();
        }
        qDebug() << filesNeedIndex.size() << "files need update.";
    }
    uint allSize = filesNeedIndex.size();
    Q_EMIT progress(IndexType::Basic, allSize, 0);
    uint batchSize = 0;
    uint finishNum = 0;
    for (const QString& path: filesNeedIndex) {
        BasicIndexer indexer(path);
        if(indexer.index()) {
            basicDb.addDocument(indexer.document());
            ++batchSize;
            ++finishNum;
        }
        if(batchSize >= 8192) {
            qDebug() << finishNum << "of" << allSize <<"finished.";
            basicDb.commit();
            Q_EMIT progress(IndexType::Basic, allSize, finishNum);
            //文件名索引很快
            if(m_indexStop->LOAD) {
                qDebug() << "Index stopped, abort basic index.";
                filesNeedIndex.clear();
                return;
            }
            batchSize = 0;
        }
    }
    //TODO:xapian默认10000条自动commit一次，需要根据内存占用情况调整。
    basicDb.commit();
    Q_EMIT progress(IndexType::Basic, allSize, finishNum);
    filesNeedIndex.clear();
    qDebug() << "Finish basic index";
}

void BatchIndexer::contentIndex()
{
    qDebug() << "Begin content index";
    if(m_contentIndexStop->LOAD) {
        qDebug() << "Index stopped, abort content index.";
        return;
    }
    WritableDatabase contentDb(DataBaseType::Content);
    if(!contentDb.open()) {
        qWarning() << "Content db open failed, fail to run content index!";
        return;
    }

    QStringList filesNeedIndex;
    QFileInfo info;

    if(m_mode == WorkMode::Rebuild) {
        contentDb.rebuild();
        if(!contentDb.open()) {
            return;
        }
    }
    if(m_mode == WorkMode::Rebuild || m_mode == WorkMode::Add) {
        for(const QString& path : m_cache) {
            info.setFile(path);
            if(FileIndexerConfig::getInstance()->contentIndexTarget()[info.suffix()] && info.isFile()) {
                if(!FileUtils::isEncrypedOrUnsupport(path, info.suffix())) {
                    filesNeedIndex.append(path);
                }
            }
        }
    } else if(m_mode == WorkMode::Update) {
        QMap<std::string, std::string> indexTimes = contentDb.getIndexTimes();
        qDebug() << indexTimes.size() << "documents recorded";
        for(const QString& path : m_cache) {
            info.setFile(path);
            if(FileIndexerConfig::getInstance()->contentIndexTarget()[info.suffix()] && info.isFile()) {
                std::string uterm = FileUtils::makeDocUterm(path);
                if(indexTimes.value(uterm) != info.lastModified().toString("yyyyMMddHHmmsszzz").toStdString()) {
                    if(!FileUtils::isEncrypedOrUnsupport(path, info.suffix())) {
                        filesNeedIndex.append(path);
                        indexTimes.remove(uterm);
                    }
                } else {
                    indexTimes.remove(uterm);
                }
            }
        }
        if(!indexTimes.isEmpty()) {
            qDebug() << indexTimes.size() << "documents need remove";
            for(const std::string& uniqueTerm : indexTimes.keys()) {
                contentDb.removeDocument(uniqueTerm);
            }
            contentDb.commit();
        }
    }

    uint allSize = filesNeedIndex.size();
    qDebug() << allSize << "files need content index.";
    Q_EMIT progress(IndexType::Contents, allSize, 0);

    uint batchSize = 0;
    uint finishNum = 0;
    for (const QString& path : filesNeedIndex) {
        if(m_contentIndexStop->LOAD) {
            qDebug() << "Index stopped, interrupt content index.";
            filesNeedIndex.clear();
            return;
        }
        fileContentIndexer indexer(path);
        if(indexer.index()) {
            contentDb.addDocument(indexer.document());
            ++batchSize;
            ++finishNum;
        } else {
//            qDebug() << "Extract fail===" << path;
        }
        if(batchSize >= 30) {
            contentDb.commit();
            qDebug() << finishNum << "of" << allSize <<"finished.";
            Q_EMIT progress(IndexType::Contents, allSize, finishNum);
            batchSize = 0;
        }
    }
    contentDb.commit();
    Q_EMIT progress(IndexType::Contents, allSize, finishNum);
    filesNeedIndex.clear();
    qDebug() << "Finish content index";
}

void BatchIndexer::ocrIndex()
{
    qDebug() << "Begin ocr content index";
    if(m_contentIndexOcrStop->LOAD) {
        qDebug() << "Index stopped, abort ocr content index.";
        return;
    }
    WritableDatabase contentDb(DataBaseType::OcrContent);
    if(!contentDb.open()) {
        qWarning() << "Content db open failed, fail to run ocr content index!";
        return;
    }

    QStringList filesNeedOCRIndex;
    QFileInfo info;

    if(m_mode == WorkMode::Rebuild) {
        contentDb.rebuild();
        if(!contentDb.open()) {
            return;
        }
    }

    if(m_mode == WorkMode::Rebuild || m_mode == WorkMode::Add) {
        for(const QString &path : m_cache) {
            info.setFile(path);
            if(FileIndexerConfig::getInstance()->ocrContentIndexTarget()[info.suffix()] && info.isFile()) {
                if(!FileUtils::isEncrypedOrUnsupport(path, info.suffix())) {
                    filesNeedOCRIndex.append(path);
                }
            }
        }
    } else {
        QMap<std::string, std::string> indexTimes = contentDb.getIndexTimes();
        qDebug() << indexTimes.size() << "documents recorded";
        for(const QString& path : m_cache) {
            info.setFile(path);
            if(FileIndexerConfig::getInstance()->ocrContentIndexTarget()[info.suffix()] && info.isFile()) {
                std::string uterm = FileUtils::makeDocUterm(path);
                if(indexTimes.value(uterm) != info.lastModified().toString("yyyyMMddHHmmsszzz").toStdString()) {
                    if(!FileUtils::isEncrypedOrUnsupport(path, info.suffix())) {
                        filesNeedOCRIndex.append(path);
                        indexTimes.remove(uterm);
                    }
                } else {
                    indexTimes.remove(uterm);
                }
            }
        }
        if(!indexTimes.isEmpty()) {
            qDebug() << indexTimes.size() << "documents need remove";
            for(const std::string& uniqueTerm : indexTimes.keys()) {
                contentDb.removeDocument(uniqueTerm);
            }
            contentDb.commit();
        }
    }

    uint allSize = filesNeedOCRIndex.size();
    qDebug() << allSize << "pictures need ocr content index.";
    Q_EMIT progress(IndexType::OCR, allSize, 0);

    uint batchSize = 0;
    uint finishNum = 0;
    for (const QString &path : filesNeedOCRIndex) {
        if(m_contentIndexOcrStop->LOAD) {
            qDebug() << "Index stopped, interrupt ocr content index.";
            filesNeedOCRIndex.clear();
            return;
        }
        fileContentIndexer indexer(path);
        if(indexer.index()) {
            contentDb.addDocument(indexer.document());
            ++batchSize;
            ++finishNum;
        } else {
//            qDebug() << "Extract fail===" << path;
        }
        if(batchSize >= 10) {
            contentDb.commit();
            qDebug() << finishNum << "of" << allSize <<"finished.";
            Q_EMIT progress(IndexType::OCR, allSize, finishNum);
            batchSize = 0;
        }
    }
    contentDb.commit();
    Q_EMIT progress(IndexType::OCR, allSize, finishNum);
    filesNeedOCRIndex.clear();
    qDebug() << "Ocr content index finished,";

}

void BatchIndexer::aiIndex()
{
    qDebug() << "Begin ai index";
    if (m_aiIndexStop->LOAD) {
        qDebug() << "Index stopped, abort ai index.";
        return;
    }
    AiIndexer indexer;
    if(!indexer.createSession()) {
        qWarning() << "Creat session failed, fail to run ai index!";
        return;
    }
    if (m_mode == WorkMode::Rebuild) {
        //暂不支持重建
        return;
    }
    QJsonObject filesNeedAiIndex;
    QJsonObject imagesNeedAiIndex;
    QJsonObject filesNeedUpdate;
    QFileInfo info;
    if (m_mode == WorkMode::Add) {
        for (const QString &path: m_cache) {
            info.setFile(path);
            if(!info.isFile()) {
                continue;
            }
            QString type;

            if (FileIndexerConfig::getInstance()->aiIndexFileTarget()[info.suffix()]) {
                if(indexer.checkFileSupported(path, type)) { //这个检查可能会读文件，不要每个文件都执行一遍
                    filesNeedAiIndex.insert(path, type);
                }
            }
            if (FileIndexerConfig::getInstance()->aiIndexImageTarget()[info.suffix()]) {
                if(indexer.checkFileSupported(path, type)) {
                    imagesNeedAiIndex.insert(path, type);
                }
            }

        }
    } else {
        QJsonObject indexTimes; //校验完整性
        if(indexer.getAllIndexedFiles(indexTimes)) {
            qDebug() << indexTimes.size() << "documents recorded";
            for (const QString &path: m_cache) {
                info.setFile(path);
                if(!info.isFile()) {
                    continue;
                }
                QString type;
                if (FileIndexerConfig::getInstance()->aiIndexFileTarget()[info.suffix()]) {
                    int modifiedTime = indexTimes.value(path).toInt(0);
                    if(modifiedTime != info.lastModified().toSecsSinceEpoch()) {
                        if(indexer.checkFileSupported(path, type)) {
                            if(modifiedTime) {
                                filesNeedUpdate.insert(path, type); //修改时间不为空时执行更新
                            } else {
                                filesNeedAiIndex.insert(path, type);
                            }
                            indexTimes.remove(path);
                        }
                    } else {
                        indexTimes.remove(path);
                    }
                } else if (FileIndexerConfig::getInstance()->aiIndexImageTarget()[info.suffix()]) {
                    int modifiedTime = indexTimes.value(path).toInt(0);
                    if(modifiedTime != info.lastModified().toSecsSinceEpoch()) {
                        if(indexer.checkFileSupported(path, type)) {
                            if(modifiedTime) {
                                filesNeedUpdate.insert(path, type); //修改时间不为空时执行更新
                            } else {
                                imagesNeedAiIndex.insert(path, type);
                            }
                            indexTimes.remove(path);
                        }
                    } else {
                        indexTimes.remove(path);
                    }
                }
            }
            if(!indexTimes.isEmpty()) {
                qDebug() << indexTimes.size() << "documents need remove";
                indexer.deleteFileIndex(indexTimes.keys());
            }
        }

    }
    uint allSize = filesNeedAiIndex.size() + imagesNeedAiIndex.size() + filesNeedUpdate.size();
    qDebug() << allSize << "files need ai index.";
    Q_EMIT progress(IndexType::Ai, allSize, 0);

    uint finishNum = 0;
    uint batchSize = 0;
    QJsonArray tmpArray;
    QJsonObject::const_iterator iter;
    auto creatIndex = [&](bool(AiIndexer::*creat)(const QJsonArray &), const QJsonObject &files){
        for(iter = files.begin(); iter != files.end(); ++iter) {
            if(m_aiIndexStop->LOAD) {
                qDebug() << "Index stopped, interrupt ai index.";
                return;
            }
            QJsonObject oneFile;
            oneFile.insert(QStringLiteral("filepath"), iter.key());
            oneFile.insert(QStringLiteral("fileformat"), iter.value());
            tmpArray.append(oneFile);
            ++batchSize;
            ++finishNum;
            if(batchSize >= 10) {
                (indexer.*creat)(tmpArray);
                qDebug() << finishNum << "of" << allSize <<"finished.";
                Q_EMIT progress(IndexType::Ai, allSize, finishNum);
                batchSize = 0;
                tmpArray = QJsonArray();
            }
        }
        if(!tmpArray.isEmpty()) {
            (indexer.*creat)(tmpArray);
            qDebug() << finishNum << "of" << allSize <<"finished.";
            Q_EMIT progress(IndexType::Ai, allSize, finishNum);
            tmpArray = QJsonArray();
        }
    };
    creatIndex(&AiIndexer::addImageFileIndex, imagesNeedAiIndex);
    creatIndex(&AiIndexer::addTextFileIndex, filesNeedAiIndex);
    creatIndex(&AiIndexer::update, filesNeedUpdate);

}