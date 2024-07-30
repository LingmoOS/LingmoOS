/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractListModel>
#include <QQmlParserStatus>
#include <QSharedPointer>
#include <QTimer>
#include <qqmlregistration.h>

#include <pipewire/pipewire.h>

class PipeWireCore;

namespace MediaRole
{
Q_NAMESPACE
QML_ELEMENT
// Match PW_KEY_MEDIA_ROLE
enum Role : int {
    Unknown = -1,
    Movie,
    Music,
    Camera,
    Screen,
    Communication,
    Game,
    Notification,
    DSP,
    Production,
    Accessibility,
    Test,
    Last = Test,
};
Q_ENUM_NS(Role)
}

namespace NodeState
{
Q_NAMESPACE
QML_ELEMENT
// Match enum pw_node_state
enum State : int {
    Error = -1, /**< error state */
    Creating = 0, /**< the node is being created */
    Suspended = 1, /**< the node is suspended, the device might be closed */
    Idle = 2, /**< the node is running but there is no active port */
    Running = 3, /**< the node is running */
};
Q_ENUM_NS(State)
}

class MediaMonitor : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

    /**
     * Role for media streams. Only media streams with their @p PW_KEY_MEDIA_ROLE matching @p role will be monitored.
     * @default MediaRole::Unknown
     */
    Q_PROPERTY(MediaRole::Role role READ role WRITE setRole NOTIFY roleChanged)

    /**
     * Whether this monitor is able to detect apps using Pipewire for media access
     */
    Q_PROPERTY(bool detectionAvailable READ detectionAvailable NOTIFY detectionAvailableChanged)

    /**
     * Total count of media streams with the given role on the system
     */
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

    /**
     * The number of media streams that are currently used by applications
     */
    Q_PROPERTY(int runningCount READ runningCount NOTIFY runningCountChanged)

    /**
     * The number of media streams that are in idle state.
     */
    Q_PROPERTY(int idleCount READ idleCount NOTIFY idleCountChanged)

public:
    enum Role {
        StateRole = Qt::UserRole + 1,
        ObjectSerialRole,
    };
    Q_ENUM(Role);

    explicit MediaMonitor(QObject *parent = nullptr);
    ~MediaMonitor() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    MediaRole::Role role() const;
    void setRole(MediaRole::Role newRole);

    bool detectionAvailable() const;
    int runningCount() const;
    int idleCount() const;

Q_SIGNALS:
    void roleChanged();
    void detectionAvailableChanged();
    void countChanged();
    void runningCountChanged();
    void idleCountChanged();

private Q_SLOTS:
    void connectToCore();
    void onPipeBroken();

private:
    struct ProxyDeleter {
        void operator()(pw_proxy *proxy) const
        {
            MediaMonitor::onProxyDestroy(pw_proxy_get_user_data(proxy));
            pw_proxy_destroy(proxy);
        }
    };

    static void onRegistryEventGlobal(void *data, uint32_t id, uint32_t permissions, const char *type, uint32_t version, const spa_dict *props);
    static void onRegistryEventGlobalRemove(void *data, uint32_t id);
    static void onProxyDestroy(void *data);
    static void onNodeEventInfo(void *data, const pw_node_info *info);

    static void readProps(const spa_dict *props, pw_proxy *proxy, bool emitSignal);

    void classBegin() override;
    void componentComplete() override;

    void disconnectFromCore();
    void reconnectOnIdle();
    void updateState();

    static pw_registry_events s_pwRegistryEvents;
    static pw_proxy_events s_pwProxyEvents;
    static pw_node_events s_pwNodeEvents;

    bool m_componentReady = false;
    MediaRole::Role m_role = MediaRole::Unknown;
    bool m_detectionAvailable = false;
    int m_runningCount = 0;
    int m_idleCount = 0;

    QSharedPointer<PipeWireCore> m_pwCore;
    pw_registry *m_registry = nullptr;
    spa_hook m_registryListener;
    std::vector<std::unique_ptr<pw_proxy, ProxyDeleter>> m_nodeList;
    QTimer m_reconnectTimer;

    bool m_inDestructor = false;
};
