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

#ifndef LINGMO_MENU_LINGMO_MENU_APPLICATION_H
#define LINGMO_MENU_LINGMO_MENU_APPLICATION_H

#include <QObject>

class QQmlEngine;
class QCommandLineOption;

namespace LingmoUIMenu {

class MenuMessageProcessor;
class MenuWindow;
class MenuDbusService;

class LingmoUIMenuApplication : public QObject
{
    Q_OBJECT
public:
    enum Command {
        Active = 0,
        Show,
        Quit,
        Hide
    };
    explicit LingmoUIMenuApplication(MenuMessageProcessor *processor);
    ~LingmoUIMenuApplication() override;
    LingmoUIMenuApplication() = delete;
    LingmoUIMenuApplication(const LingmoUIMenuApplication& obj) = delete;
    LingmoUIMenuApplication(const LingmoUIMenuApplication&& obj) = delete;

private Q_SLOTS:
    void execCommand(LingmoUIMenuApplication::Command command);

private:
    static void registerQmlTypes();
    void startLingmoUIMenu();
    void initQmlEngine();
    void loadMenuUI();

    //注册dubs
    void initDbusService();

private:
    QQmlEngine *m_engine{nullptr};
    MenuWindow *m_mainWindow{nullptr};
    MenuDbusService *m_menuDbusService{nullptr};
};

class MenuMessageProcessor : public QObject
{
    Q_OBJECT
public:
    MenuMessageProcessor();
    static bool preprocessMessage(const QStringList& message);
    static QCommandLineOption showLingmoUIMenu;
    static QCommandLineOption quitLingmoUIMenu;

public Q_SLOTS:
    void processMessage(const QString &message);

Q_SIGNALS:
    void request(LingmoUIMenuApplication::Command command);
};

}

#endif //LINGMO_MENU_LINGMO_MENU_APPLICATION_H
