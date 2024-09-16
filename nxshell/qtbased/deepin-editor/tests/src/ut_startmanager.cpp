// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_startmanager.h"
#include "src/stub.h"
#include "qdir.h"
#include "../../src/widgets/window.h"

namespace startmanagerstub {

EditWrapper *wrapperstub(const QString &filePath)
{
    return new EditWrapper;
}

StartManager::FileTabInfo getFileTabInfostub()
{
    return StartManager::FileTabInfo{1,1};
}

TextEdit *textEditorstub()
{
    return new TextEdit;
}
void returnstub()
{
    return;
}


bool returntruestub()
{
    return true;
}

bool returnfalsestub()
{
    return false;
}

QStringList entryliststub()
{
    return QStringList{"123","456"};
}

}

using namespace startmanagerstub;

UT_StartManager::UT_StartManager()
{

}

TEST(UT_StartManager_StartManager, StartManager)
{
    //StartManager startManager(nullptr);
    StartManager *pStartManager = new StartManager(nullptr);
    ASSERT_TRUE(pStartManager != nullptr);

    pStartManager->deleteLater();
}

TEST(UT_StartManager_instance, instance)
{
    StartManager *pStartManager = StartManager::instance();
    ASSERT_TRUE(pStartManager != nullptr);

    pStartManager->deleteLater();
}

TEST(UT_StartManager_openFilesInWindow, openFilesInWindow)
{
    StartManager *startManager = StartManager::instance();
    QStringList filePathList;
    filePathList<<".cache/deepin/deepin-editor";
    startManager->openFilesInWindow(filePathList);

    filePathList<<".cache/deepin/";
    startManager->openFilesInWindow(filePathList);
    ASSERT_TRUE(startManager->m_windows.at(0) != nullptr);

    startManager->deleteLater();
}

//initWindowPosition
TEST(UT_StartManager_initWindowPosition, initWindowPosition)
{
    StartManager *startManager = StartManager::instance();
    startManager->openFilesInTab(QStringList());
    ASSERT_TRUE(startManager->m_windows.at(0) != nullptr);

    startManager->deleteLater();
}

//getFileTabInfo
TEST(UT_StartManager_getFileTabInfo, getFileTabInfo)
{
    StartManager *startManager = StartManager::instance();
    StartManager::FileTabInfo fileTabInfo = startManager->getFileTabInfo(".cache/deepin/deepin-editor");

    ASSERT_TRUE(fileTabInfo.windowIndex != 0);
    startManager->deleteLater();
}

//slotCheckUnsaveTab
TEST(UT_StartManager_slotCheckUnsaveTab, slotCheckUnsaveTab)
{
    StartManager *startManager = StartManager::instance();
    startManager->openFilesInWindow(QStringList());
    startManager->slotCheckUnsaveTab();

    bool bRet = startManager->m_windows.at(0)->checkBlockShutdown();
    ASSERT_TRUE(bRet == false);

    startManager->deleteLater();
}

//checkPath
TEST(UT_StartManager_checkPath, checkPath_001)
{
    QStringList fileLists;
    QString strFilePath(QCoreApplication::applicationDirPath() + "/Makefile");
    fileLists << strFilePath;
    StartManager *pStartManager = StartManager::instance();
    pStartManager->openFilesInTab(fileLists);
    bool bRet = pStartManager->checkPath(strFilePath);
    ASSERT_TRUE(bRet == false);
    
    pStartManager->deleteLater();
}

//checkPath
TEST(UT_StartManager_checkPath, checkPath_002)
{
    StartManager *pStartManager = StartManager::instance();
    pStartManager->openFilesInTab(QStringList());
    bool bRet = pStartManager->checkPath(QString());
    ASSERT_TRUE(bRet == true);

    pStartManager->deleteLater();
}

TEST(UT_StartManager_ifKlu,ifKlu )
{
    StartManager *startManager = StartManager::instance();
    bool bRet = startManager->ifKlu();
    ASSERT_TRUE(bRet == false);

    startManager->deleteLater();
}

