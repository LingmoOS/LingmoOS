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

#include "app-search-plugin.h"

#include <LingmoUISearchTask>
#include <QThread>
#include <QTimer>

namespace LingmoUIMenu {

class AppSearchPluginPrivate : public QThread
{
    Q_OBJECT
public:
    AppSearchPluginPrivate();

Q_SIGNALS:
    void searchedOne(DataEntity app);

public Q_SLOTS:
    void startSearch(QString &keyword);
    void stopSearch();

protected:
    void run() override;

private:
    size_t m_searchId{0};
    QTimer *m_timer{nullptr};
    LingmoUISearch::LingmoUISearchTask *m_appSearchTask{nullptr};
    LingmoUISearch::DataQueue<LingmoUISearch::ResultItem> *m_dataQueue{nullptr};
};

AppSearchPluginPrivate::AppSearchPluginPrivate() : QThread(nullptr)
{
    m_appSearchTask = new LingmoUISearch::LingmoUISearchTask(this);
    m_dataQueue = m_appSearchTask->init();

    m_appSearchTask->initSearchPlugin(LingmoUISearch::SearchProperty::SearchType::Application);
    m_appSearchTask->setSearchOnlineApps(false);

    LingmoUISearch::SearchResultProperties searchResultProperties;
    searchResultProperties << LingmoUISearch::SearchProperty::SearchResultProperty::ApplicationDesktopPath
                           << LingmoUISearch::SearchProperty::SearchResultProperty::ApplicationLocalName
                           << LingmoUISearch::SearchProperty::SearchResultProperty::ApplicationIconName;
    m_appSearchTask->setResultProperties(LingmoUISearch::SearchProperty::SearchType::Application, searchResultProperties);

    m_timer = new QTimer;
    m_timer->setInterval(3000);
    m_timer->moveToThread(this);
}

void AppSearchPluginPrivate::startSearch(QString &keyword)
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
                app.setType(DataType::Normal);
                app.setId(result.getValue(LingmoUISearch::SearchProperty::ApplicationDesktopPath).toString());
                app.setName(result.getValue(LingmoUISearch::SearchProperty::ApplicationLocalName).toString());
                app.setIcon("image://appicon/" + result.getValue(LingmoUISearch::SearchProperty::ApplicationIconName).toString());

                Q_EMIT this->searchedOne(app);
            }
        }

        if(m_timer->isActive() && m_timer->remainingTime() < 0.01 && m_dataQueue->isEmpty()) {
            this->requestInterruption();
        }
    }
}

// ========AppSearchPlugin======== //
AppSearchPlugin::AppSearchPlugin()
{
    m_searchPluginPrivate = new AppSearchPluginPrivate;
    connect(m_searchPluginPrivate, &AppSearchPluginPrivate::searchedOne, this, &AppSearchPlugin::onSearchedOne);
}

int AppSearchPlugin::index()
{
    return 0;
}

QString AppSearchPlugin::id()
{
    return "search";
}

QString AppSearchPlugin::name()
{
    return tr("Search");
}

QString AppSearchPlugin::icon()
{
    return "image://appicon/search-symbolic";
}

QString AppSearchPlugin::title()
{
    return "";
}

PluginGroup::Group AppSearchPlugin::group()
{
    return PluginGroup::Button;
}

QVector<DataEntity> AppSearchPlugin::data()
{
    return {};
}

void AppSearchPlugin::forceUpdate()
{

}

void AppSearchPlugin::forceUpdate(QString &key)
{
//    qDebug() << "==AppSearchPlugin key:" << key;
    Q_EMIT dataChanged({});
    if (key.isEmpty()) {
        return;
    }

    m_searchPluginPrivate->startSearch(key);
}

void AppSearchPlugin::onSearchedOne(const DataEntity &app)
{
    Q_EMIT dataChanged({1, app}, DataUpdateMode::Append);
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
