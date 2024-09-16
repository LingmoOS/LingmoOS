// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <ctype.h>

#include "utils/utils.h"
#include "global/matcheditem.h"
#include "global/builtinsearch.h"
#include "business/config/accessrecord/accessrecord.h"

#include "interfaces/daemongrandsearchinterface.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DArrowRectangle>
#include <DGuiApplicationHelper>
#include <DDesktopServices>

#include <QtTest>
#include <QColor>
#include <QProcess>
#include <QIcon>

using namespace testing;
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE;
DGUI_USE_NAMESPACE;

TEST(UtilsTest, constructor)
{
    Utils *utils = new Utils;

    EXPECT_TRUE(utils);

    delete utils;
}

TEST(UtilsTest, sort)
{
    stub_ext::StubExt stu;

    bool ut_call_compareByString = false;

    stu.set_lamda(&Utils::compareByString, [&](){
        ut_call_compareByString = true;
        return ut_call_compareByString;
    });

    MatchedItem item;
    MatchedItems items{item, item};

    bool result = Utils::sort(items);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_compareByString);
}

TEST(UtilsTest, compareByString)
{
    QString str1("abcdABCD");
    QString str2("abcdefg");

    // 覆盖重复字符逻辑
    bool result = Utils::compareByString(str1, str2, Qt::AscendingOrder);
    EXPECT_TRUE(result);

    str1 = "abcd";
    str2 = "efgh";

    // 其他符号逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 数字逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 其他语言逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 英文逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }

    // 中文逻辑
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithHanzi, [&](const QString &text1) {

            if (text1 == str1)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_FALSE(result);
    }

    {
        stub_ext::StubExt stu;
        stu.set_lamda(&Utils::startWithSymbol, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWidthNum, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithOtherLang, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithLatin, [&]() {
            return false;
        });

        stu.set_lamda(&Utils::startWithHanzi, [&](const QString &text1) {

            if (text1 == str2)
                return true;
            return false;
        });

        result = Utils::compareByString(str1, str2);
        EXPECT_TRUE(result);
    }
}

