/*
    SPDX-FileCopyrightText: 2023 Collabora Ltd.
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediamonitor.h"

#include <cstdlib>

#include <QDebug>
#include <QMetaEnum>

#include "pipewirecore_p.h"

namespace
{
struct Node {
    MediaMonitor *monitor;
    QString deviceName;
    QString objectSerial;
    NodeState::State state = NodeState::Error;
    spa_hook proxyListener;
    spa_hook objectListener;
};

void updateProp(const spa_dict *props, const char *key, QString &prop, int role, QList<int> &changedRoles)
{
    const char *new_prop = spa_dict_lookup(props, key);
    if (!new_prop) {
        return;
    }
    if (QString newProp = QString::fromUtf8(new_prop); prop != newProp) {
        prop = std::move(newProp);
        changedRoles << role;
    }
}
}

pw_registry_events MediaMonitor::s_pwRegistryEvents = {
    .version = PW_VERSION_REGISTRY_EVENTS,
    .global = &MediaMonitor::onRegistryEventGlobal,
    .global_remove = &MediaMonitor::onRegistryEventGlobalRemove,
};

pw_proxy_events MediaMonitor::s_pwProxyEvents = {
    .version = PW_VERSION_PROXY_EVENTS,
    .destroy = &MediaMonitor::onProxyDestroy,
};

pw_node_events MediaMonitor::s_pwNodeEvents = {
    .version = PW_VERSION_NODE_EVENTS,
    .info = &MediaMonitor::onNodeEventInfo,
};

MediaMonitor::MediaMonitor(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &QAbstractListModel::rowsInserted, this, &MediaMonitor::countChanged);
    connect(this, &QAbstractListModel::rowsRemoved, this, &MediaMonitor::countChanged);
    connect(this, &QAbstractListModel::modelReset, this, &MediaMonitor::countChanged);

    m_reconnectTimer.setSingleShot(true);
    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &MediaMonitor::connectToCore);
}

MediaMonitor::~MediaMonitor()
{
    m_inDestructor = true;
    disconnectFromCore();
}

QVariant MediaMonitor::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    pw_proxy *const proxy = m_nodeList.at(index.row()).get();
    const Node *const node = static_cast<Node *>(pw_proxy_get_user_data(proxy));

    switch (role) {
    case Qt::DisplayRole:
        return node->deviceName;
    case StateRole:
        return node->state;
    case ObjectSerialRole:
        return node->objectSerial;
    default:
        return QVariant();
    }
}

int MediaMonitor::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_nodeList.size();
}

QHash<int, QByteArray> MediaMonitor::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {StateRole, QByteArrayLiteral("state")},
        {ObjectSerialRole, QByteArrayLiteral("objectSerial")},
    };
}

MediaRole::Role MediaMonitor::role() const
{
    return m_role;
}

void MediaMonitor::setRole(MediaRole::Role newRole)
{
    if (m_role == newRole) {
        return;
    }
    Q_ASSERT(newRole >= MediaRole::Unknown && newRole <= MediaRole::Last);
    m_role = std::clamp(newRole, MediaRole::Unknown, MediaRole::Last);

    if (m_reconnectTimer.isActive()) {
        Q_EMIT roleChanged();
        return;
    }

    disconnectFromCore();
    connectToCore();

    Q_EMIT roleChanged();
}

bool MediaMonitor::detectionAvailable() const
{
    return m_detectionAvailable;
}

int MediaMonitor::runningCount() const
{
    return m_runningCount;
}

int MediaMonitor::idleCount() const
{
    return m_idleCount;
}

void MediaMonitor::connectToCore()
{
    Q_ASSERT(!m_registry);
    if (!m_componentReady || m_role == MediaRole::Unknown) {
        return;
    }

    if (!m_pwCore) {
        m_pwCore = PipeWireCore::fetch(0);
    }
    if (!m_pwCore->error().isEmpty()) {
        qDebug() << "received error while creating the stream" << m_pwCore->error() << "Media monitor will not work.";
        m_pwCore.clear();
        m_reconnectTimer.start();
        return;
    }

    m_registry = pw_core_get_registry(**m_pwCore.get(), PW_VERSION_REGISTRY, 0);
    pw_registry_add_listener(m_registry, &m_registryListener, &s_pwRegistryEvents, this /*user data*/);

    m_detectionAvailable = true;
    Q_EMIT detectionAvailableChanged();

    connect(m_pwCore.get(), &PipeWireCore::pipeBroken, this, &MediaMonitor::onPipeBroken);
}

