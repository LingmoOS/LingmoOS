// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "business/config/accessrecord/accessrecord.h"
#include "global/matcheditem.h"
#include "global/searchconfigdefine.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <stub.h>

#include <QJsonObject>
#include <QIODevice>
#include <QFile>
#include <QtCore/qfiledevice.h>
#include <QtCore/qstring.h>
#include <stdio.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(AccessRecordTest, init)
{
    AccessRecord *accessRecord = new AccessRecord;

    EXPECT_TRUE(accessRecord);

    delete accessRecord;
}

TEST(AccessRecordTest, startParseRecord)
{
    auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
    AccessRecord::instance()->m_recordPath = recordPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_NAME
            +"/" + "accessrecord.json";
    QFileInfo file(recordPath);
    QFile file1(recordPath);
    file1.remove();
    AccessRecord::instance()->startParseRecord();
    bool result = file.exists();
    EXPECT_TRUE(result);
}

TEST(AccessRecordTest, updateRecord)
{
    GrandSearch::MatchedItem item;
    item.searcher = "test";
    item.item = "test1";
    qint64 time = 1;

    AccessRecord::instance()->m_recordObj.insert("version", "1.0");

    // 原记录中不包含该searcher
    QJsonObject recordObjNew;
    QJsonObject itemObj;
    QJsonArray array = {1};
    itemObj.insert("test1", array);
    recordObjNew.insert("test", itemObj);
    recordObjNew.insert("version", "1.0");

    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);

    // 原记录中包含该searcher但不包含该item
    item.item = "test2";
    itemObj.insert("test2", array);
    recordObjNew.insert("test", itemObj);
    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);

    // 原记录中包含该searcher并包含该item
    time = 2;
    array = {1, 2};
    itemObj.insert("test2", array);
    recordObjNew.insert("test", itemObj);
    AccessRecord::instance()->updateRecord(item, time);
    EXPECT_EQ(AccessRecord::instance()->m_recordObj, recordObjNew);
}

TEST(AccessRecordTest, sync)
{
    AccessRecord accessRecord;
    accessRecord.m_finished = true;
    accessRecord.m_recordPath = "./test";
    QFile file(accessRecord.m_recordPath);
    file.open(QFile::Truncate);
    file.close();

    accessRecord.m_recordObj.insert("aaa", "1");
    accessRecord.sync();
    file.open(QFile::ReadOnly);
    QByteArray array = file.readAll();
    file.close();
    QJsonDocument doc(QJsonDocument::fromJson(array));
    QJsonObject obj = doc.object();
    EXPECT_EQ(obj, accessRecord.m_recordObj);

}

TEST(AccessRecordTest, parseRecord)
{
    // 对parseJson打桩，文件存在解析数据
    bool parse = false;
    stub_ext::StubExt stu;
    stu.set_lamda(&AccessRecord::parseJson, [&]() {
        parse = true;
    });

    // 文件不存在
    QFile tempFile(AccessRecord::instance()->m_recordPath);
    tempFile.remove();

    AccessRecord::parseRecord(AccessRecord::instance()->m_recordPath);

    auto recordPath = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
    AccessRecord::instance()->m_recordPath = recordPath
            + "/" + QApplication::organizationName()
            + "/" + GRANDSEARCH_NAME
            +"/" + "accessrecord.json";
    QFileInfo file(recordPath);
    bool result = file.exists();
    EXPECT_TRUE(result);
    EXPECT_TRUE(parse);
}

TEST(AccessRecordTest, parseJson)
{
    // 文件打开成功解析json成功
    QJsonObject searcher;
    QJsonObject item;
    QJsonArray time;
    time.append(1647743325);
    const qint64 currentTimeT = QDateTime::currentDateTime().toSecsSinceEpoch();
    time.append(currentTimeT);
    item.insert("test1", time);
    searcher.insert("a", item);
    time.removeLast();
    item.insert("test1", time);
    item.insert("test2", time);
    searcher.insert("b", item);
    time.removeLast();
    for (int i = 0; i < 21; ++i) {
        time.append(currentTimeT);
    }
    time.append(1647743200);
    item.insert("test3", time);
    searcher.insert("c", item);
    searcher.insert("version", "1.0");
    QJsonDocument doc(searcher);
    QByteArray data = doc.toJson();

    // 对读取文件进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&QFile::readAll, [&]() {
        return data;
    });

    AccessRecord::parseJson(AccessRecord::instance()->m_recordPath);

    QJsonObject searcherNew;
    QJsonObject itemNew;
    QJsonArray timeNew;
    timeNew.append(currentTimeT);
    itemNew.insert("test1", timeNew);
    searcherNew.insert("a", itemNew);
    timeNew.removeLast();
    for (int i = 0; i < 21; ++i) {
        timeNew.append(currentTimeT);
    }
    QJsonObject itemAnotherNew;
    itemAnotherNew.insert("test3", timeNew);
    searcherNew.insert("c", itemAnotherNew);
    searcherNew.insert("version", "1.0");

    QHash<QString, QHash<QString, int>> searcherHash;
    QHash<QString, int> itemHash;
    itemHash.insert("test1", 1);
    searcherHash.insert("a", itemHash);
    itemHash.clear();
    itemHash.insert("test3", 20);
    searcherHash.insert("c", itemHash);

    AccessRecord::parseJson(AccessRecord::instance()->m_recordPath);
    EXPECT_EQ(searcherNew, AccessRecord::instance()->m_recordObj);
    EXPECT_EQ(searcherHash, AccessRecord::instance()->m_recordHash);
}
