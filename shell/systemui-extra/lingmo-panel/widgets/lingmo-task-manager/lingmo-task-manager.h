/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMOTASKMANAGER_H
#define LINGMOTASKMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <memory>
#include "actions.h"
namespace TaskManager {

class LingmoUITaskManager : public QAbstractListModel
{
    Q_OBJECT
public:
    enum AdditionalRoles {
        Id = Qt::UserRole + 1, //desktop文件全路径
        Name,                  //名称
        GenericName,           //应用GenericName
        Icon,                  //图标
        WinIdList,               //winID列表
        CurrentDesktopWinIdList, //当前工作区的窗口
        WinIdsOnScreens,
        WindowTitles,
        WindowIcons,
        CurrentWinIdList,
        DemandsAttentionWinIdList,  //处于demandsAttention状态的窗口列表
        HasLauncher,    //快速启动按钮位置（从0开始），-1表示无快速启动按钮
        Actions,               //可执行的action，目前只包含desktop action
        UnreadMessagesNum,     //未读消息数量
        HasActiveWindow,
        ApplicationMenuServiceName,
        ApplicationMenuObjectPath
    };
    Q_ENUM(AdditionalRoles)
    static LingmoUITaskManager &self();
    ~LingmoUITaskManager() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;

    /**
     * 快速启动按钮 增加/删除/交换位置
     * @param desktopFile 目标desktop文件
     * @param order <0 删除快速启动按钮 >=0 增加或交换位置
     */
    void setQuickLauncher(const QString &desktopFile, int order);
    void setOrder(const QModelIndex &index, int order);
    Q_INVOKABLE static void activateWindowView(const QStringList &winIds);
    /**
     * 激活窗口
     * @param winId
     */
    Q_INVOKABLE void activateWindow(const QString &winId);
    /**
     * 窗口标题
     * @param winId
     * @return
     */
    Q_INVOKABLE QString windowTitle(const QString &winId);
    /**
     * 窗口图标
     * @param winId
     * @return
     */
    Q_INVOKABLE QIcon windowIcon(const QString &winId);
    /**
     * 窗口action
     * @param winId
     * @return
     */
    Q_INVOKABLE TaskManager::Actions windowActions(const QString &winId);
    /**
     * 窗口是否被激活
     * @param winId
     * @return bool
     */
    Q_INVOKABLE static bool windowIsActivated(const QString &winId);
    /**
    * 窗口执行指定动作
    * @param Action::Type
    * @param winId
    */
    Q_INVOKABLE static void execSpecifiedAction(const TaskManager::Action::Type &type, const QString &wid);
    void launch(const QModelIndex &index);
    bool event(QEvent* ev) override;
    void requestDataChange();

Q_SIGNALS:
    void unReadMessagesNumberUpdate(const QString &desktopFile, uint num);

public Q_SLOTS:
    bool addQuickLauncher(const QString &desktopFile);
    bool checkQuickLauncher(const QString &desktopFile);
    bool removeQuickLauncher(const QString &desktopFile);
    void closeWindow(const QVariant& wid);
    void minimizeWindow(const QVariant& wid);
    void maximizeWindow(const QVariant& wid);
    void restoreWindow(const QVariant& wid);
    void keepAbove(const QVariant& wid);
    void unsetKeepAbove(const QVariant& wid);

private:
    explicit LingmoUITaskManager(QObject *parent = nullptr);

    void addQuickLauncher(const QString &desktopFile, int order);
    int quickLauncherCountBeforeRow(int row) const;
    void onAppUninstalled(const QStringList &desktopFiles);
    /**
     * 固定到任务栏/取消固定操作上传埋点数据
     * @param applicationName 目标desktop文件
     */
    void addQuickLauncherEvent(const QString &applicationName, const QString &operate);
    class Private;
    Private *d = nullptr;
    bool m_requestDataChange {false};
};
}
#endif // LINGMOTASKMANAGER_H
