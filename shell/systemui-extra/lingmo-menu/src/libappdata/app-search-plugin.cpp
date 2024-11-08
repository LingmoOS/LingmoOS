/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */
#include "app-search-plugin.h"
#include "data-entity.h"
#include "basic-app-model.h"

#include <LingmoUISearchTask>
#include <QThread>
#include <QTimer>
#include <QAbstractListModel>
#include <QDebug>

namespace LingmoUIMenu {

// ====== AppSearchPluginPrivate ======
class AppSearchPluginPrivate : public QThread
{
    Q_OBJECT
public:
    explicit AppSearchPluginPrivate(QObject *parent = nullptr);

Q_SIGNALS:
    void searchedOne(DataEntity app);

public Q_SLOTS:
    void startSearch(const QString &keyword);
    void stopSearch();

protected:
    void run() override;

private:
    size_t m_searchId{0};
    QTimer *m_timer{nullptr};
    LingmoUISearch::LingmoUISearchTask *m_appSearchTask {nullptr};
    LingmoUISearch::DataQueue<LingmoUISearch::ResultItem> *m_dataQueue{nullptr};
};

AppSearchPluginPrivate::AppSearchPluginPrivate(QObject *parent) : QThread(parent), m_appSearchTask(new LingmoUISearch::LingmoUISearchTask(this))
{
    m_dataQueue = m_appSearchTask->init();

    m_appSearchTask->initSearchPlugin(LingmoUISearch::SearchProperty::SearchType::Application);
    m_appSearchTask->setSearchOnlineApps(false);

    LingmoUISearch::SearchResultProperties searchResultProperties;
    searchResultProperties << LingmoUISearch::SearchProperty::SearchResultProperty::ApplicationDesktopPath;
    m_appSearchTask->setResultProperties(LingmoUISearch::SearchProperty::SearchType::Application, searchResultProperties);

    m_timer = new QTimer;
    m_timer->setInterval(3000);
    m_timer->moveToThread(this);
}

void AppSearchPluginPrivate::startSearch(const QString &keyword)
{
    if (!this->isRunning()) {
        this->start();
    }

    m_appSearchTask->clearKeyWords();
    m_appSearchTask->addKeyword(keyword);
    m_searchId = m_appSearchTask->startSearch(LingmoUISearch::SearchProperty::SearchType::Application);
}

void AppSearchPluginPrivate::stopSearch()
{
    m_appSearchTask->stop();
    this->requestInterruption();
}

void AppSearchPluginPrivate::run()
{
    while (!isInterruptionRequested()) {
        LingmoUISearch::ResultItem result = m_dataQueue->tryDequeue();
        if(result.getSearchId() == 0 && result.getItemKey().isEmpty() && result.getAllValue().isEmpty()) {
            if(!m_timer->isActive()) {
                // 超时退出
                m_timer->start();
            }
            msleep(100);
        } else {
            m_timer->stop();
            if (result.getSearchId() == m_searchId) {
                DataEntity app;
                QString id = result.getValue(LingmoUISearch::SearchProperty::ApplicationDesktopPath).toString();
                if (!BasicAppModel::instance()->getAppById(id, app)) {
                    BasicAppModel::instance()->databaseInterface()->getApp(id, app);
                };
                Q_EMIT this->searchedOne(app);
            }
        }

        if(m_timer->isActive() && m_timer->remainingTime() < 0.01 && m_dataQueue->isEmpty()) {
            this->requestInterruption();
        }
    }
}

// ====== AppSearchModel ====== //
class AppSearchModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AppSearchModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void appendApp(const DataEntity &app);
    void clear();

private:
    QVector<DataEntity> m_apps;
};

AppSearchModel::AppSearchModel(QObject *parent) : QAbstractListModel(parent)
{

}

int AppSearchModel::rowCount(const QModelIndex &parent) const
{
    return m_apps.size();
}

int AppSearchModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant AppSearchModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const DataEntity &app = m_apps[index.row()];
    return app.getValue(static_cast<DataEntity::PropertyName>(role));
}

void AppSearchModel::appendApp(const DataEntity &app)
{
    beginInsertRows(QModelIndex(), m_apps.size(), m_apps.size());
    m_apps.append(app);
    endInsertRows();
}

void AppSearchModel::clear()
{
    beginResetModel();
    m_apps.clear();
    endResetModel();
}

// ====== AppSearchPlugin ====== //
AppSearchPlugin::AppSearchPlugin(QObject *parent) : AppListPluginInterface(parent)
    , m_searchPluginPrivate(new AppSearchPluginPrivate(this)), m_model(new AppSearchModel(this))
{
    connect(m_searchPluginPrivate, &AppSearchPluginPrivate::searchedOne, m_model, &AppSearchModel::appendApp);
}

AppListPluginGroup::Group AppSearchPlugin::group()
{
    return AppListPluginGroup::Search;
}

QString AppSearchPlugin::name()
{
    return "Search";
}

QString AppSearchPlugin::title()
{
    return "Search";
}

QList<QAction *> AppSearchPlugin::actions()
{
    // TODO: 搜索结果排序选项
    return {};
}

QAbstractItemModel *AppSearchPlugin::dataModel()
{
    return m_model;
}

void AppSearchPlugin::search(const QString &keyword)
{
    m_model->clear();
    if (keyword.isEmpty()) {
        return;
    }

    m_searchPluginPrivate->startSearch(keyword);
}

AppSearchPlugin::~AppSearchPlugin()
{
    m_searchPluginPrivate->stopSearch();
    m_searchPluginPrivate->quit();
    m_searchPluginPrivate->wait();
    m_searchPluginPrivate->deleteLater();
}

} // LingmoUIMenu

#include "app-search-plugin.moc"
