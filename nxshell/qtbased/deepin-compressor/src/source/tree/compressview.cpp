// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "compressview.h"
#include "datamodel.h"
#include "uistruct.h"
#include "popupdialog.h"
#include "mimetypes.h"
#include "mimetypedisplaymanager.h"
#include "treeheaderview.h"
#include "openwithdialog.h"
#include "uitools.h"
#include "processopenthread.h"

#include <DMenu>

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QStandardPaths>
#include <QDateTime>
#include <QFileSystemWatcher>


CompressView::CompressView(QWidget *parent)
    : DataTreeView(parent)
{
    initUI();           // 初始化参数等
    initConnections();  // 初始化信号槽
}

CompressView::~CompressView()
{

}

void CompressView::addCompressFiles(const QStringList &listFiles)
{
    Overwrite_Result mode = OR_Cancel;
    bool applyAll = false;

    // 对新添加的文件进行判重
    m_listSelFiles = listFiles;

    foreach (QString oldPath, m_listCompressFiles) {
        QFileInfo oldFile(oldPath);  // 已存在的文件
        foreach (QString newPath, listFiles) {
            QFileInfo newFile(newPath);  // 新添加的文件
            if (oldFile.fileName() == newFile.fileName()) {  // 文件名相同的文件需要询问是否替换
                if (!applyAll) { // // 判断不是应用到全部文件，继续弹出询问对话框
                    OverwriteQueryDialog dialog(this);
                    dialog.showDialog(newFile.fileName());

                    mode = dialog.getDialogResult();
                    applyAll = dialog.getApplyAll();
                }

                if (OR_Cancel == mode || OR_Skip == mode) {  // OR_Cancel：取消  OR_Skip：跳过
                    m_listSelFiles.removeOne(newPath); // 在新添加的文件中删除该同名文件
                } else { // 替换
                    m_listCompressFiles.removeOne(oldPath); // 在已存在的文件中删除该同名文件
                }
            }
        }
    }

    m_listCompressFiles << m_listSelFiles;

    // 刷新待压缩数据
    refreshCompressedFiles();
}

QStringList CompressView::getCompressFiles()
{
    return m_listCompressFiles;
}

QList<FileEntry> CompressView::getEntrys()
{
    return m_listEntry;
}

void CompressView::refreshCompressedFiles(bool bChanged, const QString &strFileName)
{
    m_listEntry.clear();

    // 对变化的文件进行监控
    if (bChanged && !QFile::exists(strFileName)) {
        m_listCompressFiles.removeOne(strFileName);
    }

    // 刷新待压缩数据
    foreach (QString strFile, m_listCompressFiles) {
        m_listEntry << fileInfo2Entry(QFileInfo((strFile)));
    }

    // 刷新数据显示
    m_pModel->refreshFileEntry(m_listEntry);

    resizeColumnWidth();

    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);
    resetLevel();   // 重置目录层级

    // 选中新加的文件
    // 获取所有的追加文件的文件名
    QStringList listSelName;
    foreach (QString strFile, m_listSelFiles) {
        listSelName.push_back(QFileInfo(strFile).fileName());
    }

    // 设置多选
    QItemSelectionModel *pSelectionModel = selectionModel();
    pSelectionModel->select(m_pModel->getSelectItem(listSelName), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    m_listSelFiles.clear(); // 刷新完之后清空追加的文件数据

}

void CompressView::clear()
{
    // 重置数据
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听
    m_listCompressFiles.clear();
    m_listEntry.clear();
    resetLevel();
}

void CompressView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        handleDoubleClick(indexAt(event->pos()));   // 双击处理
    }

    DataTreeView::mouseDoubleClickEvent(event);
}

void CompressView::initUI()
{
    m_pFileWatcher = new QFileSystemWatcher(this);
}

void CompressView::initConnections()
{
    connect(this, &CompressView::signalDragFiles, this, &CompressView::slotDragFiles, Qt::QueuedConnection);
    connect(this, &CompressView::customContextMenuRequested, this, &CompressView::slotShowRightMenu);
    connect(m_pFileWatcher, &QFileSystemWatcher::directoryChanged, this, &CompressView::slotDirChanged); // 文件目录变化
}

FileEntry CompressView::fileInfo2Entry(const QFileInfo &fileInfo)
{
    FileEntry entry;

    entry.strFullPath = fileInfo.filePath();    // 文件全路径
    entry.strFileName = fileInfo.fileName();    // 文件名
    entry.isDirectory = fileInfo.isDir();   // 是否是文件夹

    if (entry.isDirectory) {
        // 文件夹显示子文件数目
        entry.qSize = QDir(fileInfo.filePath()).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden).count(); //目录下文件数
    } else {
        // 文件直接显示大小
        entry.qSize = fileInfo.size();
    }

    entry.uLastModifiedTime = fileInfo.lastModified().toTime_t();   // 最后一次修改时间

    return entry;
}

