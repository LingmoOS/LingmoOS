// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dndmodeplugin.h"
#include "plugins-logging-category.h"

#define DND_MODE_KEY "dnd-mode-key"
DCORE_USE_NAMESPACE

Q_LOGGING_CATEGORY(DND, "org.deepin.dde.dock.dnd-mode")

DndModePlugin::DndModePlugin(QObject *parent)
    : QObject(parent)
    , m_quickPanelWidget(new QuickPanelWidget)
    , m_item(new DndModeItem)
{

}

DndModePlugin::~DndModePlugin()
{
    if (m_quickPanelWidget) {
        delete m_quickPanelWidget;
        m_quickPanelWidget = nullptr;
    }
}

const QString DndModePlugin::pluginName() const
{
    return "dnd-mode";
}

const QString DndModePlugin::pluginDisplayName() const
{
    return tr("DND Mode");
}

void DndModePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    m_proxyInter->itemAdded(this, DND_MODE_KEY);
    m_quickPanelWidget->setDescription(pluginDisplayName());
    m_quickPanelWidget->setIcon(QIcon::fromTheme("dnd-mode-on"));

    connect(m_item, &DndModeItem::requestHideApplet, this, [this] {
        m_proxyInter->requestSetAppletVisible(this, DND_MODE_KEY, false);
    });
}

QWidget *DndModePlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == DND_MODE_KEY) {
        return m_item;
    }

    if (itemKey == Dock::QUICK_ITEM_KEY) {
        return m_quickPanelWidget;
    }

    return nullptr;
}

QWidget *DndModePlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey == DND_MODE_KEY) {
        return m_item->tipsWidget();
    }

    return nullptr;
}

QWidget *DndModePlugin::itemPopupApplet(const QString &itemKey)
{
    if (itemKey == DND_MODE_KEY) {
        return m_item->popupApplet();
    }

    return nullptr;
}

const QString DndModePlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey == DND_MODE_KEY) {
        return m_item->contextMenu();
    }

    return QString();
}

void DndModePlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    if (itemKey == DND_MODE_KEY) {
        m_item->invokeMenuItem(menuId, checked);
    }
}

int DndModePlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    return m_proxyInter->getValue(this, key, -1).toInt();
}

void DndModePlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    m_proxyInter->saveValue(this, key, order);
}

void DndModePlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == DND_MODE_KEY) {
        m_item->refreshIcon();
    }
}
