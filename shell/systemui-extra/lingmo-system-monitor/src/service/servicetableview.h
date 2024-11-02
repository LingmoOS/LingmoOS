/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __SERVICETABLEVIEW_H__
#define __SERVICETABLEVIEW_H__

#include "../controls/ktableview.h"

#include "serviceinfo.h"
#include "servicesortfilterproxymodel.h"
#include "servicetablemodel.h"
#include "kerror.h"
#include "servicemanager.h"

#include <QLabel>
#include <QSettings>
#include <QAction>
#include <QMenu>
#include <QShortcut>

class MainWindow;
class KError;

class ServiceTableView : public KTableView
{
    Q_OBJECT

public:
    explicit ServiceTableView(QSettings* svcSettings = nullptr, QWidget *parent = nullptr);
    ~ServiceTableView() override;

    /**
     * @brief eventFilter 过滤被设置事件监听的对象的指定事件
     * @param obj 被监听的对象
     * @param event 将被过滤的事件
     * @return 过滤event事件返回true，否则返回false
     */
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief onWndClose 主窗口关闭事件
     */
    virtual void onWndClose();

    /**
     * @brief hideEvent 窗口隐藏事件
     * @param event 事件对象
     */
    virtual void hideEvent(QHideEvent *event);

public slots:
    /**
     * @brief startService 启动服务接口
     */
    void startService();

    /**
     * @brief stopService 停止服务
     */
    void stopService();

    /**
     * @brief restartService 重启服务
     */
    void restartService();

    /**
     * @brief setServiceStartupMode 设置服务启动方式
     * @param autoStart 自动启动方式
     */
    void setServiceStartupMode(bool autoStart);

    /**
     * @brief refreshList 刷新服务列表数据
     */
    void refreshList();

    /**
     * @brief onSwitchOnView 从其他页面切换显示当前页面
     */
    void onSwitchOnView();

    /**
     * @brief onSearch 搜索匹配服务
     * @param text 匹配关键字
     */
    void onSearch(const QString text);

    void showMenusByShortCut();

    void showWarningMsg(const KError&ke, const QString &sname);

signals:
    void errorHappened(const KError&, const QString &sname);

protected:
    virtual void adjustColumnsSize();

protected:
    /**
     * @brief displayServiceTableContextMenu 显示服务表内容菜单
     * @param p 显示菜单的位置
     */
    void displayServiceTableContextMenu(const QPoint &p);

    /**
     * @brief displayServiceTableHeaderContextMenu 显示服务表头菜单
     * @param p 显示菜单的位置
     */
    void displayServiceTableHeaderContextMenu(const QPoint &p);

protected:
    void resizeEvent(QResizeEvent *event) override;

    void showEvent(QShowEvent *event) override;

    /**
     * @brief selectionChanged 选中条目改变
     * @param selected 被选中的条目
     * @param deselected 取消选中的条目
     */
    void selectionChanged(const QItemSelection &selected,
                          const QItemSelection &deselected) override;

    /**
     * @brief sizeHintForColumn 指点列的默认宽度
     * @param column 列的下标
     * @return 列的默认宽度
     */
    int sizeHintForColumn(int column) const override;
    void mousePressEvent(QMouseEvent * e);

private:
    /**
     * @brief loadSettings 加载界面配置信息
     * @return 返回true表示存在历史配置，否则false
     */
    bool loadSettings();

    /**
     * @brief saveSettings 备份界面配置
     */
    void saveSettings();

    /**
     * @brief initUI 初始化UI控件
     * @param settingsLoaded 是否有历史配置
     */
    void initUI(bool settingsLoaded);

    /**
     * @brief initConnections 初始化信号槽连接
     * @param settingsLoaded 是否有历史配置
     */
    void initConnections(bool settingsLoaded);

    /**
     * @brief adjustInfoLabelVisibility 调整提示信息显示
     * @param force 是否强制设置m_notFoundLabel不显示
     */
    void adjustInfoLabelVisibility(bool force = false);

    /**
     * @brief refreshServiceInfo 刷新服务信息
     * @param sname 服务名称
     */
    void refreshServiceInfo(const QString sname);

private:
    // 服务模型
    ServiceTableModel *m_model = nullptr;
    // 服务模型排序过滤代理
    ServiceSortFilterProxyModel *m_proxyModel = nullptr;
    // 表内容的右键菜单
    QMenu *m_contextMenu = nullptr;
    // 表头的右键菜单
    QMenu *m_headerContextMenu = nullptr;
    // 没有搜索结果提示
    QLabel *m_notFoundTextLabel = nullptr;
    QLabel *m_notFoundPixLabel = nullptr;
    // 当前选中的服务名称
    QVariant m_selectedSName;
    // 程序配置
    QSettings *m_svcSettings = nullptr;
    // 是否在加载数据
    bool m_isDataLoading = false;

    // 菜单
    QAction *m_actStartService = nullptr;   // 启动服务
    QAction *m_actStopService = nullptr;    // 停止服务
    QAction *m_actRestartService = nullptr; // 重启服务
    QMenu *m_menuSevieStartupMode = nullptr;// 服务启动模式
    QAction *m_actAutoStart = nullptr;      // 自动启动
    QAction *m_actManualStart = nullptr;    // 手动启动

    QAction *m_actRefresh = nullptr;        // 刷新服务列表

    QAction *m_actColumeLoadState = nullptr;     // 加载状态列
    QAction *m_actColumeActiveState = nullptr;   // 活动状态列
    QAction *m_actColumeSubState = nullptr;      // 子状态列
    QAction *m_actColumeState = nullptr;         // 状态列
    QAction *m_actColumeMainPID = nullptr;       // 主进程id列
    QAction *m_actColumeDescription = nullptr;   // 描述列
    QAction *m_actColumeStartupMode = nullptr;   // 启动模式列

    bool m_needFirstRefresh = true; // 是否需要首次刷新

    QShortcut *mApplyShortcut;
    QPoint p{0,0};
};

#endif  // __SERVICETABLEVIEW_H__
