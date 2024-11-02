/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "lingmo-menu-application.h"
#include "menu-dbus-service.h"
#include "settings.h"
#include "commons.h"
#include "menu-main-window.h"
#include "power-button.h"
#include "app-manager.h"
#include "context-menu-manager.h"
#include "event-track.h"
#include "sidebar-button-utils.h"
#include "widget-model.h"
#include "app-page-backend.h"
#include "app-group-model.h"
#include "favorite/favorites-model.h"
#include "favorite/folder-model.h"

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>

using namespace LingmoUIMenu;

LingmoUIMenuApplication::LingmoUIMenuApplication(MenuMessageProcessor *processor) : QObject(nullptr)
{
    registerQmlTypes();
    startLingmoUIMenu();
    initDbusService();
    connect(processor, &MenuMessageProcessor::request, this, &LingmoUIMenuApplication::execCommand);
}

void LingmoUIMenuApplication::startLingmoUIMenu()
{
    initQmlEngine();
    loadMenuUI();
}

void LingmoUIMenuApplication::registerQmlTypes()
{
    const char *uri = "org.lingmo.menu.core";
    int versionMajor = 1, versionMinor = 0;
    SettingModule::defineModule(uri, versionMajor, versionMinor);
    WindowModule::defineModule(uri, versionMajor, versionMinor);
    PowerButton::defineModule(uri, versionMajor, versionMinor);
    SidebarButtonUtils::defineModule(uri, versionMajor, versionMinor);

    qmlRegisterType<WidgetModel>(uri, versionMajor, versionMinor, "WidgetModel");
    qmlRegisterType<AppGroupModel>(uri, versionMajor, versionMinor, "AppGroupModel");
    //qmlRegisterType<AppPageBackend>(uri, versionMajor, versionMinor, "AppPageBackend");
    qmlRegisterSingletonInstance(uri, versionMajor, versionMinor, "AppPageBackend", AppPageBackend::instance());
    qmlRegisterUncreatableType<AppListPluginGroup>(uri, versionMajor, versionMinor, "PluginGroup", "Use enums only.");

    // commons
    qRegisterMetaType<LingmoUIMenu::DataType::Type>("DataType::Type");
    qRegisterMetaType<LingmoUIMenu::DataEntity>("DataEntity");
    qRegisterMetaType<LingmoUIMenu::MenuInfo::Location>("MenuInfo::Location");
    qmlRegisterUncreatableType<Display>(uri, versionMajor, versionMinor, "Display", "Use enums only.");
    qmlRegisterUncreatableType<LingmoUIMenu::DataType>(uri, versionMajor, versionMinor, "DataType", "Use enums only");
    qmlRegisterUncreatableType<LingmoUIMenu::MenuInfo>(uri, versionMajor, versionMinor, "MenuInfo", "Use enums only.");
    qmlRegisterUncreatableType<LingmoUIMenu::LabelItem>(uri, versionMajor, versionMinor, "LabelItem", "Use enums only.");
    qmlRegisterUncreatableType<LingmoUIMenu::WidgetMetadata>(uri, versionMajor, versionMinor, "WidgetMetadata", "Use enums only.");
//    qmlRegisterUncreatableType<LingmoUIMenu::DataEntity>(uri, versionMajor, versionMinor, "DataEntity", "unknown");

    qmlRegisterUncreatableType<EventTrack>(uri, versionMajor, versionMinor, "EventTrack", "Attached only.");
}

void LingmoUIMenuApplication::initQmlEngine()
{
    m_engine = new QQmlEngine(this);
    m_engine->addImportPath("qrc:/qml");

    QQmlContext *context = m_engine->rootContext();
    context->setContextProperty("menuSetting", MenuSetting::instance());
    context->setContextProperty("menuManager", ContextMenuManager::instance());
    context->setContextProperty("appManager", AppManager::instance());

    context->setContextProperty("favoriteModel", &FavoritesModel::instance());
    context->setContextProperty("FolderModel", &FolderModel::instance());
    // MenuMainWindow
//    const QUrl url(QStringLiteral("qrc:/qml/MenuMainWindow.qml"));
//    QQmlApplicationEngine *m_applicationEngine{nullptr};
//    m_applicationEngine = new QQmlApplicationEngine(this);
//    QObject::connect(m_applicationEngine, &QQmlApplicationEngine::objectCreated,
//                     this, [url](QObject *obj, const QUrl &objUrl) {
//            if (!obj && url == objUrl)
//                QCoreApplication::exit(-1);
//        }, Qt::QueuedConnection);
//
//    m_applicationEngine->load(url);

    connect(AppManager::instance(), &AppManager::request, this, &LingmoUIMenuApplication::execCommand);
}

