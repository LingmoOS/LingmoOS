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
#include "app-search-plugin.h"
#include <gio/gdesktopappinfo.h>
#include <QWidget>
#include <QLabel>
#include <QDeadlineTimer>
#include "file-utils.h"
#include "data-collecter.h"
using namespace LingmoUISearch;

AppSearchPlugin::AppSearchPlugin(QObject *parent) : QThread(parent), m_appSearchTask(new LingmoUISearchTask(this))
{
    SearchPluginIface::Actioninfo open { 0, tr("Open")};
    SearchPluginIface::Actioninfo addtoDesktop { 1, tr("Add Shortcut to Desktop")};
    SearchPluginIface::Actioninfo addtoPanel { 2, tr("Add Shortcut to Panel")};
    SearchPluginIface::Actioninfo install { 0, tr("Install")};
    m_actionInfo_installed << open << addtoDesktop << addtoPanel;
    m_actionInfo_not_installed << install;
//    m_pool.setMaxThreadCount(1);
//    m_pool.setExpiryTimeout(1000);
    initDetailPage();
    m_appSearchResults = m_appSearchTask->init();
    m_appSearchTask->initSearchPlugin(SearchProperty::SearchType::Application);
    m_appSearchTask->setSearchOnlineApps(true);
    m_appSearchTask->setResultProperties(SearchProperty::SearchType::Application,
                                         SearchResultProperties{SearchProperty::SearchResultProperty::ApplicationDesktopPath,
                                                                SearchProperty::SearchResultProperty::ApplicationLocalName,
                                                                SearchProperty::SearchResultProperty::ApplicationIconName,
                                                                SearchProperty::SearchResultProperty::ApplicationDescription,
                                                                SearchProperty::SearchResultProperty::IsOnlineApplication,
                                                                SearchProperty::SearchResultProperty::ApplicationPkgName});
}

AppSearchPlugin::~AppSearchPlugin()
{
    this->wait();
}

const QString AppSearchPlugin::name()
{
    return "Applications Search";
}

const QString AppSearchPlugin::description()
{
    return tr("Applications Search");
}

QString AppSearchPlugin::getPluginName()
{
    return tr("Applications Search");
}

void AppSearchPlugin::KeywordSearch(QString keyword, DataQueue<SearchPluginIface::ResultInfo> *searchResult)
{
    if (!this->isRunning()) {
        this->start();
    }
    m_searchResult = searchResult;
    m_appSearchTask->clearKeyWords();
    m_appSearchTask->addKeyword(keyword);
    m_appSearchTask->startSearch(SearchProperty::SearchType::Application);

//    AppSearch *appsearch = new AppSearch(searchResult, m_appSearchResults, m_appSearchTask, keyword, uniqueSymbol);
//    m_pool.start(appsearch);
}

void AppSearchPlugin::stopSearch()
{
    m_appSearchTask->stop();
    this->requestInterruption();
    this->wait();
}

QList<SearchPluginIface::Actioninfo> AppSearchPlugin::getActioninfo(int type)
{
    switch (type) {
    case 0:
        return m_actionInfo_installed;
        break;
    case 1:
        return m_actionInfo_not_installed;
        break;
    default:
        return QList<SearchPluginIface::Actioninfo>();
        break;
    }
}

void AppSearchPlugin::openAction(int actionkey, QString key, int type)
{
    switch (type) {
    case 0:
        switch (actionkey) {
        case 0:
            if(!launch(key)) {
                qWarning() << "Fail to launch:" << key;
            }
            break;
        case 1:
            if(!addDesktopShortcut(key)) {
                qWarning() << "Fail to add Desktop Shortcut:" << key;
            }
            break;
        case 2:
            if(!addPanelShortcut(key)) {
                qWarning() << "Fail to add Panel Shortcut:" << key;
            }
            break;
        default:
            break;
        }
        break;
    case 1:
        if(!installAppAction(key)) {
            qWarning() << "Fail to install:" << key;
        }
        break;
    default:
        break;
    }
}

