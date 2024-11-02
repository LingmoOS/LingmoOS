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
#include "lingmo-search-gui.h"
#include <QScreen>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <gperftools/malloc_extension.h>
#include <QCommandLineParser>
#include "plugin-manager.h"
#include "icon-loader.h"

using namespace LingmoUISearch;
LingmoUISearchGui::LingmoUISearchGui(int &argc,
                             char *argv[],
                             const QString &display,
                             const QString &sessionType,
                             const QString &applicationName):
                             QtSingleApplication (applicationName + display, argc, argv)
{
    qDebug()<<"lingmo search gui constructor start, session type:" << sessionType << "display:" << display;
    qApp->setProperty("display", display);
    qApp->setProperty("sessionType", sessionType);
    setApplicationVersion(QString("v%1").arg(VERSION));
    if (!this->isRunning()) {
        connect(this, &QtSingleApplication::messageReceived, [=](QString msg) {
            this->parseCmd(msg, true);
        });

        setQuitOnLastWindowClosed(false);

        //load translations.
        QTranslator *translator = new QTranslator(this);
        try {
            if(! translator->load(LINGMO_SEARCH_QM_INSTALL_PATH"/" + QLocale::system().name())) throw - 1;
            this->installTranslator(translator);
        } catch(...) {
            qDebug() << "Load translations file: " << LINGMO_SEARCH_QM_INSTALL_PATH << QLocale() << "failed!";
        }

        QTranslator *qt_translator = new QTranslator(this);
        try {
            if(! qt_translator->load("/usr/share/qt5/translations/qt_" + QLocale::system().name())) throw - 1;
            this->installTranslator(qt_translator);
        } catch(...) {
            qDebug() << "Load translations file" << QLocale() << "failed!";
        }

        QTranslator *lib_translator = new QTranslator(this);
        try {
            if(! lib_translator->load("/usr/share/lingmo-search/translations/liblingmo-search/liblingmo-search_" + QLocale::system().name())) throw - 1;
            this->installTranslator(lib_translator);
        } catch(...) {
            qDebug() << "Load translations file" << QLocale() << "failed!";
        }

        qDebug() << "Loading plugins and resources...";
        MallocExtension::Initialize();
        SearchPluginManager::getInstance();
        PluginManager::getInstance();

        m_mainWindow = new LingmoUISearch::MainWindow();
        m_dbusService = new LingmoUISearch::LingmoUISearchDbusServices(m_mainWindow, this);
        qApp->setWindowIcon(IconLoader::loadIconQt("lingmo-search"));
        this->setActivationWindow(m_mainWindow);
    }

    //parse cmd
    qDebug()<<"parse cmd";
    auto message = this->arguments().join(' ').toUtf8();
    parseCmd(message, !isRunning());

    qDebug()<<"lingmo search gui constructor end";
}

LingmoUISearchGui::~LingmoUISearchGui()
{
    if(m_mainWindow) {
        delete m_mainWindow;
        m_mainWindow = nullptr;
    }
    if(m_dbusService) {
        delete m_dbusService;
        m_dbusService = nullptr;
    }
}

void LingmoUISearchGui::parseCmd(QString msg, bool isPrimary)
{
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption quitOption(QStringList{"q","quit"}, tr("Quit lingmo-search application"));
    parser.addOption(quitOption);

    QCommandLineOption showOption(QStringList{"s","show"}, tr("Show main window"));
    parser.addOption(showOption);

    QCommandLineOption unregisterOption("unregister", tr("unregister a plugin with <pluginName>"), "pluginName");
    parser.addOption(unregisterOption);

    QCommandLineOption registerOption("register", tr("register a plugin with <pluginName>"), "pluginName");
    parser.addOption(registerOption);

    QCommandLineOption moveOption(QStringList{"m", "move"}, tr("move <pluginName> to the target pos"), "pluginName");
    parser.addOption(moveOption);

    QCommandLineOption indexOption(QStringList{"i", "index"}, tr("move plugin to <index>"), "index");
    parser.addOption(indexOption);

    if (isPrimary) {
        const QStringList args = QString(msg).split(' ');
        parser.process(args);
        if(parser.isSet(showOption)) {
            m_mainWindow->bootOptionsFilter("-s");
        }
        if (parser.isSet(quitOption)) {
            this->quit();
        }
        if (parser.isSet(unregisterOption)) {
            QString pluginName = parser.value(unregisterOption);
            qDebug() << "unregister plugin:" << pluginName << SearchPluginManager::getInstance()->unregisterPlugin(pluginName);
        }
        if (parser.isSet(registerOption)) {
            qDebug() << parser.values("register");
            QString pluginName = parser.value(registerOption);

            qDebug() << "register plugin:" << pluginName << SearchPluginManager::getInstance()->reRegisterPlugin(pluginName);
        }
        if (parser.isSet(moveOption) and parser.isSet(indexOption)) {
            QString pluginName = parser.value(moveOption);
            int pos = parser.value(indexOption).toInt();
            qDebug() << QString("move plugin:%1 to %2").arg(pluginName).arg(pos) << SearchPluginManager::getInstance()->changePluginPos(pluginName, pos);
        }
    }
    else {
        if (arguments().count() < 2) {
            parser.showHelp();
        }
        parser.process(arguments());
        sendMessage(msg);
    }
}
