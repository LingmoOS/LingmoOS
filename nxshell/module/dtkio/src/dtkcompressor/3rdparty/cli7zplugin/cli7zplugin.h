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

#ifndef CLI7ZPLUGIN_H
#define CLI7ZPLUGIN_H

#include "cliinterface.h"
#include "kpluginfactory.h"

class Cli7zPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_cli7z.json")
    Q_INTERFACES(KPluginFactory)

public:
    explicit Cli7zPluginFactory();
    ~Cli7zPluginFactory();
};

class Cli7zPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit Cli7zPlugin(QObject *parent, const QVariantList &args);
    ~Cli7zPlugin() override;

    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    bool isMultiPasswordPrompt(const QString &line) override;
    bool isOpenFileFailed(const QString &line) override;

private:
    enum ArchiveType {
        ArchiveType7z = 0,
        ArchiveTypeBZip2,
        ArchiveTypeGZip,
        ArchiveTypeXz,
        ArchiveTypeTar,
        ArchiveTypeZip,
        ArchiveTypeRar,
        ArchiveTypeUdf,
        ArchiveTypeIso
    } m_archiveType;

    void setupCliProperties();

    /**
     * @brief readListLine  解析list的命令行输出
     * @param line
     * @return
     */
    bool readListLine(const QString &line);

    bool handleLine(const QString &line, WorkType workStatus) override;

    bool isNoFilesArchive(const QString &line);  // 压缩包内没有数据

    void killProcess(bool emitFinished) override;

private:
    ParseState m_parseState = ParseStateTitle;
    FileEntry m_fileEntry;
    bool m_bWaitingPassword = false; // 是否阻塞在密码输入处
};

#endif // CLI7ZPLUGIN_H
