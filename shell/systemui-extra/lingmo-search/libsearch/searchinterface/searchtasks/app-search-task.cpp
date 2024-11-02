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
#include "app-search-task.h"
#include "index-status-recorder.h"
#include "common.h"
#include <QDir>
#include <QFile>
#include <QQueue>
#include <QDebug>

using namespace LingmoUISearch;
AppSearchTask::AppSearchTask(QObject *parent)
{
    this->setParent(parent);
    qRegisterMetaType<size_t>("size_t");
    m_pool = new QThreadPool(this);
    m_pool->setMaxThreadCount(3);
    m_dbusCallBackThread = new QThread(this);
    qDBusRegisterMetaType<QMap<QString, QString>>();
    qDBusRegisterMetaType<QList<QMap<QString, QString>>>();
}

AppSearchTask::~AppSearchTask()
{
    m_pool->clear();
    m_pool->waitForDone();
    m_dbusCallBackThread->quit();
    m_dbusCallBackThread->wait();
}

void AppSearchTask::setController(const SearchController &searchController)
{
    m_searchController = searchController;
}

const QString AppSearchTask::name()
{
    return tr("Application");
}

const QString AppSearchTask::description()
{
    return tr("Application search.");
}

QString AppSearchTask::getCustomSearchType()
{
    return "Application";
}

void AppSearchTask::startSearch()
{
    if(!m_dbusCallBackThread->isRunning()) {
        m_dbusCallBackThread->start();
    }
    AppSearchWorker *appSearchWorker = new AppSearchWorker(this);
    m_pool->start(appSearchWorker);
}

void AppSearchTask::stop()
{
    if(m_dbusCallBackThread->isRunning()) {
        m_dbusCallBackThread->exit();
    }
}

bool AppSearchTask::isSearching()
{
    return m_pool->activeThreadCount() > 0;
}

void AppSearchTask::dbusCallBack(QDBusPendingCallWatcher *call)
{
    QDBusPendingReply<QList<QMap<QString, QString>>> reply = *call;
    size_t searchId = call->property("id").value<size_t>();
    if(reply.isValid()) {
        SearchResultProperties properties = call->property("prop").value<SearchResultProperties>();
        int resultNum = call->property("num").toInt();
        for(int i = 0; i < reply.value().size(); i++) {
            if (m_searchController.beginSearchIdCheck(searchId)) {
                ResultItem item(searchId);
                item.setItemKey(reply.value().at(i).value("appname"));
                if(properties.contains(SearchProperty::SearchResultProperty::ApplicationPkgName)) {
                    item.setValue(SearchProperty::SearchResultProperty::ApplicationPkgName, reply.value().at(i).value("appname"));
                }

                if(properties.contains(SearchProperty::SearchResultProperty::ApplicationLocalName)) {
                    QString localName;
                    if(QLocale::system().language() == QLocale::Chinese) {
                        localName = reply.value().at(i).value("displayname_cn");
                    } else {
                        localName = reply.value().at(i).value("appname");
                    }
                    item.setValue(SearchProperty::SearchResultProperty::ApplicationLocalName, localName);
                }
                if(properties.contains(SearchProperty::SearchResultProperty::ApplicationIconName)) {
                    item.setValue(SearchProperty::SearchResultProperty::ApplicationIconName, reply.value().at(i).value("icon"));
                }
                if(properties.contains(SearchProperty::SearchResultProperty::ApplicationDescription)) {
                    item.setValue(SearchProperty::SearchResultProperty::ApplicationDescription, reply.value().at(i).value("discription"));
                }
                if(properties.contains(SearchProperty::SearchResultProperty::IsOnlineApplication)) {
                    item.setValue(SearchProperty::SearchResultProperty::IsOnlineApplication, 1);
                }
                m_searchController.getDataQueue()->enqueue(item);
                if(++resultNum == m_searchController.informNum()) {
                    Q_EMIT reachInformNum();
                    resultNum = 0;
                }
                m_searchController.finishSearchIdCheck();
            } else {
                qDebug() << "Search id changed!";
                m_searchController.finishSearchIdCheck();
                return;
            }
        }
        Q_EMIT searchFinished(searchId);
    } else {
        qWarning() << "SoftWareCenter dbus called failed!" << reply.error();
        Q_EMIT searchError(searchId, "SoftWareCenter dbus called failed!");
    }
    call->deleteLater();
}

