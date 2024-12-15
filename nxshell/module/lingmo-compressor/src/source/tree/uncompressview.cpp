// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uncompressview.h"
#include "datamodel.h"
#include "treeheaderview.h"
#include "openwithdialog.h"
#include "DebugTimeManager.h"
#include "datamanager.h"
#include "popupdialog.h"
#include "uitools.h"
#include <DFontSizeManager>

#include <DMenu>
#include <DFileDialog>
#include <DCheckBox>
#include <DPasswordEdit>

#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>
#include <QDateTime>
#include <QApplication>
#include <QMimeData>
#include <QItemSelectionModel>
#include <QScrollBar>
#include <QFileInfo>


UnCompressView::UnCompressView(QWidget *parent)
    : DataTreeView(parent)
{
    initUI();
    initConnections();
}

UnCompressView::~UnCompressView()
{
    clearDragData();
}

void UnCompressView::refreshArchiveData()
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();

    // 刷新第一层级文件夹子项的数目
    for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
        if (stArchiveData.listRootEntry[i].isDirectory) {
            stArchiveData.listRootEntry[i].qSize = calDirItemCount(stArchiveData.listRootEntry[i].strFullPath);
        }
    }

    // 刷新数据（显示第一层数据）
    m_pModel->refreshFileEntry(stArchiveData.listRootEntry);
    m_mapShowEntry["/"] = stArchiveData.listRootEntry;

    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);
    resetLevel();       // 重置目录层级
}

void UnCompressView::setArchivePath(const QString &strPath)
{
    m_strArchive = strPath;
    m_strArchivePath = QFileInfo(strPath).path();
}

void UnCompressView::setDefaultUncompressPath(const QString &strPath)
{
    m_strUnCompressPath = strPath;
}

void UnCompressView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_dragPos = event->pos();
    }

    viewport()->update();

    DataTreeView::mousePressEvent(event);
}

void UnCompressView:: mouseMoveEvent(QMouseEvent *event)
{

    if (m_isPressed) {
        //最小距离为防误触和双向滑动时,只触发横向或者纵向的
        int touchmindistance = 2;
        //最大步进距离是因为原地点按马上放开,则会出现-35~-38的不合理位移,加上每次步进距离没有那么大,所以设置为30
        int touchMaxDistance = 30;
        event->accept();
        double horiDelta = event->pos().x() - m_lastTouchBeginPos.x();
        double vertDelta = event->pos().y() - m_lastTouchBeginPos.y();

        if (qAbs(horiDelta) > touchmindistance && qAbs(horiDelta) < touchMaxDistance) {
            horizontalScrollBar()->setValue(static_cast<int>(horizontalScrollBar()->value() - horiDelta));
        }

        if (qAbs(vertDelta) > touchmindistance && !(qAbs(vertDelta) < m_pHeaderView->height() + 2 && qAbs(vertDelta) > m_pHeaderView->height() - 2 && m_lastTouchTime.msecsTo(QTime::currentTime()) < 100)) {
            verticalScrollBar()->setValue(static_cast<int>(verticalScrollBar()->value() - vertDelta));
        }
        m_lastTouchBeginPos = event->pos();
        return;
    }


    QModelIndexList listSel = selectedIndexes();

    if (listSel.size() < 1) {
        return;
    }

    if (!(event->buttons() & Qt::MouseButton::LeftButton) || m_pFileDragServer) {
        return;
    }

    // 曼哈顿距离处理，避免误操作
    if ((event->pos() - m_dragPos).manhattanLength() < QApplication::startDragDistance()) {
        return;
    }

    // 创建文件拖拽服务，处理拖拽到文管操作
    m_pFileDragServer = new DFileDragServer(this);
    m_pDrag = new DFileDrag(this, m_pFileDragServer);
    QMimeData *m = new QMimeData();
    m->setData("NOT_NEED_SET_TARGET_IN_DRAG", "dragextract");
    QVariant value = listSel[0].data(Qt::DecorationRole);

    if (value.isValid()) {
        if (value.type() == QVariant::Pixmap) {
            m_pDrag->setPixmap(qvariant_cast<QPixmap>(value));
        } else if (value.type() == QVariant::Icon) {
            m_pDrag->setPixmap((qvariant_cast<QIcon>(value)).pixmap(24, 24));
        }
    }

    m_pDrag->setMimeData(m);

    connect(m_pDrag, &DFileDrag::targetUrlChanged, [&] {
        QUrl url;
        m_bReceive = true;
        url = m_pDrag->targetUrl();
        if (url.isValid())
        {
            m_strSelUnCompressPath = url.toLocalFile(); // 获取拖拽提取目标路径
            if (m_bDrop && m_bReceive) {
                extract2Path(m_strSelUnCompressPath);
                clearDragData();
            }
        } else
        {
            clearDragData();
        }

    });

    Qt::DropAction result = m_pDrag->exec(Qt::CopyAction);
    m_bDrop = true;

    m_pFileDragServer->setProgress(100);
    m_pFileDragServer->deleteLater();
    m_pFileDragServer = nullptr;
    qInfo() << "sigdragLeave";

    if (result == Qt::DropAction::CopyAction) {
        if (m_bDrop && m_bReceive) {
            extract2Path(m_strSelUnCompressPath);
            clearDragData();
        }
    } else {
        clearDragData();
    }

    m_bDrop = false;    // 无论exec的返回值是否是CopyAction，当此拖拽功能结束时应该将此变量置为false，防止后续再次接收到targetUrlChanged信号会触发clearDragData
    m_strSelUnCompressPath.clear();
}

