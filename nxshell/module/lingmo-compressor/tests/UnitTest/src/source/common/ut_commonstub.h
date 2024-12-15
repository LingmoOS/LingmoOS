// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_COMMONSTUB_H
#define UT_COMMONSTUB_H

#include "popupdialog.h"
#include "libzipplugin.h"
#include "gtest/src/stub.h"

#include <DGuiApplicationHelper>

#include <QObject>
#include <QTreeView>
#include <QFileInfoList>

DGUI_USE_NAMESPACE

extern bool g_QWidget_isVisible_result;                 // QWidget isVisible返回值
extern bool g_UiTools_isArchiveFile_result;             // UiTools isArchiveFile返回值
extern QObject *g_QObject_sender_result;                // QObject sender返回值
extern int g_Dialog_exec_result;                        // DDialog exec返回值
extern bool g_UiTools_isLocalDeviceFile_result;         // UiTools isLocalDeviceFile返回值
extern ReadOnlyArchiveInterface *g_UiTools_createInterface_result;         // UiTools createInterface返回值
extern QString g_QFileDialog_getOpenFileName_result;    // QFileDialog getOpenFileName返回值
extern bool g_QThread_isRunning_result;                 // QThread isRunning返回值
extern bool g_QThread_wait_result;                      // QThread wait返回值

extern int g_TipDialog_showDialog_result;               // TipDialog showDialog返回值
extern int g_SimpleQueryDialog_showDialog_result;       // SimpleQueryDialog showDialog返回值
extern Overwrite_Result g_OverwriteQueryDialog_getDialogResult_result;  // OverwriteQueryDialog getDialogResult返回值
extern bool g_OverwriteQueryDialog_getApplyAll_result;  // OverwriteQueryDialog getApplyAll返回值
extern int g_AppendDialog_showDialog_result;            // AppendDialog showDialog返回值
extern QString g_OpenWithDialog_showOpenWithDialog_result;  // OpenWithDialog showOpenWithDialog返回值

extern QString g_QFileInfo_path_result;                 // QFileInfo path返回值
extern QString g_QFileInfo_filePath_result;             // QFileInfo filePath返回值
extern QString g_QFileInfo_fileName_result;             // QFileInfo fileName返回值
extern QString g_QFileInfo_completeBaseName_result;     // QFileInfo completeBaseName返回值
extern qint64 g_QFileInfo_size_result;                  // QFileInfo size返回值
extern bool g_QFileInfo_isDir_result;                   // QFileInfo isDir返回值
extern bool g_QFileInfo_exists_result;                  // QFileInfo exists返回值
extern bool g_QFileInfo_isWritable_result;              // QFileInfo isWritable返回值
extern bool g_QFileInfo_isExecutable_result;            // QFileInfo isExecutable返回值
extern bool g_QFileInfo_isReadable_result;              // QFileInfo isExecutable返回值
extern bool g_QFileInfo_isSymLink_result;               // QFileInfo isSymLink返回值
extern QString g_QFileInfo_completeSuffix_result;       // QFileInfo completeSuffix返回值

extern bool g_QFile_remove_result;                      // QFile remove返回值
extern bool g_QFile_open_result;                        // QFile open返回值
extern bool g_QFile_close_result;                       // QFile close返回值
extern QByteArray g_QFile_readAll_result;               // QFile readAll返回值
extern bool g_QFile_exists_result;                      // QFile exists返回值

extern bool g_QDir_exists_result;                       // QDir exists返回值
extern QFileInfoList g_QDir_entryInfoList_result;              // QDir entryInfoList返回值
extern QString g_QDir_filePath_result;              // QDir entryInfoList返回值

extern qint64 g_QElapsedTimer_elapsed_result;           // QElapsedTimer elapsed返回值

extern DGuiApplicationHelper::ColorType g_DGuiApplicationHelper_themeType_result;   // DGuiApplicationHelper themeType返回值