void CompressView::handleDoubleClick(const QModelIndex &index)
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听
    qInfo() << "gaoxiang:index.isValid()" << index.isValid();
    if (index.isValid()) {
        FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();
        qInfo() << "gaoxiang:entry.isDirectory" << entry.isDirectory;
        if (entry.isDirectory) {     // 如果是文件夹，进入下一层
            m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听
            m_iLevel++;
            m_strCurrentPath = entry.strFullPath;
            refreshDataByCurrentPath(); // 刷新数据
            handleLevelChanged();       // 处理层级变化
            resizeColumnWidth();        // 重置列宽
            // 重置排序
            m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            sortByColumn(DC_Name);

            m_vPre.push_back(entry.strFileName);
            // 空目录自动选中返回上一级目录Label bug122305
            if (model()->rowCount() == 0 && m_pHeaderView->isVisiable()) {
                m_pHeaderView->setLabelFocus(true);
            } else {
                // 自动选中第一行
                QModelIndex tmpindex = model()->index(0, 0, index);
                if (tmpindex.isValid()) {
                    setCurrentIndex(tmpindex);
                }
            }

        } else {    // 如果是文件，选择默认方式打开
            OpenWithDialog::openWithProgram(entry.strFullPath);
        }
    }
}

QList<FileEntry> CompressView::getCurrentDirFiles()
{
    // 获取当前目录下所有文件信息
    QFileInfoList lisfInfo = QDir(m_strCurrentPath).entryInfoList(QDir::AllEntries | QDir::System
                                                                  | QDir::NoDotAndDotDot | QDir::Hidden);     // 获取当前目录下所有子文件
    QList<FileEntry> listEntry;

    // 文件数据转换
    foreach (QFileInfo info, lisfInfo) {
        listEntry << fileInfo2Entry(info);
    }

    return listEntry;
}

void CompressView::handleLevelChanged()
{
    // 发送层级变化信号，通知mainwindow是否是根目录
    bool bRoot = (0 == m_iLevel ? true : false);
    setAcceptDrops(bRoot);
    emit signalLevelChanged(bRoot);
}

QString CompressView::getPrePathByLevel(const QString &strPath)
{
    // 若层级为0,即根目录，返回空
    if (0 == m_iLevel) {
        return "";
    }

    QString strResult = strPath;
    QString strTempPath = strPath;

    // 根据层级截取全路径字符串，避免目录从文件系统根目录开始
    for (int i = 0; i < m_iLevel; ++i) {
        int iIndex = strTempPath.lastIndexOf(QDir::separator());
        strTempPath = strTempPath.left(iIndex);
    }
    strTempPath += QDir::separator();

    return strResult.remove(strTempPath);
}

void CompressView::refreshDataByCurrentPath()
{
    if (0 == m_iLevel) {
        setPreLblVisible(false);
        m_pModel->refreshFileEntry(m_listEntry);    // 数据模型刷新
    } else {
        m_pFileWatcher->addPath(m_strCurrentPath);      // 添加目录监听
        setPreLblVisible(true, getPrePathByLevel(m_strCurrentPath));
        m_pModel->refreshFileEntry(getCurrentDirFiles());  // 刷新数据显示
    }

}