QWidget *AppSearchPlugin::detailPage(const ResultInfo &ri)
{
    m_currentActionKey = ri.actionKey;
    m_iconLabel->setPixmap(ri.icon.isNull() ? QIcon(":/res/icons/unknown.svg").pixmap(120, 120) : ri.icon.pixmap(120, 120));
    QFontMetrics fontMetrics = m_nameLabel->fontMetrics();
    QString showname = fontMetrics.elidedText(ri.name, Qt::ElideRight, 215); //当字体长度超过215时显示为省略号
    m_nameLabel->setText(FileUtils::setAllTextBold(showname));
    if(QString::compare(showname, ri.name)) {
        m_nameLabel->setToolTip(ri.name);
    } else {
        m_nameLabel->setToolTip("");
    }
    m_pluginLabel->setText(tr("Application"));
    if(ri.type == 1) {
        m_actionLabel1->hide();
        m_actionLabel2->hide();
        m_actionLabel3->hide();
        m_actionLabel4->show();
//        QString showDesc = fontMetrics.elidedText(ri.description.at(0).key + " " + ri.description.at(0).value, Qt::ElideRight, 3114); //当字体长度超过215时显示为省略号
        QString showDesc = FileUtils::getSnippetWithoutKeyword(ri.description.at(0).key + " " + ri.description.at(0).value, 10);
        m_descLabel->setText(FileUtils::escapeHtml(showDesc));
        m_descLabel->show();
        m_line_2->show();

    } else {
        m_descLabel->hide();
        m_line_2->hide();
        m_actionLabel1->show();
        m_actionLabel2->show();
        m_actionLabel3->show();
        m_actionLabel4->hide();
    }
    return m_detailPage;
}

void AppSearchPlugin::run()
{
    QDeadlineTimer deadline(25000);
    while(!isInterruptionRequested()) {
        ResultItem oneResult = m_appSearchResults->tryDequeue();
        SearchResultPropertyMap data = oneResult.getAllValue();
        if(oneResult.getSearchId() == 0 && oneResult.getItemKey().isEmpty() && data.isEmpty()) {
            if(deadline.remainingTime()) {
                msleep(100);
            } else {
                this->requestInterruption();
            }

        } else {
            deadline.setRemainingTime(25000);
            SearchPluginIface::ResultInfo ri;

            ri.name = data.value(SearchProperty::SearchResultProperty::ApplicationLocalName).toString();
            ri.toolTip = ri.name;
            ri.resourceType = QStringLiteral("app");
            ri.icon = IconLoader::loadIconXdg(data.value(SearchProperty::SearchResultProperty::ApplicationIconName).toString(), QIcon(":/res/icons/unknown.svg"));
            SearchPluginIface::DescriptionInfo description;
            description.key = QString(tr("Application Description:"));
            description.value = data.value(SearchProperty::SearchResultProperty::ApplicationDescription).toString();
            ri.description.append(description);
            ri.type = data.value(SearchProperty::SearchResultProperty::IsOnlineApplication).toInt();
            if (ri.type == 1) {
                ri.actionKey = data.value(SearchProperty::SearchResultProperty::ApplicationPkgName).toString();
            } else {
                ri.actionKey = data.value(SearchProperty::SearchResultProperty::ApplicationDesktopPath).toString();
            }
            m_searchResult->enqueue(ri);
        }
    }
    m_appSearchResults->clear();
}

void AppSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedHeight(128);

    m_nameFrame = new QFrame(m_detailPage);
    m_nameFrameLyt = new QHBoxLayout(m_nameFrame);
    m_nameFrame->setLayout(m_nameFrameLyt);
    m_nameFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_nameLabel = new QLabel(m_nameFrame);
    m_nameLabel->setMaximumWidth(280);
    m_pluginLabel = new QLabel(m_nameFrame);
    m_pluginLabel->setEnabled(false);
    m_nameFrameLyt->addWidget(m_nameLabel);
    m_nameFrameLyt->addStretch();
    m_nameFrameLyt->addWidget(m_pluginLabel);

    m_line_1 = new SeparationLine(m_detailPage);

    m_descLabel = new QLabel(m_detailPage);
    m_descLabel->setTextFormat(Qt::PlainText);
    m_descLabel->setContentsMargins(8, 0, 8, 0);
