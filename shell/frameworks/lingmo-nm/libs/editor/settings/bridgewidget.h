/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_BRIDGE_WIDGET_H
#define LINGMO_NM_BRIDGE_WIDGET_H

#include "lingmonm_editor_export.h"

#include <QDBusPendingCallWatcher>
#include <QListWidgetItem>
#include <QMenu>
#include <QWidget>

#include <NetworkManagerQt/BridgeSetting>

#include "settingwidget.h"

namespace Ui
{
class BridgeWidget;
}

class LINGMONM_EDITOR_EXPORT BridgeWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit BridgeWidget(const QString &masterUuid,
                          const QString &masterId,
                          const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                          QWidget *parent = nullptr,
                          Qt::WindowFlags f = {});
    ~BridgeWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void addBridge(QAction *action);
    void currentBridgeChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void bridgeAddComplete(QDBusPendingCallWatcher *watcher);

    void editBridge();
    void deleteBridge();

    void populateBridges();

private:
    QString m_uuid;
    QString m_id;
    Ui::BridgeWidget *const m_ui;
    QMenu *const m_menu;
};

#endif // LINGMO_NM_BRIDGE_WIDGET_H
