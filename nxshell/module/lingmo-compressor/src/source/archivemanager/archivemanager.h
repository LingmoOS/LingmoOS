// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include "commonstruct.h"
#include "archivejob.h"
#include "archiveinterface.h"
#include "plugin.h"
#include "queries.h"
#include "uitools.h"

#include <QObject>
#include <QMimeType>
#include <QFileInfo>

class ArchiveManager : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief get_instance  获取单例实例对象
     * @return
     */
    static ArchiveManager *get_instance(void);

    /**
     * @brief destory_instance 销毁单例实例对象
     */
    void destory_instance();

    /**
     * @brief createArchive     创建压缩包
     * @param files             待压缩文件
     * @param strDestination    文件存储在压缩包的路径（为空时，处于根目录）
     * @param stOptions           压缩参数
     * @param eType            插件选择
     * @return                  是否调用成功
     */
    bool createArchive(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &stOptions, UiTools::AssignPluginType eType);

    /**
     * @brief loadArchive           加载压缩包数据
     * @param strArchiveFullPath    压缩包全路径
     * @param eType                 插件选择
     * @return                      是否调用成功
     */
    bool loadArchive(const QString &strArchiveFullPath, UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto);

    /**
     * @brief addFiles 向压缩包中添加文件
     * @param strArchiveFullPath 压缩包全路径
     * @param listAddEntry 添加的文件(夹)
     * @param stOptions 压缩参数
     * @return 是否调用成功
     */
    bool addFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listAddEntry, const CompressOptions &stOptions);

    /**
     * @brief extractFiles    解压文件
     * @param strArchiveFullPath    压缩包全路径
     * @param files             待解压的文件（若数目为空，属于全部解压，否则为提取）
     * @param options           解压参数
     * @param eType             插件选择
     * @return                      是否调用成功
     */
    bool extractFiles(const QString &strArchiveFullPath, const QList<FileEntry> &files, const ExtractionOptions &stOptions, UiTools::AssignPluginType eType = UiTools::AssignPluginType::APT_Auto);

    /**
     * @brief extractFiles2Path     提取文件至指定目录
     * @param strArchiveFullPath        压缩包全路径
     * @param listSelEntry          选中的提取文件
     * @param stOptions             提取参数
     * @return                      是否调用成功
     */
    bool extractFiles2Path(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry, const ExtractionOptions &stOptions);

    /**
     * @brief deleteFiles       删除压缩包中的文件
     * @param strArchiveFullPath    压缩包全路径
     * @param listSelEntry      当前选中的文件
     * @return                      是否调用成功
     */
    bool deleteFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry);

    /**
     * @brief renameFiles       重命名压缩包中的文件
     * @param strArchiveFullPath    压缩包全路径
     * @param listSelEntry      当前选中的文件
     * @return                      是否调用成功
     */
    bool renameFiles(const QString &strArchiveFullPath, const QList<FileEntry> &listSelEntry);

    /**
     * @brief batchExtractFiles 批量解压
     * @param listFiles          压缩文件
     * @param strTargetPath     解压路径
     * @param bAutoCreatDir     是否自动创建文件夹
     * @return                      是否调用成功
     */
    bool batchExtractFiles(const QStringList &listFiles, const QString &strTargetPath/*, bool bAutoCreatDir*/);

    /**
     * @brief openFile          打开压缩包中文件
     * @param strArchiveFullPath    压缩包全路径
     * @param stEntry           待打开文件数据
     * @param strTargetPath     临时解压路径
     * @param strProgram        应用程序名
     * @return                      是否调用成功
     */
    bool openFile(const QString &strArchiveFullPath, const FileEntry &stEntry, const QString &strTempExtractPath, const QString &strProgram);

    /**
     * @brief updateArchiveCacheData    更新本地压缩包缓存数据
     * @param stOptions     更新选项
     * @return                      是否调用成功
     */
    bool updateArchiveCacheData(const UpdateOptions &stOptions);

    /**
     * @brief updateArchiveComment  更新压缩包注释信息
     * @param strArchiveFullPath    压缩包全路径
     * @param strComment            注释
     * @return
     */
    bool updateArchiveComment(const QString &strArchiveFullPath, const QString &strComment);

    /**
     * @brief convertArchive                压缩包格式转换
     * @param strOriginalArchiveFullPath    原始压缩包全路径
     * @param strTargetFullPath             压缩包解压的临时路径
     * @param strNewArchiveFullPath         转换之后的压缩包全路径
     * @return
     */
    bool convertArchive(const QString &strOriginalArchiveFullPath, const QString &strTargetFullPath, const QString &strNewArchiveFullPath);

    /**
     * @brief pauseOperation    暂停操作
     * @return                      是否调用成功
     */
    bool pauseOperation();

    /**
     * @brief continueOperation 继续操作
     * @return                      是否调用成功
     */
    bool continueOperation();

    /**
     * @brief cancelOperation   取消操作
     * @return                      是否调用成功
     */
    bool cancelOperation();

    /**
     * @brief getCurFilePassword    获取当前文件密码
     * @return      当前文件密码
     */
    QString getCurFilePassword();

    /**
     * @brief currentStatus  当前任务状态
     * @return 当前任务状态
     */
    bool currentStatus();

Q_SIGNALS:
    /**
     * @brief signalError       错误信号
     * @param eErrorType        错误类型
     */
    void signalError(ErrorType eErrorType);

    /**
     * @brief signalJobFinished     操作结束信号
     * @param eJobType            job类型
     * @param eFinishType           结束类型
     * @param eErrorType            错误类型
     */
    void signalJobFinished(ArchiveJob::JobType eJobType, PluginFinishType eFinishType, ErrorType eErrorType);

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName 发送当前正在操作的文件名
     * @param strName   当前文件名
     */
    void signalCurFileName(const QString &strName);

    /**
     * @brief signalFileWriteErrorName 发送创建失败的文件名
     * @param strName   当前文件名
     */
    void signalFileWriteErrorName(const QString &strName);

    /**
     * @brief signalQuery   发送询问信号
     * @param query 询问类型
     */
    void signalQuery(Query *query);

    /**
     * @brief signalCurArchiveName  当前正在操作的压缩包信号
     * @param strArchiveName        压缩包名称
     */
    void signalCurArchiveName(const QString &strArchiveName);

private:
    explicit ArchiveManager(QObject *parent = nullptr);
    ~ArchiveManager() override;

private Q_SLOTS:
    /**
     * @brief slotJobFinished
     */
    void slotJobFinished();

private:
    ArchiveJob *m_pArchiveJob = nullptr;     // 当前操作指针

    ReadOnlyArchiveInterface *m_pInterface = nullptr;   // 当前插件指针（只存储load操作的interface，方便解压等操作）
    ReadOnlyArchiveInterface *m_pTempInterface = nullptr;   // 存储job结束即需要删除的interface

    static QMutex m_mutex;//实例互斥锁。
    static QAtomicPointer<ArchiveManager> m_instance;   /*!<使用原子指针,默认初始化为0。*/

    bool m_bCancel = false;     // 是否取消
};

#endif // ARCHIVEMANAGER_H