void UnCompressView::clearDragData()
{
    if (m_pDrag) {
        m_pDrag->disconnect();      // 先断开信号，防止deleteLater过程中还有有一些处理
        m_pDrag->deleteLater();
        m_pDrag = nullptr;
    }

    if (m_pFileDragServer) {
        m_pFileDragServer->setProgress(100);
        m_pFileDragServer->deleteLater();
        m_pFileDragServer = nullptr;
    }

    m_strSelUnCompressPath.clear();
    m_bDrop = false;
    m_bReceive = false;
}

void UnCompressView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        handleDoubleClick(indexAt(event->pos()));   // 双击处理
    }

    DataTreeView::mouseDoubleClickEvent(event);
}

void UnCompressView::initUI()
{

}

void UnCompressView::initConnections()
{
    connect(this, &UnCompressView::signalDragFiles, this, &UnCompressView::slotDragFiles, Qt::QueuedConnection);    // wayland下使用Qt::QueuedConnection连接方式，防止对话框exec导致事件阻塞，图标停留在界面上
    connect(this, &UnCompressView::customContextMenuRequested, this, &UnCompressView::slotShowRightMenu);
    connect(this, &UnCompressView::sigRenameFile, this, &UnCompressView::slotRenameFile);
}

qlonglong UnCompressView::calDirItemCount(const QString &strFilePath)
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
    qlonglong qItemCount = 0;

    auto iter = stArchiveData.mapFileEntry.find(strFilePath);
    for (; iter != stArchiveData.mapFileEntry.end();) {
        // 找到以当前文件夹开头的数据，进行计算处理
        if (!iter.key().startsWith(strFilePath)) {
            break;
        } else {
            if (iter.key().size() > strFilePath.size()) {
                QString chopStr = iter.key().right(iter.key().size() - strFilePath.size());
                // 只计算当前目录中的第一层数据（包含文件和文件夹）
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    ++qItemCount;
                }
            }

            ++iter;
        }
    }

    return qItemCount;
}

void UnCompressView::handleDoubleClick(const QModelIndex &index)
{
    // qInfo() << index.data(Qt::DisplayRole);

    if (index.isValid()) {
        FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {     // 如果是文件夹，进入下一层
            m_iLevel++;
            m_strCurrentPath = entry.strFullPath;
            refreshDataByCurrentPath();     // 刷新数据
            resizeColumnWidth();            // 重置列宽
            // 重置排序
            m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
            sortByColumn(DC_Name);

            m_vPre.push_back(entry.strFileName); // 保存进入的文件夹名

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
            slotOpen();
        }
    }
}

void UnCompressView::refreshDataByCurrentPath()
{
    if (0 == m_iLevel) {
        setPreLblVisible(false);
    } else {
        QString strTempPath = m_strCurrentPath;
        int iIndex = strTempPath.lastIndexOf(QDir::separator());
        if (iIndex > 0)
            strTempPath = strTempPath.left(iIndex);
        setPreLblVisible(true, strTempPath);
    }

    // 若缓存中找不到下一层数据，从总数据中查找并同步更新到缓存数据中
    if (m_mapShowEntry.find(m_strCurrentPath) == m_mapShowEntry.end()) {
        m_mapShowEntry[m_strCurrentPath] = getCurPathFiles();
    }

    m_pModel->refreshFileEntry(m_mapShowEntry[m_strCurrentPath]);
}

