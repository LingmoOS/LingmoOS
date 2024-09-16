// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_commonstub.h"
#include "popupdialog.h"
#include "singlejob.h"
#include "pluginmanager.h"
#include "uitools.h"
#include "processopenthread.h"
#include "openwithdialog.h"
#include "ddesktopservices.h"
#include "kprocess.h"
#include "archivemanager.h"
#include "queries.h"
#include "batchjob.h"

#include <DFileDialog>
#include <DMenu>

#include <QFileInfo>
#include <QThreadPool>
#include <QProcess>

DWIDGET_USE_NAMESPACE

bool g_QWidget_isVisible_result = false;;               // QWidget isVisible返回值
bool g_UiTools_isArchiveFile_result = false;            // UiTools isArchiveFile返回值
QObject *g_QObject_sender_result = nullptr;             // QObject sender返回值
int g_Dialog_exec_result = 0;                         // DDialog exec返回值
bool g_UiTools_isLocalDeviceFile_result;         // UiTools isLocalDeviceFile返回值
ReadOnlyArchiveInterface *g_UiTools_createInterface_result = nullptr;         // UiTools createInterface返回值
QString g_QFileDialog_getOpenFileName_result = "";      // QFileDialog getOpenFileName返回值
bool g_QThread_isRunning_result = false;                 // QThread isRunning返回值
bool g_QThread_wait_result = false;                      // QThread wait返回值

int g_TipDialog_showDialog_result = 0;                  // TipDialog showDialog返回值
int g_SimpleQueryDialog_showDialog_result = 0;          // SimpleQueryDialog showDialog返回值
Overwrite_Result g_OverwriteQueryDialog_getDialogResult_result = OR_Cancel;  // OverwriteQueryDialog getDialogResult返回值
bool g_OverwriteQueryDialog_getApplyAll_result = false; // OverwriteQueryDialog getApplyAll返回值
int g_AppendDialog_showDialog_result = 0;               // AppendDialog showDialog返回值
int g_RenameDialog_showDialog_result = 0;               // RenameDialog showDialog返回值
QString g_OpenWithDialog_showOpenWithDialog_result = "";  // OpenWithDialog showOpenWithDialog返回值

QString g_QFileInfo_path_result = "";                   // QFileInfo path返回值
QString g_QFileInfo_filePath_result = "";               // QFileInfo filePath返回值
QString g_QFileInfo_fileName_result = "";               // QFileInfo fileName返回值
QString g_QFileInfo_completeBaseName_result = "";       // QFileInfo completeBaseName返回值
qint64 g_QFileInfo_size_result = 0;                     // QFileInfo size返回值
bool g_QFileInfo_isDir_result = false;                  // QFileInfo isDir返回值
bool g_QFileInfo_exists_result = false;                 // QFileInfo exists返回值
bool g_QFileInfo_isWritable_result = false;             // QFileInfo isWritable返回值
bool g_QFileInfo_isExecutable_result = false;           // QFileInfo isExecutable返回值
bool g_QFileInfo_isReadable_result = false;             // QFileInfo isReadable返回值
bool g_QFileInfo_isSymLink_result = false;              // QFileInfo isSymLink返回值
QString g_QFileInfo_completeSuffix_result = "";         // QFileInfo completeSuffix返回值

bool g_QFile_remove_result = false;                     // QFile remove返回值
bool g_QFile_open_result = false;                       // QFile open返回值
bool g_QFile_close_result = false;                      // QFile close返回值
QByteArray g_QFile_readAll_result;                      // QFile readAll返回值
bool g_QFile_exists_result = false;                     // QFile exists返回值

bool g_QDir_exists_result = false;                      // QDir exists返回值
QFileInfoList g_QDir_entryInfoList_result = QFileInfoList();    // QDir entryInfoList返回值
QString g_QDir_filePath_result = "";                    // QDir filePath返回值

qint64 g_QElapsedTimer_elapsed_result = 0;              // QElapsedTimer elapsed返回值

int g_DFileDialog_exec_result = 0;                      // DFileDialog exec
QList<QUrl> g_DFileDialog_selectedUrls_result = QList<QUrl>();   // DFileDialog selectedUrls返回值
QStringList g_DFileDialog_selectedFiles_result = QStringList();   // DFileDialog selectedUrls返回值

DGuiApplicationHelper::ColorType g_DGuiApplicationHelper_themeType_result;   // DGuiApplicationHelper themeType返回值

