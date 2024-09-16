// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "constants.h"
#include "plugin.h"
#include "widgetplugin.h"
#include "quickpluginitem.h"
#include "environments.h"

#include <QMenu>
#include <QPainter>
#include <QProcess>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QToolTip>

#include <DGuiApplicationHelper>
#include <qglobal.h>

DGUI_USE_NAMESPACE

const QString DCCIconPath = CMAKE_INSTALL_PREFIX + QString("/share/dde-dock/icons/dcc-setting/");
const static int DockItemMargins = 0;

namespace {
class Q_DECL_HIDDEN EventFilter : public QObject
{
public:
    explicit EventFilter(QObject *parent)
        : QObject(parent)
    {
    }

public:
    bool eventFilter(QObject *watched, QEvent *event)
    {
        if (event->type() == QEvent::ToolTip) {
            const auto pos = static_cast<QHelpEvent*>(event)->globalPos();
            QMetaObject::invokeMethod(this, [this, pos] () {
                updateToolTipPosition(pos);
            }, Qt::QueuedConnection);
        }
        return false;
    }
private:
    void updateToolTipPosition(const QPoint &pos)
    {
        const auto allWindows = qGuiApp->allWindows();
        const auto iter = std::find_if(allWindows.begin(), allWindows.end(), [] (QWindow *window) {
            return window->objectName() == "qtooltip_labelWindow";
        });

        if (iter == allWindows.end())
            return;

        const auto toolTipWindow = *iter;
        if (auto pluginPopup = Plugin::PluginPopup::getWithoutCreating(toolTipWindow)) {
            pluginPopup->setX(toolTipWindow->transientParent()->x() + pos.x());
            pluginPopup->setY(toolTipWindow->transientParent()->y() + pos.y());
        }
    }
};
}

