// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNCOMPRESSVIEW_H
#define UNCOMPRESSVIEW_H

#include "datatreeview.h"
#include "commonstruct.h"

#include <DWidget>
#include <DFileDragServer>
#include <DFileDrag>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE


// 解压列表
class UnCompressView : public DataTreeView
{
    Q_OBJECT
public:
    explicit UnCompressView(QWidget *parent = nullptr);
    ~UnCompressView() override;

    // 对压缩包数据更改的操作类型
    enum ChangeType {
        CT_None,        // 未做任何更改
        CT_Rename,      // 重命名
        CT_Delete,      // 删除文件
        CT_Add,         // 追加文件
    };

    /**
     * @brief refreshArchiveData   加载结束刷新数据
     */
    void refreshArchiveData();

    /**
     * @brief setArchivePath    设置压缩包路径
     * @param strPath
     */
    void setArchivePath(const QString &strPath);

    /**
     * @brief setDefaultUncompressPath  设置默认解压路径
     * @param strPath       路径
     */
    void setDefaultUncompressPath(const QString &strPath);

    /**
     * @brief refreshDataByCurrentPathChanged 刷新删除/追加的显示数据
     * 处理流程:
     * 1.删除当前目录m_mapShowEntry，并更新数据
     * 2.删除上一级m_mapShowEntry后不作处理，当返回上一级目录时会自动更新
     */
    void refreshDataByCurrentPathChanged();

    /**
     * @brief addNewFiles   追加新文件
     * @param listFiles     新文件
     */
    void addNewFiles(const QStringList &listFiles);

    /**
     * @brief getCurPath    获取当前层级路径
     * @return
     */
    QString getCurPath();

    /**
     * @brief setModifiable     设置压缩包数据是否可以更改
     * @param bModifiable   是否可更改
     * @param bMultiplePassword   追加是否可以多密码
     */
    void setModifiable(bool bModifiable, bool bMultiplePassword = false);

    /**
     * @brief isModifiable     获取压缩包数据是否可以更改
     * @return    是否可更改
     */
    bool isModifiable();

    /**
     * @brief clear 清空数据
     */
    void clear();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    /**
     * @brief mouseDoubleClickEvent 鼠标双击事件
     * @param event 双击事件
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

    /**
     * @brief calFirSize    计算文件夹中子文件项数
     * @param strFilePath
     * @return
     */
    qlonglong calDirItemCount(const QString &strFilePath);

    /**
     * @brief handleDoubleClick    处理双击事件
     * @param index 双击的index
     */
    void handleDoubleClick(const QModelIndex &index) override;

    /**
     * @brief refreshDataByCurrentPath  根据当前路径刷新数据
     */
    void refreshDataByCurrentPath();

    /**
     * @brief getCurrentDirFiles   获取当前路径下所有文件
     * @return
     */
    /**
     * @brief getFilesByParentPath  获取当前路径下文件数据
     * @return
     */
    QList<FileEntry> getCurPathFiles();

    /**
     * @brief getAllFilesByParentPath   获取某路径下所有的文件
     * @param strFullPath               路径名称
     * @param listEntry                 子文件数据
     * @param qSize                     总大小
     */
    //void getAllFilesByParentPath(const QString &strFullPath, QList<FileEntry> &listEntry, qint64 &qSize);

    /**
     * @brief getSelEntry       获取当前选择的文件数据
     */
    QList<FileEntry> getSelEntry();

    /**
     * @brief extract2Path      提取选中的文件至指定路径
     * @param strPath           指定的路径
     */
    void extract2Path(const QString &strPath);

    /**
     * @brief calEntrySizeByParentPath  根据父节点计算所有子文件大小（所有层次）
     * @param strFullPath   父节点路径
     * @param qSize        大小
     */
    void calEntrySizeByParentPath(const QString &strFullPath, qint64 &qSize);

    /**
     * @brief clearDragData 情况拖拽提取数据
     */
    void clearDragData();

Q_SIGNALS:
    /**
     * @brief signalExtract2Path    提取压缩包中文件
     * @param listSelEntry          当前选中的文件
     * @param stOptions             提取参数
     */
    void signalExtract2Path(const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief signalDeleteFiles     从删除包删除文件
     * @param listEntry              待删除的数据
     */
    //void signalDeleteFiles(const QList<FileEntry> &listEntry);

    /**
     * @brief signalDelFiels    删除压缩包中文件
     * @param listSelEntry      当前选中的文件
     * @param qTotalSize        删除文件总大小
     */
    void signalDelFiles(const QList<FileEntry> &listCurEntry, qint64 qTotalSize);
    /**
     * @brief signalDelFiels    重命名压缩包中文件
     * @param listSelEntry      当前重命名选中的文件
     * @param qTotalSize        重命名文件总大小
     */
    void signalRenameFile(const FileEntry &SelEntry, qint64 qTotalSize);

    /**
     * @brief signalOpenFile    打开压缩包中文件
     * @param entry             待打开的文件
     * @param strProgram        应用程序名（为空时，用默认应用程序打开）
     */
    void signalOpenFile(const FileEntry &entry, const QString &strProgram = "");

    /**
     * @brief signalAddFiles2Archive    向压缩包中添加文件
     * @param listFiles                 待添加的文件
     * @param strPassword               密码
     */
    void signalAddFiles2Archive(const QStringList &listFiles, const QString &strPassword);
    /**重命名信号
      */
    void sigRenameFile();

protected Q_SLOTS:
    /**
     * @brief slotPreClicked    返回上一级
     */
    virtual void slotPreClicked() override;

private slots:
    /**
     * @brief slotDragFiles     外部文件拖拽至列表处理（追加压缩）
     * @param listFiles         外部拖拽文件
     */
    void slotDragFiles(const QStringList &listFiles);

    /**
     * @brief slotShowRightMenu     显示右键菜单
     * @param pos   右键位置
     */
    void slotShowRightMenu(const QPoint &pos) override;

    /**
     * @brief slotExtract   右键提取操作
     */
    void slotExtract();

    /**
     * @brief slotExtract2Hera   右键提取到当前文件夹操作
     */
    void slotExtract2Here();

    /**
     * @brief slotDeleteFile    右键删除操作
     */
    void slotDeleteFile() override;

    /**
     * @brief slotRenameFile    重命名文件
     */
    void slotRenameFile() override;

    /**
     * @brief slotDeleteFile    右键打开操作
     */
    void slotOpen();

    /**
     * @brief slotOpenStyleClicked  右键打开方式操作
     */
    void slotOpenStyleClicked();

private:
    QMap<QString, QList<FileEntry>> m_mapShowEntry; // 显示数据（缓存，目录层级切换时从此处取数据，避免再次对总数据进行操作）
    FileEntry m_stRightEntry;       // 右键点击的文件
    QString m_strUnCompressPath;    // 默认解压路径
    QPoint m_dragPos; // 鼠标拖拽点击位置
    QString m_strArchive;   // 压缩包全路径
    QString m_strArchivePath;       // 压缩包所在路径
    bool m_bModifiable = false;     // 压缩包数据是否可更改（增、删、改）
    bool m_bMultiplePassword = false;     // 追加是否可以多密码

    ChangeType m_eChangeType = CT_None;
    QStringList m_listAddFiles;     // 追加的文件

    // 拖拽提取
    DFileDragServer *m_pFileDragServer = nullptr; // 文件拖拽服务
    DFileDrag *m_pDrag = nullptr;
    bool m_bDrop = false;
    bool m_bReceive = false;
    QString m_strSelUnCompressPath;    // 选择的解压路径
};

#endif // UNCOMPRESSVIEW_H