void UnCompressView::refreshDataByCurrentPathChanged()
{
    if (0 == m_strCurrentPath.compare("/")) { //当前目录是第一层
        m_mapShowEntry.clear();

        refreshArchiveData();
    } else { //当前目录非第一层
        ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

        // 删除上一级m_mapShowEntry
        if (1 == m_strCurrentPath.count("/")) { //当前目录是第二层
            // 刷新第一层文件夹子项的数目
            for (int i = 0; i < stArchiveData.listRootEntry.count(); ++i) {
                if (stArchiveData.listRootEntry[i].isDirectory) {
                    stArchiveData.listRootEntry[i].qSize = calDirItemCount(stArchiveData.listRootEntry[i].strFullPath);
                }
            }
            m_mapShowEntry["/"] = stArchiveData.listRootEntry;
        } else { //当前目录是第三层及之后
            m_mapShowEntry.remove(m_strCurrentPath.left(m_strCurrentPath.lastIndexOf("/", -2) + 1));
        }

        // 更新当前目录m_mapShowEntry
        foreach (auto &tmp, m_mapShowEntry.keys()) {
            if (tmp.startsWith(m_strCurrentPath)) {
                m_mapShowEntry.remove(tmp);
            }
        }

        m_mapShowEntry[m_strCurrentPath] = getCurPathFiles();

        // 刷新列表数据
        m_pModel->refreshFileEntry(m_mapShowEntry[m_strCurrentPath]);
    }

    // 重置排序
    m_pHeaderView->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    sortByColumn(DC_Name);

    // 追加时需要选中追加的文件
    if (CT_Add == m_eChangeType) {
        // 获取所有的追加文件的文件名
        QStringList listSelName;
        foreach (QString strFile, m_listAddFiles) {
            listSelName.push_back(QFileInfo(strFile).fileName());
        }

        // 设置多选
        QItemSelectionModel *pSelectionModel = selectionModel();
        pSelectionModel->select(m_pModel->getSelectItem(listSelName), QItemSelectionModel::SelectCurrent);

        m_listAddFiles.clear(); // 刷新完之后清空追加的文件数据
    }

    m_eChangeType = CT_None;    // 重置操作类型
}

void UnCompressView::addNewFiles(const QStringList &listFiles)
{
    if (listFiles.empty())
        return;

    m_listAddFiles.clear();

    // 追加选项判断
    QString strPassword;
    AppendDialog dialog(this);
    int iResult = dialog.showDialog(m_bMultiplePassword);

    // 点击取消按钮时，不进行追加操作
    if (iResult != DDialog::Accepted) {
        return;
    }
    strPassword = dialog.password();

    // 重复提示
    ArchiveData stArchiveData = DataManager::get_instance().archiveData();
    bool bApplyAll = false;
    bool bOverwrite = false;
    for (int i = 0; i < listFiles.count(); ++i) {
        if (listFiles[i] == m_strArchive) { // 追加的文件含有压缩包本身
            TipDialog tipDialog(this);
            tipDialog.showDialog(tr("You cannot add the archive to itself"), tr("OK", "button"));
            return;
        }

        // 初始化数据
        QFileInfo fileInfo(listFiles[i]);
        QString strLocalFile = fileInfo.filePath();
        QString strTempName;

        // 处理文件名（和压缩包中格式一致）
        if (fileInfo.isDir()) {
            strTempName = getCurPath() + strLocalFile.remove(0, fileInfo.path().length() + 1) + QDir::separator();  // 移除路径后添加'/'
        } else {
            strTempName = getCurPath() + strLocalFile.remove(0, fileInfo.path().length() + 1);  // 移除路径
        }

        // 文件判断是否有同名存在
        if (stArchiveData.mapFileEntry.find(strTempName) != stArchiveData.mapFileEntry.end()) {
            // 文件/文件夹已存在时，首先判断是否是全部应用
            if (bApplyAll) {
                // 全部应用处理
                if (bOverwrite) {
                    m_listAddFiles = listFiles;   // 若全部替换，追加所有文件
                    break;
                } else {
                    continue;       // 若全部跳过，重复文件不处理
                }
            } else {
                // 若非全部应用，弹出重复提示对话框
                OverwriteQueryDialog tempDialog(this);
                tempDialog.showDialog(strTempName, fileInfo.isDir());

                // 获取重复对话框操作的选项
                Overwrite_Result iResult = tempDialog.getDialogResult();
                bApplyAll = tempDialog.getApplyAll();

                switch (iResult) {
                // 点击关闭按钮直接跳过追加操作
                case OR_Cancel:
                    bOverwrite = false;
                    return;
                // 点击跳过，不追加此文件/文件夹
                case OR_Skip:
                    bOverwrite = false;
                    break;
                // 追加此文件/文件夹
                default:
                    bOverwrite = true;
                    m_listAddFiles << listFiles[i];
                    break;
                }
            }
        } else {
            // 不存在重复文件
            m_listAddFiles << listFiles[i];
        }
    }

    // 没有需要追加的文件时，直接返回，防止出现追加根目录的现象
    if (m_listAddFiles.isEmpty())
        return;

    // 去除同名称文件
    m_listAddFiles = UiTools::removeSameFileName(m_listAddFiles);

    // 发送追加信号
    m_eChangeType = CT_Add;
    emit signalAddFiles2Archive(m_listAddFiles, strPassword);
}

