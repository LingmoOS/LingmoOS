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
 */

#ifndef LINGMO_MENU_ALL_APP_DATA_PROVIDER_H
#define LINGMO_MENU_ALL_APP_DATA_PROVIDER_H

#include "data-provider-plugin-iface.h"
#include <QSet>
#include <QList>
#include <QTimer>

namespace LingmoUIMenu {

class AllAppDataProvider : public DataProviderPluginIFace
{
    Q_OBJECT
public:
    explicit AllAppDataProvider();
    int index() override;
    QString id() override;
    QString name() override;
    QString icon() override;
    QString title() override;
    PluginGroup::Group group() override;
    QVector<DataEntity> data() override;
    void forceUpdate() override;

public Q_SLOTS:
    void update(bool isShowed) override;

private Q_SLOTS:
    void onAppAdded(const QList<DataEntity>& apps);
    void onAppDeleted(QStringList idList);
    void onAppUpdated(const QList<DataEntity>& apps);
    // TODO 文件夹数据新增，删除信号处理
    void onAppFolderChanged();

private:
    inline void sendData();
    void removeApps(QStringList& idList);
    void reloadAppData();
    void reloadFolderData();
    void mergeData(QVector<DataEntity> &data);
    void updateData(const QList<DataEntity>& apps);
    void updateFolderData(QStringList& idList);
    void updateTimer();
    static bool appDataSort(const DataEntity &a, const DataEntity &b);
    static void setSortPriority(DataEntity &app);
    static void setRecentState(DataEntity &app);
    static bool letterSort(const QString &a, const QString &b);

private:
    QTimer *m_timer = nullptr;
    QMutex m_mutex;
    QVector<DataEntity> m_appData;
    QVector<DataEntity> m_folderData;
    bool m_updateStatus = false;
    bool m_windowStatus = false;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_ALL_APP_DATA_PROVIDER_H