TEST(UtilsTest, startWithSymbol)
{
    QString text;

    bool result = Utils::startWithSymbol(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithSymbol(text);
    EXPECT_FALSE(result);

    text = "+-abcd";
    result = Utils::startWithSymbol(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithHanzi)
{
    QString text;

    bool result = Utils::startWithHanzi(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithHanzi(text);
    EXPECT_FALSE(result);

    text = "测试中文";
    result = Utils::startWithHanzi(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithLatin)
{
    QString text;

    bool result = Utils::startWithLatin(text);
    EXPECT_FALSE(result);

    text = "测试";
    result = Utils::startWithLatin(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithLatin(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWithOtherLang)
{
    QString text;

    bool result = Utils::startWithOtherLang(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWithOtherLang(text);
    EXPECT_FALSE(result);

    // test 白俄罗斯语
    text = "Праверка";
    result = Utils::startWithOtherLang(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, startWidthNum)
{
    QString text;

    bool result = Utils::startWidthNum(text);
    EXPECT_FALSE(result);

    text = "abcd";
    result = Utils::startWidthNum(text);
    EXPECT_FALSE(result);

    text = "1234";
    result = Utils::startWidthNum(text);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, sortByWeight)
{
     MatchedItemMap map;
     Qt::SortOrder order;
     MatchedItems items;
     MatchedItem item;
     QString searchGroupName;
     QVariant extra;

     items.append(item);
     map.insert(searchGroupName,items);
     bool result = Utils::sortByWeight(map);
     EXPECT_TRUE(result);

     // 清空map
     map.clear();

     // 不支持排序的类目
     order = Qt::DescendingOrder;
     map.insert(GRANDSEARCH_GROUP_APP, items);
     result = Utils::sortByWeight(map, order);
     EXPECT_TRUE(result);

     // 清空map, items
     map.clear();
     items.clear();


     //降序排序
     QVariantHash itemWeight1({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 1}});
     extra = QVariant::fromValue(itemWeight1);
     item = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     QVariantHash itemWeight2({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 2}});
     extra = QVariant::fromValue(itemWeight2);
     item = {"a", "aa", "aa",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     QVariantHash itemWeight3({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 2}});
     extra = QVariant::fromValue(itemWeight3);
     item = {"a", "ab", "ab",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     map.insert(GRANDSEARCH_GROUP_FILE, items);
     result = Utils::sortByWeight(map, order);
     EXPECT_TRUE(result);

     // 清空items
     items.clear();

     // 降序排序结果
     extra = QVariant::fromValue(itemWeight2);
     item = {"a", "aa", "aa",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     extra = QVariant::fromValue(itemWeight3);
     item = {"a", "ab", "ab",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     extra = QVariant::fromValue(itemWeight1);
     item = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     MatchedItemMap mapTest;
     mapTest.insert(GRANDSEARCH_GROUP_FILE, items);
     EXPECT_EQ(map, mapTest);

     // 清空map,items
     map.clear();
     items.clear();

     // 升序排序
     order=Qt::AscendingOrder;
     extra = QVariant::fromValue(itemWeight3);
     item = {"a", "ab", "ab",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     QVariantHash itemWeight4({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 1}});
     extra = QVariant::fromValue(itemWeight4);
     item = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     QVariantHash itemWeight5({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 2}});
     extra = QVariant::fromValue(itemWeight5);
     item = {"a", "aa", "aa",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     map.insert(GRANDSEARCH_GROUP_FILE, items);
     result = Utils::sortByWeight(map, order);
     EXPECT_TRUE(result);

     // 清空items
     items.clear();

     // 升序排序结果
     extra = QVariant::fromValue(itemWeight4);
     item = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     extra = QVariant::fromValue(itemWeight3);
     item = {"a", "ab", "ab",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     extra = QVariant::fromValue(itemWeight5);
     item = {"a", "aa", "aa",GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     mapTest.insert(GRANDSEARCH_GROUP_FILE, items);
     EXPECT_EQ(map, mapTest);
}

TEST(UtilsTest, compareByWeight)
{
     MatchedItem node1;
     MatchedItem node2;
     QVariant extra;

     // 两项均有权重比较，默认降序排序
     QVariantHash itemWeight1({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 1}});
     extra = QVariant::fromValue(itemWeight1);
     node1 = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     QVariantHash itemWeight2({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 2}});
     extra = QVariant::fromValue(itemWeight2);
     node2 = {"aa", "aa", "aa",GRANDSEARCH_GROUP_FILE, " ", extra};
     bool result = Utils::compareByWeight(node1, node2);
     EXPECT_FALSE(result);

     // 仅第一个节点有权重，默认降序排序
     QVariantHash itemWeight3({{"test", 0}});
     extra = QVariant::fromValue(itemWeight3);
     node2 = {"ab", "ab", "ab", GRANDSEARCH_GROUP_FILE, " ", extra};
     result = Utils::compareByWeight(node1, node2);
     EXPECT_TRUE(result);

     //  仅第二个节点有权重，默认降序排序
     result = Utils::compareByWeight(node2, node1);
     EXPECT_FALSE(result);

     // 两项均无权重，采用名称排序
     QVariantHash itemWeight4({{"test", 0}});
     extra = QVariant::fromValue(itemWeight4);
     node1 = {"a", "a", "a",GRANDSEARCH_GROUP_FILE, " ", extra};
     bool result1 = Utils::compareByWeight(node1, node2);
     bool result2 = Utils::compareByString(node1.item, node2.item);
     EXPECT_EQ(result1, result2);
}

TEST(UtilsTest, updateItemsWeight)
{
    MatchedItemMap map;
    MatchedItems items;
    MatchedItem item;
    QString content = "a";

    stub_ext::StubExt stu;
    // 对calcFileWeight进行打桩
    stu.set_lamda(&Utils::calcFileWeight, [](const QString &path, const QString &name, const QStringList &keys)->int{
        return 10;
    });

    // 对calcSettingWeight进行打桩
    stu.set_lamda(&Utils::calcSettingWeight, [](const GrandSearch::MatchedItem &item, const QStringList &keys)->double{
        return 20;
    });

    // 对calcAppWeight进行打桩
    stu.set_lamda(&Utils::calcAppWeight, [](const GrandSearch::MatchedItem &item, const QStringList &keys)->double{
        return 30;
    });

    // 关键字不包括冒号无需解析，类目为文件, 未设置权重计算方式/设置权重计算方式
    item = {"a", "a", "a", "a", "test", QVariant()};
    items.append(item);
    QVariantHash weightMethod({{GRANDSEARCH_PROPERTY_WEIGHT_METHOD, GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE}});
    item = {"ab", "ab", "ab", GRANDSEARCH_GROUP_FILE, GRANDSEARCH_CLASS_FILE_FSEARCH, QVariant::fromValue(weightMethod)};
    items.append(item);
    map.insert(GRANDSEARCH_GROUP_FILE, items);
    Utils::updateItemsWeight(map, content);
    QVariant test = map.value(GRANDSEARCH_GROUP_FILE).last().extra;
    int tempWeight = test.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT).toInt();
    EXPECT_EQ(tempWeight, 10);

    items.clear();
    map.clear();

    // 关键字包括冒号，类目为应用
    content = "a:b";
    weightMethod.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD, GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP);
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_APP_DESKTOP, QVariant::fromValue(weightMethod)};
    items.append(item);
    map.insert(GRANDSEARCH_GROUP_APP, items);
    Utils::updateItemsWeight(map, content);
    test = map.value(GRANDSEARCH_GROUP_APP).first().extra;
    double temp = test.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT).toInt();

    EXPECT_EQ(temp, 30);

    map.clear();
    items.clear();

    // 关键字包括冒号，类目为设置
    weightMethod.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD, GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING);
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, QVariant::fromValue(weightMethod)};
    items.append(item);
    map.insert(GRANDSEARCH_GROUP_SETTING, items);
    Utils::updateItemsWeight(map, content);
    test = map.value(GRANDSEARCH_GROUP_SETTING).first().extra;
    temp = test.toHash().value(GRANDSEARCH_PROPERTY_ITEM_WEIGHT).toInt();
    EXPECT_EQ(temp, 20);
}

TEST(UtilsTest, setWeightMethod)
{
    MatchedItem item;

    // 不支持权重计算
    item = {"a", "a", "a", "a", "test", QVariant()};
    bool result = Utils::setWeightMethod(item);
    EXPECT_FALSE(result);

    // 已经设置权重数值
    QVariantHash weight({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 11}});
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_FILE_DEEPIN, QVariant::fromValue(weight)};
    result = Utils::setWeightMethod(item);
    EXPECT_FALSE(result);

    // 已经设置计算方法
    weight.clear();
    weight.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD, "test");
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_FILE_DEEPIN, QVariant::fromValue(weight)};
    result = Utils::setWeightMethod(item);
    EXPECT_TRUE(result);

    // 类目为文件
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_FILE_DEEPIN, QVariant()};
    result = Utils::setWeightMethod(item);
    QVariantHash hash;
    hash.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                GRANDSEARCH_PROPERTY_WEIGHT_METHOD_LOCALFILE);
    EXPECT_TRUE(result);
    EXPECT_EQ(hash, item.extra.toHash());

    // 类目为应用
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_APP_DESKTOP, QVariant()};
    result = Utils::setWeightMethod(item);
    hash.clear();
    hash.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                GRANDSEARCH_PROPERTY_WEIGHT_METHOD_APP);
    EXPECT_TRUE(result);
    EXPECT_EQ(hash, item.extra.toHash());

    // 类目为设置
    item = {"a", "a", "a", "a", GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, QVariant()};
    result = Utils::setWeightMethod(item);
    hash.clear();
    hash.insert(GRANDSEARCH_PROPERTY_WEIGHT_METHOD,
                GRANDSEARCH_PROPERTY_WEIGHT_METHOD_SETTING);
    EXPECT_TRUE(result);
    EXPECT_EQ(hash, item.extra.toHash());
}