extern int g_DFileDialog_exec_result;                   // DFileDialog exec
extern QList<QUrl> g_DFileDialog_selectedUrls_result;   // DFileDialog selectedUrls返回值
extern QStringList g_DFileDialog_selectedFiles_result;  // DFileDialog selectedFiles返回值

extern bool g_ArchiveManager_createArchive_result;      // ArchiveManager createArchive返回值
extern bool g_ArchiveManager_loadArchive_result;        // ArchiveManager loadArchive返回值
extern bool g_ArchiveManager_addFiles_result;           // ArchiveManager addFiles返回值
extern bool g_ArchiveManager_extractFiles_result;       // ArchiveManager extractFiles返回值
extern bool g_ArchiveManager_extractFiles2Path_result;  // ArchiveManager extractFiles2Path返回值
extern bool g_ArchiveManager_deleteFiles_result;        // ArchiveManager deleteFiles返回值
extern bool g_ArchiveManager_batchExtractFiles_result;  // ArchiveManager batchExtractFiles返回值
extern bool g_ArchiveManager_openFile_result;           // ArchiveManager openFile返回值
extern bool g_ArchiveManager_updateArchiveCacheData_result;      // ArchiveManager updateArchiveCacheData返回值
extern bool g_ArchiveManager_updateArchiveComment_result;      // ArchiveManager updateArchiveComment返回值
extern bool g_ArchiveManager_convertArchive_result;     // ArchiveManager convertArchive返回值
extern bool g_ArchiveManager_pauseOperation_result;     // ArchiveManager pauseOperation返回值
extern bool g_ArchiveManager_continueOperation_result;  // ArchiveManager continueOperation返回值
extern bool g_ArchiveManager_cancelOperation_result;    // ArchiveManager cancelOperation返回值

extern bool g_SingleJob_doPause_result;                 // SingleJob doPause返回值
extern bool g_SingleJob_doContinue_result;              // SingleJob doContinue返回值
extern bool g_SingleJob_doKill_result;                  // SingleJob doKill返回值

// 通用打桩
class CommonStub : public QObject
{
public:
    CommonStub();
    ~CommonStub();

    /**
     * @brief stub_QWidget_isVisible     针对QWidget的isVisible打桩
     * @param stub
     * @param isVisible
     */
    static void stub_QWidget_isVisible(Stub &stub, bool isVisible);

    /**
     * @brief stub_QMenu_exec     针对QMenu的exec打桩
     * @param stub
     * @return
     */
    static void stub_QMenu_exec(Stub &stub);

    /**
     * @brief stub_QTreeView_indexAt     针对QTreeView的indexAt打桩
     * @param stub
     * @return
     */
    static void stub_QTreeView_indexAt(Stub &stub);

    /**
     * @brief stub_UiTools_isArchiveFile     针对UiTools的isArchiveFile打桩
     * @param stub
     * @param isArchiveFile
     * @return
     */
    static void stub_UiTools_isArchiveFile(Stub &stub, bool isArchiveFile);

    /**
     * @brief stub_ProcessOpenThread_start     针对ProcessOpenThread的start打桩
     * @param stub
     * @return
     */
    static void stub_ProcessOpenThread_start(Stub &stub);

    /**
     * @brief stub_QObject_sender     针对QObject的sender打桩
     * @param stub
     * @param pObject
     * @return
     */
    static void stub_QObject_sender(Stub &stub, QObject *pObject);

    /**
     * @brief stub_KProcess_start     针对KProcess的start打桩
     * @param stub
     * @return
     */
    static void stub_KProcess_start(Stub &stub);

    /**
     * @brief stub_DDialog_exec     针对DDialog的exec打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_DDialog_exec(Stub &stub, int iResult);

    /**
     * @brief stub_QDialog_exec     针对QDialog的exec打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_QDialog_exec(Stub &stub, int iResult);

    /**
     * @brief stub_DAbstractDialog_exec     针对DAbstractDialog的exec打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_DAbstractDialog_exec(Stub &stub, int iResult);

    /**
     * @brief stub_QDialog_open     针对QDialog的open打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_QDialog_open(Stub &stub);

    /**
     * @brief stub_UiTools_isLocalDeviceFile     针对UiTools的isLocalDeviceFile打桩
     * @param stub
     * @param isLocalDeviceFile
     * @return
     */
    static void stub_UiTools_isLocalDeviceFile(Stub &stub, bool isLocalDeviceFile);

