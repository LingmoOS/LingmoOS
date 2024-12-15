// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCFILEMANAGE_H
#define SYNCFILEMANAGE_H

#include "syncoperation.h"
#include "dschedule.h"
#include <QCryptographicHash>
#include "dataencrypt.h"
#include <QSqlDatabase>

const QString syncDBname = "synccalendarDB.db";
const QString syncDBpassword = "calendar123";


class SyncFileManage : public QObject
{
    Q_OBJECT
public:
    explicit SyncFileManage(QObject *parent = nullptr);
    ~SyncFileManage();

    /**
     * @brief SyncDataUpload   云端文件下载接口
     * @param   UID：用户UID信息
     * @return  filename: 下载数据库文件路径
     *          返回值： -1 数据库文件创建失败
     *                  云同步错误码
     */
    bool SyncDataDownload(const QString &uid, QString &filepath, int &errorcode);

    /**
     * @brief SyncDataUpload   同步数据上传
     * @param
     * @return
     */
    bool SyncDataUpload(const QString &filepath, int &errorcode);

    /**
     * @brief syncDataDelete    删除临时数据
     * @param filepath
     * @return
     */
    bool syncDataDelete(const QString &filepath);
    /**
     * @brief getuserInfo   获取用户信息
     * @param
     * @return
     */
    DAccount::Ptr getuserInfo();

    Syncoperation *getSyncoperation();

private:
    /**
     * @brief SyncDbCreate   同步数据库文件创建
     * @param
     * @return
     */
    bool SyncDbCreate(const QString &DBpath);
    /**
     * @brief SyncDbCreate   同步数据库文件创建
     * @param
     * @return
     */
    bool SyncDbDelete(const QString &DBpath);

    void UserSyncDirectory(const QString &dir);
private:
    Syncoperation *m_syncoperation = nullptr;
    DAccount::Ptr m_account;
};

#endif // SYNCFILEMANAGE_H