QString UnCompressView::getCurPath()
{
    return (0 == m_iLevel) ? "" : m_strCurrentPath;     // 根目录提取时上级赋值为空
}

void UnCompressView::setModifiable(bool bModifiable, bool bMultiplePassword)
{
    m_bModifiable = bModifiable;                // 压缩包数据是否可修改
    m_bMultiplePassword = bMultiplePassword;    // 压缩包是否支持多密码

    setAcceptDrops(m_bModifiable);
}

bool UnCompressView::isModifiable()
{
    return m_bModifiable;
}

void UnCompressView::clear()
{
    // 重置数据
    m_stRightEntry = FileEntry();
    m_mapShowEntry.clear();
    DataManager::get_instance().resetArchiveData();
    resetLevel();
    m_strUnCompressPath = "";
    m_bModifiable = false;
    m_bMultiplePassword = false;
    clearDragData();
}

QList<FileEntry> UnCompressView::getCurPathFiles()
{
    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
    QList<FileEntry> listEntry;

    auto iter = stArchiveData.mapFileEntry.find(m_strCurrentPath);
    for (; iter != stArchiveData.mapFileEntry.end() ;) {
        if (iter.key().left(m_strCurrentPath.size()) != m_strCurrentPath) {
            break;
        } else {
            QString chopStr = iter.key().right(iter.key().size() - m_strCurrentPath.size());
            if (!chopStr.isEmpty()) {
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    FileEntry &entry = iter.value();

                    // 如果是文件夹，刷新大小（同步更新map压缩包中缓存数据）
                    if (entry.isDirectory) {
                        entry.qSize = calDirItemCount(entry.strFullPath);
                    }

                    listEntry << iter.value();
                }
            }
            ++iter;
        }
    }

//    int iiii = 0; // 未使用
    return listEntry;
}

//void UnCompressView::getAllFilesByParentPath(const QString &strFullPath, QList<FileEntry> &listEntry, qint64 &qSize)
//{
//    ArchiveData &stArchiveData =  DataManager::get_instance().archiveData();
//    listEntry.clear();
//    qSize = 0;

//    auto iter = stArchiveData.mapFileEntry.find(strFullPath);
//    for (; iter != stArchiveData.mapFileEntry.end() ;) {

//        if (!iter.key().startsWith(strFullPath)) {
//            break;
//        } else {
//            if (!iter.key().endsWith("/")) {
//                qSize += iter.value().qSize; //文件大小
//            }

//            listEntry << iter.value();

//            ++iter;
//        }
//    }
//}

QList<FileEntry> UnCompressView::getSelEntry()
{
    QList<FileEntry> listSelEntry;

    QModelIndexList listModelIndex = selectionModel()->selectedRows();

    foreach (QModelIndex index, listModelIndex) {
        if (index.isValid()) {
            FileEntry entry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据
            listSelEntry << entry;
        }
    }

    return listSelEntry;
}