TEST(UtilsTest, calcFileWeight)
{
     QString path = "ut_filestatisticsthread.cpp";
     QString name = "ut_filestatisticsthread.cpp";
     QStringList keys;

     // 名称中包含关键字
     keys.append("u");
     int result=Utils::calcFileWeight(path, name, keys);
     EXPECT_EQ(result, 100);

     // 名称中不包含关键字
     keys.clear();
     keys.append("aaaaa");
     result = Utils::calcFileWeight(path, name, keys);
     EXPECT_EQ(result, 57);
}

TEST(UtilsTest, calcDateDiff)
{
     QDate dateTest1 = QDate(2022, 3, 27);
     QDate dateTest2 = QDate(2022, 3, 28);
     QString strTime1 = "12:00";
     QString strTime2 = "12:01";
     QTime timeTest1 = QTime::fromString(strTime1, "hh:mm");
     QTime timeTest2 = QTime::fromString(strTime2, "hh:mm");
     QDateTime date1(dateTest1, timeTest1);
     QDateTime date2(dateTest2, timeTest2);

     // 日期相隔计算是否正确
     qint64 result = Utils::calcDateDiff(date1, date2);
     EXPECT_EQ(result, 1);
}

TEST(UtilsTest, calcWeightByDateDiff)
{
     qint64 diff;
     int type;

     // 最近一天内创建
     diff = 0;
     type = 1;
     int result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 24);

     // 最近三天内创建
     diff = 1;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 16);

     diff = 2;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 16);

     // 最近七天内创建
     diff = 3;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 4;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 5;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 6;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     // 超过七天创建
     diff = 7;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 0);

     // 最近一天内修改
     diff = 0;
     type = 2;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 24);

     // 最近三天内修改
     diff = 1;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 16);

     diff = 2;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 16);

     // 最近七天内修改
     diff = 3;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 4;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 5;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     diff = 6;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 8);

     // 超过七天修改
     diff = 7;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 0);

     // 最近一天内访问
     diff = 0;
     type = 3;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 9);

     // 最近三天内访问
     diff = 1;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 6);

     diff = 2;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 6);

     // 最近七天内访问
     diff = 3;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 3);

     diff = 4;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 3);

     diff = 5;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 3);

     diff = 6;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 3);

     // 超过七天访问
     diff = 7;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 0);

     // 非创建,修改,访问属性
     type = 4;
     result = Utils::calcWeightByDateDiff(diff, type);
     EXPECT_EQ(result, 0);
}

