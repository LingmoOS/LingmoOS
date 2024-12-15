/*
 * Copyright (c) 2007 Henrique Pinto <henrique.pinto@kdemail.net>
 * Copyright (c) 2008-2009 Harald Hvaal <haraldhv@stud.ntnu.no>
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

#ifndef READWRITELIBARCHIVEPLUGIN_H
#define READWRITELIBARCHIVEPLUGIN_H

#include "libarchiveplugin.h"
#include "kpluginfactory.h"

#include <QSaveFile>
#include <QSet>

#include <archive.h>

class ReadWriteLibarchivePluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libarchive.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit ReadWriteLibarchivePluginFactory();
    ~ReadWriteLibarchivePluginFactory();
};

class ReadWriteLibarchivePlugin : public LibarchivePlugin
{
public:
    explicit ReadWriteLibarchivePlugin(QObject *parent, const QVariantList &args);
    ~ReadWriteLibarchivePlugin() override;

    // ReadOnlyArchiveInterface interface
public:
//    bool testArchive() override;
//    bool extractFiles(const QList<FileEntry> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;

    // ReadWriteArchiveInterface interface
public:
    /**
     * @brief addFiles 压缩追加操作
     * 压缩：
     *  1.生成新压缩包
     * 追加:
     *  1.生成新压缩包
     *  2.保留原压缩包中未覆盖的文件，并将其复制到新压缩包中
     * @param files
     * @param options
     * @return
     */
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
//    bool moveFiles(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &options) override;
//    bool copyFiles(const QList<FileEntry> &files, const QString &strDestination, const CompressOptions &options) override;
    /**
     * @brief deleteFiles 删除压缩包内指定文件
     * 删除流程:
     *  1.初始化
     *  2.筛选是否是需要删除的文件
     *  3.是:跳过; 否: 复制保留文件数据到新的压缩包中
     *  4.更新存放压缩包信息的map
     *  5.替换原压缩包
     * @param files 选中的文件
     * @return
     */
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    /**
     * @brief renameFiles 重命名压缩包内指定文件
     * 重命名流程:
     *  1.初始化
     *  2.筛选是否是需要重命名的文件
     *  3.是:跳过; 否: 复制保留文件数据到新的压缩包中
     *  4.更新存放压缩包信息的map
     *  5.替换原压缩包
     * @param files 选中的文件
     * @return
     */
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
//    bool addComment(const QString &comment) override;

private:
    QSaveFile m_tempFile;
    QSet<QString> m_writtenFilesSet; //已经压缩完的文件(使用QSet查找性能更佳)
    ArchiveWrite m_archiveWriter;
    qlonglong m_currentAddFilesSize = 0;//当前已经压缩的文件大小（能展示出来的都已经压缩）

    bool initializeWriter(const bool creatingNewFile = false, const CompressOptions &options = CompressOptions());
    /**
     * @brief initializeWriterFilters 设置过滤器(追加)
     * @return
     */
    bool initializeWriterFilters();
    /**
     * @brief initializeNewFileWriterFilters 设置过滤器(压缩)
     * @param options
     * @return
     */
    bool initializeNewFileWriterFilters(const CompressOptions &options);
    void finish(const bool isSuccessful);
    /**
     * @brief writeFileTodestination 将文件夹内文件(夹)写入压缩包
     * @param sourceFileFullPath
     * @param destination
     * @param externalPath 文件夹路径
     * @param totalsize 原文件总大小
     * @param strAlias 文件别名
     * @return
     */
    bool writeFileTodestination(const QString &sourceFileFullPath, const QString &destination, const QString &externalPath, const qlonglong &totalsize, const QString &strAlias = "");
    /**
     * @brief writeFileFromEntry 将文件写入压缩包
     * @param relativeName 本地文件(夹)(全路径)
     * @param destination 压缩包内路径
     * @param pEntry
     * @param totalsize 原文件总大小
     * @return
     */
    bool writeFileFromEntry(const QString &relativeName, const QString destination, FileEntry &pEntry, const qlonglong &totalsize);
    /**
     * @brief copyData 本地数据写入压缩包
     * @param filename 本地文件
     * @param dest 目标压缩包
     * @param totalsize 原文件总大小
     * @param bInternalDuty
     */
    bool copyData(const QString &filename, struct archive *dest, const qlonglong &totalsize, bool bInternalDuty = true);
    /**
     * @brief copyDataFromSourceAdd 追加操作复制原压缩包中保留的数据
     * @param source
     * @param dest
     * @param totalsize 压缩包原文件总大小-被追加文件覆盖的文件大小
     */
    void copyDataFromSourceAdd(struct archive *source, struct archive *dest, const qlonglong &totalsize);
    /**
     * @brief deleteEntry 具体删除操作
     * 筛选是否是需要删除的文件，复制保留文件数据到新的压缩包中
     * @param files 选中的文件
     * @return
     */
    bool deleteEntry(const QList<FileEntry> &files);
    /**
     * @brief renameEntry 具体重命名操作
     * 筛选是否是需要重命名的文件，复制保留文件数据到新的压缩包中
     * @param files 选中的文件
     * @return
     */
    bool renameEntry(const QList<FileEntry> &files);
    /**
     * @brief writeEntryDelete 删除操作，复制压缩包保留文件数据
     * @param entry
     * @param totalSize
     * @return
     */
    bool writeEntryDelete(struct archive_entry *entry, const qlonglong &totalSize);
    /**
    * @brief writeEntryAdd 追加操作数据处理
    * @param entry
    * @param totalSize 压缩包原文件总大小-被追加文件覆盖的文件大小
    * @return
    */
    bool writeEntryAdd(struct archive_entry *entry, const qlonglong &totalSize);
    /**
     * @brief processOldEntries_Add 处理需要保留的entry
     * @param totalSize 压缩包原文件总大小
     * @return
     */
    bool processOldEntries_Add(qlonglong &totalSize);
};



#endif // READWRITELIBARCHIVEPLUGIN_H
