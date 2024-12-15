// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "accessrecord.h"
#include "global/searchconfigdefine.h"
#include "utils/utils.h"

#include <QStandardPaths>
#include <QFileInfo>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QTime>
#include <QString>
#include <QApplication>
#include <QtConcurrent>

using namespace GrandSearch;

class AccessRecordGlobal : public AccessRecord {};
Q_GLOBAL_STATIC(AccessRecordGlobal, accessRecordGlobal)

AccessRecord *AccessRecord::instance()
{
    return accessRecordGlobal;
}

void AccessRecord::startParseRecord()
{
    std::call_once(m_initFlag, [](){
        // 异步解析
        QtConcurrent::run(&AccessRecord::parseRecord, AccessRecord::instance()->m_recordPath);
    });
}

/**
 * @brief AccessRecord::getRecord 主线程获取数据
 */
QHash<QString, QHash<QString, int>> AccessRecord::getRecord()
{
    return m_recordHash;
}

/**
 * @brief AccessRecord::updateRecord 根据用户的点击更新记录
 * @param matchedItem 用户点击的类目
 * @param time 用户点击的时间
 */
void AccessRecord::updateRecord(const MatchedItem &matchedItem, qint64 time)
{
    const QString &seacher = matchedItem.searcher;
    const QString &item = matchedItem.item;

    QJsonObject recordObj = m_recordObj;

    if (!recordObj.contains(seacher)) {
        QJsonArray times;
        times.append(time);
        QJsonObject itemObj;
        itemObj.insert(item, times);
        recordObj.insert(seacher, itemObj);
    } else {
        QJsonObject::iterator searcherIt = recordObj.begin();
        while (searcherIt != recordObj.end()) {
            if (searcherIt.key() == seacher) {
                QJsonObject itemsObj = searcherIt.value().toObject();
                if (!itemsObj.contains(item)) {
                    QJsonArray times;
                    times.append(time);
                    itemsObj.insert(item, times);
                } else {
                    QJsonObject::iterator itemIt = itemsObj.begin();
                    while (itemIt != itemsObj.end()) {
                        if (itemIt.key() == item) {
                            QJsonArray times = itemIt.value().toArray();
                            times.append(double(time));
                            itemsObj.insert(item, times);
                            break;
                        }
                        ++itemIt;
                    }
                }
                recordObj.insert(seacher, itemsObj);
                break;
            }
            ++searcherIt;
        }
    }

    m_recordObj = recordObj;
}

/**
 * @brief AccessRecord::sync 主线程将更新后的数据回写到文件中
 */
void AccessRecord::sync()
{
    if (!m_finished) {
        qWarning() << "parsing is not finished, do not sync.";
        return;
    }
    QFile file(m_recordPath);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "fail to open record" << m_recordPath;
        return;
    }

    QJsonDocument doc(m_recordObj);
    QByteArray data = doc.toJson();
    file.write(data);
    file.close();
}

AccessRecord::AccessRecord()
{
     qRegisterMetaType<QHash<QString, QHash<QString, int>>>();
     auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
     m_recordPath = recordPath
             + "/" + QApplication::organizationName()
             + "/" + GRANDSEARCH_NAME
             +"/" + "accessrecord.json";

     connect(this, &AccessRecord::sigParseFinished, this, [this](const QJsonObject &recordObj,
             const QHash<QString, QHash<QString, int>> &recordHash) {
         qInfo() << "parse finished, update data.";
         m_recordObj = recordObj;
         m_recordHash = recordHash;
         m_finished = true;
     });
}

AccessRecord::~AccessRecord()
{
}

/**
 * @brief AccessRecord::ParseRecord 解析记录文件
 */
void AccessRecord::parseRecord(QString recordPath)
{
    QFileInfo recordFile(recordPath);
    // 创建文件
    if (!recordFile.exists()) {
         recordFile.absoluteDir().mkpath(".");
         QFile file(recordPath);
         file.open(QFile::NewOnly);
         QJsonObject version;
         version.insert("version", "1.0");
         QJsonDocument doc(version);
         QByteArray data = doc.toJson();
         file.write(data);
         file.close();
         qInfo() << "create record" << recordPath;
    }
    parseJson(recordPath);
}

/**
 * @brief AccessRecord::parseJson 解析json数据
 * @param recordPath 文件路径
 */
void AccessRecord::parseJson(const QString &recordPath)
{
    QFile file(recordPath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "failed to open record file" << recordPath;
        return;
    }

    QByteArray allData = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument doc(QJsonDocument::fromJson(allData, &jsonError));

    static const int validTime = 7 * 24 * 60 * 60;

    // 记录计算次数的searcher
    QHash<QString, QHash<QString, int>> searcherHash;
    QJsonObject searcherObj;
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning() << "parse json error" << jsonError.errorString();
        emit AccessRecord::instance()->sigParseFinished(searcherObj, searcherHash, QPrivateSignal());
        return;
    }

    searcherObj = doc.object();
    for (auto searcherIt = searcherObj.begin(); searcherIt != searcherObj.end(); ) {
        if (searcherIt.key() == "version") {
            ++searcherIt;
            continue;
        }

        QJsonObject itemsObj = searcherIt.value().toObject();
        // 记录计算次数的item
        QHash<QString, int> itemsHash;
        for (auto itemIt = itemsObj.begin(); itemIt != itemsObj.end(); ) {
            QJsonArray timeArray = itemIt.value().toArray();
            for (auto timeIt = timeArray.begin(); timeIt != timeArray.end();) {
                qint64 timeT = (*timeIt).toDouble();
                // 获取当前时间戳
                const qint64 currentTimeT = QDateTime::currentDateTime().toSecsSinceEpoch();
                // 计算两个时间戳的差
                const qint64 intervalTime = currentTimeT - timeT;
                // 时间戳的差大于七天
                if (intervalTime > validTime || intervalTime < 0) {
                    timeIt = timeArray.erase(timeIt);
                } else {
                    ++timeIt;
                }
            }

            if (timeArray.isEmpty()) {
                // 时间列表为空，删除该item对象
                itemIt = itemsObj.erase(itemIt);
            } else {
                auto newIt = itemsObj.insert(itemIt.key(), timeArray);
                Q_ASSERT(newIt == itemIt);
                ++itemIt;
            }
        }

        if (itemsObj.empty()) {
            // item对象为空，删除该searcher对象
            searcherIt = searcherObj.erase(searcherIt);
        } else {
            for (auto tempItemIt = itemsObj.begin(); tempItemIt != itemsObj.end(); ) {
                QJsonArray timeArray = tempItemIt.value().toArray();
                int times = timeArray.size();
                if (times > 20) {
                    itemsHash.insert(tempItemIt.key(), 20);
                } else {
                    itemsHash.insert(tempItemIt.key(), times);
                }
                ++tempItemIt;
            }
            searcherHash.insert(searcherIt.key(), itemsHash);
            auto newIt = searcherObj.insert(searcherIt.key(), itemsObj);
            Q_ASSERT(newIt == searcherIt);
            ++searcherIt;
        }
    }

    emit AccessRecord::instance()->sigParseFinished(searcherObj, searcherHash, QPrivateSignal());
    qDebug() << "parse json completed";
}


