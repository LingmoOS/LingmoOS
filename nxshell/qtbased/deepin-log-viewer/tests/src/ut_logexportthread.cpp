// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logexportthread.h"
#include "structdef.h"
#include "ut_stuballthread.h"
#include "../../3rdparty/DocxFactory/include/DocxFactory/WordProcessingMerger/WordProcessingMerger.h"
#include <stub.h>

#include <DApplication>
#include <DStandardItem>

#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QStandardItemModel>

#include <iostream>
#include <gtest/gtest.h>
void stub_load(const std::string &p_fileName)
{
    Q_UNUSED(p_fileName);
}

void stub_setClipboardValue(const std::string &p_itemName, const std::string &p_fieldName, const std::string &p_value)
{
    Q_UNUSED(p_itemName);
    Q_UNUSED(p_fieldName);
    Q_UNUSED(p_value);
}

void stub_paste(const std::string &p_itemName)
{
    Q_UNUSED(p_itemName);
}

void stub_save(const std::string &p_fileName)
{
    Q_UNUSED(p_fileName);
}

bool stub_dfwexists()
{
    return true;
}

bool stub_open(void *obj,QFile::OpenMode flags)
{
    QFile *o=reinterpret_cast<QFile*>(obj);
    Q_UNUSED(flags);
    return true;
}

QString stub_text()
{
    return "test";
}

class LogExportthread_UT : public testing::Test
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        exportThread = new LogExportThread();
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete exportThread;
    }
    LogExportThread *exportThread;
};

TEST_F(LogExportthread_UT, UT_ExportToTxtPublic_001){
    QStandardItemModel model;
    QStandardItem item;
    QList<QStandardItem *> itemList;
    itemList.append(&item);
    model.appendRow(&item);
    model.insertColumn(0, itemList);
    model.setData(model.index(0, 0), QVariant("ddd"), Qt::UserRole + 6);

    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString str("test");


    exportThread->exportToTxtPublic("test", &model, m_flag);
    EXPECT_EQ(exportThread->m_fileName,"test");
    EXPECT_EQ(exportThread->m_pModel,&model);
    EXPECT_EQ(exportThread->m_flag,m_flag);
    exportThread->exportToTxtPublic("test", m_journalList, QStringList() << "test", m_flag);
    EXPECT_EQ(exportThread->m_jList.size(),m_journalList.size());
    exportThread->exportToTxtPublic("test", m_appList, QStringList() << "test", str);
    EXPECT_EQ(exportThread->m_appList.size(),m_appList.size());
    exportThread->exportToTxtPublic("test", m_dpkgList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_dpkgList.size(),m_dpkgList.size());
    exportThread->exportToTxtPublic("test", m_bootList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_bootList.size(),m_bootList.size());
    exportThread->exportToTxtPublic("test", m_xorgList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_xorgList.size(),m_xorgList.size());
    exportThread->exportToTxtPublic("test", m_normalList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_normalList.size(),m_normalList.size());
    exportThread->exportToTxtPublic("test", m_kwinList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_kwinList.size(),m_kwinList.size());
    exportThread->exportToTxtPublic("test", m_dmesgList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_dmesgList.size(),m_dmesgList.size());
    exportThread->exportToTxtPublic("test", m_dnfList, QStringList() << "test");
    EXPECT_EQ(exportThread->m_dnfList.size(),m_dnfList.size());
}

