// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtWaylandCompositor/QWaylandShellSurfaceTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandResource>
#include <cstdint>

#include "qwayland-server-plugin-manager-v1.h"

class PluginSurface;
class PluginPopup;
class PluginManager : public QWaylandCompositorExtensionTemplate<PluginManager>, public QtWaylandServer::plugin_manager_v1
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(uint32_t dockPosition READ dockPosition WRITE setDockPosition)
    Q_PROPERTY(uint32_t dockColorTheme READ dockColorTheme WRITE setDockColorTheme)
    Q_PROPERTY(QSize dockSize READ dockSize WRITE setDockSize NOTIFY dockSizeChanged FINAL)

public:
    PluginManager(QWaylandCompositor *compositor = nullptr);
    void initialize() override;

    Q_INVOKABLE void updateDockOverflowState(int state);
    Q_INVOKABLE void setPopupMinHeight(int height);

    uint32_t dockPosition() const;
    void setDockPosition(uint32_t dockPosition);

    uint32_t dockColorTheme() const;
    void setDockColorTheme(uint32_t type);

    void setEmbedPanelMinHeight(int height);

    QSize dockSize() const;
    void setDockSize(const QSize &newDockSize);

    void removePluginSurface(PluginSurface *plugin);

Q_SIGNALS:
    void pluginPopupCreated(PluginPopup*);
    void pluginSurfaceCreated(PluginSurface*);
    void pluginSurfaceDestroyed(PluginSurface*);
    void messageRequest(PluginSurface *, const QString &msg);
    void dockSizeChanged();

protected:
    virtual void plugin_manager_v1_request_message(Resource *resource, const QString &plugin_id, const QString &item_key, const QString &msg) override;
    virtual void plugin_manager_v1_create_popup_at(Resource *resource, const QString &plugin_id, const QString &item_key, int32_t type, int32_t x, int32_t y, struct ::wl_resource *surface, uint32_t id) override;
    virtual void plugin_manager_v1_create_plugin(Resource *resource, const QString &plugin_id, const QString &item_key, const QString &display_name, int32_t plugin_flags, int32_t type, int32_t size_policy, struct ::wl_resource *surface, uint32_t id) override;

private:
    static QJsonObject getRootObj(const QString &jsonStr);
    static QString toJson(const QJsonObject &jsonObj);
    void sendEventMsg(const QString &msg);
    void sendEventMsg(Resource *target, const QString &msg);
    QString dockSizeMsg() const;
    QString popupMinHeightMsg() const;

private:
    QList<PluginSurface*> m_pluginSurfaces;

    uint32_t m_dockPosition;
    uint32_t m_dockColorTheme;
    QSize m_dockSize;
    int m_popupMinHeight = 0;
};

class PluginSurface : public QWaylandShellSurfaceTemplate<PluginSurface>, public QtWaylandServer::plugin
{
    Q_OBJECT
    Q_PROPERTY(QString pluginId READ pluginId CONSTANT)
    Q_PROPERTY(QString itemKey READ itemKey CONSTANT)
    Q_PROPERTY(uint32_t pluginFlags READ pluginFlags CONSTANT)
    Q_PROPERTY(uint32_t pluginType READ pluginType CONSTANT)
    Q_PROPERTY(uint32_t pluginSizePolicy READ pluginSizePolicy CONSTANT)
    Q_PROPERTY(QString displayName READ displayName CONSTANT)
    Q_PROPERTY(QSize size READ pluginSize NOTIFY sizeChanged)
    Q_PROPERTY(bool isItemActive WRITE setItemActive READ isItemActive NOTIFY itemActiveChanged)
    Q_PROPERTY(QString dccIcon READ dccIcon CONSTANT)
    Q_PROPERTY(int margins READ margins WRITE setMargins NOTIFY marginsChanged FINAL)

public:
    PluginSurface(const PluginSurface &) = delete;
    PluginSurface(PluginSurface &&) = delete;
    PluginSurface &operator=(const PluginSurface &) = delete;
    PluginSurface &operator=(PluginSurface &&) = delete;
    PluginSurface(PluginManager *shell, const QString &pluginId,
                  const QString &itemKey, const QString &displayName,
                  int pluginFlags, int pluginType, int sizePolicy,
                  QWaylandSurface *surface, const QWaylandResource &resource);
    QWaylandQuickShellIntegration *
    createIntegration(QWaylandQuickShellSurfaceItem *item) override;

    QWaylandSurface *surface() const;

    QString pluginId() const;
    QString itemKey() const;
    QString displayName() const;
    QString contextMenu() const;
    uint32_t pluginType() const;
    uint32_t pluginFlags() const;
    uint32_t pluginSizePolicy() const;
    QSize pluginSize() const;
    QString dccIcon() const;

    void setItemActive(bool isActive);
    bool isItemActive() const;

    Q_INVOKABLE void updatePluginGeometry(const QRect &geometry);
    Q_INVOKABLE void setGlobalPos(const QPoint &pos);

    int margins() const;
    void setMargins(int newMargins);

signals:
    void itemActiveChanged();
    void sizeChanged();
    void recvMouseEvent(QEvent::Type type);

    void marginsChanged();
    void aboutToDestroy();

protected:
    virtual void plugin_mouse_event(Resource *resource, int32_t type) override;
    virtual void plugin_dcc_icon(Resource *resource, const QString &icon) override;
    virtual void plugin_destroy_resource(Resource *resource) override;
    virtual void plugin_destroy(Resource *resource) override;

private:
    PluginManager* m_manager;
    QWaylandSurface* m_surface;

    QString m_itemKey;
    QString m_pluginId;
    QString m_displayName;
    QString m_dccIcon;

    uint32_t m_flags;
    uint32_t m_pluginType;
    uint32_t m_sizePolicy;

    bool m_isItemActive = false;
    int m_margins = 0;
};

class PluginPopup : public QWaylandShellSurfaceTemplate<PluginPopup>, public QtWaylandServer::plugin_popup
{
    Q_OBJECT
    Q_PROPERTY(int32_t x READ x WRITE setX NOTIFY xChanged)
    Q_PROPERTY(int32_t y READ y WRITE setY NOTIFY yChanged)
    Q_PROPERTY(QString pluginId READ pluginId)
    Q_PROPERTY(QString itemKey READ itemKey)
    Q_PROPERTY(int32_t popupType READ popupType)

public:
    PluginPopup(PluginManager* shell, const QString &pluginId, const QString &itemKey, int x, int y, int popupType,
                QWaylandSurface *surface, const QWaylandResource &resource);
    QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) override;

    QWaylandSurface *surface() const;

    Q_INVOKABLE void close() { send_close(); }
    Q_INVOKABLE void updatePluginGeometry(const QRect &geometry);
    Q_INVOKABLE void setEmbedPanelMinHeight(int height);

    QString pluginId() const;
    QString itemKey() const;

    int32_t x() const;
    int32_t y() const;

    void setX(int32_t x);
    void setY(int32_t y);

    int32_t popupType() const;

signals:
    void aboutToDestroy();

protected:
    virtual void plugin_popup_set_position(Resource *resource, int32_t x, int32_t y) override;
    virtual void plugin_popup_destroy_resource(Resource *resource) override;
    virtual void plugin_popup_destroy(Resource *resource) override;

Q_SIGNALS:
    void xChanged();
    void yChanged();

private:
    PluginManager* m_manager;
    QWaylandSurface* m_surface;

    QString m_itemKey;
    QString m_pluginId;
    int32_t m_popupType;

    int32_t m_x;
    int32_t m_y;
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(PluginManager)
