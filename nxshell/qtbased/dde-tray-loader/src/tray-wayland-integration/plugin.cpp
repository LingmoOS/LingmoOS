// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin.h"

#include <QMap>
#include <cstdint>

namespace Plugin {
class EmbedPluginPrivate
{
public:
    explicit EmbedPluginPrivate(QWindow* window)
        : parentWindow(window)
    {}

    QWindow* parentWindow;
    QString pluginId;
    QString itemKey;
    QString displayName;
    int pluginFlags;
    int pluginType;
    int sizePolicy;
    QPoint globalPos;
    QString dccIcon;
};

EmbedPlugin::EmbedPlugin(QWindow* window)
    : QObject(window)
    , d(new EmbedPluginPrivate(window))
{

}

EmbedPlugin::~EmbedPlugin()
{
    d.reset(nullptr);
}

QString EmbedPlugin::pluginId() const
{
    return d->pluginId;
}

QString EmbedPlugin::itemKey() const
{
    return d->itemKey;
}

int EmbedPlugin::pluginType() const
{
    return d->pluginType;
}

int EmbedPlugin::pluginFlags() const
{
    return d->pluginFlags;
}

int EmbedPlugin::pluginSizePolicy() const
{
    return d->sizePolicy;
}

QPoint EmbedPlugin::rawGlobalPos() const
{
    return d->globalPos;
}

void EmbedPlugin::setRawGlobalPos(const QPoint& pos)
{
    if (d->globalPos == pos) {
        return;
    }

    d->globalPos = pos;
    Q_EMIT rawGlobalPosChanged();
}

void EmbedPlugin::setPluginId(const QString& pluginid)
{
    if (d->pluginId == pluginid) {
        return;
    }

    d->pluginId = pluginid;
    Q_EMIT pluginIdChanged();
}

void EmbedPlugin::setItemKey(const QString& itemkey)
{
    if (d->itemKey == itemkey) {
        return;
    }

    d->itemKey = itemkey;
    Q_EMIT itemKeyChanged();
}

void EmbedPlugin::setPluginType(int type)
{
    if (d->pluginType == type) {
        return;
    }

    d->pluginType = type;
    Q_EMIT pluginTypeChanged();
}

void EmbedPlugin::setPluginFlags(int flags)
{
    if (d->pluginFlags == flags) {
        return;
    }

    d->pluginFlags = flags;
    Q_EMIT pluginFlagsChanged();
}

void EmbedPlugin::setPluginSizePolicy(int sizePolicy)
{
    if (d->sizePolicy == sizePolicy) {
        return;
    }

    d->sizePolicy = sizePolicy;
    Q_EMIT pluginSizePolicyChanged();
}

static QMap<QWindow*, EmbedPlugin*> s_map;
EmbedPlugin *EmbedPlugin::getWithoutCreating(QWindow *window)
{
    if (contains(window))
        return get(window);
    return nullptr;
}
EmbedPlugin* EmbedPlugin::get(QWindow* window)
{
    auto plugin = s_map.value(window);
    if (!plugin) {
        plugin = new EmbedPlugin(window);
        s_map.insert(window, plugin);
        QObject::connect(plugin, &EmbedPlugin::destroyed, window, [window] () {
            s_map.remove(window);
        });

        QObject::connect(window, &QWindow::visibleChanged, window, [window, plugin] (bool visible) {
            if (!visible) {
                plugin->deleteLater();
                s_map.remove(window);
            }
        });
    }

    return plugin;
}

bool EmbedPlugin::contains(QWindow *window)
{
    return s_map.keys().contains(window);
}

bool EmbedPlugin::contains(const QString &pluginId, int type, const QString &itemKey)
{
    for (const auto *plugin : s_map.values()) {
        if (itemKey.isEmpty()) {
            if (pluginId == plugin->pluginId() && type == plugin->pluginType()) {
                return true;
            }
        } else {
            if (pluginId == plugin->pluginId() && itemKey == plugin->itemKey() && type == plugin->pluginType()) {
                return true;
            }
        }
    }

    return false;
}

QList<EmbedPlugin *> EmbedPlugin::all()
{
    return s_map.values();
}

QString EmbedPlugin::displayName() const
{
    return d->displayName;
}

void EmbedPlugin::setDisplayName(const QString &displayName)
{
    d->displayName = displayName;
}

QString EmbedPlugin::dccIcon() const
{
    return d->dccIcon;
}

void EmbedPlugin::setDccIcon(const QString &dccIcon)
{
    if (d->dccIcon == dccIcon) {
        return;
    }
    d->dccIcon = dccIcon;
    Q_EMIT dccIconChanged(d->dccIcon);
}

class PluginPopupPrivate
{
public:
    explicit PluginPopupPrivate(QWindow* window)
        : parentWindow(window)
    {}

    QWindow* parentWindow;
    QString pluginId;
    QString itemKey;
    int pluginFlags;
    int popupType;
    int x = 0;
    int y = 0;
};

PluginPopup::PluginPopup(QWindow* window)
    : QObject(window)
    , d(new PluginPopupPrivate(window))
{

}

PluginPopup::~PluginPopup()
{
    d.reset(nullptr);
}

QString PluginPopup::pluginId() const
{
    return d->pluginId;
}

QString PluginPopup::itemKey() const
{
    return d->itemKey;
}

int PluginPopup::popupType() const
{
    return d->popupType;
}

void PluginPopup::setPluginId(const QString& pluginid)
{
    if (d->pluginId == pluginid) {
        return;
    }

    d->pluginId = pluginid;
    Q_EMIT pluginIdChanged();
}

void PluginPopup::setItemKey(const QString& itemkey)
{
    if (d->itemKey == itemkey) {
        return;
    }

    d->itemKey = itemkey;
    Q_EMIT itemKeyChanged();
}

void PluginPopup::setPopupType(const int& type)
{
    if (d->popupType == type) {
        return;
    }

    d->popupType = type;
    Q_EMIT popupTypeChanged();
}

int PluginPopup::x() const
{
    return d->x;
}

int PluginPopup::y() const
{
    return d->y;
}

void PluginPopup::setX(const int& x)
{
    if (d->x == x) {
        return;
    }

    d->x = x;
    Q_EMIT xChanged();
}

void PluginPopup::setY(const int& y)
{
    if (d->y == y) {
        return;
    }

    d->y = y;
    Q_EMIT yChanged();
}

static QMap<QWindow*, PluginPopup*> s_popupMap;
PluginPopup* PluginPopup::getWithoutCreating(QWindow* window)
{
    if (contains(window))
        return get(window);
    return nullptr;
}
PluginPopup* PluginPopup::get(QWindow* window)
{
    auto popup = s_popupMap.value(window);
    if (!popup) {
        popup = new PluginPopup(window);
        s_popupMap.insert(window, popup);
        QObject::connect(popup, &PluginPopup::destroyed, window, [window] () {
            s_popupMap.remove(window);
        });

        QObject::connect(window, &QWindow::visibleChanged, popup, [window, popup] (bool visible) {
            if (!visible) {
                s_popupMap.remove(window);
                popup->deleteLater();
            }
        });
    }

    return popup;
}

void PluginPopup::remove(QWindow *window)
{
    if (auto popup = s_popupMap.value(window)) {
        s_popupMap.remove(window);
        window->close();
        // don't use deleteLater, avoid remove other's Popup.
        delete popup;
    }
}

bool PluginPopup::contains(QWindow *window)
{
    return s_popupMap.keys().contains(window);
}
}