TEST(UtilsTest, calcAppWeight)
{
    // 对计算点选权重函数进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&Utils::calcRecordWeight, [&](){
       return 0.5;
    });

    // 名称包含关键字
    QStringList keys;
    keys.append("a");
    keys.append("b");
    GrandSearch::MatchedItem item;
    item.name = "aa";
    double result = Utils::calcAppWeight(item, keys);
    EXPECT_EQ(result, 80.5);

    // 名称不包含关键字
    item.name = "ccc";
    result = Utils::calcAppWeight(item, keys);
    EXPECT_EQ(result, 60.5);
}

TEST(UtilsTest, calcSettingWeight)
{
    // 对计算点选权重函数进行打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&Utils::calcRecordWeight, [&](){
       return 1.5;
    });

    // 名称包含关键字
    QStringList keys;
    keys.append("a");
    keys.append("b");
    GrandSearch::MatchedItem item;
    item.name = "aa";
    double result = Utils::calcSettingWeight(item, keys);
    EXPECT_EQ(result, 81.5);

    // 名称不包含关键字
    item.name = "ccc";
    result = Utils::calcSettingWeight(item, keys);
    EXPECT_EQ(result, 61.5);
}

TEST(UtilsTest, packageBestMatch)
{
    // todo:最佳匹配增加了应用与设置，该用例需要补充完善

     MatchedItemMap map;
     int maxQuantity = 4;
     MatchedItems items;
     MatchedItem item;
     QVariant extra;

     // 匹配项为空
     Utils::packageBestMatch(map, maxQuantity);
     MatchedItems itemsTest;
     itemsTest = map.value(GRANDSEARCH_GROUP_BEST);
     EXPECT_TRUE(itemsTest.isEmpty());

     // 匹配项中不包含最佳匹配类目
     item = {"a", "a", "a", "a", "a", extra};
     items.append(item);
     item = {"a", "ab", "ab", "ab", "ab", extra};
     items.append(item);
     map.insert(GRANDSEARCH_GROUP_APP, items);
     Utils::packageBestMatch(map, maxQuantity);
     itemsTest = map.value(GRANDSEARCH_GROUP_BEST);
     EXPECT_TRUE(itemsTest.isEmpty());

     // 清空map,items
     map.clear();
     items.clear();

     // 最大限制最佳匹配项数量小于等于0
     maxQuantity = 0;
     QVariantHash itemWeight({{GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 32}});
     extra = QVariant::fromValue(itemWeight);
     item = {"a", "a", "a", GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     itemWeight.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 27);
     extra = QVariant::fromValue(itemWeight);
     item = {"a", "ab", "ab", GRANDSEARCH_GROUP_FILE, " ", extra};
     items.append(item);
     map.insert(GRANDSEARCH_GROUP_FILE, items);
     Utils::packageBestMatch(map, maxQuantity);
     itemsTest = map.value(GRANDSEARCH_GROUP_BEST);
     EXPECT_TRUE(itemsTest.isEmpty());

     map.clear();
     items.clear();

     // 匹配项中包含文件、应用、设置
     for (int i = 0; i < 5; ++i) {
         item = {"a", "a", "a", "testa", GRANDSEARCH_CLASS_FILE_DEEPIN, extra};
         items.append(item);
     }
     map.insert(GRANDSEARCH_GROUP_FILE, items);
     items.clear();
     itemWeight.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 49);
     extra = QVariant::fromValue(itemWeight);
     for (int i = 0; i < 5; ++i) {
         item = {"b", "b", "b", "testb", GRANDSEARCH_CLASS_APP_DESKTOP, extra};
         items.append(item);
     }
     map.insert(GRANDSEARCH_GROUP_APP, items);
     items.clear();
     itemWeight.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 52);
     extra = QVariant::fromValue(itemWeight);
     item = {"c", "c", "c", "testc", GRANDSEARCH_CLASS_SETTING_CONTROLCENTER, extra};
     items.append(item);
     map.insert(GRANDSEARCH_GROUP_SETTING, items);
     Utils::packageBestMatch(map, 4);

     MatchedItems tempBestList;
     tempBestList.append(item);
     itemWeight.insert(GRANDSEARCH_PROPERTY_ITEM_WEIGHT, 49);
     extra = QVariant::fromValue(itemWeight);
     item = {"b", "b", "b", "testb", GRANDSEARCH_CLASS_APP_DESKTOP, extra};
     tempBestList.append(item);
     tempBestList.append(item);
     tempBestList.append(item);
     MatchedItems list = map.value(GRANDSEARCH_GROUP_BEST);
     EXPECT_EQ(tempBestList, list);
}

