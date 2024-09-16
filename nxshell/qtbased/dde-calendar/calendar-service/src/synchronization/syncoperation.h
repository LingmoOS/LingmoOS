// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SYNCOPERATION_H
#define SYNCOPERATION_H

#include <QtDBus>
#include <QtDBus/qtdbusglobal.h>
#include <QtDBus/qdbusabstractinterface.h>
#include <QtDBus/qdbusconnection.h>
#include "dunioniddbus.h"

//云同步DBUS接口信息
#define SYNC_DBUS_PATH              "com.deepin.sync.Daemon"
#define SYNC_DBUS_INTERFACE         "/com/deepin/utcloud/Daemon"


//云同步错误码
#define SYNC_No_Error                   0               /*执行正常*/
#define SYNC_Internal_Error             7500            /*内部错误*/
#define SYNC_Parameter_Error            7501            /*参数错误*/
#define SYNC_Login_Expired              7502            /*未登录或登录过期*/
#define SYNC_No_Access                  7503            /*禁止访问，不在白名单内*/
#define SYNC_Data_Not_Exist             7504            /*数据不存在*/
#define SYNC_File_Operation_Failed      7505            /*文件操作失败*/
#define SYNC_Network_Request_Error      7506            /*网络请求出错错误*/
#define SYNC_Oss_Operation_Error        7507            /*oss操作出错*/
#define SYNC_Space_Not_Available        7508            /*空间不可用*/
#define SYNC_File_Or_Path_Error         7509            /*文件或者路径出错*/
#define SYNC_Invalid_File_Size          7510            /*文件大小不合法*/
#define SYNC_Metadata_Check_Error       7511            /*元数据校验出错*/

const QString utcloudcalendatpath = CALENDAR_SERVICE_PATH"/dde-calendar-service";

using SyncInter = com::deepin::sync::cloudopt;

struct SyncoptResult {
    QString data = "";  //执行结果数据
    bool ret = false;   //执行云同步操作的结果
    int error_code = 0; //错误码
    bool switch_state = false; //开关状态
};

class Syncoperation : public QObject
{
    Q_OBJECT
public:
    explicit Syncoperation(QObject *parent = nullptr);
    ~Syncoperation();

    void optlogin();

    void optlogout();

    SyncoptResult optUpload(const QString &key);

    SyncoptResult optDownload(const QString &key, const QString &path);

    SyncoptResult optDelete(const QString &key);

    SyncoptResult optMetadata(const QString &key);

    bool optUserData(QVariantMap &userInfoMap);

    /**
     * @brief hasAvailable      判断云同步是否提供Available方法
     * @return
     */
    bool hasAvailable();

    //获取总开关状态
    SyncoptResult optGetMainSwitcher();
    //获取日历开关状态
    SyncoptResult optGetCalendarSwitcher();

Q_SIGNALS:
    void signalLoginStatusChange(const int staus);
    void LoginStatus(const int data);
    void SwitcherChange(const bool state);

private Q_SLOTS:
    void slotDbusCall(const QDBusMessage &msg);
    void onPropertiesChanged(const QString &interfaceName,
                             const QVariantMap &changedProperties,
                             const QStringList &invalidatedProperties);
private:
    SyncInter *m_syncInter;
};

#endif // SYNCUPLOAD_H