    /**
     * @brief stub_OverwriteQuery_execute     针对OverwriteQuery的execute打桩
     * @param stub
     */
    static void stub_OverwriteQuery_execute(Stub &stub);

    /**
     * @brief stub_QThread_start     针对QThread的start打桩
     * @param stub
     */
    static void stub_QThread_start(Stub &stub);

    /**
     * @brief stub_QThread_isRunning     针对QThread的isRunning打桩
     * @param stub
     */
    static void stub_QThread_isRunning(Stub &stub, bool bResult);

    /**
     * @brief stub_QThread_wait     针对QThread的wait打桩
     * @param stub
     */
    static void stub_QThread_wait(Stub &stub, bool bResult);

    /**
     * @brief stub_QThreadPool_waitForDone     针对QThreadPool的waitForDone打桩
     * @param stub
     */
    static void stub_QThreadPool_waitForDone(Stub &stub);

    /**
     * @brief stub_QProcess_startDetached     针对QProcess的startDetached打桩
     * @param stub
     */
    static void stub_QProcess_startDetached(Stub &stub);

    /**
     * @brief stub_UiTools_createInterface     针对UiTools的createInterface打桩
     * @param stub
     * @param pInterface
     */
    static void stub_UiTools_createInterface(Stub &stub, ReadOnlyArchiveInterface *pInterface);

    /**
     * @brief stub_QFileDialog_getOpenFileName     针对QFileDialog的getOpenFileName打桩
     * @param stub
     * @param strFileName
     */
    static void stub_QFileDialog_getOpenFileName(Stub &stub, const QString &strFileName);
};


// 针对自定义Dialog的打桩
class CustomDialogStub : public QObject
{
public:
    CustomDialogStub();
    ~CustomDialogStub();

    /**
     * @brief stub_TipDialog_showDialog     针对TipDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_TipDialog_showDialog(Stub &stub, int iResult);

    /**
     * @brief stub_SimpleQueryDialog_showDialog     针对SimpleQueryDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult);

    /**
     * @brief stub_OverwriteQueryDialog_showDialog     针对OverwriteQueryDialog的showDialog打桩
     * @param stub
     * @return
     */
    static void stub_OverwriteQueryDialog_showDialog(Stub &stub);

    /**
     * @brief stub_OverwriteQueryDialog_getDialogResult     针对OverwriteQueryDialog的getDialogResult打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_OverwriteQueryDialog_getDialogResult(Stub &stub, Overwrite_Result iResult);

    /**
     * @brief stub_OverwriteQueryDialog_getApplyAll     针对OverwriteQueryDialog的getApplyAll打桩
     * @param stub
     * @param bApplyAll
     * @return
     */
    static void stub_OverwriteQueryDialog_getApplyAll(Stub &stub, bool bApplyAll);

    /**
     * @brief stub_AppendDialog_showDialog     针对AppendDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_AppendDialog_showDialog(Stub &stub, int iResult);

    /**
     * @brief stub_RenameDialog_showDialog     针对RenameDialog的showDialog打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_RenameDialog_showDialog(Stub &stub, int iResult);

    /**
     * @brief stub_OpenWithDialog_showOpenWithDialog     针对OpenWithDialog的showOpenWithDialog打桩
     * @param stub
     * @param strResult
     * @return
     */
    static void stub_OpenWithDialog_showOpenWithDialog(Stub &stub, const QString &strResult);

    /**
     * @brief stub_OpenWithDialog_openWithProgram     针对OpenWithDialog的openWithProgram打桩
     * @param stub
     * @return
     */
    static void stub_OpenWithDialog_openWithProgram(Stub &stub);
};


