/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_TEAM_WIDGET_H
#define PLASMA_NM_TEAM_WIDGET_H

#include "plasmanm_editor_export.h"

#include <QDBusPendingCallWatcher>
#include <QListWidgetItem>
#include <QMenu>
#include <QWidget>

#include "settingwidget.h"

#include <NetworkManagerQt/TeamSetting>

namespace Ui
{
class TeamWidget;
}

class PLASMANM_EDITOR_EXPORT TeamWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit TeamWidget(const QString &masterUuid,
                        const QString &masterId,
                        const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = {});
    ~TeamWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void addTeam(QAction *action);
    void currentTeamChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void teamAddComplete(QDBusPendingCallWatcher *watcher);

    void editTeam();
    void deleteTeam();

    void populateTeams();

    void importConfig();

private:
    QString m_uuid;
    QString m_id;
    Ui::TeamWidget *const m_ui;
    QMenu *const m_menu;
};

#endif // PLASMA_NM_TEAM_WIDGET_H