TEST(UtilsTest, calcRecordWeight)
{
    MatchedItem item;

    // 包含最佳匹配项
    // 对getRecord打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&AccessRecord::getRecord, [](){
        QHash<QString, QHash<QString, int>> hash;
        QHash<QString, int> item;
        item.insert("test", 2);
        hash.insert(GRANDSEARCH_CLASS_APP_DESKTOP, item);
        return hash;
    });

    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    item.item = "test";
    double result = Utils::calcRecordWeight(item);
    EXPECT_EQ(result, 1);
}

TEST(UtilsTest, isResetSeacher)
{
    QString searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    bool result = Utils::isResetSearcher(searcher);
    EXPECT_TRUE(result);

    searcher = "aaa";
    result = Utils::isResetSearcher(searcher);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, appIconName)
{
    MatchedItem item;

    // 名称为空的逻辑
    QString name = Utils::appIconName(item);
    EXPECT_TRUE(name.isEmpty());

    // 设置item
    item.item = "testItem";

    // 不支持显示图标的逻辑
    bool ut_isShowAppIcon = false;

    stub_ext::StubExt stu;
    stu.set_lamda(&Utils::isShowAppIcon, [&]() {
        return ut_isShowAppIcon;
    });
    name = Utils::appIconName(item);
    EXPECT_TRUE(name.isEmpty());

    // 设置支持
    ut_isShowAppIcon = true;

    // 结果为应用
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;
    item.icon = "test.png";
    name = Utils::appIconName(item);
    EXPECT_EQ(name, item.icon);

    // 结果为WEB
    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;
    QString defaultDesktopFile("/usr/share/applications/dde-control-center.desktop");
    stu.set_lamda(&Utils::defaultBrowser, [&](){
        return defaultDesktopFile;
    });
    name = Utils::appIconName(item);
    if (QFile::exists(defaultDesktopFile))
        EXPECT_FALSE(name.isEmpty());
    else
        EXPECT_TRUE(name.isEmpty());

    // 结果为控制中心
    item.searcher = GRANDSEARCH_CLASS_SETTING_CONTROLCENTER;

    name = Utils::appIconName(item);
    EXPECT_FALSE(name.isEmpty());

    // 结果为文件
    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    stu.set_lamda(&Utils::getFileMimetype, [&]() {
        return "inode/directory";
    });

    name = Utils::appIconName(item);
    EXPECT_FALSE(name.isEmpty());
}

