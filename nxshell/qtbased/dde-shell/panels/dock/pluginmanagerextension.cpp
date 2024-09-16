// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanagerextension_p.h"
#include "pluginmanagerintegration_p.h"
#include "constants.h"

#include <cstdint>

#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>
#include <QtWaylandCompositor/QWaylandCompositor>

#include <QJsonObject>
#include <QJsonParseError>

PluginSurface::PluginSurface(PluginManager* manager, const QString& pluginId, const QString& itemKey, const QString &displayName, int pluginFlags, int pluginType, int sizePolicy, QWaylandSurface *surface, const QWaylandResource &resource)
    : m_manager(manager)
    , m_surface(surface)
    , m_itemKey(itemKey)
    , m_pluginId(pluginId)
    , m_displayName(displayName)
    , m_flags(pluginFlags)
    , m_pluginType(pluginType)
    , m_sizePolicy(sizePolicy)
    , m_margins(0)
{
    init(resource.resource());
    setExtensionContainer(surface);
    QWaylandCompositorExtension::initialize();

    connect(surface, &QWaylandSurface::bufferSizeChanged, this, &PluginSurface::sizeChanged);
}

QWaylandQuickShellIntegration* PluginSurface::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new PluginManagerIntegration(item);
}

QWaylandSurface* PluginSurface::surface() const
{
    return m_surface;
}

QString PluginSurface::pluginId() const
{
    return m_pluginId;
}

QString PluginSurface::itemKey() const
{
    return m_itemKey;
}

QString PluginSurface::displayName() const
{
    return m_displayName;
}

uint32_t PluginSurface::pluginType() const
{
    return m_pluginType;
}

uint32_t PluginSurface::pluginFlags() const
{
    return m_flags;
}

uint32_t PluginSurface::pluginSizePolicy () const
{
    return m_sizePolicy;
}

QSize PluginSurface::pluginSize() const
{
    return m_surface->bufferSize() / qApp->devicePixelRatio();
}

QString PluginSurface::dccIcon() const
{
    return m_dccIcon;
}

void PluginSurface::setItemActive(bool isActive)
{
    if (m_isItemActive == isActive) {
        return;
    }

    m_isItemActive = isActive;
    Q_EMIT itemActiveChanged();
}

bool PluginSurface::isItemActive() const
{
    return m_isItemActive;
}

void PluginSurface::updatePluginGeometry(const QRect &geometry)
{
    send_geometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void PluginSurface::plugin_mouse_event(QtWaylandServer::plugin::Resource *resource, int32_t type)
{
    qInfo() << "server plugin surface receive mouse event:" << type;
    Q_EMIT recvMouseEvent((QEvent::Type)type);
}

void PluginSurface::plugin_dcc_icon(Resource *resource, const QString &icon)
{
    qInfo() << "dcc_icon:" << icon;
    m_dccIcon = icon;
}

void PluginSurface::plugin_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    Q_EMIT aboutToDestroy();
    m_manager->removePluginSurface(this);
    delete this;
}

void PluginSurface::plugin_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void PluginSurface::setGlobalPos(const QPoint &pos)
{
    QRect g = qApp->primaryScreen() ? qApp->primaryScreen()->geometry() : QRect();
    for (auto *screen : qApp->screens())
    {
        const QRect &sg = screen->geometry();
        if (sg.contains(pos))
        {
            g = sg;
            break;
        }
    }

    auto p = g.topLeft() + (pos - g.topLeft()) * qApp->devicePixelRatio();
    send_raw_global_pos(p.x(), p.y());
}

PluginPopup::PluginPopup(PluginManager* manager, const QString &pluginId, const QString &itemKey, int x, int y, int popupType, QWaylandSurface *surface, const QWaylandResource &resource)
    : m_manager(manager)
    , m_surface(surface)
    , m_pluginId(pluginId)
    , m_itemKey(itemKey)
    , m_popupType(popupType)
{
    init(resource.resource());
    setExtensionContainer(surface);
    QWaylandCompositorExtension::initialize();
}

QWaylandSurface* PluginPopup::surface() const
{
    return m_surface;
}

