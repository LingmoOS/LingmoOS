// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../application/utils.h"
#include "ut_logViewerPlugin.h"

#include <stub.h>

LogViewerPlugin_UT::LogViewerPlugin_UT()
{

}

void LogViewerPlugin_UT::SetUp()
{
    m_instance = new LogViewerPlugin;
    qDebug() << "SetUp" << endl;
}

void LogViewerPlugin_UT::TearDown()
{
    delete m_instance;
}

void QThreadPool_start(QRunnable *runnable, int priority)
{
    qDebug() << "QThreadPool_start--";
}


void QThread_start(QThread::Priority)
{
    qDebug() << "QThread_start--";
}

TEST_F(LogViewerPlugin_UT, generateAppFile_UT001)
{
    Utils::sleep(200);
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), QThreadPool_start);
    stub.set(ADDR(QThread, start), QThread_start);

    LogViewerPlugin *p = new LogViewerPlugin;
    p->generateAppFile(Utils::homePath + "/.cache/deepin/deepin-log-viewer/deepin-log-viewer.log", ALL, INF, "");
    connect(p, &LogViewerPlugin::sigAppData, this, [=](int index, QList<LOG_MSG_APPLICATOIN> iList) {
        qDebug() << "index:" << index << endl;
        qDebug() << "appDatas:" << endl;
        int listCount = iList.size();
        for (int i = 0; i < listCount; i++) {
            qDebug() << "datetime=" << iList[i].dateTime
                     << " level=" << iList[i].level
                     << " src=" << iList[i].src
                     << " msg=" << iList[i].msg
                     << " detailInfo=" << iList[i].detailInfo << endl;
        }
    });
    connect(p, &LogViewerPlugin::sigAppFinished, p, &LogViewerPlugin::deleteLater);
}

TEST_F(LogViewerPlugin_UT, exportLogFile_UT001)
{
    Utils::sleep(200);
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), QThreadPool_start);
    stub.set(ADDR(QThread, start), QThread_start);

    LogViewerPlugin *p = new LogViewerPlugin;
    p->exportAppLogFile("/home/Desktop/test-plugin.txt", ALL, INF, "deepin-log-viewer");
    connect(p, &LogViewerPlugin::sigExportResult, p, &LogViewerPlugin::deleteLater);
}

class LogViewerPlugin_generateAppFile_UT_Param
{
public:
    explicit LogViewerPlugin_generateAppFile_UT_Param(BUTTONID id)
    {
        time = id;
    }
    BUTTONID time;
};

class LogViewerPlugin_generateAppFile_UT : public ::testing::TestWithParam<LogViewerPlugin_generateAppFile_UT_Param>
{
    virtual void nothing();//消除编译警告
};

void LogViewerPlugin_generateAppFile_UT::nothing(){}

INSTANTIATE_TEST_CASE_P(LogViewerPlugin, LogViewerPlugin_generateAppFile_UT,
                        ::testing::Values(LogViewerPlugin_generateAppFile_UT_Param(ALL), LogViewerPlugin_generateAppFile_UT_Param(ONE_DAY),
                        LogViewerPlugin_generateAppFile_UT_Param(THREE_DAYS), LogViewerPlugin_generateAppFile_UT_Param(ONE_WEEK),
                        LogViewerPlugin_generateAppFile_UT_Param(ONE_MONTH), LogViewerPlugin_generateAppFile_UT_Param(THREE_MONTHS)));

TEST_P(LogViewerPlugin_generateAppFile_UT, LogViewerPlugin_generateAppFile_UT_001)
{
    Stub stub;
    stub.set((void (QThreadPool::*)(QRunnable *, int))ADDR(QThreadPool, start), QThreadPool_start);
    stub.set(ADDR(QThread, start), QThread_start);

    LogViewerPlugin_generateAppFile_UT_Param param = GetParam();
    LogViewerPlugin *p = new LogViewerPlugin();
    EXPECT_NE(p, nullptr);
    Utils::sleep(200);
    p->generateAppFile("", param.time, INF, "");
    p->deleteLater();
}

//TEST_P(LogViewerPlugin_generateAppFile_UT, LogViewerPlugin_generateAppFile_UT_001)
//{
//    Stub stub;
//    stub.set(ADDR(JournalBootWork, getReplaceColorStr), stub_getReplaceColorStr_bootwork);
//    stub.set(ADDR(journalWork, getReplaceColorStr), stub_getReplaceColorStr_bootwork);
//    stub.set(ADDR(LogFileParser, parseByApp), LogFileParser_parseByApp);
//    LogViewerPlugin_generateAppFile_UT_Param param = GetParam();
//    QPluginLoader loader("ProcessMultimeterD.dll");
//    if(loader.load())
//    {
//        if(QObject * plugin = loader.instance())
//        {
//            LogViewerPluginInterface *p = dynamic_cast<LogViewerPluginInterface *>(plugin);
//            qDebug()<<p;
//            p->generateAppFile("", param.time, 7, "");
//        }
//    }
//    else
//    {
//        qDebug()<<loader.errorString();
//    }
//}

