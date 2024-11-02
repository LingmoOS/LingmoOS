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
#include "web-search-plugin.h"
#include "global-settings.h"
#include <QDBusReply>
#include <QDBusInterface>
#include <QDesktopServices>
#include <QDebug>
#include <QApplication>
#include "data-collecter.h"

#define LINGMO_SEARCH_SCHEMAS "org.lingmo.search.settings"
#define BROWSERTYPE "x-scheme-handler/http"
#define DESKTOPPATH "/usr/share/applications/"
#define WEB_ENGINE_KEY "webEngine"

using namespace LingmoUISearch;
WebSearchPlugin::WebSearchPlugin(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled(LINGMO_SEARCH_SCHEMAS)) {
        m_settings = new QGSettings(LINGMO_SEARCH_SCHEMAS, QByteArray(), this);
        if (m_settings->keys().contains(WEB_ENGINE_KEY)) {
            m_webEngine = m_settings->get(WEB_ENGINE_KEY).toString();
        }
        connect(m_settings, &QGSettings::changed, this, [ & ] (const QString& key) {
            if (key == WEB_ENGINE_KEY) {
                m_webEngine = m_settings->get(WEB_ENGINE_KEY).toString();
            }
        });
    }
    SearchPluginIface::Actioninfo open { 0, tr("Start browser search")};
    m_actionInfo << open;
    initDetailPage();
}

const QString LingmoUISearch::WebSearchPlugin::name()
{
    return "Web Page";
}

const QString LingmoUISearch::WebSearchPlugin::description()
{
    return tr("Web Page");
}

QString LingmoUISearch::WebSearchPlugin::getPluginName()
{
    return tr("Web Page");
}

void LingmoUISearch::WebSearchPlugin::KeywordSearch(QString keyword, DataQueue<LingmoUISearch::SearchPluginIface::ResultInfo> *searchResult)
{
    m_keyWord = keyword;
    ResultInfo resultInfo;
    resultInfo.name = m_keyWord.replace("\r", " ").replace("\n", " ");
    resultInfo.toolTip = m_keyWord;
    resultInfo.resourceType = QStringLiteral("web");
    resultInfo.type = 0;

    QIcon appIcon;
    GAppInfo * app = g_app_info_get_default_for_type(BROWSERTYPE, false);
    if(app) {
        GIcon *gIcon = g_app_info_get_icon(app);
        if(G_IS_ICON(gIcon)) {
            const gchar* const* iconNames = g_themed_icon_get_names(G_THEMED_ICON(gIcon));
            if(iconNames) {
                auto p = iconNames;
                while(*p) {
                    appIcon = IconLoader::loadIconXdg(*p);
                    if(appIcon.isNull()) {
                        p++;
                    } else {
                        break;
                    }
                }
            }
        }
        g_object_unref(gIcon);
        g_object_unref(app);
    }
    resultInfo.icon = appIcon.isNull()? IconLoader::loadIconQt("unknown", QIcon(":/res/icons/unknown.svg")) : appIcon;
    resultInfo.actionKey = m_keyWord;
    searchResult->enqueue(resultInfo);
}

void WebSearchPlugin::stopSearch()
{
}

QList<LingmoUISearch::SearchPluginIface::Actioninfo> LingmoUISearch::WebSearchPlugin::getActioninfo(int type)
{
    Q_UNUSED(type)
    return m_actionInfo;
}

void LingmoUISearch::WebSearchPlugin::openAction(int actionkey, QString key, int type)
{
    DataCollecter::collectLaunchEvent(QStringLiteral("webSearch"), QStringLiteral("open"));
    Q_UNUSED(actionkey)
    Q_UNUSED(key)
    Q_UNUSED(type)
    QString address;
    if(!m_webEngine.isEmpty()) {
        if(m_webEngine == "360") {
            address = "https://www.so.com/s?q=" + m_keyWord; //360
        } else if(m_webEngine == "sougou") {
            address = "https://www.sogou.com/web?query=" + m_keyWord; //搜狗
        } else {
            address = "http://baidu.com/s?word=" + m_keyWord; //百度
        }
    } else { //默认值
        address = "http://baidu.com/s?word=" + m_keyWord ; //百度
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
        QDBusReply<void> reply = appLaunchInterface->call("LaunchDefaultAppWithUrl", address);
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
    if (res)
        return;
    QDesktopServices::openUrl(address);
}

QWidget *LingmoUISearch::WebSearchPlugin::detailPage(const LingmoUISearch::SearchPluginIface::ResultInfo &ri)
{
    Q_UNUSED(ri)
    return m_detailPage;
}

void LingmoUISearch::WebSearchPlugin::initDetailPage()
{
    m_detailPage = new QWidget();
    m_detailPage->setFixedWidth(360);
    m_detailPage->setAttribute(Qt::WA_TranslucentBackground);
    m_detailLyt = new QVBoxLayout(m_detailPage);
    m_detailLyt->setContentsMargins(8, 0, 16, 0);
    m_iconLabel = new QLabel(m_detailPage);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    QString type = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString();
    if (type == "lingmo-dark") {
        m_iconLabel->setPixmap(QIcon(":/res/icons/search-web-dark.svg").pixmap(128, 128));
    } else {
        m_iconLabel->setPixmap(QIcon(":/res/icons/search-web-default.svg").pixmap(128, 128));
    }
    connect(qApp, &QApplication::paletteChanged, this, [=] () {
        QString type = GlobalSettings::getInstance().getValue(STYLE_NAME_KEY).toString();
        if (type == "lingmo-dark") {
            m_iconLabel->setPixmap(QIcon(":/res/icons/search-web-dark.svg").pixmap(128, 128));
        } else {
            m_iconLabel->setPixmap(QIcon(":/res/icons/search-web-default.svg").pixmap(128, 128));
        }
    });

    m_actionFrame = new QFrame(m_detailPage);
    m_actionFrameLyt = new QVBoxLayout(m_actionFrame);
    m_actionFrameLyt->setContentsMargins(0, 0, 0, 0);
    m_actionFrameLyt->setAlignment(Qt::AlignCenter);
    m_actionLabel1 = new ActionLabel(tr("Start browser search"), m_currentActionKey, m_actionFrame);
    m_actionLabel1->adjustSize();

    m_actionFrameLyt->addWidget(m_actionLabel1);
    m_actionFrame->setLayout(m_actionFrameLyt);

    m_detailLyt->addSpacing(146);
    m_detailLyt->addWidget(m_iconLabel);
    //m_detailLyt->addSpacing(6);
    m_detailLyt->addWidget(m_actionFrame);
    m_detailPage->setLayout(m_detailLyt);
    m_detailLyt->addStretch();

    connect(m_actionLabel1, &ActionLabel::actionTriggered, [ & ](){
        openAction(0, m_currentActionKey, 0);
    });
}

QString WebSearchPlugin::getDefaultAppId(const char *contentType)
{
    GAppInfo * app = g_app_info_get_default_for_type(contentType, false);
    if(app != NULL){
        const char * id = g_app_info_get_id(app);
        QString strId(id);
        g_object_unref(app);
        return strId;
    } else {
        return QString("");
    }
}