void UnCompressView::extract2Path(const QString &strPath)
{
    QList<FileEntry> listSelEntry = getSelEntry();    // 待提取的文件数据
    ExtractionOptions stOptions;    // 提取参数
    stOptions.strTargetPath = strPath;
    stOptions.strDestination = (0 == m_iLevel) ? "" : m_strCurrentPath;     // 根目录提取时上级赋值为空

    // 获取所有文件数据
    foreach (FileEntry entry, listSelEntry) {
        if (entry.isDirectory) {
            QList<FileEntry> listEntry;
            calEntrySizeByParentPath(entry.strFullPath, stOptions.qSize);
        } else {
            stOptions.qSize += entry.qSize;
        }
    }

    // 发送提取信号
    emit signalExtract2Path(listSelEntry, stOptions);
}

void UnCompressView::calEntrySizeByParentPath(const QString &strFullPath, qint64 &qSize)
{
    ArchiveData stArchiveData =  DataManager::get_instance().archiveData();
    qSize = 0;

    auto iter = stArchiveData.mapFileEntry.find(strFullPath);
    for (; iter != stArchiveData.mapFileEntry.end() ;) {
        if (!iter.key().startsWith(strFullPath)) {
            break;
        } else {
            if (!iter.key().endsWith("/")) {
                qSize += iter.value().qSize; //文件大小
            }
            ++iter;
        }
    }
}

void UnCompressView::slotDragFiles(const QStringList &listFiles)
{
    addNewFiles(listFiles);
}

