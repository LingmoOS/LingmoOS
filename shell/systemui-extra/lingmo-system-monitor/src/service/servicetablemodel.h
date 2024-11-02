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

#ifndef __SERVICE_TABLE_MODEL_H__
#define __SERVICE_TABLE_MODEL_H__

#include "serviceinfo.h"

#include <QAbstractTableModel>
#include <QList>
#include <QMap>

class ServiceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * @brief 服务列表列下标
     */
    enum Column {
        ServiceNameColumn = 0, // 服务名称
        ServiceLoadStateColumn, // 服务加载状态
        ServiceActiveStateColumn, // 服务活动状态
        ServiceSubStateColumn, // 服务子状态
        ServiceStateColumn, // 服务状态
        ServiceDescriptionColumn, // 服务描述
        ServiceMainPIDColumn, // 服务主进程id
        ServiceStartupModeColumn, // 服务启动模式

        ServiceColumnCount
    };

    /**
     * @brief ServiceTableModel 服务列表模型构造
     * @param parent 父对象指针
     */
    explicit ServiceTableModel(QObject *parent = nullptr);

    /**
     * @brief rowCount 模型总行数
     * @param parent 父索引
     * @return 模型总行数
     */
    int rowCount(const QModelIndex &parent = {}) const override;

    /**
     * @brief columnCount 模型总列数
     * @param parent 父索引
     * @return 模型总列数
     */
    int columnCount(const QModelIndex &parent = {}) const override;

    /**
     * @brief data 返回指定索引的数据条目内容
     * @param index 数据索引
     * @param role 数据角色
     * @return 指定索引的数据条目内容
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief headerData 返回指定表头列的内容
     * @param section 表头列下标
     * @param orientation 表头方向
     * @param role 数据角色
     * @return 指定表头列的内容
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * @brief Set the data for the given role and section in the header with the specified orientation
     * @param section Section of the header
     * @param orientation Orientation of the header
     * @param value Set data
     * @param role Data role
     * @return Set result
     */
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);

    /**
     * @brief flags 返回指定索引条目的标识（可选、使能等）
     * @param index 条目的索引
     * @return 索引条目的标识集
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief getServiceState 获取服务状态根据指定的模型索引
     * @param index 模型索引
     * @return 服务状态
     */
    QString getServiceState(const QModelIndex &index) const;

    /**
     * @brief getServiceName 获取服务名称根据指定的模型索引
     * @param index 模型索引
     * @return 服务名称
     */
    QString getServiceName(const QModelIndex &index) const;

    /**
     * @brief getServiceActiveState 获取服务活动状态根据指定的模型索引
     * @param index 模型索引
     * @return 服务活动状态
     */
    QString getServiceActiveState(const QModelIndex &index) const;

    /**
     * @brief getServiceDescription 获取服务描述根据指定的模型索引
     * @param index 模型索引
     * @return 服务描述
     */
    QString getServiceDescription(const QModelIndex &index) const;

    /**
     * @brief fetchMore 根据父索引获取更多的信息
     * @param parent 父索引
     */
    void fetchMore(const QModelIndex &parent) override;

    /**
     * @brief canFetchMore 根据父索引判断是否还有更多的信息可获取
     * @param parent 父索引
     * @return 返回true表示还可以获取更多信息，否则返回false为已获取全部信息
     */
    bool canFetchMore(const QModelIndex &parent) const override;

    /**
     * @brief reset 模型重置
     */
    void reset();

Q_SIGNALS:
    /**
     * @brief itemRowChanged 条目行号改变信号
     * @param row 行号
     */
    void itemRowChanged(int row);

private Q_SLOTS:
    /**
     * @brief updateServiceList 刷新服务列表槽
     */
    void updateServiceList(const QList<ServiceInfo> &list);

    /**
     * @brief updateServiceInfo 更新单个服务信息
     */
    void updateServiceInfo(const ServiceInfo &info);

private:
    // 服务名称列表
    QList<QString> m_svcNameList;
    // 服务信息列表
    QMap<QString, ServiceInfo> m_svcInfoList;
    // 当前显示在模型总的项数
    int m_nItemCount = 0;

    QStringList m_listStateTr;  // 状态翻译列表
    QMap<int, QString> headerDataMap;
};

#endif  // __SERVICE_TABLE_MODEL_H__