QWaylandQuickShellIntegration* PluginPopup::createIntegration(QWaylandQuickShellSurfaceItem *item)
{
    return new PluginPopupIntegration(item);
}

void PluginPopup::updatePluginGeometry(const QRect &geometry)
{
    send_geometry(geometry.x(), geometry.y(), geometry.width(), geometry.height());
}

void PluginPopup::setEmbedPanelMinHeight(int height)
{
    m_manager->setEmbedPanelMinHeight(height);
}

QString PluginPopup::pluginId() const
{
    return m_pluginId;
}

QString PluginPopup::itemKey() const
{
    return m_itemKey;
}

int32_t PluginPopup::x() const
{
    return m_x;
}

int32_t PluginPopup::y() const
{
    return m_y;
}

void PluginPopup::setX(int32_t x)
{
    m_x = x;
    Q_EMIT xChanged();
}

void PluginPopup::setY(int32_t y)
{
    m_y = y;
    Q_EMIT yChanged();
}

int32_t PluginPopup::popupType() const
{
    return m_popupType;
}

void PluginPopup::plugin_popup_set_position(Resource *resource, int32_t x, int32_t y)
{
    setX(x);
    setY(y);
}

void PluginPopup::plugin_popup_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    // TODO why we get a same address with the object when new PluginPopup, if we delete the object.
    Q_EMIT aboutToDestroy();
    delete this;
}

void PluginPopup::plugin_popup_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}
PluginManager::PluginManager(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate(compositor)
{
}

void PluginManager::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);
}

void PluginManager::updateDockOverflowState(int state)
{
    QJsonObject obj;
    obj[dock::MSG_TYPE] = dock::MSG_UPDATE_OVERFLOW_STATE;
    obj[dock::MSG_DATA] = state;

    sendEventMsg(toJson(obj));
}

void PluginManager::setPopupMinHeight(int height)
{
    setEmbedPanelMinHeight(height);
}

uint32_t PluginManager::dockPosition() const
{
    return m_dockPosition;
}

void PluginManager::setDockPosition(uint32_t dockPosition)
{
    if (m_dockPosition == dockPosition)
        return;

    m_dockPosition = dockPosition;
    foreach (PluginSurface *plugin, m_pluginSurfaces) {
        Resource *target = resourceMap().value(plugin->surface()->waylandClient());
        if (target) {
            send_position_changed(target->handle, m_dockPosition);
        }
    }
}

uint32_t PluginManager::dockColorTheme() const
{
    return m_dockColorTheme;
}

void PluginManager::setDockColorTheme(uint32_t type)
{
    if (type == m_dockColorTheme)
        return;

    m_dockColorTheme = type;
    foreach (PluginSurface *plugin, m_pluginSurfaces) {
        Resource *target = resourceMap().value(plugin->surface()->waylandClient());
        if (target) {
            send_color_theme_changed(target->handle, m_dockColorTheme);
        }
    }
}

void PluginManager::setEmbedPanelMinHeight(int height)
{
    if (m_popupMinHeight == height)
        return;
    m_popupMinHeight = height;

    sendEventMsg(popupMinHeightMsg());
}

void PluginManager::plugin_manager_v1_request_message(Resource *resource, const QString &plugin_id, const QString &item_key, const QString &msg)
{
    qInfo() << "server pluginManager receive client:" << plugin_id << item_key << " msg:" << msg;
    PluginSurface *dstPlugin = nullptr;
    for (PluginSurface *plugin : m_pluginSurfaces) {
        if (plugin->pluginId() == plugin_id && plugin->itemKey() == item_key) {
            dstPlugin = plugin;
            break;
        }
    }

    if (!dstPlugin) {
        return;
    }

    auto rootObj = getRootObj(msg);
    if (rootObj.isEmpty()) {
        return;
    }

    const auto &msgType = rootObj.value(dock::MSG_TYPE);
    if (msgType == dock::MSG_SUPPORT_FLAG_CHANGED) {
        // todo
    } else if (msgType == dock::MSG_ITEM_ACTIVE_STATE) {
        dstPlugin->setItemActive(rootObj.value(dock::MSG_DATA).toBool());
    } else if (msgType == dock::MSG_UPDATE_TOOLTIPS_VISIBLE) {

    }
}