//loadTheme
TEST(UT_StartManager_loadThem,loadTheme)
{
    StartManager *startManager = StartManager::instance();
    startManager->loadTheme("Dark");
    ASSERT_TRUE(startManager != nullptr);

    startManager->deleteLater();
}

// bool isMultiWindow();
TEST(UT_StartManager_isMultiWindow, isMultiWindow_001)
{
    QStringList fileLists;
    QString strMakeFilePath(QCoreApplication::applicationDirPath() + "/Makefile");
    QString strCMakePath(QCoreApplication::applicationDirPath() + "/CMakeCache.txt");
    fileLists << strMakeFilePath << strCMakePath;
    StartManager *pStartManager = StartManager::instance();
    bool bRet = pStartManager->isMultiWindow();
    ASSERT_TRUE(bRet == true);
    
    pStartManager->deleteLater();
}

// bool isMultiWindow();
TEST(UT_StartManager_isMultiWindow, isMultiWindow_002)
{
    StartManager *pStartManager = StartManager::instance();
    pStartManager->m_windows.clear();
    bool bRet = pStartManager->isMultiWindow();
    ASSERT_TRUE(bRet == false);

    pStartManager->deleteLater();
}

TEST(UT_StartManager_isTemFilesEmpty, isTemFilesEmpty_001)
{
    QStringList fileLists;
    QString strFilePath(QCoreApplication::applicationDirPath() + "/Makefile");
    fileLists << strFilePath;
    StartManager *pStartManager = StartManager::instance();
    pStartManager->m_qlistTemFile.clear();
    pStartManager->openFilesInTab(fileLists);
    pStartManager->m_qlistTemFile << QString("");
    bool bRet = pStartManager->isTemFilesEmpty();
    ASSERT_TRUE(bRet == true);
    
    pStartManager->deleteLater();
}

TEST(UT_StartManager_isTemFilesEmpty, isTemFilesEmpty_002)
{
    StartManager *pStartManager = StartManager::instance();
    pStartManager->m_qlistTemFile.clear();
    bool bRet = pStartManager->isTemFilesEmpty();
    ASSERT_TRUE(bRet == false);

    pStartManager->deleteLater();
}

TEST(UT_StartManager_autoBackupFile,autoBackupFile)
{
    StartManager *startManager = StartManager::instance();
    Window* w1 = new Window;

    EditWrapper* wr1 = new EditWrapper;

    TextEdit* e1 = new TextEdit;
    e1->m_wrapper = wr1;
    e1->m_listBookmark = {1,2};
    wr1->m_bIsTemFile = true;
    wr1->m_pTextEdit = e1;

    QList<Window*> windows = {w1};
    QMap<QString, EditWrapper *> wrappers{ {"wr1",wr1}};
    startManager->m_windows = windows;
    w1->m_wrappers = wrappers;

    Stub s1;
    s1.set(ADDR(QStringList,replace),returnstub);
    Stub s2;
    s2.set(ADDR(StartManager,getFileTabInfo),getFileTabInfostub);
    s2.set(ADDR(EditWrapper,saveTemFile),returnstub);

    startManager->autoBackupFile();
    EXPECT_NE(startManager , nullptr);

    startManager->deleteLater();
    w1->deleteLater();
    wr1->deleteLater();
    e1->deleteLater();
}

TEST(UT_StartManager_recoverFile,recoverFile_001)
{
    StartManager *startManager = StartManager::instance();
    QString c1 = "{\"bookMark\":\"7,7,8,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\",\"cursorPosition\":\"7\",\"focus\":true,\"localPath\":\"/home/uos/.local/share/deepin/deepin-editor/blank-files/blank_file_2021-09-22_09-43-10-824\",\"modify\":true}";
    QString c2 = "{\"bookMark\":\"1,0,1,0,0\",\"cursorPosition\":\"23\",\"localPath\":\"/home/uos/Desktop/563/526.txt\",\"modify\":false}";
    startManager->m_qlistTemFile.push_back(c1);
    startManager->m_qlistTemFile.push_back(c2);

    Stub s1;
    s1.set(ADDR(Window,wrapper),wrapperstub);
    Stub s2;
    s2.set(ADDR(StartManager,popupExistTabs),returnstub);

    Window *pWindow = new Window;
    int iRet = startManager->recoverFile(pWindow);
    ASSERT_TRUE(iRet == 0);
    
    pWindow->deleteLater();
    startManager->deleteLater();
}

