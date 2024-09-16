// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreviewlistdatathread.h"
#include "dfmxmlwrapper.h"
#include "dfontpreviewlistview.h"
#include "dcopyfilesmanager.h"
#include "dcomworker.h"

#include <QFontDatabase>
#include <QApplication>

static DFontPreviewListDataThread *INSTANCE = nullptr;

QList<DFontPreviewItemData> DFontPreviewListDataThread::m_fontModelList = QList<DFontPreviewItemData>();
QList<DFontPreviewItemData> DFontPreviewListDataThread::m_startModelList = QList<DFontPreviewItemData>();
//系统字体路径
const QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";

const QString FONTS_UP_DIR = QDir::homePath() + "/.local/share/";

DFontPreviewListDataThread *DFontPreviewListDataThread::instance(DFontPreviewListView *view)
{
    if (!INSTANCE) {
        INSTANCE = new DFontPreviewListDataThread(view);
    }

    return INSTANCE;
}

DFontPreviewListDataThread *DFontPreviewListDataThread::instance()
{
    return INSTANCE;
}

/*************************************************************************
 <Function>      DFontPreviewListDataThread
 <Description>    构造函数
 <Author>        null
 <Input>
    <param1>     view            Description:父指针
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontPreviewListDataThread::DFontPreviewListDataThread(DFontPreviewListView *view)
    : m_view(view)
    , m_fsWatcher(nullptr)
    , m_mutex(nullptr)
{
    if (view != nullptr)
        m_mutex = view->getMutex();

    m_dbManager = DFMDBManager::instance();
    moveToThread(&mThread);
    QObject::connect(&mThread, SIGNAL(started()), this, SLOT(doWork()));
    connect(this, &DFontPreviewListDataThread::requestDeleted, this, &DFontPreviewListDataThread::onFileDeleted);
    connect(this, &DFontPreviewListDataThread::requestAdded, this, &DFontPreviewListDataThread::onFileAdded);
    connect(this, &DFontPreviewListDataThread::requestForceDeleteFiles, this, &DFontPreviewListDataThread::forceDeleteFiles);
    connect(this, &DFontPreviewListDataThread::requestRemoveFileWatchers, this, &DFontPreviewListDataThread::onRemoveFileWatchers);
    connect(this, &DFontPreviewListDataThread::requestAutoDirWatchers, this, &DFontPreviewListDataThread::onAutoDirWatchers);
    connect(this, &DFontPreviewListDataThread::requestExportFont, this, &DFontPreviewListDataThread::onExportFont);

    QTimer::singleShot(3, [this] {
        mThread.start();
    });
}

DFontPreviewListDataThread::~DFontPreviewListDataThread()
{
}

/*************************************************************************
 <Function>      doWork
 <Description>   线程函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::doWork()
{
    QMutexLocker locker(m_mutex);
    initFileSystemWatcher();
    m_fontModelList.clear();

    qDebug() << "doWork thread id = " << QThread::currentThreadId();

    //withoutDbRefreshDb(m_allFontPathList);//想把它放在第1个，getStartFontList之前
    QStringList disableFontList = DFMXmlWrapper::getFontConfigDisableFontPathList();

    if (disableFontList.count()  > 0) {
        syncFontEnableDisableStatusData(disableFontList);
    }

    //获取启动时的需要用到的字体列表
    FontManager::instance()->getStartFontList();
    if (!m_startModelList.isEmpty()) {
        //从fontconfig配置文件同步字体启用/禁用状态数据
        //syncFontEnableDisableStatusData(disableFontList);
        refreshStartupFontListData();
        m_view->onFinishedDataLoad(); //当前只加载了50条数据所以切换到用户数据显示未安装
        return;
    }
    //withoutDbRefreshDb(m_allFontPathList);
    Q_EMIT m_view->multiItemsAdded(m_fontModelList, DFontSpinnerWidget::StartupLoad);
}

/*************************************************************************
 <Function>      initFileSystemWatcher
 <Description>   初始化系统文件监视器
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::initFileSystemWatcher()
{
    if (m_fsWatcher == nullptr)
        m_fsWatcher = new QFileSystemWatcher(this);
    QDir dir(FONTS_DIR);
    if (!dir.exists())
        dir.mkpath(FONTS_DIR);

    m_deleteFileRecivetimer = new QTimer;

    m_fsWatcher->addPath(FONTS_DIR);
    m_fsWatcher->addPath(FONTS_UP_DIR);

    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged,
    this, [ = ](const QString & path) {
        //启动定时器收集需要删除的字体
        m_deleteFileRecivetimer->start(20);
        m_waitForDeleteFiles << path;
    });

    //定时器停止表示没有需要删除的字体了,开始进行进行删除
    connect(m_deleteFileRecivetimer, &QTimer::timeout, this, [ = ] {
        qDebug() << m_waitForDeleteFiles.count();
        //删除前显示加载动画
        m_view->updateSpinner(DFontSpinnerWidget::Delete, false);
        m_view->updateChangedFile(m_waitForDeleteFiles);
        //删除后加载动画消失
        Q_EMIT m_view->requestShowSpinner(false, true, DFontSpinnerWidget::Delete);
        m_deleteFileRecivetimer->stop();

        m_waitForDeleteFiles.clear();
    });

    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged,
    this, [ = ](const QString & path) {
        QFileInfo f(path);
        if (!f.isDir())
            return ;

        //删除前显示加载动画
        m_view->updateSpinner(DFontSpinnerWidget::Delete, false);
        updateChangedDir();

        //删除后加载动画消失
        Q_EMIT m_view->requestShowSpinner(false, true, DFontSpinnerWidget::Delete);

        if (!dir.exists()) {
            m_fsWatcher->removePath(FONTS_DIR);
        }
    });
}

/*************************************************************************
 <Function>      updateChangedDir
 <Description>   文件夹修改后触发函数
 <Author>        null
 <Input>
    <param1>     path            Description:路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateChangedDir()
{
    QMutexLocker locker(m_mutex);
    m_view->updateChangedDir();
}

/*************************************************************************
 <Function>      addPathWatcher
 <Description>   添加文件监视器
 <Author>        null
 <Input>
    <param1>     path            Description:需要添加的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::addPathWatcher(const QString &path)
{
    if (m_fsWatcher == nullptr)
        return;

    if (!QFileInfo(path).exists())
        return;

    if (m_fsWatcher->addPath(path)) {
        if (!m_fsWatcher->directories().contains(FONTS_DIR))
            m_fsWatcher->addPath(FONTS_DIR);

        if (!m_fsWatcher->directories().contains(FONTS_UP_DIR))
            m_fsWatcher->addPath(FONTS_UP_DIR);
    }
}

/*************************************************************************
 <Function>      removePathWatcher
 <Description>   移除特定路径的文件监视器
 <Author>        null
 <Input>
    <param1>     path            Description:需要移除的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::removePathWatcher(const QString &path)
{
    if (m_fsWatcher == nullptr)
        return;
    m_fsWatcher->removePath(path);
}

/*************************************************************************
 <Function>      onFileDeleted
 <Description>   文件删除响应函数
 <Author>        null
 <Input>
    <param1>     files            Description:删除的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onFileDeleted(QStringList &files)
{
    qDebug() << __FUNCTION__ << files.size();
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteCurFonts(files, false);
}

/*************************************************************************
 <Function>      onFileAdded
 <Description>   增加文件响应函数
 <Author>        null
 <Input>
    <param1>     files            Description:增加的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onFileAdded(const QStringList &files)
{
    if (files.isEmpty()) {
        return;
    }

    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    qDebug() << __func__ << "S" << QThread::currentThreadId() << endl;
    refreshFontListData(false, files);
}

/*************************************************************************
 <Function>      getFontModelList
 <Description>   获取当前列表所有项的数据链表
 <Author>        null
 <Input>
    <param1>     null                         Description:null
 <Return>        QList<DFontPreviewItemData>  Description:列表所有项的数据链表
 <Note>          null
*************************************************************************/
QList<DFontPreviewItemData> DFontPreviewListDataThread::getFontModelList()
{
    if (m_view->isListDataLoadFinished())
        return m_fontModelList;
    return QList<DFontPreviewItemData>();
}

