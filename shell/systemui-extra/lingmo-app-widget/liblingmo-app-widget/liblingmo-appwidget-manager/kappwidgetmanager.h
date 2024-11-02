/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#ifndef KAPPWIDGETMANAGER_H
#define KAPPWIDGETMANAGER_H

#include "liblingmo-appwidget-manager_global.h"

#include <QDBusInterface>

namespace AppWidget
{
/**
 * @defgroup 小插件管理服务接口模块
 */

/**
 * @brief 小插件属性枚举
 */
enum AppWidgetConfig {
    NAME = 1,                //小插件名字
    PROVIDER_NAME,           //提供端名
    PREVIEW_PATH,            //预览图
    ZOOM,                    //是否支持缩放
    RIGHT_BUTTON,            //是否支持右键
    REPEAT_PLACEMENT,        //是否可以重复放置
    MIN_WIDTH,               //最小宽度
    MIN_HEIGHT,              //最小高度
    TARGET_CELL_WIDTH,       //小插件默认宽度所占屏幕的单位格数
    TARGET_CELL_HEIGHT,      //小插件默认高度所占屏幕的单位格数
    UPDATE_PERIODMILLIS,     //是否定时更新
    DESCRIBE,                //小插件描述
};
/**
 * @brief 使用端状态属性枚举
 */
enum UserStatus {
    NORMAL = 1,          //正常状态
    EDITABLE = 2,      //编辑状态
};

class LIBLINGMOAPPWIDGETMANAGER_EXPORT KAppWidgetManager : public QDBusInterface
{
    Q_OBJECT
public:
    KAppWidgetManager(QObject *parent = nullptr);
    static KAppWidgetManager* getInstance() {
        static KAppWidgetManager s_instance;
        return &s_instance;
    }
    /**
     * @brief 获取小插件列表
     * @return app widget list
     */
    QStringList getAppWidgetList();
    /**
     * @brief 获取小插件ui文件路径
     * @param appwidgetname
     * @return path of ui file
     */
    QString getAppWidgetUiFile(const QString &appwidgetname);
    /**
     * @brief 获取小插件的config属性
     * @param appwidgetname config
     * @return config value of appwidgetname
     */
    QString getAppWidgetConfig(const QString &appwidgetname, const AppWidgetConfig& config);
    /**
     * @brief 获取小插件的所有属性
     * @param appwidgetname
     * @return config list of widget
     */
    QMap<QString, QVariant> getAppWidgetConfigs(const QString &appwidgetname);
    /**
     * @brief 注册小插件
     * @param appwidgetname username
     * @return appwidget id
     */
    int registerAppWidget(const QString &appwidgetname, const QString &username);
    /**
     * @brief 注销小插件
     * @param appwidgetid
     * @return unregister success return true, else false
     */
    bool unregisterAppWidget(const int &appwidgetid);
    /**
     * @brief 获取小插件共享数据
     * @param appwidgetid
     * @return data from provider
     */
    QMap<QString, QVariant> getAppWidgetData(const int &appwidgetid);
    /**
     * @brief 更新小插件共享数据
     * @param providername data
     */
    void updateAppWidget(const QString &providername, const QMap<QString, QVariant> &data);
    /**
     *@brief 更新使用端状态
     *@param username userstatus
     *
     */
    void updateUserStatus(const QString &username, const UserStatus& userstatus);
    void notifyProviderUpdate(int id);
    QMap<QString, QString> getAppWidgetLanguageFiles(const QString &appwidgetname);

signals:
    /**
     * @brief 小插件资源文件增加信号
     * @param appwidget name
     */
    void appWidgetAdded(QString appwidgetname);
    /**
     * @brief 小插件资源文件删除信号
     * @param appwidget name
     */
    void appWidgetDeleted (QString appwidgetname);
private:
    /**
     * @brief 转换AppWidgetConfig枚举为字符串
     * @param AppWidgetConfig config
     * @return config of string
     */
    static QString enumToSTring(const AppWidgetConfig& config);
    /**
     * @brief 转换UserStatus枚举为字符串
     * @param UserStatus config
     * @return config of string
     */
    static QString enumUserStatusToSTring(const UserStatus& userstatus);



};
}// namespace AppWidget


#endif // KAPPWIDGETMANAGER_H