void MediaMonitor::onPipeBroken()
{
    m_registry = nullptr; // When pipe is broken, the registered object is also gone
    disconnectFromCore();
    reconnectOnIdle();
}

void MediaMonitor::onRegistryEventGlobal(void *data, uint32_t id, uint32_t /*permissions*/, const char *type, uint32_t /*version*/, const spa_dict *props)
{
    auto monitor = static_cast<MediaMonitor *>(data);

    if (!props || !(spa_streq(type, PW_TYPE_INTERFACE_Node))) {
        return;
    }

    static const QMetaEnum metaEnum = QMetaEnum::fromType<MediaRole::Role>();
    if (const char *prop_str = spa_dict_lookup(props, PW_KEY_MEDIA_ROLE); !prop_str || (strcmp(prop_str, metaEnum.valueToKey(monitor->m_role)) != 0)) {
        return;
    }

    auto proxy = static_cast<pw_proxy *>(pw_registry_bind(monitor->m_registry, id, PW_TYPE_INTERFACE_Node, PW_VERSION_NODE, sizeof(Node)));
    auto node = static_cast<Node *>(pw_proxy_get_user_data(proxy));
    node->monitor = monitor;
    readProps(props, proxy, false);

    monitor->beginInsertRows(QModelIndex(), monitor->m_nodeList.size(), monitor->m_nodeList.size());
    monitor->m_nodeList.emplace_back(proxy);
    monitor->endInsertRows();

    pw_proxy_add_listener(proxy, &node->proxyListener, &s_pwProxyEvents, node);
    pw_proxy_add_object_listener(proxy, &node->objectListener, &s_pwNodeEvents, node);
}

void MediaMonitor::onRegistryEventGlobalRemove(void *data, uint32_t id)
{
    auto monitor = static_cast<MediaMonitor *>(data);
    const auto proxyIt = std::find_if(monitor->m_nodeList.cbegin(), monitor->m_nodeList.cend(), [id](const auto &proxy) {
        return pw_proxy_get_bound_id(proxy.get()) == id;
    });
    if (proxyIt == monitor->m_nodeList.cend()) {
        return;
    }
    const int row = std::distance(monitor->m_nodeList.cbegin(), proxyIt);
    monitor->beginRemoveRows(QModelIndex(), row, row);
    monitor->m_nodeList.erase(proxyIt);
    monitor->endRemoveRows();
}

void MediaMonitor::onProxyDestroy(void *data)
{
    auto node = static_cast<Node *>(data);
    spa_hook_remove(&node->proxyListener);
    spa_hook_remove(&node->objectListener);
}

void MediaMonitor::onNodeEventInfo(void *data, const pw_node_info *info)
{
    auto node = static_cast<Node *>(data);

    NodeState::State newState;
    switch (info->state) {
    case PW_NODE_STATE_ERROR:
        newState = NodeState::Error;
        break;
    case PW_NODE_STATE_CREATING:
        newState = NodeState::Creating;
        break;
    case PW_NODE_STATE_SUSPENDED:
        newState = NodeState::Suspended;
        break;
    case PW_NODE_STATE_IDLE:
        newState = NodeState::Idle;
        break;
    case PW_NODE_STATE_RUNNING:
        newState = NodeState::Running;
        break;
    default:
        Q_ASSERT_X(false, "MediaMonitor", "Unknown node state");
        return;
    }

    const auto proxyIt = std::find_if(node->monitor->m_nodeList.cbegin(), node->monitor->m_nodeList.cend(), [data](const auto &proxy) {
        return pw_proxy_get_user_data(proxy.get()) == data;
    });
    if (node->state != newState) {
        node->state = newState;
        const int row = std::distance(node->monitor->m_nodeList.cbegin(), proxyIt);
        const QModelIndex idx = node->monitor->index(row, 0);
        node->monitor->dataChanged(idx, idx, {StateRole});
    }

    readProps(info->props, proxyIt->get(), true);
    node->monitor->updateState();
}

