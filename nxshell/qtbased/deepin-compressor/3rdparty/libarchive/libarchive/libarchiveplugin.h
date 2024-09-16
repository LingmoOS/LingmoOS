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

#ifndef LIBARCHIVEPLUGIN_H
#define LIBARCHIVEPLUGIN_H

#include "archiveinterface.h"
#include "commonstruct.h"

#include <archive.h>
#include <QSet>

/**
 * @brief The HandleWorkingDir class
 * change 用于更改应用所在当前路径及恢复
 * ~HandleWorkingDir 析构时自动恢复应用所在当前路径
 */
class HandleWorkingDir
{
public:
    explicit HandleWorkingDir(QString *oldWorkingDir);
    void change(const QString &newWorkingDir);
    ~HandleWorkingDir();
private:
    QString *m_oldWorkingDir;
};

struct FileProgressInfo {
    float fileProgressProportion = 0.0; //内部百分值范围
    float fileProgressStart;            //上次的百分值
    float totalFileSize;
};

class LibarchivePlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibarchivePlugin(QObject *parent, const QVariantList &args);
    ~LibarchivePlugin() override;

    // ReadOnlyArchiveInterface interface
public:
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType renameFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    /**
     * @brief updateArchiveData   更新压缩包数据
     * @return
     */
    PluginFinishType updateArchiveData(const UpdateOptions &options) override;

    /**
     * @brief pauseOperation    暂停操作
     */
    void pauseOperation() override;

    /**
     * @brief continueOperation 继续操作
     */
    void continueOperation() override;

    /**
     * @brief doKill 强行取消
     */
    bool doKill() override;

protected:
    /**
     * @brief initializeReader 读取压缩包数据之前一系列操作
     * @return
     */
    bool initializeReader();

    struct ArchiveReadCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_read_free(a);
            }
        }
    };
    struct ArchiveWriteCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_write_free(a);
            }
        }
    };


    typedef QScopedPointer<struct archive, ArchiveReadCustomDeleter> ArchiveRead;
    typedef QScopedPointer<struct archive, ArchiveWriteCustomDeleter> ArchiveWrite;

    ArchiveRead m_archiveReader;
    ArchiveRead m_archiveReadDisk;

    /**
     * @brief copyDataFromSource 压缩包数据写到本地文件
     * @param source 读句柄
     * @param dest 写句柄
     */
    void copyDataFromSource(struct archive *source, struct archive *dest, const qlonglong &totalSize);

private:
    PluginFinishType list_New();
    QString convertCompressionName(const QString &method);
    /**
     * @brief emitEntryForIndex 构建压缩包内数据
     * @param aentry
     */
    void emitEntryForIndex(archive_entry *aentry);
    /**
     * @brief deleteTempTarPkg 删除list时解压出来的临时tar包
     * @param tars
     */
    void deleteTempTarPkg(const QStringList &tars);
    /**
     * @brief extractionFlags 选择要还原的属性
     * @return
     */
    int extractionFlags() const;

private:
    int m_ArchiveEntryCount = 0; //压缩包内文件(夹)总数量
    QString m_strOldArchiveName; //原压缩包名(全路径)
    QStringList m_tars; //list时解压出来的临时tar包
    QString m_oldWorkingDir;
    QString m_extractDestDir; //解压目的路径
//    QString destDirName; //取消解压，需要该变量

protected:
    QMap<QString, QByteArray> m_mapCode;   // 存储文件名-编码（解压无需再次探测，提高解压速率）
    QMap<QString, int> m_mapLongName;       // 存储截取的文件名称和截取的次数（不包含001之类的）
    QMap<QString, int> m_mapLongDirName;    // 长文件夹统计
    QMap<QString, int> m_mapRealDirValue;    // 长文件真实文件统计
    QSet<QString> m_setLongName;            // 存储被截取之后的文件名称（包含001之类的）
};





#endif // LIBARCHIVEPLUGIN_H