void LingmoUIMenuApplication::loadMenuUI()
{
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    m_mainWindow = new MenuWindow(m_engine, nullptr);
    m_mainWindow->setSource(url);
    connect(m_mainWindow, &QQuickView::activeFocusItemChanged, m_mainWindow, [this] {
        if (m_mainWindow->activeFocusItem()) {
            return;
        }

        execCommand(Hide);
    });
}

void LingmoUIMenuApplication::initDbusService()
{
    m_menuDbusService = new MenuDbusService(QGuiApplication::instance()->property("display").toString(), this);
    if (m_menuDbusService) {
        connect(m_menuDbusService, &MenuDbusService::menuActive, this, [this] {
            execCommand(Active);
        });
//        connect(m_menuDbusService, &MenuDbusService::reloadConfigSignal, this, [this] {
////            reload ...
//        });
    }
}

void LingmoUIMenuApplication::execCommand(Command command)
{
    switch (command) {
        case Active: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(!m_mainWindow->isVisible());
            }
            break;
        }
        case Show: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(true);
            }
            break;
        }
        case Quit: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(false);
                m_engine->quit();
            }
            QCoreApplication::quit();
            break;
        }
        case Hide: {
            if (m_mainWindow) {
                m_mainWindow->activeMenuWindow(false);
            }
            break;
        }
        default:
            break;
    }

    // TODO: 发送窗口显示隐藏信号到插件
}

LingmoUIMenuApplication::~LingmoUIMenuApplication()
{
    if (m_mainWindow) {
        m_mainWindow->deleteLater();
    }
}

// == MenuMessageProcessor == //

QCommandLineOption MenuMessageProcessor::showLingmoUIMenu({"s", "show"}, QObject::tr("Show lingmo-menu."));
QCommandLineOption MenuMessageProcessor::quitLingmoUIMenu({"q", "quit"}, QObject::tr("Quit lingmo-menu."));

MenuMessageProcessor::MenuMessageProcessor() : QObject(nullptr) {}

void MenuMessageProcessor::processMessage(const QString &message)
{
    QStringList options = QString(message).split(' ');

    QCommandLineParser parser;
    parser.addOption(MenuMessageProcessor::showLingmoUIMenu);
    parser.addOption(MenuMessageProcessor::quitLingmoUIMenu);

    parser.parse(options);

    if (parser.isSet(MenuMessageProcessor::showLingmoUIMenu)) {
        Q_EMIT request(LingmoUIMenuApplication::Show);
        return;
    }

    if (parser.isSet(MenuMessageProcessor::quitLingmoUIMenu)) {
        Q_EMIT request(LingmoUIMenuApplication::Quit);
        return;
    }
}

/**
 * 已知问题，使用命令quit时，会出现 QFileSystemWatcher::removePaths: list is empty
 * 可见该bug: https://bugreports.qt.io/browse/QTBUG-68607
 * @param message
 * @return
 */
bool MenuMessageProcessor::preprocessMessage(const QStringList& message)
{
    qDebug() << "Hey, there.";

    QCommandLineParser parser;
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();

    parser.addOption(MenuMessageProcessor::showLingmoUIMenu);
    parser.addOption(MenuMessageProcessor::quitLingmoUIMenu);

    parser.parse(message);

    if (message.size() <= 1 || parser.isSet(helpOption) || !parser.unknownOptionNames().isEmpty()) {
        if (!parser.unknownOptionNames().isEmpty()) {
            qDebug() << "Unknown options:" << parser.unknownOptionNames();
        }
        parser.showHelp();
        return false;
    }

    if (parser.isSet(versionOption)) {
        parser.showVersion();
        return false;
    }

    return parser.isSet(MenuMessageProcessor::showLingmoUIMenu) || parser.isSet(MenuMessageProcessor::quitLingmoUIMenu);
}