// 针对QFileInfo的打桩
class QFileInfoStub : public QObject
{
public:
    QFileInfoStub();
    ~QFileInfoStub();


    /**
     * @brief stub_QFileInfo_path     针对QFileInfo的path打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_path(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_filePath     针对QFileInfo的filePath打桩
     * @param stub
     * @param strfilePath
     * @return
     */
    static void stub_QFileInfo_filePath(Stub &stub, const QString &strfilePath);

    /**
     * @brief stub_QFileInfo_fileName     针对QFileInfo的fileName打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_fileName(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_completeBaseName     针对QFileInfo的completeBaseName打桩
     * @param stub
     * @param strPath
     * @return
     */
    static void stub_QFileInfo_completeBaseName(Stub &stub, const QString &strPath);

    /**
     * @brief stub_QFileInfo_size     针对QFileInfo的size打桩
     * @param stub
     * @param size
     * @return
     */
    static void stub_QFileInfo_size(Stub &stub, const qint64 &size);

    /**
     * @brief stub_QFileInfo_isDir     针对QFileInfo的isDir打桩
     * @param stub
     * @param isDir
     * @return
     */
    static void stub_QFileInfo_isDir(Stub &stub, bool isDir);

    /**
     * @brief stub_QFileInfo_exists     针对QFileInfo的exists打桩
     * @param stub
     * @param isExists
     * @return
     */
    static void stub_QFileInfo_exists(Stub &stub, bool isExists);

    /**
     * @brief stub_QFileInfo_isWritable     针对QFileInfo的isWritable打桩
     * @param stub
     * @param isWritable
     * @return
     */
    static void stub_QFileInfo_isWritable(Stub &stub, bool isWritable);

    /**
     * @brief stub_QFileInfo_isExecutable     针对QFileInfo的isExecutable打桩
     * @param stub
     * @param isExecutable
     * @return
     */
    static void stub_QFileInfo_isExecutable(Stub &stub, bool isExecutable);

    /**
     * @brief stub_QFileInfo_isReadable     针对QFileInfo的isReadable打桩
     * @param stub
     * @param isReadable
     * @return
     */
    static void stub_QFileInfo_isReadable(Stub &stub, bool isReadable);

    /**
     * @brief stub_QFileInfo_isSymLink     针对QFileInfo的isSymLink打桩
     * @param stub
     * @param isSymLink
     * @return
     */
    static void stub_QFileInfo_isSymLink(Stub &stub, bool isSymLink);

    /**
     * @brief stub_QFileInfo_completeSuffix     针对QFileInfo的completeSuffix打桩
     * @param stub
     * @param strCompleteSuffix
     * @return
     */
    static void stub_QFileInfo_completeSuffix(Stub &stub, const QString &strCompleteSuffix);

};

// 针对PluginManager的打桩
class PluginManagerStub : public QObject
{
public:
    PluginManagerStub();
    ~PluginManagerStub();

    /**
     * @brief stub_PluginManager_supportedWriteMimeTypes     针对PluginManager的supportedWriteMimeTypes打桩
     * @param stub
     * @return
     */
    static void stub_PluginManager_supportedWriteMimeTypes(Stub &stub);
};


// 针对QFile的打桩
class QFileStub : public QObject
{
public:
    QFileStub();
    ~QFileStub();

    /**
     * @brief stub_QFile_remove     针对QFile的remove打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_QFile_remove(Stub &stub, bool bResult);

    /**
     * @brief stub_QFile_open 针对QFile的open打桩
     * @param stub
     * @param bResult
     */
    static void stub_QFile_open(Stub &stub, bool bResult);

    /**
     * @brief stub_QFile_close 针对QFile的close打桩
     * @param stub
     * @param bResult
     */
    static void stub_QFile_close(Stub &stub, bool bResult);

