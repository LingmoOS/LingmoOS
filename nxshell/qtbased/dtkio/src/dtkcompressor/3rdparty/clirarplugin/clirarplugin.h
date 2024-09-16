/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2009 Harald Hvaal <haraldhv@stud.ntnu.no>
 * Copyright (C) 2009-2010 Raphael Kubo da Costa <rakuco@FreeBSD.org>
 * Copyright (C) 2015-2016 Ragnar Thomsen <rthomsen6@gmail.com>
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

#ifndef CLIRARPLUGIN_H
#define CLIRARPLUGIN_H

#include "cliinterface.h"
#include "kpluginfactory.h"

class CliRarPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clirar.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit CliRarPluginFactory();
    ~CliRarPluginFactory();
};

class CliRarPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit CliRarPlugin(QObject *parent, const QVariantList &args);
    ~CliRarPlugin() override;

    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    bool isMultiPasswordPrompt(const QString &line) override;
    bool isOpenFileFailed(const QString &line) override;

private:
    void setupCliProperties();

    /**
     * @brief readListLine  解析加载压缩包的命令输出
     * @param line 待解析内容
     * @return
     */
    bool readListLine(const QString &line);

    bool handleLine(const QString &line, WorkType workStatus) override;

    void killProcess(bool emitFinished) override;

private:
    ParseState m_parseState = ParseStateTitle;
    FileEntry m_fileEntry;
    QString m_comment = "";  // 压缩包注释信息
    QString m_replaceLine = ""; // 拼接被截断的命令行内容
};

#endif // CLIRARPLUGIN_H