TEST(UtilsTest, isShowAppIcon)
{
    MatchedItem item;
    item.searcher = GRANDSEARCH_CLASS_SETTING_CONTROLCENTER;

    bool result = Utils::isShowAppIcon(item);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, getFileMimetype)
{
    QString filePath("testFilePath");

    QString mimetype = Utils::getFileMimetype(filePath);

    EXPECT_EQ(mimetype, QString("text/plain"));
}

TEST(UtilsTest, getFileMimetypeByGio)
{
    QString path("testPath");

    QString mimetype = Utils::getFileMimetypeByGio(path);
    EXPECT_TRUE(mimetype.isEmpty());

    path = "/usr/bin/";
    mimetype = Utils::getFileMimetypeByGio(path);
    EXPECT_EQ(mimetype, QString("inode/directory"));
}

TEST(UtilsTest, getFileMimetypeByQt)
{
    // 后缀判断
    QString path("/usr/share/applications/dde-control-center.desktop");

    QString mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_FALSE(mimetype.isEmpty());

    // 后缀为空
    path = "/usr/bin/ls";
    mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_FALSE(mimetype.isEmpty());

    // 文件不存在
    path.clear();
    mimetype = Utils::getFileMimetypeByQt(path);
    EXPECT_TRUE(mimetype.isEmpty());
}

TEST(UtilsTest, getDefaultAppDesktopFileByMimeType)
{
    QString mimetype("test");

    QString defaultApp = Utils::getDefaultAppDesktopFileByMimeType(mimetype);
    EXPECT_TRUE(defaultApp.isEmpty());

    mimetype = "inode/directory";
    defaultApp = Utils::getDefaultAppDesktopFileByMimeType(mimetype);
    EXPECT_FALSE(defaultApp.isEmpty());
}

TEST(UtilsTest, isResultFromBuiltSearch)
{
    MatchedItem item;

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;

    bool result = Utils::isResultFromBuiltSearch(item);
    EXPECT_TRUE(result);

    item.searcher = "test";
    result = Utils::isResultFromBuiltSearch(item);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, openMatchedItem)
{
    stub_ext::StubExt stu;

    // 非内置搜索项
    bool ut_isResultFromBuiltSearch = false;
    stu.set_lamda(&Utils::isResultFromBuiltSearch, [&]() {
        return ut_isResultFromBuiltSearch;
    });

    bool ut_call_openExtendSearchMatchedItem = false;
    stu.set_lamda(&Utils::openExtendSearchMatchedItem, [&]() {
        ut_call_openExtendSearchMatchedItem = true;
        return ut_call_openExtendSearchMatchedItem;
    });

    MatchedItem item;
    bool result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openExtendSearchMatchedItem);

    // 内置搜索项
    ut_isResultFromBuiltSearch = true;
    ut_call_openExtendSearchMatchedItem = false;

    // item为空
    result = Utils::openMatchedItem(item);
    EXPECT_FALSE(result);
    EXPECT_FALSE(ut_call_openExtendSearchMatchedItem);

    // 设置item
    item.item = "testItem";

    // 应用
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;

    bool ut_call_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
       ut_call_launchApp = true;
       return ut_call_launchApp;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_launchApp);

    // 浏览器
    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;

    bool ut_call_openWithBrowser = false;
    stu.set_lamda(&Utils::openWithBrowser, [&](){
       ut_call_openWithBrowser = true;
       return ut_call_openWithBrowser;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openWithBrowser);

    // 文件
    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;

    bool ut_call_openFile = false;
    stu.set_lamda(&Utils::openFile, [&](){
        ut_call_openFile = true;
        return ut_call_openFile;
    });

    result = Utils::openMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openFile);
}

