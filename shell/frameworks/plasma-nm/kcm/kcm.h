/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_KCM_H
#define PLASMA_NM_KCM_H

#include "connectioneditortabwidget.h"
#include "handler.h"

#include <KCModule>

class KMessageWidget;
class QQuickWidget;
class QHBoxLayout;

class KCMNetworkmanagement : public KCModule
{
    Q_OBJECT
public:
    explicit KCMNetworkmanagement(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
    ~KCMNetworkmanagement() override;

    Q_INVOKABLE void onRequestCreateConnection(int connectionType, const QString &vpnType, const QString &specificType, bool shared);
    Q_INVOKABLE void onSelectedConnectionChanged(const QString &connectionPath);
    Q_INVOKABLE void onRequestExportConnection(const QString &connectionPath);
    Q_INVOKABLE void onRequestToChangeConnection(const QString &connectionName, const QString &connectionPath);

public Q_SLOTS:
    void defaults() override;
    void load() override;
    void save() override;

private Q_SLOTS:
    void onConnectionAdded(const QString &connection);

private:
    struct ImportResult {
        bool success;
        QString errorMessage;

        static ImportResult pass();
        static ImportResult fail(const QString &message);
    };

    void addConnection(const NetworkManager::ConnectionSettings::Ptr &connectionSettings);
    [[nodiscard]] KCMNetworkmanagement::ImportResult importVpn();
    void kcmChanged(bool kcmChanged);
    void loadConnectionSettings(const NetworkManager::ConnectionSettings::Ptr &connectionSettings);
    void resetSelection();

    QString m_currentConnectionPath;
    QString m_createdConnectionUuid;
    Handler *const m_handler;
    ConnectionEditorTabWidget *m_tabWidget = nullptr;
    QTimer *m_timer = nullptr;
    KMessageWidget *m_errorWidget;
    QHBoxLayout *m_layout = nullptr;
    QQuickWidget *m_connectionView = nullptr;
};

#endif