AppSearchWorker::AppSearchWorker(AppSearchTask *AppSarchTask): m_appSearchTask(AppSarchTask)
{
    m_controller = &m_appSearchTask->m_searchController;
    m_currentSearchId = m_controller->getCurrentSearchId();
}

void AppSearchWorker::run()
{
    ApplicationProperties applicationProperties;
    SearchResultProperties properties = m_controller->getResultProperties(SearchProperty::SearchType::Application);
    if(properties.contains(SearchProperty::SearchResultProperty::ApplicationDesktopPath)) {
        applicationProperties.append(ApplicationProperty::DesktopFilePath);
    }
    if(properties.contains(SearchProperty::SearchResultProperty::ApplicationLocalName)) {
        applicationProperties.append(ApplicationProperty::LocalName);
    }
    if(properties.contains(SearchProperty::SearchResultProperty::ApplicationIconName)) {
        applicationProperties.append(ApplicationProperty::Icon);
    }
    ApplicationInfoMap data = m_appSearchTask->m_appinfo.searchApp(applicationProperties, m_controller->getKeyword(), ApplicationPropertyMap{{ApplicationProperty::DontDisplay, 0}, {ApplicationProperty::AutoStart, 0}});
    for (const QString &desktop : data.keys()) {
        if (m_controller->beginSearchIdCheck(m_currentSearchId)) {
           ResultItem item(desktop);
           item.setSearchId(m_currentSearchId);

           ApplicationPropertyMap oneResult = data.value(desktop);
           if(oneResult.contains(ApplicationProperty::DesktopFilePath)) {
               item.setValue(SearchProperty::SearchResultProperty::ApplicationDesktopPath, oneResult.value(ApplicationProperty::DesktopFilePath).toString());
           }
           if(oneResult.contains(ApplicationProperty::LocalName)) {
               item.setValue(SearchProperty::SearchResultProperty::ApplicationLocalName, oneResult.value(ApplicationProperty::LocalName).toString());
           }
           if(oneResult.contains(ApplicationProperty::Icon)) {
               item.setValue(SearchProperty::SearchResultProperty::ApplicationIconName, oneResult.value(ApplicationProperty::Icon).toString());
           }
           m_controller->getDataQueue()->enqueue(item);
           if(++m_resultNum == m_controller->informNum()) {
               QMetaObject::invokeMethod(m_appSearchTask, "reachInformNum");
               m_resultNum = 0;
           }
           m_controller->finishSearchIdCheck();
        } else {
            qDebug() << "Search id changed!";
            m_controller->finishSearchIdCheck();
            return;
        }
    }
    if (m_controller->isSearchOnlineApps()) {
        //online app search
        for (auto keyword : m_controller->getKeyword()) {
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("com.lingmo.softwarecenter.getsearchresults"),
                                                                         QStringLiteral("/com/lingmo/softwarecenter/getsearchresults"),
                                                                         QStringLiteral("com.lingmo.getsearchresults"),
                                                                         QStringLiteral("get_search_result"));
            msg.setArguments({keyword});
            QDBusPendingCall call = QDBusConnection::sessionBus().asyncCall(msg);
            auto *watcher = new QDBusPendingCallWatcher(call);
            watcher->setProperty("id", QVariant::fromValue(m_currentSearchId));
            watcher->setProperty("prop", QVariant::fromValue(properties));
            watcher->setProperty("num", m_resultNum);
            watcher->moveToThread(m_appSearchTask->m_dbusCallBackThread);
            QObject::connect(watcher, &QDBusPendingCallWatcher::finished, m_appSearchTask, &AppSearchTask::dbusCallBack, Qt::DirectConnection);
        }
    } else {
        QMetaObject::invokeMethod(m_appSearchTask, "searchFinished", Q_ARG(size_t, m_currentSearchId));
    }
}

void AppSearchWorker::sendErrorMsg(const QString &msg)
{
    QMetaObject::invokeMethod(m_appSearchTask, "searchError",
                              Q_ARG(size_t, m_currentSearchId),
                              Q_ARG(QString, msg));
}