//更新itemDataList的itemData状态

/*************************************************************************
 <Function>      updateItemStatus
 <Description>   更新itemDataList的itemData状态
 <Author>        null
 <Input>
    <param1>     index               Description:需要更新的位置
    <param2>     itemData            Description:需要更新的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateItemStatus(int index, const DFontPreviewItemData &itemData)
{
    m_fontModelList.replace(index, itemData);
}

/*************************************************************************
 <Function>      forceDeleteFiles
 <Description>   强制删除文件
 <Author>        null
 <Input>
    <param1>     files            Description:需要删除的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::forceDeleteFiles(QStringList &files)
{
    qDebug() << __FUNCTION__ << files << m_mutex;
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteFontFiles(files, true);

    Q_EMIT requestBatchReInstallContinue();
    qDebug() << __FUNCTION__ << files << " end ";
}

/*************************************************************************
 <Function>      onRemoveFileWatchers
 <Description>   批量移除文件监视器
 <Author>        null
 <Input>
    <param1>     files            Description:需要移除的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onRemoveFileWatchers(const QStringList &files)
{
    if (!files.isEmpty()) {
        //消除files为空的警告
        qDebug() << __FUNCTION__ << files.size() << m_fsWatcher->removePaths(files);
    }
    m_fsWatcher->removePath(FONTS_DIR);
    m_fsWatcher->removePath(FONTS_UP_DIR);
}

/*************************************************************************
 <Function>      onAutoDirWatchers
 <Description>   添加特定文件夹的文件监视器
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onAutoDirWatchers()
{
    m_fsWatcher->addPath(FONTS_DIR);
    m_fsWatcher->addPath(FONTS_UP_DIR);
}

/**
* @brief DFontPreviewListDataThread::onExportFont 导出文件列表槽函数
* @param fontList 要导出的文件列表
* @return void
*/
void DFontPreviewListDataThread::onExportFont(QStringList &fontList)
{
    qDebug() << __FUNCTION__ << fontList.size();

    QDir dir(FONTS_DESKTOP_DIR);
    if (!dir.exists())
        dir.mkpath(FONTS_DESKTOP_DIR);

    DCopyFilesManager::instance()->copyFiles(CopyFontThread::EXPORT, fontList);
    Q_EMIT exportFontFinished(fontList.size());
}