void CompressView::slotShowRightMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);

    if (index.isValid()) {

        m_stRightEntry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据

        DMenu menu(this);

        menu.setMinimumWidth(202);

        // 右键-打开
        menu.addAction(tr("Open"), this, [ = ] {
            if (m_stRightEntry.isDirectory)
            {
                // 文件夹进入下一层
                handleDoubleClick(index);
            } else
            {
                // 文件使用默认应用打开
                OpenWithDialog::openWithProgram(m_stRightEntry.strFullPath);
            }

        });
        // 右键-重命名
        QAction *renameAct = menu.addAction(tr("Rename"), this, &CompressView::slotRenameFile);
        if(selectionModel()->selectedRows().count() != 1) {
            renameAct->setEnabled(false);
        }
        // 右键-删除
        menu.addAction(tr("Delete"), this, &CompressView::slotDeleteFile);

        // 右键-打开方式
        DMenu openMenu(tr("Open with"), this);
        menu.addMenu(&openMenu);

        // 右键-选择默认应用程序
        openMenu.addAction(tr("Select default program"), this, SLOT(slotOpenStyleClicked()));
        int nMax = 0;
        QFontMetrics fm(openMenu.font());
        if (m_stRightEntry.isDirectory) {
            openMenu.setEnabled(false);
        } else {
            // 右键-打开方式选项
            QAction *pAction = nullptr;
            // 获取支持的打开方式列表
            QList<DesktopFile> listOpenType = OpenWithDialog::getOpenStyle(m_stRightEntry.strFullPath);
            // 添加菜单选项
            for (int i = 0; i < listOpenType.count(); ++i) {
                QIcon icon = QIcon::fromTheme(listOpenType[i].getIcon());
                pAction = openMenu.addAction(icon, listOpenType[i].getDisplayName(), this, SLOT(slotOpenStyleClicked()));
                if(!icon.isNull()) {
                    int nWidth = fm.width(listOpenType[i].getDisplayName()) * qApp->devicePixelRatio();
                    if(nWidth > nMax) {
                        nMax = nWidth;
                    }
                }
                pAction->setData(listOpenType[i].getExec());
            }
        }
        if(nMax > menu.minimumWidth()) {
            openMenu.setMinimumWidth(nMax);
        } else {
            openMenu.setMinimumWidth(menu.minimumWidth());
        }
        menu.exec(viewport()->mapToGlobal(pos));
    }
}

void CompressView::slotDeleteFile()
{
    QModelIndexList selectedIndex = selectionModel()->selectedRows(0);    // 获取所有待删除的第一行index

    if (0 == m_iLevel) { // 如果是根目录，删除缓存文件名

        // 从内存中删除选中的文件
        foreach (QModelIndex index, selectedIndex) {
            if (index.isValid()) {
                FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();
                int iRemoveIndex = m_listCompressFiles.indexOf(entry.strFullPath);

                // 从缓存中同步移除数据
                m_listCompressFiles.removeAt(iRemoveIndex);
                m_listEntry.removeAt(iRemoveIndex);
            }
        }

        m_pModel->refreshFileEntry(m_listEntry);   // 刷新列表数据

    } else { // 提示是否删除本地文件

        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("It will permanently delete the file(s). Are you sure you want to continue?"), tr("Cancel", "button"), DDialog::ButtonNormal, tr("Confirm", "button"), DDialog::ButtonWarning);
        if (1 == iResult) {     // 如果点击确定，移动本地文件至回收站中
            // 从本地文件中将需要删除的文件移动到回收站中
            foreach (QModelIndex index, selectedIndex) {
                if (index.isValid()) {
                    FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();
                    QFileInfo fi(entry.strFullPath);
                    if (fi.isDir()) {   // 删除文件夹
                        QDir dir(entry.strFullPath);
                        dir.removeRecursively();
                    } else {            // 删除文件
                        QFile::remove(entry.strFullPath);
                    }
                }
            }
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    }
}