void MediaMonitor::readProps(const spa_dict *props, pw_proxy *proxy, bool emitSignal)
{
    auto node = static_cast<Node *>(pw_proxy_get_user_data(proxy));
    QList<int> changedRoles;

    updateProp(props, PW_KEY_NODE_NICK, node->deviceName, Qt::DisplayRole, changedRoles);
    if (node->deviceName.isEmpty()) {
        changedRoles.clear();
        updateProp(props, PW_KEY_NODE_NAME, node->deviceName, Qt::DisplayRole, changedRoles);
    }
    if (node->deviceName.isEmpty()) {
        changedRoles.clear();
        updateProp(props, PW_KEY_NODE_DESCRIPTION, node->deviceName, Qt::DisplayRole, changedRoles);
    }

    updateProp(props, PW_KEY_OBJECT_SERIAL, node->objectSerial, ObjectSerialRole, changedRoles);

    if (emitSignal && !changedRoles.empty()) {
        const auto proxyIt = std::find_if(node->monitor->m_nodeList.cbegin(), node->monitor->m_nodeList.cend(), [proxy](const auto &p) {
            return p.get() == proxy;
        });
        const int row = std::distance(node->monitor->m_nodeList.cbegin(), proxyIt);
        const QModelIndex idx = node->monitor->index(row, 0);
        node->monitor->dataChanged(idx, idx, changedRoles);
    }
}

void MediaMonitor::classBegin()
{
}

void MediaMonitor::componentComplete()
{
    m_componentReady = true;
    connectToCore();
}

void MediaMonitor::disconnectFromCore()
{
    if (!m_pwCore) {
        return;
    }

    if (m_runningCount) {
        m_runningCount = 0;
        Q_EMIT runningCountChanged();
    }

    if (m_idleCount) {
        m_idleCount = 0;
        Q_EMIT idleCountChanged();
    }

    m_detectionAvailable = false;
    Q_EMIT detectionAvailableChanged();

    if (!m_inDestructor) {
        beginResetModel();
        m_nodeList.clear();
        endResetModel();
    }

    if (m_registry) {
        pw_proxy_destroy(reinterpret_cast<struct pw_proxy *>(m_registry));
        spa_hook_remove(&m_registryListener);
        m_registry = nullptr;
    }
    disconnect(m_pwCore.get(), &PipeWireCore::pipeBroken, this, &MediaMonitor::onPipeBroken);
}

void MediaMonitor::reconnectOnIdle()
{
    if (m_reconnectTimer.isActive()) {
        return;
    }

    static unsigned retryCount = 0;
    if (retryCount > 100) {
        qWarning() << "Camera indicator receives too many errors. Aborting...";
        return;
    }
    ++retryCount;
    m_reconnectTimer.start();
}

void MediaMonitor::updateState()
{
    int newIdleCount = 0;
    int newRunningCount = 0;
    for (const auto &proxy : m_nodeList) {
        switch (static_cast<Node *>(pw_proxy_get_user_data(proxy.get()))->state) {
        case NodeState::Idle:
            ++newIdleCount;
            break;
        case NodeState::Running:
            ++newRunningCount;
            break;
        default:
            break;
        }
    }

    const bool idleChanged = m_idleCount != newIdleCount;
    m_idleCount = newIdleCount;
    const bool runningChanged = m_runningCount != newRunningCount;
    m_runningCount = newRunningCount;

    if (idleChanged) {
        Q_EMIT idleCountChanged();
    }
    if (runningChanged) {
        Q_EMIT runningCountChanged();
    }
}

#include "moc_mediamonitor.cpp"