DFontPreviewListView *DFontPreviewListDataThread::getView() const
{
    return m_view;
}

void DFontPreviewListDataThread::onRefreshUserAddFont(QList<DFontInfo> &fontInfoList)
{
    if (fontInfoList.size() < 1) {
        return;
    }
    int index =  m_dbManager->getCurrMaxFontId() + 1;
    DFMDBManager *fDbManager = DFMDBManager::instance();
    m_diffFontModelList.clear();

    for (auto fileInfo : fontInfoList) {
        index = insertFontItemData(fileInfo, index, m_chineseFontPathList, m_monoSpaceFontPathList, false, true);
    }
    fDbManager->commitAddFontInfo();
    m_isAllLoaded = true; //用户字体加载完成才是全部完成
    emit m_view->refreshListview(m_diffFontModelList);
}
/*************************************************************************
 <Function>      updateDb
 <Description>   当数据库表结构更新时（数据被清空），从系统读取字体配置，并将旧数据库数据更新到数据库
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateDb()
{
    withoutDbRefreshDb();
    return;
}

/*************************************************************************
 <Function>      withoutDbRefreshDb
 <Description>   没有数据库时刷新数据库
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::withoutDbRefreshDb()
{
    if(!m_dbManager->isDBDeleted()){
        return;
    }
    qDebug() << "strAllFontList.size()" << m_allFontPathList.size() << endl;

    int index = 0;
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QList<DFontInfo> fontInfolist;
    DFontInfo info;

    //没有数据库时通过fclist获取到的所有已安装的字体文件，经过去重添加到数据库中
    for (QString &filePath : m_allFontPathList) {
        if (filePath.length() > 0) {
            info = fontInfoMgr->getFontInfo(filePath, true);
            if (!fontInfolist.contains(info)) {
                fontInfolist << info;
                index = insertFontItemData(info, index, m_chineseFontPathList, m_monoSpaceFontPathList, true);
            }
        }
    }

    m_dbManager->commitAddFontInfo();

    m_dbManager->syncOldRecords();//数据库t_fontmanager表被重建时，先saveRecord，再updateOld2Record。
    m_view->onFinishedDataLoad();
}

/*************************************************************************
 <Function>      appendItemData
 <Description>   添加符合条件的itemData
 <Author>        null
 <Input>
    <param1>     itemData            Description:ItemModel信息结构体
    <param1>     isStartup           Description:是否为第一次启动
 <Return>        void                Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::appendItemData(const DFontPreviewItemData &itemData, const bool &isStartup)
{
    m_dbManager->addFontInfo(itemData);

    if (!m_fontModelList.contains(itemData) || itemData.fontInfo.isSystemFont) {
        m_fontModelList.append(itemData);
    }

    if (!isStartup) {
        if (!m_diffFontModelList.contains(itemData) || itemData.fontInfo.isSystemFont) {
            m_diffFontModelList.append(itemData);
        }
    }

}

/*************************************************************************
 <Function>      insertFontItemData
 <Description>   将需要添加项的字体数据收集放人list中.
 <Author>        null
 <Input>
    <param1>     filepath            Description:文件路径
    <param2>     index               Description:null添加的位置
    <param3>     chineseFontPathList           Description:中文字体列表
    <param4>     monoSpaceFontPathList           Description:等宽字体列表
    <param5>     isStartup           Description:是否为第一次启动
    <param6>     isEnabled           Description:是否可以换启用

 <Return>        int                 Description:null
 <Note>          null
*************************************************************************/
int DFontPreviewListDataThread::insertFontItemData(const DFontInfo info,
                                                   int index,
                                                   const QStringList &chineseFontPathList,
                                                   const QStringList &monoSpaceFontPathList,
                                                   bool isStartup, bool isEnabled)
{
    DFontPreviewItemData itemData;
    itemData.fontInfo = info;

    if (!itemData.fontInfo.styleName.isEmpty() && !itemData.fontInfo.familyName.endsWith(itemData.fontInfo.styleName) && itemData.fontInfo.familyName != QLatin1String("UntitledFont")) {
        itemData.fontData.strFontName = QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
    } else {
        itemData.fontData.strFontName = itemData.fontInfo.familyName;
    }

    itemData.strFontId = QString::number(index);
    itemData.fontData.setEnabled(isEnabled);
    itemData.fontData.setCollected(false);
    itemData.fontData.setChinese(chineseFontPathList.contains(info.filePath) && (itemData.fontInfo.previewLang & FONT_LANG_CHINESE));
    itemData.fontData.setMonoSpace(monoSpaceFontPathList.contains(info.filePath));
    itemData.fontData.setFontType(itemData.fontInfo.type);
    itemData.fontData.isSystemFont = m_dbManager->isSystemFont(info.filePath);

    itemData.fontInfo.isInstalled = true;

    // ttc文件包含多个ttf字体，需要找出每一个ttf字体
    if (itemData.fontInfo.filePath.endsWith(".ttc", Qt::CaseInsensitive)) {
        /* Bug#16821 UT000591  添加字体后需要加入到Qt的字体数据库中，否则无法使用*/
//        qDebug() << "addApplicationFont s"  << endl;
        int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
//        qDebug() << "addApplicationFont e"  << appFontId << itemData.fontData.strFontName << endl;
        itemData.appFontId = appFontId;

        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        if (fontFamilyList.size() > 1) {
            for (QString &fontFamily : fontFamilyList) {
                itemData.strFontId = QString::number(index++);
                itemData.fontInfo.familyName = fontFamily;
                itemData.fontInfo.sp3FamilyName = fontFamily;
                if (itemData.fontInfo.styleName.length() > 0) {
                    itemData.fontData.strFontName =
                        QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
                } else {
                    itemData.fontData.strFontName = itemData.fontInfo.familyName;
                }
                appendItemData(itemData, isStartup);
            }
            index--;
        } else {
            appendItemData(itemData, isStartup);
        }
    } else {
        appendItemData(itemData, isStartup);
    }

//    Q_EMIT m_view->itemAdded(itemData);
    addPathWatcher(info.filePath);

    return (index + 1);
}