    /**
     * @brief stub_QFile_readAll 针对QFile的readAll打桩
     * @param stub
     * @param allByteArray
     */
    static void stub_QFile_readAll(Stub &stub, const QByteArray &allByteArray);

    /**
     * @brief stub_QFile_exists     针对QFile的exists打桩
     * @param stub
     * @param isExists
     * @return
     */
    static void stub_QFile_exists(Stub &stub, bool isExists);
};

// 针对QDir的打桩
class QDirStub : public QObject
{
public:
    QDirStub();
    ~QDirStub();

    /**
     * @brief stub_QDir_exists     针对QDir的exists打桩
     * @param stub
     * @param isExists
     * @return
     */
    static void stub_QDir_exists(Stub &stub, bool isExists);

    /**
     * @brief stub_QDir_entryInfoList 针对QDir的entryInfoList打桩
     * @param stub
     * @param filters
     * @return
     */
    static void stub_QDir_entryInfoList(Stub &stub, const QFileInfoList &entryInfoList);

    /**
     * @brief stub_QDir_filePath 针对QDir的filePath打桩
     * @param stub
     * @param strfilePath
     * @return
     */
    static void stub_QDir_filePath(Stub &stub, const QString &strfilePath);
};

// 针对DGuiApplicationHelper的打桩
class DGuiApplicationHelperStub : public QObject
{
public:
    DGuiApplicationHelperStub();
    ~DGuiApplicationHelperStub();

    /**
     * @brief stub_DGuiApplicationHelper_themeType     针对DGuiApplicationHelper的themeType打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_DGuiApplicationHelper_themeType(Stub &stub, DGuiApplicationHelper::ColorType themeType);
};


// 针对QElapsedTimer的打桩
class QElapsedTimerStub : public QObject
{
public:
    QElapsedTimerStub();
    ~QElapsedTimerStub();

    /**
     * @brief stub_QElapsedTimer_start     针对QElapsedTimer的start打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_start(Stub &stub);

    /**
     * @brief stub_QElapsedTimer_restart     针对QElapsedTimer的restart打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_restart(Stub &stub);

    /**
     * @brief stub_QElapsedTimer_elapsed     针对QElapsedTimer的elapsed打桩
     * @param stub
     * @return
     */
    static void stub_QElapsedTimer_elapsed(Stub &stub, qint64 qTime);
};


// 针对QElapsedTimer的打桩
class DFileDialogStub : public QObject
{
public:
    DFileDialogStub();
    ~DFileDialogStub();

    /**
     * @brief stub_DFileDialog_exec     针对DFileDialog的selectedUrls打桩
     * @param stub
     * @param iResult
     * @return
     */
    static void stub_DFileDialog_exec(Stub &stub, int iResult);

    /**
     * @brief stub_DFileDialog_selectedUrls     针对DFileDialog的selectedUrls打桩
     * @param stub
     * @param listUrls
     * @return
     */
    static void stub_DFileDialog_selectedUrls(Stub &stub, const QList<QUrl> &listUrls);

    /**
     * @brief stub_DFileDialog_selectedFiles     针对DFileDialog的selectedFiles打桩
     * @param stub
     * @param listFiles
     * @return
     */
    static void stub_DFileDialog_selectedFiles(Stub &stub, const QStringList &listFiles);
};


// 针对DDesktopServicesThread的打桩
class DDesktopServicestub : public QObject
{
public:
    DDesktopServicestub();
    ~DDesktopServicestub();

    /**
     * @brief stub_DDesktopServicestub_showFolder     针对DDesktopServices的showFolder打桩
     * @param stub
     * @return
     */
    static void stub_DDesktopServicestub_showFolder(Stub &stub);

    /**
     * @brief stub_DDesktopServicestub_showFileItem     针对DDesktopServices的showFileItem打桩
     * @param stub
     * @return
     */
    static void stub_DDesktopServicestub_showFileItem(Stub &stub);
};


// 针对ArchiveManager的打桩
class ArchiveManagerStub : public QObject
{
public:
    ArchiveManagerStub();
    ~ArchiveManagerStub();