bool g_ArchiveManager_createArchive_result = false;      // ArchiveManager createArchive返回值
bool g_ArchiveManager_loadArchive_result = false;        // ArchiveManager loadArchive返回值
bool g_ArchiveManager_addFiles_result = false;           // ArchiveManager addFiles返回值
bool g_ArchiveManager_extractFiles_result = false;       // ArchiveManager extractFiles返回值
bool g_ArchiveManager_extractFiles2Path_result = false;  // ArchiveManager extractFiles2Path返回值
bool g_ArchiveManager_deleteFiles_result = false;        // ArchiveManager deleteFiles返回值
bool g_ArchiveManager_renameFiles_result = false;        // ArchiveManager renameFiles返回值
bool g_ArchiveManager_batchExtractFiles_result = false;  // ArchiveManager batchExtractFiles返回值
bool g_ArchiveManager_openFile_result = false;           // ArchiveManager openFile返回值
bool g_ArchiveManager_updateArchiveCacheData_result = false;      // ArchiveManager updateArchiveCacheData返回值
bool g_ArchiveManager_updateArchiveComment_result = false;      // ArchiveManager updateArchiveComment返回值
bool g_ArchiveManager_convertArchive_result = false;     // ArchiveManager convertArchive返回值
bool g_ArchiveManager_pauseOperation_result = false;     // ArchiveManager pauseOperation返回值
bool g_ArchiveManager_continueOperation_result = false;  // ArchiveManager continueOperation返回值
bool g_ArchiveManager_cancelOperation_result = false;    // ArchiveManager cancelOperation返回值

bool g_SingleJob_doPause_result = false;                // SingleJob doPause返回值
bool g_SingleJob_doContinue_result = false;             // SingleJob doContinue返回值
bool g_SingleJob_doKill_result = false;                 // SingleJob doKill返回值

/*************************************CommonStub*************************************/
CommonStub::CommonStub()
{

}

CommonStub::~CommonStub()
{

}

int qWidget_isVisible_stub()
{
    return g_QWidget_isVisible_result;
}

QAction *menu_exec_stub(const QPoint &, QAction *)
{
    return nullptr;
}

QModelIndex treeView_indexAt_stub(void *obj, const QPoint &p)
{
    QTreeView *o = (QTreeView *)obj;
    if (o) {
        return o->model()->index(0, 0);
    }
    return QModelIndex();
}

bool uiTools_isArchiveFile_stub(const QString &)
{
    return g_UiTools_isArchiveFile_result;
}

bool uiTools_isLocalDeviceFile_stub(const QString &)
{
    return g_UiTools_isLocalDeviceFile_result;
}

void processOpenThread_start_stub(void *obj)
{
    QThread *pThread = (QThread *)obj;
    delete pThread;
    return;
}

void kProcess_start_stub()
{
    return;
}

QObject *qObject_sender_stub()
{
    return g_QObject_sender_result;
}

int dialog_exec_stub()
{
    return g_Dialog_exec_result;
}

void dialog_open_stub()
{
    return;
}

void query_execute_stub()
{
    return;
}

void qThread_start_stub()
{
    return;
}

bool qThread_isRunning_stub()
{
    return g_QThread_isRunning_result;
}

bool qThread_wait_stub(unsigned long)
{
    return g_QThread_wait_result;
}

bool qThreadPool_waitForDone_stub()
{
    return true;
}

bool qProcess_startDetached_stub(const QString &, const QStringList &)
{
    return true;
}

QString qFileDialog_getOpenFileName_stub(QWidget *, const QString &, const QString &, const QString &, QString *, QFileDialog::Options)
{
    return g_QFileDialog_getOpenFileName_result;
}

ReadOnlyArchiveInterface *uiTools_createInterface_stub(const QString &, bool, UiTools::AssignPluginType)
{
    return g_UiTools_createInterface_result;
}

void CommonStub::stub_QWidget_isVisible(Stub &stub, bool isVisible)
{
    g_QWidget_isVisible_result = isVisible;
    stub.set(ADDR(QWidget, isVisible), qWidget_isVisible_stub);
}

void CommonStub::stub_QMenu_exec(Stub &stub)
{
    stub.set((QAction * (DMenu::*)(const QPoint &, QAction * at))ADDR(DMenu, exec), menu_exec_stub);
}

void CommonStub::stub_QTreeView_indexAt(Stub &stub)
{
    typedef QModelIndex(*fptr)(QTreeView *, int);
    fptr A_foo = (fptr)(&QTreeView::indexAt);   //获取虚函数地址
    stub.set(A_foo, treeView_indexAt_stub);
}

void CommonStub::stub_UiTools_isArchiveFile(Stub &stub, bool isArchiveFile)
{
    g_UiTools_isArchiveFile_result = isArchiveFile;
    stub.set(ADDR(UiTools, isArchiveFile), uiTools_isArchiveFile_stub);
}

void CommonStub::stub_ProcessOpenThread_start(Stub &stub)
{
    stub.set(ADDR(ProcessOpenThread, start), processOpenThread_start_stub);
}

void CommonStub::stub_QObject_sender(Stub &stub, QObject *pObject)
{
    g_QObject_sender_result = pObject;
    stub.set(ADDR(QObject, sender), qObject_sender_stub);
}

void CommonStub::stub_KProcess_start(Stub &stub)
{
    stub.set(ADDR(KProcess, start), kProcess_start_stub);
}

