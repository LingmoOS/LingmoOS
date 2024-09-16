// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wtmpparse.h"

#include <stub.h>

#include <QDebug>

#include <gtest/gtest.h>
#include <string.h>

TEST(wtmpparse_wtmp_open_UT, wtmpparse_wtmp_open_UT)
{
    int result = wtmp_open(QString(WTMP_FILE).toLatin1().data());
    EXPECT_NE(result, -1);
}

TEST(wtmpparse_wtmp_next_UT, wtmp_open_back_UT)
{
    char fileName[]="test_test";
    int result = wtmp_open_back(fileName);
    EXPECT_EQ(result, -1);
}

TEST(wtmpparse_wtmp_next_UT, seek_end_UT)
{
    int result = seek_end();
    EXPECT_EQ(result, 0);
}

TEST(wtmpparse_wtmp_next_UT, wtmp_reload_back_UT)
{
    int result = wtmp_reload_back();
    EXPECT_NE(result, 0);
}

TEST(wtmpparse_wtmp_next_UT, wtmp_back_UT)
{
    struct utmp *test = wtmp_back();
    EXPECT_EQ(test, nullptr);
}

TEST(wtmpparse_wtmp_next_UT, show_end_time_UT)
{
//    const char *endTime= show_end_time(233);
//    EXPECT_NE(strlen(endTime), 0)<<"check the status after show_end_time(233)";

}

TEST(wtmpparse_wtmp_next_UT, show_start_time_UT)
{
//    const char * startTime= show_start_time(233);
//    EXPECT_NE(strlen(startTime), 0)<<"check the status after show_start_time(233)";
}

TEST(wtmpparse_wtmp_next_UT, wtmpparse_wtmp_next_UT_001)
{
     utmp *tmp=wtmp_next();
     EXPECT_EQ(fdWtmp, -1)<<"check the status after wtmp_next()";
     EXPECT_EQ(tmp, nullptr)<<"check the status after wtmp_next()";
}

TEST(wtmpparse_wtmp_close_UT, wtmpparse_wtmp_close_UT_001)
{
    int result = wtmp_open(QString(WTMP_FILE).toLatin1().data());
    EXPECT_NE(result, -1);
    wtmp_close();
}

TEST(wtmpparse_wtmp_close_UT, wtmpparse_wtmp_close_UT_002)
{
    wtmp_close();
    EXPECT_EQ(fdWtmp, -1)<<"check the status after wtmp_close()";
}

TEST(wtmpparse_st_list_init_UT, wtmpparse_st_list_init_UT_001)
{
    utmp_list *utmListPtr = st_list_init();
    EXPECT_EQ(utmListPtr->value.ut_type, 0)<<"check the status after st_list_init()";
    free(utmListPtr);
}

TEST(wtmpparse_st_utmp_init_UT, wtmpparse_st_utmp_init_UT_001)
{
    utmp *utmptr = st_utmp_init();
    EXPECT_EQ(utmptr->ut_type, 0)<<"check the status after st_utmp_init()";
    free(utmptr);
}

TEST(wtmpparse_list_insert_UT, wtmpparse_list_insert_UT_001)
{
    QList<utmp *> plist;
    utmp *info = static_cast<utmp*>(malloc(sizeof(struct utmp)));
    list_insert(plist, info);
    EXPECT_EQ(plist.size(), 1);
    free(info);
}

TEST(wtmpparse_list_delete_UT, wtmpparse_list_delete_UT)
{
    utmp_list *list = st_list_init();
    utmp info;
    list->value = info;
    utmp_list *list2 = st_list_init();

    list2->value = info;
    list->next = list2;
    list= list_delete(list);
    EXPECT_EQ(list, list2)<<"check the status after list_delete()";
    free(list2);
}

TEST(wtmpparse_list_get_ele_and_del_UT, wtmpparse_list_get_ele_and_del_UT)
{
    QList<utmp> list;
    utmp info1;
    strcpy(info1.ut_line, "test1");
    list.append(info1);
    utmp info2;
    strcpy(info2.ut_line, "test2");
    list.append(info2);
    int result = -1;

    utmp info3 = list_get_ele_and_del(list, "test1", result);
    qDebug() << "info3.ut_line" << info3.ut_line;
    EXPECT_EQ(result, 0);
    EXPECT_EQ(QString(info3.ut_line) == QString("test1"), true);
    EXPECT_EQ(list.size(), 1);
}

TEST(wtmpparse_show_base_info_UT, wtmpparse_show_base_info_UT)
{
    utmp info1;
    strcpy(info1.ut_line, "test");
    show_base_info(&info1);
    EXPECT_NE(&info1, nullptr)<<"check the status after show_base_info()";
}