    /**
     * @brief stub_ArchiveManager_createArchive     针对ArchiveManager的createArchive打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_createArchive(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_loadArchive     针对ArchiveManager的loadArchive打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_loadArchive(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_addFiles     针对ArchiveManager的addFiles打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_addFiles(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_extractFiles     针对ArchiveManager的extractFiles打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_extractFiles(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_extractFiles2Path     针对ArchiveManager的extractFiles2Path打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_extractFiles2Path(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_deleteFiles     针对ArchiveManager的deleteFiles打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_deleteFiles(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_renameFiles     针对ArchiveManager的renameFiles打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_renameFiles(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_batchExtractFiles     针对ArchiveManager的batchExtractFiles打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_batchExtractFiles(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_openFile     针对ArchiveManager的openFile打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_openFile(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_updateArchiveCacheData     针对ArchiveManager的updateArchiveCacheData打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_updateArchiveCacheData(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_updateArchiveComment     针对ArchiveManager的updateArchiveComment打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_updateArchiveComment(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_convertArchive     针对ArchiveManager的convertArchive打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_convertArchive(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_pauseOperation     针对ArchiveManager的pauseOperation打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_pauseOperation(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_continueOperation     针对ArchiveManager的continueOperation打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_continueOperation(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_cancelOperation     针对ArchiveManager的cancelOperation打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_ArchiveManager_cancelOperation(Stub &stub, bool bResult);

    /**
     * @brief stub_ArchiveManager_getCurFilePassword     针对ArchiveManager的getCurFilePassword打桩
     * @param stub
     * @return
     */
    static void stub_ArchiveManager_getCurFilePassword(Stub &stub);

};


// 针对SingleJob的打桩
class JobStub : public QObject
{
public:
    JobStub();
    ~JobStub();

    /**
     * @brief stub_SingleJob_start     针对SingleJob的start打桩
     * @param stub
     * @return
     */
    static void stub_SingleJob_start(Stub &stub);

    /**
     * @brief stub_ConvertJob_start     针对ConvertJob的start打桩
     * @param stub
     * @return
     */
    static void stub_ConvertJob_start(Stub &stub);

    /**
     * @brief stub_StepExtractJob_start     针对ComplexJob的start打桩
     * @param stub
     * @return
     */
    static void stub_StepExtractJob_start(Stub &stub);

    /**
     * @brief stub_BatchExtractJob_start     针对BatchExtractJob的start打桩
     * @param stub
     * @return
     */
    static void stub_BatchExtractJob_start(Stub &stub);

    /**
     * @brief stub_UpdateJob_start     针对UpdateJob的start打桩
     * @param stub
     * @return
     */
    static void stub_UpdateJob_start(Stub &stub);

    /**
     * @brief stub_ArchiveJob_kill     针对ArchiveJob的kill打桩
     * @param stub
     * @return
     */
    static void stub_ArchiveJob_kill(Stub &stub);

    /**
     * @brief stub_SingleJob_doPause     针对SingleJob的doPause打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_SingleJob_doPause(Stub &stub, bool bResult);

    /**
     * @brief stub_SingleJob_doContinue     针对SingleJob的doContinue打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_SingleJob_doContinue(Stub &stub, bool bResult);

    /**
     * @brief stub_SingleJob_doKill     针对SingleJob的doKill打桩
     * @param stub
     * @param bResult
     * @return
     */
    static void stub_SingleJob_doKill(Stub &stub, bool bResult);

    /**
     * @brief stub_CreateJob_doWork     针对CreateJob的doKill打桩
     * @param stub
     * @return
     */
    static void stub_CreateJob_doWork(Stub &stub);

    /**
     * @brief stub_ExtractJob_doWork     针对ExtractJob的doKill打桩
     * @param stub
     * @return
     */
    static void stub_ExtractJob_doWork(Stub &stub);

};


#endif // UT_COMMONSTUB_H
