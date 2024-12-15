// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchitem.h"
#include "../constants.h"
#include "applet.h"
#include "pluginfactory.h"

#include <DDBusSender>
#include <DDciIcon>
#include <DGuiApplicationHelper>

#include <QBuffer>
#include <QGuiApplication>
#include <QProcess>

DGUI_USE_NAMESPACE
namespace dock
{

const QString grandSearchService = "com.lingmo.ocean.GrandSearch";
const QString grandSearchPath = "/com/lingmo/ocean/GrandSearch";
const QString grandSearchInterface = "com.lingmo.ocean.GrandSearch";
static DDBusSender searchDbus()
{
    return DDBusSender().service(grandSearchService).path(grandSearchPath).interface(grandSearchInterface);
}

SearchItem::SearchItem(QObject *parent)
    : DApplet(parent)
    , m_visible(true)
    , m_grandSearchVisible(false)
{
}

void SearchItem::toggleGrandSearch()
{
    searchDbus().method("SetVisible").arg(true).call();
}

void SearchItem::toggleGrandSearchConfig()
{
    QProcess::startDetached("ocean-grand-search", QStringList() << "--setting");
}

DockItemInfo SearchItem::dockItemInfo()
{
    DockItemInfo info;
    info.name = "search";
    info.displayName = tr("GrandSearch");
    info.itemKey = "search";
    info.settingKey = "search";
    info.visible = m_visible;
    info.oceanuiIcon = DCCIconPath + "search.svg";
    return info;
}

void SearchItem::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;

        Q_EMIT visibleChanged(visible);
    }
}

void SearchItem::onGrandSearchVisibleChanged(bool visible)
{
    if (m_grandSearchVisible != visible) {
        m_grandSearchVisible = visible;

        Q_EMIT grandSearchVisibleChanged(visible);
    }
}

D_APPLET_CLASS(SearchItem)
}

#include "searchitem.moc"
