/*
    SPDX-FileCopyrightText: 2013-2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONNECTION_EDITOR_BASE_H
#define PLASMA_NM_CONNECTION_EDITOR_BASE_H

#include "plasmanm_editor_export.h"

#include <QDBusPendingCallWatcher>
#include <QWidget>

#include <NetworkManagerQt/ConnectionSettings>

class ConnectionWidget;
class SettingWidget;
class WifiSecurity;

class PLASMANM_EDITOR_EXPORT ConnectionEditorBase : public QWidget
{
    Q_OBJECT
public:
    explicit ConnectionEditorBase(const NetworkManager::ConnectionSettings::Ptr &connection, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    explicit ConnectionEditorBase(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~ConnectionEditorBase() override;

    // When reimplementing do not forget to call the base method as well to initialize widgets
    virtual void setConnection(const NetworkManager::ConnectionSettings::Ptr &connection);

    NMVariantMapMap setting() const;

    // Returns whether the editor is fully initialized (including secrets)
    bool isInitialized() const;

    // Returns whether the filled values are valid
    bool isValid() const;

Q_SIGNALS:
    // The default value is supposed to be false, watch this property for validity change after
    // proper initialization with secrets
    void validityChanged(bool valid);

    // Emitted when user changed configuration
    void settingChanged();

private Q_SLOTS:
    void replyFinished(QDBusPendingCallWatcher *watcher);
    void validChanged(bool valid);
    void onAllUsersChanged();

protected:
    // Subclassed widget is supposed to take care of layouting for setting widgets
    virtual void addWidget(QWidget *widget, const QString &text) = 0;

    // Subclassed widget is supposed to provide an UI (input label) for editing connection name separately
    virtual QString connectionName() const = 0;

    // Subclassed widget is supposed to call initialization after the UI is initialized
    void initialize();

private:
    bool m_initialized;
    bool m_valid;
    int m_pendingReplies;
    NetworkManager::ConnectionSettings::Ptr m_connection;
    ConnectionWidget *m_connectionWidget = nullptr;
    QList<SettingWidget *> m_settingWidgets;
    WifiSecurity *m_wifiSecurity = nullptr;

    void addConnectionWidget(ConnectionWidget *widget, const QString &text);
    void addSettingWidget(SettingWidget *widget, const QString &text);
};

#endif // PLASMA_NM_CONNECTION_EDITOR_BASE_H