void CompressView::slotRenameFile()
{
    QList<FileEntry> listSelEntry;

    QModelIndexList listModelIndex = selectionModel()->selectedRows();

    foreach (QModelIndex index, listModelIndex) {
        if (index.isValid()) {
            FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据
            listSelEntry << entry;
        }
    }
    // 重命名只操作单个文件
    if(listSelEntry.size() != 1) {
        return;
    }
    // 提示重命名文件
    RenameDialog dialog(this);
    // 获取重命名所需文件
    FileEntry entry = listSelEntry.first();
    bool isRepeat = true;
    if(sender())
        isRepeat = !sender()->property("repeat").isNull();
    int iResult = dialog.showDialog(entry.strFullPath, entry.strAlias, entry.isDirectory, isRepeat);
    if (0 == m_iLevel) { // 如果是根目录，重命名缓存文件名
        if (1 == iResult) {     // 如果点击确定，重命名文件
            // 从缓存中同步重命名数据
            int iRemoveIndex = m_listCompressFiles.indexOf(entry.strFullPath);
            if(iRemoveIndex == -1) return;
            FileEntry tmp = m_listEntry.at(iRemoveIndex);
            QString strAlias = QFileInfo(dialog.getNewNameText()).fileName();
            if(strAlias == entry.strFileName || strAlias == entry.strAlias) { //名字与原来相同不做处理
                qInfo() << entry.strAlias <<" | The name already exists. Level" << m_iLevel;
                if(!sender()) return;
                sender()->setProperty("repeat", "true");
                slotRenameFile();
                return;
            }
            QString strAliasEndPath = QDir::separator() + strAlias;
            for(FileEntry tmpentry: m_listEntry) {
                if(tmpentry.isDirectory == entry.isDirectory && tmpentry.strFullPath.endsWith(strAliasEndPath)) {
                    qInfo() << entry.strFullPath << " | The name already exists。 Level" << m_iLevel;
                    if(!sender()) return;
                    sender()->setProperty("repeat", "true");
                    slotRenameFile();
                    return;
                }
            }
            tmp.strAlias = strAlias;
            m_listEntry.replace(iRemoveIndex, tmp);
            m_pModel->refreshFileEntry(m_listEntry);   // 刷新列表数据
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    } else{
        if (1 == iResult) {     // 如果点击确定，重命名本地文件
            // 重命名本地文件
            QFileInfo fi(entry.strFullPath);
            if (fi.isDir()) {   // 重命名文件夹
                QDir dir(entry.strFullPath);
                bool bRename = dir.rename(entry.strFullPath, dialog.getNewNameText());
                if(!bRename) {
                    qInfo() << entry.strFullPath <<" | The name already exists. Level" << m_iLevel;
                    if(!sender()) return;
                    sender()->setProperty("repeat", "true");
                    slotRenameFile();
                }
            } else {            // 重命名文件
                QFile file(entry.strFullPath);
                bool bRename = file.rename(dialog.getNewNameText());
                if(!bRename) {
                    qInfo() << entry.strFullPath <<" | The name already exists. Level" << m_iLevel;
                    if(!sender()) return;
                    sender()->setProperty("repeat", "true");
                    slotRenameFile();
                }
            }
        } else {    // 点击关闭或者取消，不操作
            return;
        }
    }
}

void CompressView::slotDirChanged()
{
    // 刷新显示数据
    m_pModel->refreshFileEntry(getCurrentDirFiles());
}

void CompressView::slotDragFiles(const QStringList &listFiles)
{
    // 在压缩列表添加一个压缩文件
    if (listFiles.count() == 1 && UiTools::isArchiveFile(listFiles.at(0)) && m_listCompressFiles.count() > 0) {
        SimpleQueryDialog dialog;  // 询问添加压缩文件到压缩目录或者在新窗口打开压缩文件
        int ret = dialog.showDialog(tr("Do you want to add the archive to the list or open it in new window?"),
                                    tr("Cancel", "button"),  DDialog::ButtonNormal,
                                    tr("Add", "button"), DDialog::ButtonNormal,
                                    tr("Open in new window"), DDialog::ButtonRecommend);

        if (1 == ret) { // 添加压缩文件到目录
            addCompressFiles(listFiles);
        } else if (ret == 2) { // 在新窗口打开压缩文件
            ProcessOpenThread *p = new ProcessOpenThread;
            p->setProgramPath(OpenWithDialog::getProgramPathByExec("deepin-compressor"));
            p->setArguments(QStringList() << listFiles.at(0));
            p->start();
        }
    } else {
        addCompressFiles(listFiles);
    }
}

void CompressView::slotOpenStyleClicked()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if (nullptr == pAction) {
        return;
    }

    QString strText = pAction->text();

    if (tr("Select default program") == strText) {
        // 选择默认应用程序（弹出窗口）
        qInfo() << "选择默认应用程序打开";
        OpenWithDialog dialog(m_stRightEntry.strFullPath);
        dialog.showOpenWithDialog(OpenWithDialog::OpenType);
    } else {
        // 用选择的应用程序打开
        qInfo() << "选择打开方式：" << strText;
        OpenWithDialog::openWithProgram(m_stRightEntry.strFullPath, pAction->data().toString());
    }
}

void CompressView::slotPreClicked()
{
    m_pFileWatcher->removePath(m_strCurrentPath);       // 删除目录监听
    m_iLevel--;     // 目录层级减1

    if (0 == m_iLevel) {    // 如果返回到根目录，显示待压缩文件
        resetLevel();
    } else {        // 否则传递上级目录
        int iIndex = m_strCurrentPath.lastIndexOf(QDir::separator());
        m_strCurrentPath = m_strCurrentPath.left(iIndex);   // 当前路径截掉最后一级目录

    }

    refreshDataByCurrentPath();     // 刷新数据

    // 重置宽度
    resizeColumnWidth();
    // 目录层级变化处理
    handleLevelChanged();
    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);

    //自动选中第一行
    QModelIndex tmpindex = m_pModel->getListEntryIndex(m_vPre.back()); // 获取上层文件夹对应的QModelIndex
    m_vPre.pop_back();
    if (tmpindex.isValid()) {
        setCurrentIndex(tmpindex);
        setFocus(); //焦点丢失，需手动设置焦点
    }
}