TEST(UtilsTest, openMatchedItemWithCtrl)
{
    stub_ext::StubExt stu;
    bool ut_call_openMatchedItem = false;
    stu.set_lamda(&Utils::openMatchedItem, [&](){
        ut_call_openMatchedItem = true;
        return ut_call_openMatchedItem;
    });

    MatchedItem item;
    bool result = Utils::openMatchedItemWithCtrl(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openMatchedItem);


    bool ut_call_openInFileManager = false;
    stu.set_lamda(&Utils::openInFileManager, [&](){
        ut_call_openInFileManager = true;
        return ut_call_openInFileManager;
    });
    stu.set_lamda(&QFileInfo::isDir, [](){
        return false;
    });

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    result = Utils::openMatchedItemWithCtrl(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_openInFileManager);
}

TEST(UtilsTest, openInFileManager)
{
    stub_ext::StubExt stu;
    bool ut_call_showFileItem = false;
    bool (*showFileItem_addr)(QString, const QString &) = &DDesktopServices::showFileItem;
    stu.set_lamda(showFileItem_addr, [&](){
        ut_call_showFileItem = true;
        return ut_call_showFileItem;
    });

    MatchedItem item;
    bool result = Utils::openInFileManager(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_showFileItem);
}

TEST(UtilsTest, openExtendSearchMatchedItem)
{
    stub_ext::StubExt stu;

    bool ut_OpenWithPlugin = false;
    stu.set_lamda(&DaemonGrandSearchInterface::OpenWithPlugin, [&]() {
        ut_OpenWithPlugin = true;
        return QDBusPendingReply<bool>();
    });

    MatchedItem item;

    bool result = Utils::openExtendSearchMatchedItem(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_OpenWithPlugin);
}

TEST(UtilsTest, openFile)
{
    MatchedItem item;

    // item为空
    bool result =Utils::openFile(item);
    EXPECT_FALSE(result);

    // 设置item
    item.item = "/home/";

    stub_ext::StubExt stu;

    QString ut_getDefaultAppDesktopFileByMimeType;
    stu.set_lamda(&Utils::getDefaultAppDesktopFileByMimeType, [&]() {
        return ut_getDefaultAppDesktopFileByMimeType;
    });

    bool ut_startDetached = false;

    stu.set_lamda((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), [&](){
       ut_startDetached = true;
       return ut_startDetached;
    });

    bool ut_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
        ut_launchApp = true;
        return ut_launchApp;
    });

    result = Utils::openFile(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_startDetached);
    EXPECT_FALSE(ut_launchApp);

    ut_getDefaultAppDesktopFileByMimeType = "inode/directory";
    ut_startDetached = false;
    ut_launchApp = false;

    result = Utils::openFile(item);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_launchApp);
    EXPECT_FALSE(ut_startDetached);
}

TEST(UtilsTest, launchApp)
{
    stub_ext::StubExt stu;

    bool ut_call_launchAppByDBus = false;
    stu.set_lamda(&Utils::launchAppByDBus, [&]() {
        return ut_call_launchAppByDBus;
    });

    bool ut_call_launchAppByGio = false;
    stu.set_lamda(&Utils::launchAppByGio, [&]() {
        return ut_call_launchAppByGio;
    });

    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_FALSE(result);

    ut_call_launchAppByGio = true;
    result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_TRUE(result);

    ut_call_launchAppByDBus = true;
    result = Utils::launchApp(desktopFile, filePaths);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, launchAppByDBus)
{
    stub_ext::StubExt stu;

    bool ut_call_asyncCallWithArgumentList = false;
    stu.set_lamda(&QDBusAbstractInterface::asyncCallWithArgumentList, [&]() {
        ut_call_asyncCallWithArgumentList = true;
        return QDBusPendingCall::fromError(QDBusError());
    });

    bool ut_call_reply = false;
    stu.set_lamda(&QDBusPendingCall::reply, [&]() {
       ut_call_reply = true;
       return QDBusMessage();
    });

    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchAppByDBus(desktopFile, filePaths);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ut_call_asyncCallWithArgumentList);
    EXPECT_TRUE(ut_call_reply);
}

TEST(UtilsTest, launchAppByGio)
{
    QString desktopFile;
    QStringList filePaths;

    bool result = Utils::launchAppByGio(desktopFile, filePaths);
    EXPECT_FALSE(result);

    desktopFile = "/usr/share/applications/dde-file-manager.desktop";
    filePaths << "/home/" << "/usr/bin/";

    bool ut_gio_launch = false;

    stub_ext::StubExt stu;

    stu.set_lamda(g_app_info_launch, [&]() {
        ut_gio_launch = true;
        return 0;
    });

    bool ut_startDetached = false;

    stu.set_lamda((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), [&](){
       ut_startDetached = true;
       return ut_startDetached;
    });

    result = Utils::launchAppByGio(desktopFile, filePaths);
    EXPECT_FALSE(result);
    EXPECT_TRUE(ut_gio_launch);
    EXPECT_TRUE(ut_startDetached);
}