namespace dock {
WidgetPlugin::WidgetPlugin(PluginsItemInterface* pluginsItemInterface, QObject *pluginInstance)
    : QObject()
    , m_pluginsItemInterface(pluginsItemInterface)
    , m_pluginInstance(pluginInstance)
{
    QMetaObject::invokeMethod(this, [this](){
        m_pluginsItemInterface->init(this);
    });

    auto pluginsItemInterfaceV2 = dynamic_cast<PluginsItemInterfaceV2 *>(pluginsItemInterface);
    if (pluginsItemInterfaceV2) {
        pluginsItemInterfaceV2->setMessageCallback(WidgetPlugin::messageCallback);
    }

    // transparent background for tooltip.
    auto pt = QToolTip::palette();
    pt.setBrush(QPalette::Window, Qt::transparent);
    QToolTip::setPalette(pt);

    qApp->installEventFilter(new EventFilter(this));
}

WidgetPlugin::~WidgetPlugin()
{
}

void WidgetPlugin::itemAdded(PluginsItemInterface * const itemInter, const QString &itemKey)
{
    qDebug() << "itemAdded:" << itemKey;
    auto flag = getPluginFlags();
    if (flag & Dock::Type_Quick) {
        if (!Plugin::EmbedPlugin::contains(itemInter->pluginName(), Plugin::EmbedPlugin::Quick)) {
            PluginItem *item = new QuickPluginItem(itemInter, itemKey);
            item->init();
            item->setPluginFlags(flag);
            Plugin::EmbedPlugin* plugin = Plugin::EmbedPlugin::get(item->windowHandle());
            initConnections(plugin, item);
            plugin->setPluginFlags(flag);
            plugin->setPluginId(itemInter->pluginName());
            plugin->setDisplayName(itemInter->pluginDisplayName());
            plugin->setItemKey(Dock::QUICK_ITEM_KEY);
            plugin->setPluginType(Plugin::EmbedPlugin::Quick);
            plugin->setPluginSizePolicy(itemInter->pluginSizePolicy());
            item->windowHandle()->hide();
            item->show();
        } else {
            auto quickItem = m_pluginsItemInterface->itemWidget(Dock::QUICK_ITEM_KEY);
            if(quickItem) {
                quickItem->window()->windowHandle()->show();
            }
        }
    }
    if (flag & Dock::Type_Quick || flag & Dock::Type_System
        || flag & Dock::Type_Tray) {
        if (PluginItem::isForceUndock(flag)) {
            return;
        }

        if (!Plugin::EmbedPlugin::contains(itemInter->pluginName(), Plugin::EmbedPlugin::Tray, itemKey) && m_pluginsItemInterface->itemWidget(itemKey)) {
            PluginItem *item = new PluginItem(itemInter, itemKey);
            item->init();
            item->updatePluginContentMargin(DockItemMargins);
            item->setPluginFlags(flag);
            Plugin::EmbedPlugin* plugin = Plugin::EmbedPlugin::get(item->windowHandle());
            initConnections(plugin, item);
            plugin->setPluginFlags(flag);
            plugin->setPluginId(itemInter->pluginName());
            plugin->setDisplayName(itemInter->pluginDisplayName());
            plugin->setItemKey(itemKey);
            plugin->setPluginType(Plugin::EmbedPlugin::Tray);
            plugin->setPluginSizePolicy(itemInter->pluginSizePolicy());
            if (flag & Dock::Attribute_CanSetting) {
                const QString path = DCCIconPath + itemInter->pluginName() + ".svg";
                plugin->setDccIcon(path);
            }
            item->windowHandle()->hide();
            item->show();
        } else {
            auto trayItem = m_pluginsItemInterface->itemWidget(itemKey);
            if (trayItem) {
                trayItem->window()->windowHandle()->show();
            }
        }
    }
    if (flag & Dock::Type_Tool) {
        if (!Plugin::EmbedPlugin::contains(itemInter->pluginName(), Plugin::EmbedPlugin::Fixed, itemKey)) {
            PluginItem *item = new PluginItem(itemInter, itemKey);
            item->init();
            item->updatePluginContentMargin(DockItemMargins);
            item->setPluginFlags(flag);
            Plugin::EmbedPlugin* plugin = Plugin::EmbedPlugin::get(item->windowHandle());
            initConnections(plugin, item);
            plugin->setPluginFlags(flag);
            plugin->setDisplayName(itemInter->pluginDisplayName());
            plugin->setPluginId(itemInter->pluginName());
            plugin->setItemKey(itemKey);
            plugin->setPluginType(Plugin::EmbedPlugin::Fixed);
            plugin->setPluginSizePolicy(itemInter->pluginSizePolicy());
            if (flag & Dock::Attribute_CanSetting) {
                const QString path = DCCIconPath + itemInter->pluginName() + ".svg";
                plugin->setDccIcon(path);
            }
            item->windowHandle()->hide();
            item->show();
        }
    }
}

void WidgetPlugin::itemUpdate(PluginsItemInterface * const itemInter, const QString &itemKey)
{
    if(m_pluginItem) m_pluginItem->update();

    auto widget = m_pluginsItemInterface->itemWidget(itemKey);
    if (widget) widget->update();

    auto quickPanel = m_pluginsItemInterface->itemWidget(Dock::QUICK_ITEM_KEY);
    if(quickPanel) quickPanel->update();

    auto popupWidget = m_pluginsItemInterface->itemPopupApplet(itemKey);
    if (popupWidget) popupWidget->update();

    auto tipsWidget = m_pluginsItemInterface->itemTipsWidget(itemKey);
    if (tipsWidget) tipsWidget->update();

}
void WidgetPlugin::itemRemoved(PluginsItemInterface * const itemInter, const QString &itemKey)
{
    auto widget = m_pluginsItemInterface->itemWidget(itemKey);
    if(widget && widget->window() && widget->window()->windowHandle()) {
        widget->window()->windowHandle()->hide();
    }

    auto quickPanel = m_pluginsItemInterface->itemWidget(Dock::QUICK_ITEM_KEY);
    if(quickPanel && quickPanel->window() && quickPanel->window()->windowHandle()) {
        quickPanel->window()->windowHandle()->hide();
    }

    auto popupWidget = m_pluginsItemInterface->itemPopupApplet(itemKey);
    if(popupWidget && popupWidget->windowHandle()) popupWidget->windowHandle()->hide();

    auto tipsWidget = m_pluginsItemInterface->itemTipsWidget(itemKey);
    if(tipsWidget && tipsWidget->windowHandle()) tipsWidget->windowHandle()->hide();
}

void WidgetPlugin::requestWindowAutoHide(PluginsItemInterface * const itemInter, const QString &itemKey, const bool autoHide)
{
}

void WidgetPlugin::requestRefreshWindowVisible(PluginsItemInterface * const itemInter, const QString &itemKey)
{
}

void WidgetPlugin::requestSetAppletVisible(PluginsItemInterface * const itemInter, const QString &itemKey, const bool visible)
{
    if (!visible) return;

    QWidget* appletWidget = itemInter->itemPopupApplet(itemKey);
    if (!appletWidget) {
        qWarning() << itemKey << " plugin applet popup is null";
        return;
    }

    updateDockContainerState(itemInter, false);
    if (Plugin::PluginPopup::contains(appletWidget->windowHandle())) {
        Plugin::PluginPopup::remove(appletWidget->windowHandle());
    }

    appletWidget->winId();
    appletWidget->setParent(nullptr);
    appletWidget->setAttribute(Qt::WA_TranslucentBackground);

    bool hasCreated = Plugin::PluginPopup::contains(appletWidget->windowHandle());
    auto pluginPopup = Plugin::PluginPopup::get(appletWidget->windowHandle());
    if (!hasCreated) {
        connect(pluginPopup, &Plugin::PluginPopup::eventGeometry, appletWidget, [appletWidget](const QRect &geometry) {
            appletWidget->setFixedSize(geometry.size());
            appletWidget->update();
        });
    }

    pluginPopup->setPluginId(m_pluginsItemInterface->pluginName());
    pluginPopup->setItemKey(itemKey);
    pluginPopup->setPopupType(Plugin::PluginPopup::PopupTypeEmbed);
    appletWidget->show();
}

void WidgetPlugin::saveValue(PluginsItemInterface * const itemInter, const QString &key, const QVariant &value)
{
}

const QVariant WidgetPlugin::getValue(PluginsItemInterface *const itemInter, const QString &key, const QVariant& fallback)
{
    return fallback;
}

void WidgetPlugin::removeValue(PluginsItemInterface *const itemInter, const QStringList &keyList)
{
}

void WidgetPlugin::updateDockContainerState(PluginsItemInterface *itemInter, bool onDock)
{
    auto iter = dynamic_cast<PluginsItemInterfaceV2 *>(itemInter);
    if (!iter)
        return;

    QJsonObject obj;
    obj[Dock::MSG_TYPE] = Dock::MSG_APPLET_CONTAINER;
    obj[Dock::MSG_DATA] = onDock ? Dock::APPLET_CONTAINER_DOCK
                                 : Dock::APPLET_CONTAINER_QUICK_PANEL;

    QJsonDocument msg;
    msg.setObject(obj);

    iter->message(msg.toJson());
}

void WidgetPlugin::onDockPositionChanged(uint32_t position)
{
    qApp->setProperty(PROP_POSITION, position);
    m_pluginsItemInterface->positionChanged(static_cast<Dock::Position>(position));
}

void WidgetPlugin::onDockDisplayModeChanged(uint32_t displayMode)
{
    qApp->setProperty(PROP_DISPLAY_MODE, displayMode);
    m_pluginsItemInterface->displayModeChanged(static_cast<Dock::DisplayMode>(displayMode));
}

void WidgetPlugin::onDockEventMessageArrived(const QString &message)
{
    auto pluginsItemInterfaceV2 = dynamic_cast<PluginsItemInterfaceV2 *>(m_pluginsItemInterface);
    if (pluginsItemInterfaceV2) {
        pluginsItemInterfaceV2->message(message);
    }
}

Plugin::EmbedPlugin* WidgetPlugin::getPlugin(QWidget* widget)
{
    widget->setParent(nullptr);
    widget->winId();
    return Plugin::EmbedPlugin::get(widget->windowHandle());
}

void WidgetPlugin::initConnections(Plugin::EmbedPlugin *plugin, PluginItem *pluginItem)
{
    if (!plugin)
        return;

    connect(plugin, &Plugin::EmbedPlugin::dockColorThemeChanged, this, [](uint32_t type){
        DGuiApplicationHelper::instance()->setPaletteType(static_cast<DGuiApplicationHelper::ColorType>(type));
    }, Qt::UniqueConnection);

    connect(plugin, &Plugin::EmbedPlugin::dockPositionChanged, this, &WidgetPlugin::onDockPositionChanged, Qt::UniqueConnection);
    connect(plugin, &Plugin::EmbedPlugin::eventMessage, this, &WidgetPlugin::onDockEventMessageArrived, Qt::UniqueConnection);

    connect(plugin, &Plugin::EmbedPlugin::eventGeometry, this, [this, plugin, pluginItem](const QRect &geometry) {
        if (plugin->pluginType() == Plugin::EmbedPlugin::Quick) {
            pluginItem->updateItemWidgetSize(geometry.size());
        } else {
            if (geometry.width() == -1 || geometry.height() == -1) {
                if (geometry.width() < 0 && geometry.height() < 0) return;
                QSize size = geometry.width() == -1 ? QSize(geometry.height(), geometry.height()) : QSize(geometry.width(), geometry.width());
                pluginUpdateDockSize(size);
            }
        }
    });

    connect(plugin, &Plugin::EmbedPlugin::contentMarginChanged, pluginItem, &PluginItem::updatePluginContentMargin);

    connect(pluginItem, &PluginItem::recvMouseEvent, plugin, &Plugin::EmbedPlugin::pluginRecvMouseEvent);
}

int WidgetPlugin::getPluginFlags()
{
    const Dock::PluginFlags defaultPluginFlags = Dock::PluginFlag::Type_Tray | Dock::PluginFlag::Attribute_Normal;

    auto pluginsItemInterfaceV2 = dynamic_cast<PluginsItemInterfaceV2 *>(m_pluginsItemInterface);
    if (pluginsItemInterfaceV2) {
        Dock::PluginFlags flags;
        flags = pluginsItemInterfaceV2->flags();
        if (pluginsItemInterfaceV2->pluginName() == "grand-search") {
            flags |= (Dock::PluginFlag)Attribute_ForceUnDock;
        }
        return flags;
    } else {
        bool ok;
        auto flags = m_pluginInstance->property("pluginFlags").toInt(&ok);
        if (!ok) {
            qWarning() << "failed to pluginFlags toInt!";
            return defaultPluginFlags;
        }
        return flags;
    }
}

QString WidgetPlugin::messageCallback(PluginsItemInterfaceV2 *pluginItem, const QString &msg)
{
    qInfo() << "Plugin callback message:" << pluginItem->pluginName() << msg;

    QJsonObject ret;
    ret["code"] = -1;

    auto rootObj = getRootObj(msg);
    if (rootObj.isEmpty()) {
        qWarning() << "Root object is empty";
        return toJson(ret);
    }

    auto msgType = rootObj.value(Dock::MSG_TYPE);
    if (msgType == Dock::MSG_SUPPORT_FLAG_CHANGED) {
        bool changed = supportFlag(pluginItem);
        foreach (auto plugin, Plugin::EmbedPlugin::all()) {
            Q_EMIT plugin->pluginSupportFlagChanged(changed);
        }
    } else {
        foreach (auto plugin, Plugin::EmbedPlugin::all()) {
            Q_EMIT plugin->requestMessage(msg);
        }
    }

    ret["code"] = 0;
    return toJson(ret);
}

QJsonObject WidgetPlugin::getRootObj(const QString &jsonStr)
{
    QJsonParseError jsonParseError;
    const QJsonDocument &resultDoc = QJsonDocument::fromJson(jsonStr.toLocal8Bit(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || resultDoc.isEmpty()) {
        qWarning() << "Result json parse error";
        return QJsonObject();
    }

    return resultDoc.object();
}

QString WidgetPlugin::toJson(const QJsonObject &jsonObj)
{
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

bool WidgetPlugin::supportFlag(PluginsItemInterfaceV2 *pluginV2)
{
    if (!pluginV2) {
        return true;
    }

    QJsonObject obj;
    obj[Dock::MSG_TYPE] = Dock::MSG_GET_SUPPORT_FLAG;

    auto ret = pluginV2->message(toJson(obj));
    if (ret.isEmpty())
        return true;

    auto rootObj = getRootObj(ret);
    if (rootObj.isEmpty() || !rootObj.contains(Dock::MSG_SUPPORT_FLAG)) {
        return true;
    }

    return rootObj.value(Dock::MSG_SUPPORT_FLAG).toBool(true);
}

void WidgetPlugin::pluginUpdateDockSize(const QSize &size)
{
    auto pluginsItemInterfaceV2 = dynamic_cast<PluginsItemInterfaceV2 *>(m_pluginsItemInterface);
    if (!pluginsItemInterfaceV2) {
        return;
    }

    QJsonObject sizeData;
    sizeData["width"] = size.width();
    sizeData["height"] = size.height();

    QJsonObject obj;
    obj[Dock::MSG_TYPE] = Dock::MSG_DOCK_PANEL_SIZE_CHANGED;
    obj[Dock::MSG_DATA] = sizeData;

    QJsonDocument doc;
    doc.setObject(obj);

    pluginsItemInterfaceV2->message(doc.toJson());
}

QString WidgetPlugin::activeStateMessage(bool isActive)
{
    QJsonObject msg;
    msg[Dock::MSG_TYPE] = Dock::MSG_ITEM_ACTIVE_STATE;
    msg[Dock::MSG_DATA] = isActive;
    QJsonDocument doc;
    doc.setObject(msg);
    return doc.toJson();
}

}