void PluginManager::plugin_manager_v1_create_plugin(Resource *resource, const QString &pluginId, const QString &itemKey, const QString &display_name, int32_t plugin_flags, int32_t type, int32_t size_policy, struct ::wl_resource *surface, uint32_t id)
{
    QWaylandSurface *qwaylandSurface = QWaylandSurface::fromResource(surface);

    QWaylandResource shellSurfaceResource(wl_resource_create(resource->client(), &::plugin_interface,
                                                           wl_resource_get_version(resource->handle), id));

    send_position_changed(resource->handle, m_dockPosition);
    send_color_theme_changed(resource->handle, m_dockColorTheme);

    auto plugin = new PluginSurface(this, pluginId, itemKey, display_name, plugin_flags, type, size_policy, qwaylandSurface, shellSurfaceResource);
    m_pluginSurfaces << plugin;
    Q_EMIT pluginSurfaceCreated(plugin);

    sendEventMsg(resource, dockSizeMsg());
    sendEventMsg(resource, popupMinHeightMsg());
}

void PluginManager::plugin_manager_v1_create_popup_at(Resource *resource, const QString &pluginId, const QString &itemKey, int32_t type, int32_t x, int32_t y, struct ::wl_resource *surface, uint32_t id)
{
    QWaylandSurface *qwaylandSurface = QWaylandSurface::fromResource(surface);
    QWaylandResource shellSurfaceResource(wl_resource_create(resource->client(), &::plugin_popup_interface,
                                                           wl_resource_get_version(resource->handle), id));

    auto plugin = new PluginPopup(this, pluginId, itemKey, x, y, type, qwaylandSurface, shellSurfaceResource);
    plugin->setX(x), plugin->setY(y);
    Q_EMIT pluginPopupCreated(plugin);
}

QJsonObject PluginManager::getRootObj(const QString &jsonStr) {
    QJsonParseError jsonParseError;
    const QJsonDocument &resultDoc = QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || resultDoc.isEmpty()) {
        qWarning() << "Result json parse error";
        return QJsonObject();
    }

    return resultDoc.object();
}

QString PluginManager::toJson(const QJsonObject &jsonObj)
{
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

void PluginManager::sendEventMsg(const QString &msg)
{
    foreach (PluginSurface *plugin, m_pluginSurfaces) {
        Resource *target = resourceMap().value(plugin->surface()->waylandClient());
        sendEventMsg(target, msg);
    }
}

void PluginManager::sendEventMsg(Resource *target, const QString &msg)
{
    if (target && !msg.isEmpty()) {
        send_event_message(target->handle, msg);
    }
}

int PluginSurface::margins() const
{
    return m_margins;
}

void PluginSurface::setMargins(int newMargins)
{
    if (m_margins == newMargins)
        return;
    m_margins = newMargins;
    send_margin(m_margins);

    emit marginsChanged();
}

QSize PluginManager::dockSize() const
{
    return m_dockSize;
}

void PluginManager::setDockSize(const QSize &newDockSize)
{
    if (m_dockSize == newDockSize)
        return;
    m_dockSize = newDockSize;
    sendEventMsg(dockSizeMsg());
    emit dockSizeChanged();
}

void PluginManager::removePluginSurface(PluginSurface *plugin)
{
    Q_EMIT pluginSurfaceDestroyed(plugin);
    m_pluginSurfaces.removeAll(plugin);
}

QString PluginManager::dockSizeMsg() const
{
    if (m_dockSize.isEmpty())
        return QString();

    QJsonObject sizeData;
    sizeData["width"] = m_dockSize.width();
    sizeData["height"] = m_dockSize.height();

    QJsonObject obj;
    obj[dock::MSG_TYPE] = dock::MSG_DOCK_PANEL_SIZE_CHANGED;
    obj[dock::MSG_DATA] = sizeData;
    return toJson(obj);
}

QString PluginManager::popupMinHeightMsg() const
{
    if (m_popupMinHeight <= 0)
        return QString();

    QJsonObject obj;
    obj[dock::MSG_TYPE] = dock::MSG_SET_APPLET_MIN_HEIGHT;
    obj[dock::MSG_DATA] = m_popupMinHeight;

    return toJson(obj);
}
