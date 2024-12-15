// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIBMINIZIPPLUGIN_H
#define LIBMINIZIPPLUGIN_H

#include "kpluginfactory.h"
#include "archiveinterface.h"

#include <QObject>

#include <minizip/unzip.h>

class Common;

class LibminizipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libminizip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibminizipPluginFactory();
    ~LibminizipPluginFactory() override;
};

class LibminizipPlugin : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    explicit LibminizipPlugin(QObject *parent, const QVariantList &args);
    ~LibminizipPlugin() override;


    // ReadOnlyArchiveInterface interface
public:
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;
    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

private:
    /**
     * @brief handleArchiveData 处理压缩包数据
     * @param archive   压缩包
     * @param index 索引
     * * @return
     */
    bool handleArchiveData(unzFile zipfile);

    /**
     * @brief extractEntry  解压单文件
     * @param zipfile       压缩包数据
     * @param file_info     文件信息
     * @param options       解压参数
     * @param qExtractSize  已解压的大小
     * @param strFileName   当前解压的文件名
     * @return              错误类型
     */
    ErrorType extractEntry(unzFile zipfile, unz_file_info file_info, const ExtractionOptions &options, qlonglong &qExtractSize, QString &strFileName);

    /**
     * @brief getSelFiles    根据选择的文件获取所有需要操作的
     * @param setFiles             选择的文件
     */
    QStringList getSelFiles(const QList<FileEntry> &files);


Q_SIGNALS:
    //void error(const QString &message = "", const QString &details = "");

private:


};

#endif // LibminizipPLUGIN_H