TEST(UtilsTest, openWithBrowser)
{
    QString words;

    bool result = Utils::openWithBrowser(words);
    EXPECT_FALSE(result);

    words = "testWord";
    stub_ext::StubExt stu;

    QString defaultBorwser;
    stu.set_lamda(&Utils::defaultBrowser, [&]() {
        return defaultBorwser;
    });

    result = Utils::openWithBrowser(words);
    EXPECT_FALSE(result);

    defaultBorwser = "testDefaultBorwser";

    bool ut_call_launchApp = false;
    stu.set_lamda(&Utils::launchApp, [&](){
       ut_call_launchApp = true;
       return ut_call_launchApp;
    });

    result = Utils::openWithBrowser(words);
    EXPECT_TRUE(result);
}

TEST(UtilsTest, defaultIcon)
{
    MatchedItem item;
    item.searcher = GRANDSEARCH_CLASS_APP_DESKTOP;

    QIcon result = Utils::defaultIcon(item);
    EXPECT_EQ(result.name(), QIcon::fromTheme("application-x-desktop").name());

    item.searcher = GRANDSEARCH_CLASS_FILE_DEEPIN;
    result = Utils::defaultIcon(item);
    EXPECT_TRUE(result.isNull());

    item.searcher = GRANDSEARCH_CLASS_WEB_STATICTEXT;

    stub_ext::StubExt stu;

    QString ut_appIconName;
    stu.set_lamda(&Utils::appIconName, [&]() {
        return ut_appIconName;
    });

    result = Utils::defaultIcon(item);
    EXPECT_EQ(result.name(), QIcon::fromTheme("application-x-desktop").name());

    item.searcher = "testSearcher";
    result = Utils::defaultIcon(item);
    EXPECT_TRUE(result.isNull());
}

TEST(UtilsTest, defaultBrowser)
{
    stub_ext::StubExt stu;

    QString defaultBorwser("tsetBorwser");
    stu.set_lamda(&Utils::getDefaultAppDesktopFileByMimeType, [&]() {
        return defaultBorwser;
    });

    QString result = Utils::defaultBrowser();
    EXPECT_EQ(result, defaultBorwser);
}

TEST(UtilsTest, isLevelItem)
{
    MatchedItem item;
    int level = -1;

    bool result = Utils::isLevelItem(item, level);
    EXPECT_FALSE(result);

    QVariantHash showLevelHash({{GRANDSEARCH_PROPERTY_ITEM_LEVEL, 1}});
    item.extra = QVariant::fromValue(showLevelHash);

    result = Utils::isLevelItem(item, level);
    EXPECT_TRUE(result);
    EXPECT_EQ(level, 1);
}

TEST(UtilsTest, isLevelGroup)
{
    QString searchGroupName(GRANDSEARCH_GROUP_FILE);

    bool result = Utils::isLevelGroup(searchGroupName);
    EXPECT_TRUE(result);

    searchGroupName = "testOther";
    result = Utils::isLevelGroup(searchGroupName);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, canPreview)
{
    QString searchGroupName(GRANDSEARCH_GROUP_FILE);

    bool result = Utils::canPreview(searchGroupName);
    EXPECT_TRUE(result);

    searchGroupName = "testOther";
    result = Utils::canPreview(searchGroupName);
    EXPECT_FALSE(result);
}

TEST(UtilsTest, iconThemeSuffix)
{
    stub_ext::StubExt stu;

    DGuiApplicationHelper::ColorType ut_themeType = DGuiApplicationHelper::DarkType;
    stu.set_lamda(ADDR(DGuiApplicationHelper, themeType), [&](){
        return ut_themeType;
    });

    QString result = Utils::iconThemeSuffix();
    EXPECT_EQ(result, QString("-light"));

    ut_themeType = DGuiApplicationHelper::LightType;
    result = Utils::iconThemeSuffix();
    EXPECT_EQ(result, QString("-dark"));
}