/*************************************************************************
 <Function>      refreshFontListData
 <Description>    刷新字体列表的数据
 <Author>        null
 <Input>
    <param1>     isStartup              Description:是否为第一次启动
    <param2>     installFont            Description:安装的字体文件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread:: refreshFontListData(bool isStartup, const QStringList &installFont)
{
    qDebug() << __FUNCTION__ << " begin" << isStartup << installFont.size() << QThread::currentThreadId() << m_fontModelList.size();

    if (isStartup) {
        for (const QString &filePath : installFont) {
            if (!m_allFontPathList.contains(filePath))
                m_allFontPathList << filePath;
        }
    } else {
        FontManager::instance()->getFontListInSequence();
    }

    QList<DFontPreviewItemData> datalist;
    if (SignalManager::m_isDataLoadFinish == false) {
        datalist = DFMDBManager::instance()->getFontInfo(DFMDBManager::recordList, &m_delFontInfoList);

        m_fontModelList.append(datalist);
        for (DFontPreviewItemData &itemData : m_delFontInfoList) {
            //如果字体文件已经不存在，则从t_manager表中删除
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            m_view ->enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
        }

        DFMDBManager::instance()->commitDeleteFontInfo();
        m_view->enableFonts();

        foreach (auto it, m_startModelList) {
            addPathWatcher(it.fontInfo.filePath);
        }
        m_delFontInfoList.clear();
        SignalManager::m_isDataLoadFinish = true;
    }

    QSet<QString> dbFilePathSet;
    for (DFontPreviewItemData &itemData : m_fontModelList) {
        QString filePath = itemData.fontInfo.filePath.trimmed();

        dbFilePathSet.insert(filePath);
        if (isStartup)
            addPathWatcher(filePath);
    }

    if (isStartup) {
        for (DFontPreviewItemData &itemData : m_delFontInfoList) {
            //如果字体文件已经不存在，则从t_manager表中删除
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            m_view->enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
        }
    }

    DFMDBManager::instance()->commitDeleteFontInfo();
    m_view->enableFonts();

    m_delFontInfoList.clear();
    m_diffFontModelList.clear();

    //根据文件路径比较出不同的字体文件
    QSet<QString> allFontListSet = m_allFontPathList.toSet();
    QSet<QString> diffSet = allFontListSet.subtract(dbFilePathSet);

    qInfo() << "diffSet count:" << diffSet.count();
    if (diffSet.count() > 0) {
        int maxFontId = m_dbManager->getCurrMaxFontId();
        QList<QString> diffFilePathList = diffSet.toList();
        int index = maxFontId + 1;
        DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
        DFontInfo info;
        for (QString &filePath : diffFilePathList) {
            if (m_dbManager->isSystemFont(filePath) || installFont.contains(filePath)) {
                bool isEnabled = (isStartup && installFont.contains(filePath)) ? false : true;
                info = fontInfoMgr->getFontInfo(filePath, true);
                index = insertFontItemData(info, index, m_chineseFontPathList, m_monoSpaceFontPathList, isStartup, isEnabled);
            }
        }

        m_dbManager->commitAddFontInfo();
    }
    qInfo() << __FUNCTION__ << " end " << QThread::currentThreadId() << m_diffFontModelList.size() << m_fontModelList.size();

    if (!isStartup && !installFont.isEmpty()) {
        Q_EMIT m_view->multiItemsAdded(datalist, DFontSpinnerWidget::NoLabel);
        Q_EMIT m_view->multiItemsAdded(m_diffFontModelList, DFontSpinnerWidget::NoLabel);
        Q_EMIT m_view->itemsSelected(installFont);
    } else if (isStartup) {
        QList<DFontPreviewItemData> startuplist = m_fontModelList.mid(0, 50);
        Q_EMIT m_view->multiItemsAdded(startuplist, DFontSpinnerWidget::Load);
    } else {
        Q_EMIT m_view->multiItemsAdded(m_fontModelList, DFontSpinnerWidget::Load);
    }
}

/*************************************************************************
 <Function>      refreshStartupFontListData
 <Description>   启动时把已经处理好的50个数据添加到listview里
 <Author>        null
 <Input>
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::refreshStartupFontListData()
{
    for (DFontPreviewItemData &itemData : m_delFontInfoList) {
        //如果字体文件已经不存在，则从t_manager表中删除
        //删除字体之前启用字体，防止下次重新安装后就被禁用
        m_view->enableFont(itemData.fontInfo.filePath);
        DFMDBManager::instance()->deleteFontInfo(itemData);
    }

    DFMDBManager::instance()->commitDeleteFontInfo();
    m_view->enableFonts();

    foreach (auto it, m_startModelList) {
        addPathWatcher(it.fontInfo.filePath);
    }

    Q_EMIT m_view->multiItemsAdded(m_startModelList, DFontSpinnerWidget::StartupLoad);

    m_delFontInfoList.clear();

}


/*************************************************************************
 <Function>      syncFontEnableDisableStatusData
 <Description>   更新从配置文件中读到的禁用字体的信息
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::syncFontEnableDisableStatusData(const QStringList &disableFontPathList)
{
    //disableFontPathList为被禁用的字体路径列表
//    if (disableFontPathList.size() == 0) {
//        return;
//    }

    QMap<QString, bool> disableFontMap;
    for (const QString &disableFontPath : disableFontPathList) {
        disableFontMap.insert(disableFontPath, true);
    }

    for (DFontPreviewItemData &fontItemData : m_fontModelList) {
        QString keyFilePath = fontItemData.fontInfo.filePath;

        if (fontItemData.fontData.isEnabled() != disableFontMap.value(keyFilePath))
            continue;

        //disableFontMap为被禁用的字体map
        if (disableFontMap.value(keyFilePath)) {
            fontItemData.fontData.setEnabled(false);
        } else {
            fontItemData.fontData.setEnabled(true);
        }

        m_dbManager->updateFontInfo(fontItemData, "isEnabled");
    }

    m_dbManager->commitUpdateFontInfo();
}


/*************************************************************************
 <Function>      updateFontId
 <Description>   更新字体信息中的fontid
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateFontId(const DFontPreviewItemData &itemData, int id)
{
    if (id < 0)
        return;
    int index = m_fontModelList.indexOf(itemData);
    if (index > -1) {
        m_fontModelList[index].appFontId = id;
        if (m_fontModelList[index].fontInfo.sp3FamilyName != itemData.fontInfo.sp3FamilyName)
            m_fontModelList[index].fontInfo.sp3FamilyName = itemData.fontInfo.sp3FamilyName;
    }
}