TEST_F(LogExportthread_UT, UT_ExportToDocPublic_001){
    QStandardItemModel m_model;
    m_model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;
    exportThread->exportToDocPublic("test", &m_model, m_flag);
    exportThread->exportToDocPublic("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToDocPublic("test", m_appList, QStringList() << "test", test);
    exportThread->exportToDocPublic("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_bootList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_xorgList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_normalList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_kwinList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_dnfList, QStringList() << "test");
    exportThread->exportToDocPublic("test", m_dmesgList, QStringList() << "test");

    EXPECT_EQ(exportThread->m_fileName,"test");
    EXPECT_EQ(exportThread->m_pModel,&m_model);
    EXPECT_EQ(exportThread->m_flag,m_flag);
    EXPECT_EQ(exportThread->m_jList.size(),m_journalList.size());
    EXPECT_EQ(exportThread->m_appList.size(),m_appList.size());
    EXPECT_EQ(exportThread->m_bootList.size(),m_bootList.size());
    EXPECT_EQ(exportThread->m_dpkgList.size(),m_dpkgList.size());
    EXPECT_EQ(exportThread->m_dmesgList.size(),m_dmesgList.size());
    EXPECT_EQ(exportThread->m_dnfList.size(),m_dnfList.size());
}

TEST_F(LogExportthread_UT, UT_ExportToHtmlPublic_001){
    QStandardItemModel model;
    model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;
    exportThread->exportToHtmlPublic("test", &model, m_flag);
    exportThread->exportToHtmlPublic("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToHtmlPublic("test", m_appList, QStringList() << "test", test);
    exportThread->exportToHtmlPublic("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_bootList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_xorgList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_normalList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_kwinList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_dnfList, QStringList() << "test");
    exportThread->exportToHtmlPublic("test", m_dmesgList, QStringList() << "test");

    EXPECT_EQ(exportThread->m_fileName,"test");
    EXPECT_EQ(exportThread->m_pModel,&model);
    EXPECT_EQ(exportThread->m_flag,m_flag);
    EXPECT_EQ(exportThread->m_jList.size(),m_journalList.size());
    EXPECT_EQ(exportThread->m_appList.size(),m_appList.size());
    EXPECT_EQ(exportThread->m_bootList.size(),m_bootList.size());
    EXPECT_EQ(exportThread->m_dpkgList.size(),m_dpkgList.size());
    EXPECT_EQ(exportThread->m_dmesgList.size(),m_dmesgList.size());
    EXPECT_EQ(exportThread->m_dnfList.size(),m_dnfList.size());
}

TEST_F(LogExportthread_UT, UT_ExportToXlsPublic_001){
    QStandardItemModel m_model;
    m_model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;

    exportThread->exportToXlsPublic("test", &m_model, m_flag);
    exportThread->exportToXlsPublic("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToXlsPublic("test", m_appList, QStringList() << "test", test);
    exportThread->exportToXlsPublic("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_bootList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_xorgList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_normalList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_kwinList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_dnfList, QStringList() << "test");
    exportThread->exportToXlsPublic("test", m_dmesgList, QStringList() << "test");

    EXPECT_EQ(exportThread->m_fileName,"test");
    EXPECT_EQ(exportThread->m_pModel,&m_model);
    EXPECT_EQ(exportThread->m_flag,m_flag);
    EXPECT_EQ(exportThread->m_jList.size(),m_journalList.size());
    EXPECT_EQ(exportThread->m_appList.size(),m_appList.size());
    EXPECT_EQ(exportThread->m_bootList.size(),m_bootList.size());
    EXPECT_EQ(exportThread->m_dpkgList.size(),m_dpkgList.size());
    EXPECT_EQ(exportThread->m_dmesgList.size(),m_dmesgList.size());
    EXPECT_EQ(exportThread->m_dnfList.size(),m_dnfList.size());
}


TEST_F(LogExportthread_UT, ExportToText_UT)
{
    Stub stub;
    stub.set(ADDR(DocxFactory::WordProcessingMerger, load), stub_load);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, paste), stub_paste);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, save), stub_save);
    stub.set((void (DocxFactory::WordProcessingMerger::*)(const std::string &, const std::string &, const std::string &))ADDR(DocxFactory::WordProcessingMerger, setClipboardValue), stub_setClipboardValue);
    stub.set(ADDR(QStandardItem, text), stub_text);
    QStandardItemModel model;
    QStandardItem item;
    QList<QStandardItem *> itemList;
    itemList.append(&item);
    model.appendRow(&item);
    model.insertColumn(0, itemList);
    model.setData(model.index(0, 0), QVariant("ddd"), Qt::UserRole + 6);

    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString str("test");
    exportThread->m_canRunning = true;

    m_flag = LOG_FLAG::APP;
    bool res=exportThread->exportToTxt("/var/log/kern.log", &model, m_flag);
    EXPECT_EQ(res,false);
    bool res1=exportThread->exportToTxt("test", &model, m_flag);
    EXPECT_EQ(res1,true);

    m_flag = LOG_FLAG::JOURNAL;
    bool res2= exportThread->exportToTxt("test", &model, m_flag);
    EXPECT_EQ(res2,true);
    exportThread->exportToTxt("test", m_journalList, QStringList() << "test", m_flag);
    bool res3=exportThread->exportToTxt("/var/log/kern.log", m_journalList, QStringList() << "test", m_flag);
    EXPECT_EQ(res3,false);
    m_flag = LOG_FLAG::KERN;
    exportThread->exportToTxt("test", &model, m_flag);
    exportThread->exportToTxt("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToTxt("/var/log/kern.log", m_appList, QStringList() << "test", str);
    exportThread->exportToTxt("test", m_appList, QStringList() << "test", str);
    exportThread->exportToTxt("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_dpkgList, QStringList() << "test");
    exportThread->exportToTxt("test", m_bootList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_bootList, QStringList() << "test");
    exportThread->exportToTxt("test", m_xorgList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_xorgList, QStringList() << "test");
    exportThread->exportToTxt("test", m_normalList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_normalList, QStringList() << "test");
    exportThread->exportToTxt("test", m_kwinList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_kwinList, QStringList() << "test");
    exportThread->exportToTxt("test", m_dmesgList, QStringList() << "test");
    exportThread->exportToTxt("/var/log/kern.log", m_dmesgList, QStringList() << "test");
    exportThread->exportToTxt("test", m_dnfList, QStringList() << "test");
    bool res4=exportThread->exportToTxt("/var/log/kern.log", m_dnfList, QStringList() << "test");
    EXPECT_EQ(res4,false);

    exportThread->m_canRunning=false;
    exportThread->exportToTxt("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToTxt("test", &model, m_flag);
    exportThread->exportToTxt("test", m_appList, QStringList() << "test", str);
    exportThread->exportToTxt("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToTxt("test", m_bootList, QStringList() << "test");
    exportThread->exportToTxt("test", m_xorgList, QStringList() << "test");;
    exportThread->exportToTxt("test", m_normalList, QStringList() << "test");
    exportThread->exportToTxt("test", m_kwinList, QStringList() << "test");
    exportThread->exportToTxt("test", m_dmesgList, QStringList() << "test");
    bool res5=exportThread->exportToTxt("test", m_dnfList, QStringList() << "test");
    EXPECT_EQ(res5,false);

    QStandardItemModel *modelptr=&model;
    modelptr=nullptr;
    exportThread->exportToTxt("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToTxt("test", modelptr, m_flag);
    exportThread->exportToTxt("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToTxt("test", m_appList, QStringList() << "test", str);
    exportThread->exportToTxt("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToTxt("test", m_bootList, QStringList() << "test");
    exportThread->exportToTxt("test", m_xorgList, QStringList() << "test");;
    exportThread->exportToTxt("test", m_normalList, QStringList() << "test");
    exportThread->exportToTxt("test", m_kwinList, QStringList() << "test");
    exportThread->exportToTxt("test", m_dmesgList, QStringList() << "test");
    bool res6=exportThread->exportToTxt("test", m_dnfList, QStringList() << "test");
    EXPECT_EQ(res6,false);
}

TEST_F(LogExportthread_UT, ExportToDoc_UT)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_dfwexists);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, load), stub_load);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, paste), stub_paste);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, save), stub_save);
    stub.set((void (DocxFactory::WordProcessingMerger::*)(const std::string &, const std::string &, const std::string &))ADDR(DocxFactory::WordProcessingMerger, setClipboardValue), stub_setClipboardValue);
    QStandardItemModel m_model;
    m_model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;


    exportThread->exportToDoc("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToDoc("test", m_appList, QStringList() << "test", test);
    exportThread->exportToDoc("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToDoc("test", m_bootList, QStringList() << "test");
    exportThread->exportToDoc("test", m_xorgList, QStringList() << "test");
    exportThread->exportToDoc("test", m_normalList, QStringList() << "test");
    exportThread->exportToDoc("test", m_kwinList, QStringList() << "test");
    exportThread->exportToDoc("test", m_dmesgList, QStringList() << "test");
    exportThread->exportToDoc("test", m_dnfList, QStringList() << "test");

    m_flag = LOG_FLAG::KERN;
    exportThread->exportToDoc("test", m_journalList, QStringList() << "test", m_flag);
    m_flag =LOG_FLAG::BOOT;
    exportThread->exportToDoc("test", m_journalList, QStringList() << "test", m_flag);

    m_flag =LOG_FLAG::JOURNAL;
    exportThread->m_canRunning=false;
    exportThread->exportToDoc("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToDoc("test", m_appList, QStringList() << "test", test);
    exportThread->exportToDoc("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToDoc("test", m_bootList, QStringList() << "test");
    exportThread->exportToDoc("test", m_xorgList, QStringList() << "test");
    exportThread->exportToDoc("test", m_normalList, QStringList() << "test");
    exportThread->exportToDoc("test", m_kwinList, QStringList() << "test");
    exportThread->exportToDoc("test", m_dmesgList, QStringList() << "test");
    bool res=exportThread->exportToDoc("test", m_dnfList, QStringList() << "test");
    EXPECT_EQ(res,false);
}

TEST_F(LogExportthread_UT, ExportToHtml_UT)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_dfwexists);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, load), stub_load);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, paste), stub_paste);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, save), stub_save);
    stub.set((void (DocxFactory::WordProcessingMerger::*)(const std::string &, const std::string &, const std::string &))ADDR(DocxFactory::WordProcessingMerger, setClipboardValue), stub_setClipboardValue);
    stub.set(ADDR(QStandardItem, text), stub_text);
    QStandardItemModel model;
    model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;


    model.clear();
    QStandardItem item;
    QList<QStandardItem *> itemList;
    itemList.append(&item);
    model.appendRow(&item);
    model.insertColumn(0, itemList);
    model.setData(model.index(0, 0), QVariant("ddd"), Qt::UserRole + 6);

    m_flag = LOG_FLAG::APP;
     exportThread->exportToHtml("/var/log/kern.log", &model, m_flag);
    exportThread->exportToHtml("test", &model, m_flag);
    m_flag = LOG_FLAG::JOURNAL;
    exportThread->exportToHtml("test", &model, m_flag);
    exportThread->exportToHtml("/var/log/kern.log", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToHtml("test", m_journalList, QStringList() << "test", m_flag);
    m_flag = LOG_FLAG::KERN;
    exportThread->exportToHtml("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToHtml("/var/log/kern.log", m_appList, QStringList() << "test", test);
    exportThread->exportToHtml("/var/log/kern.log", m_dpkgList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_bootList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_xorgList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_normalList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_kwinList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_dnfList, QStringList() << "test");
    exportThread->exportToHtml("/var/log/kern.log", m_dmesgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_appList, QStringList() << "test", test);
    exportThread->exportToHtml("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_bootList, QStringList() << "test");
    exportThread->exportToHtml("test", m_xorgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_normalList, QStringList() << "test");
    exportThread->exportToHtml("test", m_kwinList, QStringList() << "test");
    exportThread->exportToHtml("test", m_dnfList, QStringList() << "test");
    exportThread->exportToHtml("test", m_dmesgList, QStringList() << "test");

    exportThread->m_canRunning=false;
    exportThread->exportToHtml("test", &model, m_flag);
    exportThread->exportToHtml("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToHtml("test", m_appList, QStringList() << "test", test);
    exportThread->exportToHtml("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_bootList, QStringList() << "test");
    exportThread->exportToHtml("test", m_xorgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_normalList, QStringList() << "test");
    exportThread->exportToHtml("test", m_kwinList, QStringList() << "test");
    exportThread->exportToHtml("test", m_dnfList, QStringList() << "test");
    exportThread->exportToHtml("test", m_dmesgList, QStringList() << "test");

    QStandardItemModel *modelptr=&model;
    modelptr=nullptr;
    exportThread->exportToHtml("test", modelptr, m_flag);
    exportThread->exportToHtml("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToHtml("test", m_appList, QStringList() << "test", test);
    exportThread->exportToHtml("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_bootList, QStringList() << "test");
    exportThread->exportToHtml("test", m_xorgList, QStringList() << "test");
    exportThread->exportToHtml("test", m_normalList, QStringList() << "test");
    exportThread->exportToHtml("test", m_kwinList, QStringList() << "test");
    exportThread->exportToHtml("test", m_dnfList, QStringList() << "test");
    bool res=exportThread->exportToHtml("test", m_dmesgList, QStringList() << "test");
    EXPECT_EQ(res,false);
}

TEST_F(LogExportthread_UT, ExportToXls_UT)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_dfwexists);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, load), stub_load);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, paste), stub_paste);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, save), stub_save);
    stub.set((void (DocxFactory::WordProcessingMerger::*)(const std::string &, const std::string &, const std::string &))ADDR(DocxFactory::WordProcessingMerger, setClipboardValue), stub_setClipboardValue);
    QStandardItemModel m_model;
    m_model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    QString test = "test";
    exportThread->m_canRunning = true;

    exportThread->exportToXls("test", m_journalList, QStringList() << "test", m_flag);
    m_flag = LOG_FLAG::KERN;
    exportThread->exportToXls("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToXls("test", m_appList, QStringList() << "test", test);
    exportThread->exportToXls("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToXls("test", m_bootList, QStringList() << "test");
    exportThread->exportToXls("test", m_xorgList, QStringList() << "test");
    exportThread->exportToXls("test", m_normalList, QStringList() << "test");
    exportThread->exportToXls("test", m_kwinList, QStringList() << "test");
    exportThread->exportToXls("test", m_dnfList, QStringList() << "test");
    exportThread->exportToXls("test", m_dmesgList, QStringList() << "test");

    exportThread->m_canRunning=false;
    exportThread->exportToXls("test", m_journalList, QStringList() << "test", m_flag);
    exportThread->exportToXls("test", m_appList, QStringList() << "test", test);
    exportThread->exportToXls("test", m_dpkgList, QStringList() << "test");
    exportThread->exportToXls("test", m_bootList, QStringList() << "test");
    exportThread->exportToXls("test", m_xorgList, QStringList() << "test");
    exportThread->exportToXls("test", m_normalList, QStringList() << "test");
    exportThread->exportToXls("test", m_kwinList, QStringList() << "test");
    exportThread->exportToXls("test", m_dnfList, QStringList() << "test");
    bool res=exportThread->exportToXls("test", m_dmesgList, QStringList() << "test");
    EXPECT_EQ(res,false);
}

TEST_F(LogExportthread_UT, ExportRun_UT)
{
    Stub stub;
    typedef bool (QFile::*fptr)() const;
    fptr A_foo = (fptr)(&QFile::exists); //获取虚函数地址
    stub.set(A_foo, stub_dfwexists);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, load), stub_load);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, paste), stub_paste);
    stub.set(ADDR(DocxFactory::WordProcessingMerger, save), stub_save);
    stub.set((void (DocxFactory::WordProcessingMerger::*)(const std::string &, const std::string &, const std::string &))ADDR(DocxFactory::WordProcessingMerger, setClipboardValue), stub_setClipboardValue);
    QStandardItemModel m_model;
    m_model.appendRow(new QStandardItem());
    LOG_FLAG m_flag = LOG_FLAG::JOURNAL;
    struct LOG_MSG_JOURNAL m_journal = {"20190503", "10:02", "uos", "d", "1", "2"};
    struct LOG_MSG_APPLICATOIN m_app = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_DPKG m_dpkg = {"20190503", "10:02", "uos"};
    struct LOG_MSG_BOOT m_boot = {"20190503", "10:02"};
    struct LOG_MSG_XORG m_xorg = {"20190503", "10:02"};
    struct LOG_MSG_NORMAL m_normal = {"20190503", "10:02", "uos", "d"};
    struct LOG_MSG_KWIN m_kwin = {"test"};
    struct LOG_MSG_DNF m_dnf = {"20190503", "waring", "test"};
    struct LOG_MSG_DMESG m_dmesg = {"20190503", "waring", "test"};

    QList<LOG_MSG_JOURNAL> m_journalList {m_journal};
    QList<LOG_MSG_APPLICATOIN> m_appList {m_app};
    QList<LOG_MSG_DPKG> m_dpkgList {m_dpkg};
    QList<LOG_MSG_BOOT> m_bootList {m_boot};
    QList<LOG_MSG_XORG> m_xorgList {m_xorg};
    QList<LOG_MSG_NORMAL> m_normalList {m_normal};
    QList<LOG_MSG_KWIN> m_kwinList {m_kwin};
    QList<LOG_MSG_DNF> m_dnfList {m_dnf};
    QList<LOG_MSG_DMESG> m_dmesgList {m_dmesg};
    exportThread->m_canRunning = false;

    exportThread->m_runMode = LogExportThread::HtmlModel;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlAPP;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlDNF;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlBOOT;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlDPKG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlKWIN;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlDMESG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlXORG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlNORMAL;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::HtmlJOURNAL;
    exportThread->run();

    exportThread->m_runMode = LogExportThread::TxtModel;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtAPP;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtDNF;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtBOOT;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtDPKG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtKWIN;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtDMESG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtXORG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtNORMAL;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::TxtJOURNAL;
    exportThread->run();

    exportThread->m_runMode = LogExportThread::DocAPP;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocDNF;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocBOOT;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocDPKG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocKWIN;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocDMESG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocXORG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocNORMAL;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::DocJOURNAL;
    exportThread->run();

    exportThread->m_runMode = LogExportThread::XlsAPP;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsDNF;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsBOOT;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsDPKG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsKWIN;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsDMESG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsXORG;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsNORMAL;
    exportThread->run();
    exportThread->m_runMode = LogExportThread::XlsJOURNAL;
    exportThread->run();
    EXPECT_NE(exportThread,nullptr);
}

TEST_F(LogExportthread_UT, stopImmediately_UT){
    exportThread->stopImmediately();
    EXPECT_EQ(false,exportThread->m_canRunning);
}

TEST_F(LogExportthread_UT, isProcessing_UT){
    bool res=exportThread->isProcessing();
     EXPECT_EQ(res,false);
}