//    m_descLabel->setWordWrap(true);
    m_line_2 = new SeparationLine(m_detailPage);

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(8, 0, 0, 0);
    m_actionLabel1 = new ActionLabel(tr("Open"), m_currentActionKey, m_actionFrame);
    m_actionLabel2 = new ActionLabel(tr("Add Shortcut to Desktop"), m_currentActionKey, m_actionFrame);
    m_actionLabel3 = new ActionLabel(tr("Add Shortcut to Panel"), m_currentActionKey, m_actionFrame);
    m_actionLabel4 = new ActionLabel(tr("Install"), m_currentActionKey, m_actionFrame);

    m_actionFrameLyt->addWidget(m_actionLabel1);
    m_actionFrameLyt->addWidget(m_actionLabel2);
    m_actionFrameLyt->addWidget(m_actionLabel3);
    m_actionFrameLyt->addWidget(m_actionLabel4);
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addSpacing(50);
    m_detailLyt->addWidget(m_iconLabel);
    m_detailLyt->addWidget(m_nameFrame);
    m_detailLyt->addWidget(m_line_1);
    m_detailLyt->addWidget(m_descLabel);
    m_detailLyt->addWidget(m_line_2);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        launch(m_currentActionKey);
    });
    connect(m_actionLabel2, &ActionLabel::actionTriggered, [ & ](){
        addDesktopShortcut(m_currentActionKey);
    });
    connect(m_actionLabel3, &ActionLabel::actionTriggered, [ & ](){
        addPanelShortcut(m_currentActionKey);
    });
    connect(m_actionLabel4, &ActionLabel::actionTriggered, [ & ](){
        installAppAction(m_currentActionKey);
    });
}

bool AppSearchPlugin::launch(const QString &path)
{
    DataCollecter::collectLaunchEvent(QStringLiteral("applicationSearch"), QStringLiteral("launch"));
    if(QFileInfo(path).fileName() == "lingmo-screenshot.desktop") {
        invokeActions(InvokableAction::HideUI);
    }
    bool res(false);
    QDBusInterface * appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                             QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                             QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                             QDBusConnection::sessionBus());
    if(!appLaunchInterface->isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        res = false;
    } else {
        appLaunchInterface->setTimeout(10000);
        QDBusReply<void> reply = appLaunchInterface->call("LaunchApp", path);
        if(reply.isValid()) {
            res = true;
        } else {
            qWarning() << "ProcessManager dbus called failed!" << reply.error();
            res = false;
        }
    }
    if(appLaunchInterface) {
        delete appLaunchInterface;
    }
    appLaunchInterface = NULL;
    if (res)
        return true;

    GDesktopAppInfo * desktopAppInfo = g_desktop_app_info_new_from_filename(path.toLocal8Bit().data());
    res = static_cast<bool>(g_app_info_launch(G_APP_INFO(desktopAppInfo), nullptr, nullptr, nullptr));
    g_object_unref(desktopAppInfo);
    return res;
}

bool AppSearchPlugin::addPanelShortcut(const QString& path)
{
    DataCollecter::collectLaunchEvent(QStringLiteral("applicationSearch"), QStringLiteral("addPanelShortcut"));
    QDBusInterface iface("org.lingmo.taskManager",
                         "/taskManager",
                         "org.lingmo.taskManager",
                         QDBusConnection::sessionBus());
    if(iface.isValid()) {
        QDBusReply<bool> isExist = iface.call("checkQuickLauncher", path);
        if (isExist) {
            qWarning() << "Add shortcut to panel failed, because it is already existed!";
            return false;
        }
        QDBusReply<QVariant> ret = iface.call("addQuickLauncher", path);
        if (ret.isValid()) {
            qDebug() << "Add shortcut to taskManager successed!";
            return true;
        } else {
            qWarning() << "Fail to add shortcut to taskManager!";
        }
    }
    return false;
}

bool AppSearchPlugin::addDesktopShortcut(const QString& path)
{
    DataCollecter::collectLaunchEvent(QStringLiteral("applicationSearch"), QStringLiteral("addDesktopShortcut"));
    QString dirpath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QFileInfo fileInfo(path);
    QString desktopfn = fileInfo.fileName();
    QFile file(path);
    QString newName = QString(dirpath + "/" + desktopfn);
    bool ret = file.copy(QString(dirpath + "/" + desktopfn));
    if(ret) {
        QProcess::startDetached("chmod", {"a+x", newName});
        return true;
    }
    return false;
}

