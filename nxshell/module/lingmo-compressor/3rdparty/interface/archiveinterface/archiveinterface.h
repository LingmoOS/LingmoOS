/*
 * Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (c) 2009-2012 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (c) 2016 Vladyslav Batyrenko <mvlabat@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ARCHIVEINTERFACE_H
#define ARCHIVEINTERFACE_H

#include "commonstruct.h"
#include "kpluginmetadata.h"
#include "common.h"

#include <QObject>
#include <QString>
#include <QMimeType>
#include <QFileDevice>
#include <QSet>

class Query;

// 只读（查看和解压等）
class ReadOnlyArchiveInterface : public QObject
{
    Q_OBJECT

public:
    explicit ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadOnlyArchiveInterface() override;

    // 插件类型
    enum Plugintype {
        PT_UnKnown,
        PT_Cliinterface,
        PT_LibArchive,
        PT_Libzip,
        PT_Libminizip,
        PT_Libpigz
    };

//    // 工作类型
//    enum WorkType {
//        WT_List,
//        WT_Extract,
//        WT_Add,
//        WT_Delete,
//        WT_Move,
//        WT_Copy,
//        WT_Comment,
//        WT_Test
//    };

    /**
     * @brief list      加载压缩包
     * @return
     */
    virtual PluginFinishType list() = 0;

    /**
     * @brief testArchive   测试压缩包
     * @return
     */
    virtual PluginFinishType testArchive() = 0;

    /**
     * @brief extractFiles          解压
     * @param files                 待解压的文件，若数目为0,则是全部解压，否则为部分提取
     * @param options               解压参数
     * @return                      是否解压成功
     */
    virtual PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) = 0;

    /**
     * @brief waitForFinished   判断是否通过线程调用
     * @return true：线程调用
     */
    bool waitForFinished();

    /**
     * @brief setPassword   设置解压密码
     * @param strPassword   解压密码
     */
    void setPassword(const QString &strPassword);

    /**
     * @brief getPassword   获取当前文件密码
     * @return      当前文件密码
     */
    QString getPassword();

    /**
     * @brief errorType     获取错误类型
     * @return
     */
    ErrorType errorType();

    /**
     * @brief pauseOperation    暂停操作
     */
    virtual void pauseOperation() = 0;

    /**
     * @brief continueOperation 继续操作
     */
    virtual void continueOperation() = 0;

    /**
     * @brief doKill    强杀操作
     * @return
     */
    virtual bool doKill();

    /**
     * @brief status 状态
     * @return 状态
     */
    bool status();

    bool isUserCancel() {return m_bCancel;}

protected:
    /**
     * Setting this option to true will NOT run the functions in their own thread.
     * Instead it will be necessary to call finished(bool) when the operation is actually finished.
     */
    void setWaitForFinishedSignal(bool value);

    /**
     * @brief getPermissions    权限转换
     * @param perm              文件权限
     * @return
     */
    QFileDevice::Permissions getPermissions(const mode_t &perm);

    /**
     * @brief handleEntry       处理文件数据（主要针对加载时未列出文件夹的情况）
     * @param entry             文件数据
     */
    void handleEntry(const FileEntry &entry);

    /**
     * @brief isInsufficientDiskSpace 判断磁盘空间是否不足
     * @param diskPath 压缩或解压目的路径
     * @param 暂取小于10M作为磁盘空间不足的判断标准
     * @return true:磁盘空间不足 false:磁盘空间充足
     */
    bool isInsufficientDiskSpace(QString diskPath, qint64 standard = 10 * 1024 * 1024);

Q_SIGNALS:
    /**
     * @brief signalFinished    结束信号
     * @param eType   结束类型
     */
    void signalFinished(PluginFinishType eType);
    void error(const QString &message = "", const QString &details = "");

    /**
     * @brief signalprogress    进度信号
     * @param iPercent  进度值
     */
    void signalprogress(double dPercentage);

    /**
     * @brief signalCurFileName     发送当前正在操作的文件名
     * @param filename      文件名
     */
    void signalCurFileName(const QString &filename);

    /**
     * @brief signalFileWriteErrorName 发送创建失败的文件名
     * @param filename      文件名
     */
    void signalFileWriteErrorName(const QString &filename);

    /**
     * @brief signalQuery   发送询问信号
     * @param query 询问类型
     */
    void signalQuery(Query *query);

    /**
     * @brief signalCancel  取消信号
     */
    void signalCancel();

public:
    Plugintype m_ePlugintype = PT_UnKnown;

//    bool getHandleCurEntry() const;

protected:
    bool m_bWaitForFinished = false;    // 等待结束 true:CLI false:调接口
    uint m_numberOfEntries; //原有的归档数量
    KPluginMetaData m_metaData;
    QString m_strArchiveName; //1、压缩：最终的压缩包名 2、解压：加载的压缩包名
    CustomMimeType m_mimetype;
    Common *m_common = nullptr; // 通用工具类
    bool m_bOverwriteAll = false;        //是否全部覆盖
    bool m_bSkipAll = false;             // 是否全部跳过
//    bool m_bHandleCurEntry = false; //false:提取使用选中文件及子文件 true:提取使用选中文件
    WorkType m_workStatus = WT_List;  // 记录当前工作状态（add、list、extract...）

    ErrorType m_eErrorType = ET_NoError;    // 错误类型
    QString m_strPassword;          // 密码

    bool m_bCancel = false;     // 是否取消
    bool m_bPause = false;      // 是否暂停

    QSet<QString> m_setHasRootDirs;  // 存储list时已经处理过的文件夹名称
    QSet<QString> m_setHasHandlesDirs;  // 存储list时已经处理过的文件夹名称
};

// 可读可写（可用来压缩、查看、解压等）
class ReadWriteArchiveInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit ReadWriteArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadWriteArchiveInterface() override;

    /**
     * @brief addFiles          压缩文件
     * @param files             待压缩文件
     * @param options           压缩参数
     * @return
     */
    virtual PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          移动压缩文件
     * @param files             待移动文件
     * @param options           压缩参数
     * @return
     */
    virtual PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          拷贝压缩文件
     * @param files             待拷贝文件
     * @param options           压缩参数
     * @return
     */
    virtual PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) = 0;

    /**
     * @brief addFiles          删除压缩文件
     * @param files             待删除文件
     * @return
     */
    virtual PluginFinishType deleteFiles(const QList<FileEntry> &files) = 0;

    /**
     * @brief renameFiles          重命名压缩文件
     * @param files             待重命名文件
     * @return
     */
    virtual PluginFinishType renameFiles(const QList<FileEntry> &files) = 0;
    /**
     * @brief addComment        添加注释
     * @param comment           注释内容
     * @return
     */
    virtual PluginFinishType addComment(const QString &comment) = 0;

    /**
     * @brief getArchiveName 获取最终的压缩包名，提供给压缩取消删除使用
     * @return
     */
    QString getArchiveName();

    /**
     * @brief updateArchiveData   更新压缩包数据
     * @return
     */
    virtual PluginFinishType updateArchiveData(const UpdateOptions &options) = 0;
};

#endif // ARCHIVEINTERFACE_H
