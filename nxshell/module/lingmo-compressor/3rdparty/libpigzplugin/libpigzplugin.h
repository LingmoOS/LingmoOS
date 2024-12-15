/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2010 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (c) 2016 Vladyslav Batyrenko <mvlabat@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef libPigzPlugin_H
#define libPigzPlugin_H

#include "cliinterface.h"
#include "kpluginfactory.h"

class LibPigzPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libpigz.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit LibPigzPluginFactory();
    ~LibPigzPluginFactory();
};

class LibPigzPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibPigzPlugin(QObject *parent, const QVariantList &args);
    ~LibPigzPlugin() override;

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
private:
    bool handleLine(const QString &line);

    void killProcess(bool emitFinished = true);

    void deleteProcess();

    void getChildProcessId(qint64 processId, const QStringList &listKey, QVector<qint64> &childprocessid);

private Q_SLOTS:
    void readStdout(bool handleAll = false);

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    KPtyProcess *m_process = nullptr;  // 工作进程
    bool m_isProcessKilled = false;  // 进程已经结束
    QByteArray m_stdOutData;  // 存储命令行输出数据
    qint64  m_processId;  // 进程Id
    QVector<qint64> m_childProcessId; // 压缩tar.gz文件的子进程Id
    qlonglong m_qTotalSize; // 源文件总大小
};

#endif // libPigzPlugin_H
