/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_BOND_WIDGET_H
#define LINGMO_NM_BOND_WIDGET_H

#include "lingmonm_editor_export.h"

#include <QDBusPendingCallWatcher>
#include <QListWidgetItem>
#include <QMenu>
#include <QWidget>

#include <NetworkManagerQt/BondSetting>

#include "settingwidget.h"

namespace Ui
{
class BondWidget;
}

class LINGMONM_EDITOR_EXPORT BondWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit BondWidget(const QString &masterUuid,
                        const QString &masterId,
                        const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = {});
    ~BondWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void addBond(QAction *action);
    void currentBondChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void bondAddComplete(QDBusPendingCallWatcher *watcher);

    void editBond();
    void deleteBond();

    void populateBonds();

private:
    QString m_uuid;
    QString m_id;
    Ui::BondWidget *const m_ui;
    QMenu *const m_menu;
};

#endif // LINGMO_NM_BOND_WIDGET_H