void CommonStub::stub_DDialog_exec(Stub &stub, int iResult)
{
    g_Dialog_exec_result = iResult;
    typedef int (*fptr)(DDialog *);
    fptr A_foo = (fptr)(&DDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
}

void CommonStub::stub_QDialog_exec(Stub &stub, int iResult)
{
    g_Dialog_exec_result = iResult;
    typedef int (*fptr)(QDialog *);
    fptr A_foo = (fptr)(&QDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
}

void CommonStub::stub_DAbstractDialog_exec(Stub &stub, int iResult)
{
    g_Dialog_exec_result = iResult;
    typedef int (*fptr)(QDialog *);
    fptr A_foo = (fptr)(&DAbstractDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dialog_exec_stub);
}

void CommonStub::stub_QDialog_open(Stub &stub)
{
    typedef void (*fptr)(QDialog *);
    fptr A_foo = (fptr)(&QDialog::open);   //获取虚函数地址
    stub.set(A_foo, dialog_open_stub);
}

void CommonStub::stub_UiTools_isLocalDeviceFile(Stub &stub, bool isLocalDeviceFile)
{
    g_UiTools_isLocalDeviceFile_result = isLocalDeviceFile;
    stub.set(ADDR(UiTools, isLocalDeviceFile), uiTools_isLocalDeviceFile_stub);
}

void CommonStub::stub_OverwriteQuery_execute(Stub &stub)
{
    typedef void (*fptr)(OverwriteQuery *);
    fptr A_foo = (fptr)(&OverwriteQuery::execute);   //获取虚函数地址
    stub.set(A_foo, query_execute_stub);
}

void CommonStub::stub_QThread_start(Stub &stub)
{
    stub.set(ADDR(QThread, start), qThread_start_stub);
}

void CommonStub::stub_QThread_isRunning(Stub &stub, bool bResult)
{
    g_QThread_isRunning_result = bResult;
    stub.set(ADDR(QThread, isRunning), qThread_isRunning_stub);
}

void CommonStub::stub_QThread_wait(Stub &stub, bool bResult)
{
    g_QThread_wait_result = bResult;
//    stub.set(ADDR(QThread, wait), qThread_wait_stub);
    stub.set((bool(QThread::*)(unsigned long))ADDR(QThread, wait), qThread_wait_stub);
}

void CommonStub::stub_QThreadPool_waitForDone(Stub &stub)
{
    stub.set(ADDR(QThreadPool, waitForDone), qThreadPool_waitForDone_stub);
}

void CommonStub::stub_QProcess_startDetached(Stub &stub)
{
#if !defined(Q_QDOC)
    stub.set((bool(*)(const QString &, const QStringList &))ADDR(QProcess, startDetached), qProcess_startDetached_stub);
#endif
}

void CommonStub::stub_UiTools_createInterface(Stub &stub, ReadOnlyArchiveInterface *pInterface)
{
    g_UiTools_createInterface_result = pInterface;
    stub.set((ReadOnlyArchiveInterface * (*)(const QString &, bool bWrite, UiTools::AssignPluginType))ADDR(UiTools, createInterface), uiTools_createInterface_stub);
}

void CommonStub::stub_QFileDialog_getOpenFileName(Stub &stub, const QString &strFileName)
{
    g_QFileDialog_getOpenFileName_result = strFileName;
    stub.set(ADDR(QFileDialog, getOpenFileName), qFileDialog_getOpenFileName_stub);
}


/*************************************CustomDialogStub*************************************/
CustomDialogStub::CustomDialogStub()
{

}

CustomDialogStub::~CustomDialogStub()
{

}

int tipDialog_showDialog_stub(const QString &strDesText, const QString btnMsg, DDialog::ButtonType btnType)
{
    Q_UNUSED(strDesText)
    Q_UNUSED(btnMsg)
    Q_UNUSED(btnType)
    return g_TipDialog_showDialog_result;
}

int simpleQueryDialog_showDialog_stub(const QString &strDesText, const QString btnMsg1, DDialog::ButtonType btnType1, const QString btnMsg2, DDialog::ButtonType btnType2, const QString btnMsg3, DDialog::ButtonType btnType3)
{
    Q_UNUSED(strDesText)
    Q_UNUSED(btnMsg1)
    Q_UNUSED(btnType1)
    Q_UNUSED(btnMsg2)
    Q_UNUSED(btnType2)
    Q_UNUSED(btnMsg3)
    Q_UNUSED(btnType3)
    return g_SimpleQueryDialog_showDialog_result;
}

void overwriteQueryDialog_showDialog_stub(QString file, bool bDir)
{
    Q_UNUSED(file)
    Q_UNUSED(bDir)
    return;
}

Overwrite_Result overwriteQueryDialog_getDialogResult_stub()
{
    return g_OverwriteQueryDialog_getDialogResult_result;
}

bool overwriteQueryDialog_getApplyAll_stub()
{
    return g_OverwriteQueryDialog_getApplyAll_result;
}

int appendDialog_showDialog_stub()
{
    return g_AppendDialog_showDialog_result;
}

int renameDialog_showDialog_stub()
{
    return g_RenameDialog_showDialog_result;
}

QString openWithDialog_showOpenWithDialog_stub()
{
    return g_OpenWithDialog_showOpenWithDialog_result;
}

void openWithDialog_openWithProgram_stub(const QString &, const QString &)
{
    return;
}

void CustomDialogStub::stub_TipDialog_showDialog(Stub &stub, int iResult)
{
    g_TipDialog_showDialog_result = iResult;
    stub.set(ADDR(TipDialog, showDialog), tipDialog_showDialog_stub);
}

void CustomDialogStub::stub_SimpleQueryDialog_showDialog(Stub &stub, int iResult)
{
    g_SimpleQueryDialog_showDialog_result = iResult;
    stub.set(ADDR(SimpleQueryDialog, showDialog), simpleQueryDialog_showDialog_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_showDialog(Stub &stub)
{
    stub.set(ADDR(OverwriteQueryDialog, showDialog), overwriteQueryDialog_showDialog_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_getDialogResult(Stub &stub, Overwrite_Result iResult)
{
    g_OverwriteQueryDialog_getDialogResult_result = iResult;
    stub.set(ADDR(OverwriteQueryDialog, getDialogResult), overwriteQueryDialog_getDialogResult_stub);
}

void CustomDialogStub::stub_OverwriteQueryDialog_getApplyAll(Stub &stub, bool bApplyAll)
{
    g_OverwriteQueryDialog_getApplyAll_result = bApplyAll;
    stub.set(ADDR(OverwriteQueryDialog, getApplyAll), overwriteQueryDialog_getApplyAll_stub);
}

void CustomDialogStub::stub_AppendDialog_showDialog(Stub &stub, int iResult)
{
    g_AppendDialog_showDialog_result = iResult;
    stub.set(ADDR(AppendDialog, showDialog), appendDialog_showDialog_stub);
}

void CustomDialogStub::stub_RenameDialog_showDialog(Stub &stub, int iResult)
{
    g_RenameDialog_showDialog_result = iResult;
    stub.set(ADDR(RenameDialog, showDialog), renameDialog_showDialog_stub);
}

void CustomDialogStub::stub_OpenWithDialog_showOpenWithDialog(Stub &stub, const QString &strResult)
{
    g_OpenWithDialog_showOpenWithDialog_result = strResult;
    stub.set(ADDR(OpenWithDialog, showOpenWithDialog), openWithDialog_showOpenWithDialog_stub);
}

void CustomDialogStub::stub_OpenWithDialog_openWithProgram(Stub &stub)
{
    stub.set(ADDR(OpenWithDialog, openWithProgram), openWithDialog_openWithProgram_stub);
}


/*************************************QFileInfoStub*************************************/
QFileInfoStub::QFileInfoStub()
{

}

QFileInfoStub::~QFileInfoStub()
{

}

QString qfileinfo_path_stub()
{
    return g_QFileInfo_path_result;
}

QString qfileinfo_filePath_stub()
{
    return g_QFileInfo_filePath_result;
}

QString qfileinfo_fileName_stub()
{
    return g_QFileInfo_fileName_result;
}

QString qfileinfo_completeBaseName_stub()
{
    return g_QFileInfo_completeBaseName_result;
}

qint64 qfileinfo_size_stub()
{
    return g_QFileInfo_size_result;
}

bool qfileinfo_isDir_stub()
{
    return g_QFileInfo_isDir_result;
}

bool qfileinfo_exists_stub()
{
    return g_QFileInfo_exists_result;
}

bool qfileinfo_isWritable_stub()
{
    return g_QFileInfo_isWritable_result;
}

bool qfileinfo_isExecutable_stub()
{
    return g_QFileInfo_isExecutable_result;
}

bool qfileinfo_isReadable_stub()
{
    return g_QFileInfo_isReadable_result;
}

bool qfileinfo_isSymLink_stub()
{
    return g_QFileInfo_isSymLink_result;
}

QString qfileinfo_completeSuffix_stub()
{
    return g_QFileInfo_completeSuffix_result;
}


void QFileInfoStub::stub_QFileInfo_path(Stub &stub, const QString &strPath)
{
    g_QFileInfo_path_result = strPath;
    stub.set(ADDR(QFileInfo, path), qfileinfo_path_stub);
}

void QFileInfoStub::stub_QFileInfo_filePath(Stub &stub, const QString &strfilePath)
{
    g_QFileInfo_filePath_result = strfilePath;
    stub.set(ADDR(QFileInfo, filePath), qfileinfo_filePath_stub);
}

void QFileInfoStub::stub_QFileInfo_fileName(Stub &stub, const QString &strPath)
{
    g_QFileInfo_fileName_result = strPath;
    stub.set(ADDR(QFileInfo, fileName), qfileinfo_fileName_stub);
}

void QFileInfoStub::stub_QFileInfo_completeBaseName(Stub &stub, const QString &strPath)
{
    g_QFileInfo_completeBaseName_result = strPath;
    stub.set(ADDR(QFileInfo, completeBaseName), qfileinfo_completeBaseName_stub);
}

void QFileInfoStub::stub_QFileInfo_size(Stub &stub, const qint64 &size)
{
    g_QFileInfo_size_result = size;
    stub.set(ADDR(QFileInfo, size), qfileinfo_size_stub);
}

void QFileInfoStub::stub_QFileInfo_isDir(Stub &stub, bool isDir)
{
    g_QFileInfo_isDir_result = isDir;
    stub.set(ADDR(QFileInfo, isDir), qfileinfo_isDir_stub);
}

void QFileInfoStub::stub_QFileInfo_exists(Stub &stub, bool isExists)
{
    g_QFileInfo_exists_result = isExists;

    typedef bool (QFileInfo::*fptr)()const ;
    fptr A_foo = (fptr)(&QFileInfo::exists);   //获取虚函数地址
    stub.set(A_foo, qfileinfo_exists_stub);
}

void QFileInfoStub::stub_QFileInfo_isWritable(Stub &stub, bool isWritable)
{
    g_QFileInfo_isWritable_result = isWritable;
    stub.set(ADDR(QFileInfo, isWritable), qfileinfo_isWritable_stub);
}

void QFileInfoStub::stub_QFileInfo_isExecutable(Stub &stub, bool isExecutable)
{
    g_QFileInfo_isExecutable_result = isExecutable;
    stub.set(ADDR(QFileInfo, isExecutable), qfileinfo_isExecutable_stub);
}

void QFileInfoStub::stub_QFileInfo_isReadable(Stub &stub, bool isReadable)
{
    g_QFileInfo_isReadable_result = isReadable;
    stub.set(ADDR(QFileInfo, isReadable), qfileinfo_isReadable_stub);
}

void QFileInfoStub::stub_QFileInfo_isSymLink(Stub &stub, bool isSymLink)
{
    g_QFileInfo_isSymLink_result = isSymLink;
    stub.set(ADDR(QFileInfo, isSymLink), qfileinfo_isSymLink_stub);
}

void QFileInfoStub::stub_QFileInfo_completeSuffix(Stub &stub, const QString &strCompleteSuffix)
{
    g_QFileInfo_completeSuffix_result = strCompleteSuffix;
    stub.set(ADDR(QFileInfo, completeSuffix), qfileinfo_completeSuffix_stub);
}


/*************************************PluginManagerStub*************************************/
PluginManagerStub::PluginManagerStub()
{

}

PluginManagerStub::~PluginManagerStub()
{

}

QStringList pluginManager_supportedWriteMimeTypes_stub(PluginManager::MimeSortingMode mode)
{
    QStringList listMimeTypes = QStringList() << "application/zip" << "application/x-7z-compressed";
    return listMimeTypes;
}

void PluginManagerStub::stub_PluginManager_supportedWriteMimeTypes(Stub &stub)
{
    stub.set(ADDR(PluginManager, supportedWriteMimeTypes), pluginManager_supportedWriteMimeTypes_stub);
}


/*************************************PluginManagerStub*************************************/
QFileStub::QFileStub()
{

}

QFileStub::~QFileStub()
{

}

bool qfile_remove_stub()
{
    return g_QFile_remove_result;
}

bool qfile_open_stub(QIODevice::OpenMode flags)
{
    Q_UNUSED(flags)

    return g_QFile_open_result;
}

bool qfile_close_stub()
{
    return g_QFile_close_result;
}

bool qfile_exists_stub()
{
    return g_QFile_exists_result;
}

QByteArray qfile_readAll_stub()
{
    return g_QFile_readAll_result;
}

void QFileStub::stub_QFile_remove(Stub &stub, bool bResult)
{
    g_QFile_remove_result = bResult;
    typedef bool (QFile::*fptr)();
    fptr A_foo = (fptr)(&QFile::remove);   //获取虚函数地址
    stub.set(A_foo, qfile_remove_stub);
}

void QFileStub::stub_QFile_open(Stub &stub, bool bResult)
{
    g_QFile_open_result = bResult;
    typedef bool (*fptr)(QFile *, QIODevice::OpenMode);
    fptr A_foo = (fptr)((bool(QFile::*)(QIODevice::OpenMode))&QFile::open);   //获取虚函数地址
    stub.set(A_foo, qfile_open_stub);
}

void QFileStub::stub_QFile_close(Stub &stub, bool bResult)
{

    g_QFile_close_result = bResult;
    typedef bool (*fptr)(QFile *);
    fptr A_foo = (fptr)(&QFile::close);   //获取虚函数地址
    stub.set(A_foo, qfile_close_stub);
}

void QFileStub::stub_QFile_readAll(Stub &stub, const QByteArray &allByteArray)
{
    g_QFile_readAll_result = allByteArray;

    stub.set(ADDR(QFile, readAll), qfile_readAll_stub);
}

void QFileStub::stub_QFile_exists(Stub &stub, bool isExists)
{
    g_QFile_exists_result = isExists;

    typedef bool (QFile::*fptr)()const ;
    fptr A_foo = (fptr)(&QFile::exists);   //获取虚函数地址
    stub.set(A_foo, qfile_exists_stub);
}

/*************************************DGuiApplicationHelperStub*************************************/
DGuiApplicationHelperStub::DGuiApplicationHelperStub()
{

}

DGuiApplicationHelperStub::~DGuiApplicationHelperStub()
{

}

DGuiApplicationHelper::ColorType dGuiApplicationHelper_themeType_stub()
{
    return g_DGuiApplicationHelper_themeType_result;
}

void DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(Stub &stub, DGuiApplicationHelper::ColorType themeType)
{
    g_DGuiApplicationHelper_themeType_result = themeType;
    stub.set(ADDR(DGuiApplicationHelper, themeType), dGuiApplicationHelper_themeType_stub);
}


/*************************************DGuiApplicationHelperStub*************************************/
QElapsedTimerStub::QElapsedTimerStub()
{

}

QElapsedTimerStub::~QElapsedTimerStub()
{

}

void qElapsedTimer_start_stub()
{
    return;
}

void qElapsedTimer_restart_stub()
{
    return;
}

qint64 qElapsedTimer_elapsed_stub()
{
    return g_QElapsedTimer_elapsed_result;
}

void QElapsedTimerStub::stub_QElapsedTimer_start(Stub &stub)
{
    stub.set(ADDR(QElapsedTimer, start), qElapsedTimer_start_stub);
}

void QElapsedTimerStub::stub_QElapsedTimer_restart(Stub &stub)
{
    stub.set(ADDR(QElapsedTimer, restart), qElapsedTimer_restart_stub);
}

void QElapsedTimerStub::stub_QElapsedTimer_elapsed(Stub &stub, qint64 qTime)
{
    g_QElapsedTimer_elapsed_result = qTime;
    stub.set(ADDR(QElapsedTimer, elapsed), qElapsedTimer_elapsed_stub);
}
/*************************************DGuiApplicationHelperStub*************************************/


/*************************************DFileDialogStub*************************************/
DFileDialogStub::DFileDialogStub()
{

}

DFileDialogStub::~DFileDialogStub()
{

}

int dFileDialog_exec_stub()
{
    return g_DFileDialog_exec_result;
}

QList<QUrl> dFileDialog_selectedUrls_stub()
{
    return g_DFileDialog_selectedUrls_result;
}

QStringList dFileDialog_selectedFiles_stub()
{
    return g_DFileDialog_selectedFiles_result;
}

void DFileDialogStub::stub_DFileDialog_exec(Stub &stub, int iResult)
{
    g_DFileDialog_exec_result = iResult;

    typedef int (*fptr)(DFileDialog *);
    fptr A_foo = (fptr)(&DFileDialog::exec);   //获取虚函数地址
    stub.set(A_foo, dFileDialog_exec_stub);
}

void DFileDialogStub::stub_DFileDialog_selectedUrls(Stub &stub, const QList<QUrl> &listUrls)
{
    g_DFileDialog_selectedUrls_result = listUrls;
    stub.set(ADDR(DFileDialog, selectedUrls), dFileDialog_selectedUrls_stub);
}

void DFileDialogStub::stub_DFileDialog_selectedFiles(Stub &stub, const QStringList &listFiles)
{
    g_DFileDialog_selectedFiles_result = listFiles;
    stub.set(ADDR(DFileDialog, selectedFiles), dFileDialog_selectedFiles_stub);
}
/*************************************DFileDialogStub*************************************/


/*************************************QDirStub*************************************/

QDirStub::QDirStub()
{

}

QDirStub::~QDirStub()
{

}

bool qdir_exists_stub()
{
    return g_QDir_exists_result;
}

QFileInfoList qdir_entryInfoList_stub(QDir::Filters filters, QDir::SortFlags sort)
{
    Q_UNUSED(filters)
    Q_UNUSED(sort)

    return g_QDir_entryInfoList_result;
}

QString qdir_filePath_stub(const QString &fileName)
{
    Q_UNUSED(fileName)

    return g_QDir_filePath_result;
}

void QDirStub::stub_QDir_exists(Stub &stub, bool isExists)
{
    g_QDir_exists_result = isExists;

    typedef bool (QDir::*fptr)()const ;
    fptr A_foo = (fptr)(&QDir::exists);   //获取虚函数地址
    stub.set(A_foo, qdir_exists_stub);
}

void QDirStub::stub_QDir_entryInfoList(Stub &stub, const QFileInfoList &entryInfoList)
{
    g_QDir_entryInfoList_result = entryInfoList;

    typedef QFileInfoList(QDir::*fptr)(QDir::Filters, QDir::SortFlags) const;
    fptr A_foo = (fptr)(&QDir::entryInfoList);   //获取虚函数地址
    stub.set(A_foo, qdir_entryInfoList_stub);
}

void QDirStub::stub_QDir_filePath(Stub &stub, const QString &strfilePath)
{
    g_QDir_filePath_result = strfilePath;

    stub.set(ADDR(QDir, filePath), qdir_filePath_stub);
}
/*************************************QDirStub*************************************/


/*************************************DDesktopServicestub*************************************/

DDesktopServicestub::DDesktopServicestub()
{

}

DDesktopServicestub::~DDesktopServicestub()
{

}

bool dDesktopServices_showFolder_stub(QString, const QString &)
{
    return true;
}

bool dDesktopServices_showFileItem_stub(QString, const QString &)
{
    return true;
}

void DDesktopServicestub::stub_DDesktopServicestub_showFolder(Stub &stub)
{
    bool (*db)(QString, const QString &) = &DDesktopServices::showFolder;
    stub.set(db, dDesktopServices_showFolder_stub);
}

void DDesktopServicestub::stub_DDesktopServicestub_showFileItem(Stub &stub)
{

    bool (*db)(QString, const QString &) = &DDesktopServices::showFileItem;
    stub.set(db, dDesktopServices_showFileItem_stub);
}
/*************************************DDesktopServicestub*************************************/


/*************************************ArchiveManagerStub*************************************/
ArchiveManagerStub::ArchiveManagerStub()
{

}

ArchiveManagerStub::~ArchiveManagerStub()
{

}

bool archiveManager_createArchive_stub(const QList<FileEntry> &, const QString &, const CompressOptions &, UiTools::AssignPluginType)
{
    return g_ArchiveManager_createArchive_result;
}

bool archiveManager_loadArchive_stub(const QString &, UiTools::AssignPluginType)
{
    return g_ArchiveManager_loadArchive_result;
}

bool archiveManager_addFiles_stub(const QString &, const QList<FileEntry> &, const CompressOptions &)
{
    return g_ArchiveManager_addFiles_result;
}

bool archiveManager_extractFiles_stub(const QString &, const QList<FileEntry> &, const ExtractionOptions &, UiTools::AssignPluginType)
{
    return g_ArchiveManager_extractFiles_result;
}

bool archiveManager_extractFiles2Path_stub(const QString &, const QList<FileEntry> &, const ExtractionOptions &)
{
    return g_ArchiveManager_extractFiles2Path_result;
}

bool archiveManager_deleteFiles_stub(const QString &, const QList<FileEntry> &)
{
    return g_ArchiveManager_deleteFiles_result;
}

bool archiveManager_renameFiles_stub(const QString &, const QList<FileEntry> &)
{
    return g_ArchiveManager_renameFiles_result;
}

bool archiveManager_batchExtractFiles_stub(const QStringList &, const QString &)
{
    return g_ArchiveManager_batchExtractFiles_result;
}

bool archiveManager_openFile_stub(const QString &, const FileEntry &, const QString &, const QString &)
{
    return g_ArchiveManager_openFile_result;
}

bool archiveManager_updateArchiveCacheData_stub()
{
    return g_ArchiveManager_updateArchiveCacheData_result;
}

bool archiveManager_updateArchiveComment_stub(const QString &, const QString &)
{
    return g_ArchiveManager_updateArchiveComment_result;
}

bool archiveManager_convertArchive_stub(const QString &, const QString &, const QString &)
{
    return g_ArchiveManager_convertArchive_result;
}

bool archiveManager_pauseOperation_stub()
{
    return g_ArchiveManager_pauseOperation_result;
}

bool archiveManager_continueOperation_stub()
{
    return g_ArchiveManager_continueOperation_result;
}

bool archiveManager_cancelOperation_stub()
{
    return g_ArchiveManager_cancelOperation_result;
}

QString archiveManager_getCurFilePassword_stub()
{
    return "123";
}

void ArchiveManagerStub::stub_ArchiveManager_createArchive(Stub &stub, bool bResult)
{
    g_ArchiveManager_createArchive_result = bResult;
    stub.set(ADDR(ArchiveManager, createArchive), archiveManager_createArchive_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_loadArchive(Stub &stub, bool bResult)
{
    g_ArchiveManager_loadArchive_result = bResult;
    stub.set(ADDR(ArchiveManager, loadArchive), archiveManager_loadArchive_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_addFiles(Stub &stub, bool bResult)
{
    g_ArchiveManager_addFiles_result = bResult;
    stub.set(ADDR(ArchiveManager, addFiles), archiveManager_addFiles_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_extractFiles(Stub &stub, bool bResult)
{
    g_ArchiveManager_extractFiles_result = bResult;
    stub.set(ADDR(ArchiveManager, extractFiles), archiveManager_extractFiles_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_extractFiles2Path(Stub &stub, bool bResult)
{
    g_ArchiveManager_extractFiles2Path_result = bResult;
    stub.set(ADDR(ArchiveManager, extractFiles2Path), archiveManager_extractFiles2Path_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_deleteFiles(Stub &stub, bool bResult)
{
    g_ArchiveManager_deleteFiles_result = bResult;
    stub.set(ADDR(ArchiveManager, deleteFiles), archiveManager_deleteFiles_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_renameFiles(Stub &stub, bool bResult)
{
    g_ArchiveManager_renameFiles_result = bResult;
    stub.set(ADDR(ArchiveManager, renameFiles), archiveManager_renameFiles_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_batchExtractFiles(Stub &stub, bool bResult)
{
    g_ArchiveManager_batchExtractFiles_result = bResult;
    stub.set(ADDR(ArchiveManager, batchExtractFiles), archiveManager_batchExtractFiles_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_openFile(Stub &stub, bool bResult)
{
    g_ArchiveManager_openFile_result = bResult;
    stub.set(ADDR(ArchiveManager, openFile), archiveManager_openFile_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_updateArchiveCacheData(Stub &stub, bool bResult)
{
    g_ArchiveManager_updateArchiveCacheData_result = bResult;
    stub.set(ADDR(ArchiveManager, updateArchiveCacheData), archiveManager_updateArchiveCacheData_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_updateArchiveComment(Stub &stub, bool bResult)
{
    g_ArchiveManager_updateArchiveComment_result = bResult;
    stub.set(ADDR(ArchiveManager, updateArchiveComment), archiveManager_updateArchiveComment_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_convertArchive(Stub &stub, bool bResult)
{
    g_ArchiveManager_convertArchive_result = bResult;
    stub.set(ADDR(ArchiveManager, convertArchive), archiveManager_convertArchive_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_pauseOperation(Stub &stub, bool bResult)
{
    g_ArchiveManager_pauseOperation_result = bResult;
    stub.set(ADDR(ArchiveManager, pauseOperation), archiveManager_pauseOperation_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_continueOperation(Stub &stub, bool bResult)
{
    g_ArchiveManager_continueOperation_result = bResult;
    stub.set(ADDR(ArchiveManager, continueOperation), archiveManager_continueOperation_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_cancelOperation(Stub &stub, bool bResult)
{
    g_ArchiveManager_cancelOperation_result = bResult;
    stub.set(ADDR(ArchiveManager, cancelOperation), archiveManager_cancelOperation_stub);
}

void ArchiveManagerStub::stub_ArchiveManager_getCurFilePassword(Stub &stub)
{
    stub.set(ADDR(ArchiveManager, getCurFilePassword), archiveManager_getCurFilePassword_stub);
}
/*************************************ArchiveManagerStub*************************************/


/*************************************SingleJobStub*************************************/
JobStub::JobStub()
{

}

JobStub::~JobStub()
{

}

void job_start_stub()
{
    return;
}

void job_kill_stub()
{
    return;
}

bool singleJob_doPause_stub()
{
    return g_SingleJob_doPause_result;
}

bool singleJob_doContinue_stub()
{
    return g_SingleJob_doContinue_result;
}

bool singleJob_doKill_stub()
{
    return g_SingleJob_doKill_result;
}

void job_doWork_stub()
{
    return;
}

void JobStub::stub_SingleJob_start(Stub &stub)
{
    typedef void (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_StepExtractJob_start(Stub &stub)
{
    typedef void (*fptr)(StepExtractJob *);
    fptr A_foo = (fptr)(&StepExtractJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_ConvertJob_start(Stub &stub)
{
    typedef void (*fptr)(ConvertJob *);
    fptr A_foo = (fptr)(&ConvertJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_BatchExtractJob_start(Stub &stub)
{
    typedef void (*fptr)(BatchExtractJob *);
    fptr A_foo = (fptr)(&BatchExtractJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_UpdateJob_start(Stub &stub)
{
    typedef void (*fptr)(UpdateJob *);
    fptr A_foo = (fptr)(&UpdateJob::start);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_ArchiveJob_kill(Stub &stub)
{
    typedef void (*fptr)(ArchiveJob *);
    fptr A_foo = (fptr)(&ArchiveJob::kill);   //获取虚函数地址
    stub.set(A_foo, job_kill_stub);
}

void JobStub::stub_SingleJob_doPause(Stub &stub, bool bResult)
{
    g_SingleJob_doPause_result = bResult;
    typedef bool (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::doPause);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_SingleJob_doContinue(Stub &stub, bool bResult)
{
    g_SingleJob_doContinue_result = bResult;
    typedef bool (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::doContinue);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_SingleJob_doKill(Stub &stub, bool bResult)
{
    g_SingleJob_doKill_result = bResult;
    typedef bool (*fptr)(SingleJob *);
    fptr A_foo = (fptr)(&SingleJob::doKill);   //获取虚函数地址
    stub.set(A_foo, job_start_stub);
}

void JobStub::stub_CreateJob_doWork(Stub &stub)
{
    typedef void (*fptr)(CreateJob *);
    fptr A_foo = (fptr)(&CreateJob::doWork);   //获取虚函数地址
    stub.set(A_foo, job_doWork_stub);
}

void JobStub::stub_ExtractJob_doWork(Stub &stub)
{
    typedef void (*fptr)(ExtractJob *);
    fptr A_foo = (fptr)(&ExtractJob::doWork);   //获取虚函数地址
    stub.set(A_foo, job_doWork_stub);
}
/*************************************SingleJobStub*************************************/