bool AppSearchPlugin::installAppAction(const QString & name)
{
    DataCollecter::collectLaunchEvent(QStringLiteral("applicationSearch"), QStringLiteral("installAppAction"));
    QDBusInterface * interface = new QDBusInterface("com.lingmo.softwarecenter",
                "/com/lingmo/softwarecenter",
                "com.lingmo.utiliface",
                QDBusConnection::sessionBus());
    bool res(false);
    if(interface->isValid()) {
        //软件商店已打开，直接跳转
        interface->call("show_search_result", name);
        res = QDBusReply<bool>(interface->call(QString("show_search_result"), name));
    } else {
        //软件商店未打开，打开软件商店下载此软件
        qDebug() << "Softwarecenter has not been launched, now launch it." << name;
        QDBusInterface * appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                                 QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                                 QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                                 QDBusConnection::sessionBus());
        if(!appLaunchInterface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
            res = false;
        } else {
            appLaunchInterface->setTimeout(10000);
            QDBusReply<void> reply = appLaunchInterface->call("LaunchAppWithArguments", "/usr/share/applications/lingmo-software-center.desktop", QStringList() << "-find" << name);
            if(reply.isValid()) {
                res = true;
            } else {
                qWarning() << "ProcessManager dbus called failed!" << reply.error();
                res = false;
            }
        }
        if (appLaunchInterface) {
            delete appLaunchInterface;
        }
        appLaunchInterface = nullptr;
        if (!res) {
            res = QProcess::startDetached("lingmo-software-center", {"-find", name});
        }
    }
    if (interface) {
        delete interface;
    }
    interface = nullptr;
    return res;
}

//AppSearch::AppSearch(DataQueue<SearchPluginIface::ResultInfo> *searchResult, DataQueue<ResultItem>* appSearchResults, LingmoUISearchTask *appSearchTask, QString keyword, size_t uniqueSymbol)
//{
//    this->setAutoDelete(true);
//    m_searchResult = searchResult;
//    m_appSearchResults = appSearchResults;
//    m_appSearchTask = appSearchTask;
//    m_appSearchTask->clearKeyWords();
//    m_appSearchTask->addKeyword(keyword);
//    m_uniqueSymbol = uniqueSymbol;
//}

//AppSearch::~AppSearch()
//{
//}

//void AppSearch::run()
//{
////    m_appSearchTask->startSearch(SearchType::Application);
//    QTimer timer;
//    timer.setInterval(3000);
//    bool is_empty;
//    while(1) {
//        is_empty = false;
//        if(!m_appSearchResults->isEmpty()) {

//            ResultItem oneResult = m_appSearchResults->dequeue();

//            SearchPluginIface::ResultInfo ri;
//            ri.actionKey = oneResult.getExtral().at(0).toString();
//            ri.name = oneResult.getExtral().at(1).toString();
//            ri.icon = oneResult.getExtral().at(2).value<QIcon>();
//            SearchPluginIface::DescriptionInfo description;
//            description.key = QString(tr("Application Description:"));
//            description.value = oneResult.getExtral().at(3).toString();
//            ri.description.append(description);
//            ri.type = oneResult.getExtral().at(4).toInt();
////            if (isUniqueSymbolChanged()) {
////                m_appSearchResults->clear();
////                break;
////            }
//            m_searchResult->enqueue(ri);
//        } else {
//            is_empty = true;
//        }
//        if (isUniqueSymbolChanged()) {
//            break;
//        }
//        if(timer.isActive() && timer.remainingTime() < 0.01) {
//            qWarning()<<"-------------->stopped by itself";
//            break;
//        }
//        if(is_empty && !timer.isActive()) {
//            timer.start();
//        } else if(!is_empty) {
//            timer.stop();

//        } else {
//            QThread::msleep(100);
//        }
//    }
//}

//bool AppSearch::isUniqueSymbolChanged()
//{
//    QMutexLocker locker(&AppSearchPlugin::mutex);
//    if (m_uniqueSymbol != AppSearchPlugin::uniqueSymbol) {
//        qDebug() << "uniqueSymbol changged, app search finished!";
//        return true;
//    } else {
//        return false;
//    }
//}
