/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef APPSEARCHTASK_H
#define APPSEARCHTASK_H

#include <QIcon>
#include <QThreadPool>
#include <QRunnable>
#include <QDBusInterface>
#include <QtDBus>
#include "search-task-plugin-iface.h"
#include "search-controller.h"
#include "result-item.h"
#include "application-info.h"
#include "search-result-property.h"

namespace LingmoUISearch {
class AppSearchTask : public SearchTaskPluginIface
{
    friend class AppSearchWorker;
    Q_OBJECT
public:
    explicit AppSearchTask(QObject *parent = nullptr);
    ~AppSearchTask();
    void setController(const SearchController &searchController);
    PluginType pluginType() {return PluginType::SearchTaskPlugin;}
    const QString name();
    const QString description();
    const QIcon icon() {return QIcon::fromTheme("appsearch");}
    void setEnable(bool enable) {Q_UNUSED(enable)}
    bool isEnable() { return true;}

    SearchProperty::SearchType getSearchType() {return SearchProperty::SearchType::Application;}
    QString getCustomSearchType();
    void startSearch();
    void stop();
    bool isSearching();

private Q_SLOTS:
    void dbusCallBack(QDBusPendingCallWatcher *call);

private:
    ApplicationInfo m_appinfo;
    SearchController m_searchController;
    QThreadPool *m_pool = nullptr;
    QThread *m_dbusCallBackThread = nullptr;
};

class AppSearchWorker : public QRunnable
{
public:
    explicit AppSearchWorker(AppSearchTask *AppSarchTask);

protected:
    void run();

private:
    void sendErrorMsg(const QString &msg);

private:
    SearchController* m_controller = nullptr;
    AppSearchTask *m_appSearchTask = nullptr;
    size_t m_currentSearchId = 0;
    int m_resultNum = 0;
};
}
#endif // APPSEARCHTASK_H