TEST(UT_StartManager_recoverFile,recoverFile_002)
{
    StartManager *startManager = StartManager::instance();
    QString c1 = "{\"bookMark\":\"7,7,8,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\",\"cursorPosition\":\"7\",\"focus\":true,\"localPath\":\"/home/uos/.local/share/deepin/deepin-editor/blank-files/blank_file_2021-09-22_09-43-10-824\",\"modify\":true}";
    QString c2 = "{\"bookMark\":\"1,0,1,0,0\",\"cursorPosition\":\"23\",\"localPath\":\"/home/uos/Desktop/563/526.txt\",\"modify\":false}";
    startManager->m_qlistTemFile.push_back(c1);
    startManager->m_qlistTemFile.push_back(c2);

    Stub s1;
    s1.set(ADDR(Window,wrapper),wrapperstub);
    Stub s2;
    s2.set(ADDR(StartManager,popupExistTabs),returnstub);
    Stub s3;
    s3.set((bool(QJsonObject::*)(const QString &) const)ADDR(QJsonObject,contains), returntruestub);
    Stub s4;
    s4.set(ADDR(QString,isEmpty),returnfalsestub);
    Stub s5;
    s5.set(ADDR(Utils,fileExists),returntruestub);
    Stub s6;
    s6.set(ADDR(QJsonValue,isDouble),returntruestub);


    Window w;
    int iRet = startManager->recoverFile(&w);
    ASSERT_TRUE(iRet == 0);

    startManager->deleteLater();
}

TEST(UT_StartManager_openFilesInTab,openFilesInTab_001)
{
    StartManager *startManager = StartManager::instance();
    startManager->m_windows.clear();

    Stub s1;
    s1.set(ADDR(Window,wrapper),wrapperstub);
    Stub s2;
    s2.set(ADDR(StartManager,checkPath),returntruestub);


    startManager->openFilesInTab(QStringList());
    startManager->openFilesInTab(QStringList(".cache/deepin/deepin-editor"));

    startManager->slotCloseWindow();
    startManager->slotCreatNewwindow();

    ASSERT_TRUE(startManager->m_windows.count() > 0);
    startManager->deleteLater();
}

TEST(UT_StartManager_openFilesInTab,openFilesInTab_002)
{
    StartManager *startManager = StartManager::instance();
    startManager->m_windows.clear();

    Stub s1;
    s1.set(ADDR(Window,wrapper),wrapperstub);
    Stub s2;
    s2.set(ADDR(StartManager,checkPath),returntruestub);
    Stub s3;
    s3.set(ADDR(StartManager,isTemFilesEmpty),returntruestub);

    startManager->openFilesInTab(QStringList());
    startManager->openFilesInTab(QStringList(".cache/deepin/deepin-editor"));

    startManager->slotCloseWindow();
    startManager->slotCreatNewwindow();

    ASSERT_TRUE(startManager->m_windows.count() > 0);
    startManager->deleteLater();
}

TEST(UT_StartManager_openFilesInTab,openFilesInTab_003)
{
    StartManager *startManager = StartManager::instance();
    startManager->m_windows.clear();

    Stub s1;
    s1.set(ADDR(Window,wrapper),wrapperstub);
    Stub s2;
    s2.set(ADDR(StartManager,checkPath),returntruestub);
    Stub s3;
    s3.set(ADDR(StartManager,isTemFilesEmpty),returnfalsestub);

    startManager->openFilesInTab(QStringList());
    startManager->openFilesInTab(QStringList(".cache/deepin/deepin-editor"));

    startManager->slotCloseWindow();
    startManager->slotCreatNewwindow();

    ASSERT_TRUE(startManager->m_windows.count() > 0);
    startManager->deleteLater();
}

