// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DMainWindow>

#include <QWidget>
#include <QStackedLayout>
#include <QCloseEvent>

class DBusWorker;
class InitialWidget;
class UpgradeCheckWidget;
class ImagePreparationWidget;
class SystemUpgradeWidget;
class BackgroundFrame;
class MigrateWidget;

class MainWindow: public DTK_WIDGET_NAMESPACE::DMainWindow
{
    Q_OBJECT
public:
    bool    m_askForClose;

    explicit MainWindow(QWidget *parent = nullptr);
    void initUI();
    void initConnections();
    void showFailedWindow(QString msg);
    void startMigration();
    static MainWindow* getInstance(QWidget *parent = nullptr);

protected:
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void goToInitWidget();
    void goToMainContentWidget();

private:
    static MainWindow *m_instance;
    BackgroundFrame         *m_backgroundFrame;
    QWidget                 *m_centerWidget;
    DBusWorker              *m_dbusWorker;
    InitialWidget           *m_initialWidget;
    QStackedLayout          *m_stackedlayout;
    UpgradeCheckWidget      *m_upgradeCheckWidget;
    ImagePreparationWidget  *m_imagePreparationWidget;
    SystemUpgradeWidget     *m_systemUpgradeWidget;
    MigrateWidget           *m_migrateWidget;
};