void UnCompressView::slotShowRightMenu(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);

    if (index.isValid()) {
        m_stRightEntry = index.data(Qt::UserRole).value<FileEntry>();  // 获取文件数据

        DMenu menu(this);
        menu.setMinimumWidth(202);

        // 右键-提取
        menu.addAction(tr("Extract", "提取"), this, &UnCompressView::slotExtract);
        // 右键-提取到当前文件夹
        menu.addAction(tr("Extract to current directory"), this, &UnCompressView::slotExtract2Here);
        // 右键-打开
        menu.addAction(tr("Open"), this, &UnCompressView::slotOpen);
        // 右键-重命名
        QAction *renameAct = menu.addAction(tr("Rename"), this, &UnCompressView::slotRenameFile);
        if(selectionModel()->selectedRows().count() != 1 || !m_bModifiable) {
            renameAct->setEnabled(false);
        }
        // 右键-删除
        QAction *pAction = menu.addAction(tr("Delete"), this, &UnCompressView::slotDeleteFile);

        if (!m_bModifiable) {
            pAction->setEnabled(false); // 若压缩包数据不能更改，深处按钮设置成不可用
        }

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
            // 获取支持的打开方式列表
            QList<DesktopFile> listOpenType = OpenWithDialog::getOpenStyle(m_stRightEntry.strFullPath);
            // 添加菜单选项
            for (int i = 0; i < listOpenType.count(); ++i) {
                QIcon icon = QIcon::fromTheme(listOpenType[i].getIcon());
                QAction *pOpenAction = openMenu.addAction(icon, listOpenType[i].getDisplayName(), this, SLOT(slotOpenStyleClicked()));
                if(!icon.isNull()) {
                    int nWidth = fm.width(listOpenType[i].getDisplayName()) * qApp->devicePixelRatio();
                    if(nWidth > nMax) {
                        nMax = nWidth;
                    }
                }
                pOpenAction->setData(listOpenType[i].getExec());
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

void UnCompressView::slotExtract()
{
    // 创建文件选择对话框
    DFileDialog dialog(this);
    dialog.setAcceptMode(DFileDialog::AcceptOpen);
    dialog.setFileMode(DFileDialog::Directory);
    dialog.setDirectory(m_strUnCompressPath);

    const int mode = dialog.exec();

    if (mode != QDialog::Accepted) {
        return;
    }

    // 获取选择的路径
    QList<QUrl> listUrl = dialog.selectedUrls();
    if (listUrl.count() > 0) {
        m_strSelUnCompressPath = listUrl.at(0).toLocalFile();
        extract2Path(m_strSelUnCompressPath);
    }
}

void UnCompressView::slotExtract2Here()
{
    extract2Path(m_strArchivePath);
}

void UnCompressView::slotDeleteFile()
{
    if (m_bModifiable) { // 压缩包数据是否可更改
        // 询问删除对话框
        SimpleQueryDialog dialog(this);
        int iResult = dialog.showDialog(tr("Do you want to delete the selected file(s)?"), tr("Cancel", "button"), DDialog::ButtonNormal, tr("Confirm", "button"), DDialog::ButtonRecommend);
        if (1 == iResult) {
            // 删除压缩包数据
            QList<FileEntry> listSelEntry = getSelEntry();    // 待删除的文件数据
            qint64 qSize = 0;       // 所有需要删除的文件总大小

            // 获取所有文件数据
            foreach (FileEntry entry, listSelEntry) {
                if (entry.isDirectory) {
                    QList<FileEntry> listEntry;
                    calEntrySizeByParentPath(entry.strFullPath, qSize);
                } else {
                    qSize += entry.qSize;
                }
            }

            // 发送删除信号
            m_eChangeType = CT_Delete;
            emit signalDelFiles(listSelEntry, qSize);
        }
    }
}

void UnCompressView::slotRenameFile()
{
    if (!m_bModifiable) return; // 压缩包数据是否可更改
    // 询问重命名对话框
    // 重命名数据
    QList<FileEntry> listSelEntry = getSelEntry();    // 待重命名的文件数据

    // 重命名只操作单个文件
    if(listSelEntry.size() != 1) {
        return;
    }
    FileEntry &entry = listSelEntry[0];

    RenameDialog dialog(this);
    QString strShowName;
    if(entry.strAlias.isEmpty() || entry.strAlias.isNull()) {
        strShowName = entry.strFullPath;
    } else {
        strShowName = entry.strAlias;
    }
    bool isRepeat = (sender() == this);
    int iResult = dialog.showDialog(entry.strFileName, entry.strAlias, entry.isDirectory, isRepeat);
    if (1 == iResult) {
        entry.strAlias = QFileInfo(dialog.getNewNameText()).fileName();
        // 获取所有文件数据
        qint64 qSize = 0;       // 所有需要删除的文件总大小
        foreach (FileEntry entry, listSelEntry) {
            if (entry.isDirectory) {
                calEntrySizeByParentPath(entry.strFullPath, qSize);
            } else {
                qSize += entry.qSize;
            }
        }
        // 发送重命名信号
        m_eChangeType = CT_Rename;
        emit signalRenameFile(entry, qSize);
    }

}

void UnCompressView::slotOpen()
{
    // 获取当前点击的文件
    QModelIndex curIndex = currentIndex();

    if (curIndex.isValid()) {
        FileEntry entry = curIndex.data(Qt::UserRole).value<FileEntry>();

        if (entry.isDirectory) {
            // 文件夹进入下一层
            handleDoubleClick(curIndex);
        } else {
            // 文件 解压再用默认应用程序打开
            emit signalOpenFile(entry);
        }
    }
}

void UnCompressView::slotOpenStyleClicked()
{
    QAction *pAction = qobject_cast<QAction *>(sender());
    if (nullptr == pAction) {
        return;
    }

    QString strText = pAction->text();

    if (tr("Select default program") == strText) {
        // 用选择的应用程序打开
        OpenWithDialog dialog(m_stRightEntry.strFullPath);
        QString str = dialog.showOpenWithDialog(OpenWithDialog::SelectType);
        if (!str.isEmpty())
            // 发送打开信号（以xx应用打开）
            emit signalOpenFile(m_stRightEntry, str);
    } else {
        // 发送打开信号（以xx应用打开）
        emit signalOpenFile(m_stRightEntry, pAction->data().toString());
    }
}

void UnCompressView::slotPreClicked()
{
    m_iLevel--;     // 目录层级减1

    if (0 == m_iLevel) {    // 如果返回到根目录，显示待压缩文件
        resetLevel();
    } else {        // 否则传递上级目录
        // 如果以'/'结尾,先移除最后一个'/',方便接下来截取倒数第二个'/'
        if (m_strCurrentPath.endsWith(QDir::separator())) {
            m_strCurrentPath.chop(1);
        }
        int iIndex = m_strCurrentPath.lastIndexOf(QDir::separator());
        m_strCurrentPath = m_strCurrentPath.left(iIndex + 1); // 当前路径截掉最后一级目录(保留'/')

    }

    refreshDataByCurrentPath();     // 刷新数据

    // 重置宽度
    resizeColumnWidth();

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