TEST(UT_StartManager_analyzeBookmakeInfo,analyzeBookmakeInfo)
{
    StartManager *startManager = StartManager::instance();
    QList<int> list = startManager->analyzeBookmakeInfo(QString());
    ASSERT_TRUE(!list.isEmpty());
    
    list = {0, 1, 2};
    EXPECT_EQ(startManager->analyzeBookmakeInfo("0,1,2"), list);
    list = {999, 2, 1};
    EXPECT_EQ(startManager->analyzeBookmakeInfo("999, 2, 1"), list);
    list = {0, 1, 999};
    EXPECT_EQ(startManager->analyzeBookmakeInfo("0, 1, 999"), list);

    startManager->deleteLater();
}

TEST(UT_StartManager_slotCloseWindow,slotCloseWindow)
{
    StartManager *startManager = StartManager::instance();
    startManager->m_windows.clear();

    Stub s1;
    s1.set((QStringList(QDir::*)(QDir::Filters ,QDir::SortFlags) const)ADDR(QDir,entryList), entryliststub);
    Stub s2;
    s2.set((bool(QString::*)(const QString &,Qt::CaseSensitivity cs) const)ADDR(QString,contains), returntruestub);
    Stub s3;
    s3.set(ADDR(QList<Window*>,isEmpty), returntruestub);

    startManager->slotCloseWindow();

    startManager->deleteLater();
}

TEST(UT_StartManager_bookmark, recordBookmark_normal_pass)
{
    StartManager *startManager = new StartManager;
    QString path("test");
    QList<int> bookmark {0, 1, 2};
    startManager->recordBookmark(path, bookmark);

    EXPECT_FALSE(startManager->m_bookmarkTable.isEmpty());
    EXPECT_EQ(startManager->m_bookmarkTable.value(path), bookmark);
    delete startManager;
}

TEST(UT_StartManager_bookmark, findBookmark_normal_pass)
{
    StartManager *startManager = new StartManager;
    QString path("test");
    QList<int> bookmark {0, 1, 2};
    startManager->m_bookmarkTable.insert(path, bookmark);

    EXPECT_EQ(startManager->findBookmark(path), bookmark);
    EXPECT_TRUE(startManager->findBookmark(QString("")).isEmpty());

    delete startManager;
}

// stub打桩函数
QStringList toStringListStub()
{
    return {"{\"bookmark\":\"0,1\",\"localPath\":\"test\"}"};
}

bool fileIsExistsStub(const QString &path)
{
    Q_UNUSED(path)
    return true;
}

TEST(UT_StartManager_bookmark, initBookmark_normal_pass)
{
    StartManager *startManager = new StartManager;

    typedef bool (*FileExistFunc)(const QString &);
    Stub stub;
    stub.set(ADDR(QVariant, toStringList), toStringListStub);
    stub.set((FileExistFunc)ADDR(QFileInfo, exists), fileIsExistsStub);

    startManager->initBookmark();

    EXPECT_FALSE(startManager->m_bookmarkTable.isEmpty());
    QList<int> bookmark {0, 1};
    EXPECT_EQ(startManager->findBookmark(QString("test")), bookmark);

    delete startManager;
}

TEST(UT_StartManager_bookmark, saveBookmark_normal_pass)
{
    StartManager *startManager = new StartManager;

    typedef bool (*FileExistFunc)(const QString &);
    Stub stub;
    stub.set((FileExistFunc)ADDR(QFileInfo, exists), fileIsExistsStub);

    QString path("test");
    QList<int> bookmark {0, 1};
    startManager->m_bookmarkTable.insert(path, bookmark);
    startManager->saveBookmark();

    QStringList saveData = Settings::instance()->settings->value("advance.editor.bookmark").toStringList();
    EXPECT_FALSE(saveData.isEmpty());
    if (!saveData.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(saveData.first().toUtf8());
        QJsonObject obj = doc.object();
        EXPECT_EQ(obj.value("localPath").toString(), path);
        QList<int> parseBookmark = startManager->analyzeBookmakeInfo(obj.value("bookmark").toString());
        EXPECT_EQ(bookmark, parseBookmark);
    }

    delete startManager;
}
