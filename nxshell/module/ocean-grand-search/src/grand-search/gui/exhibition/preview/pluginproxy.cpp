// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginproxy.h"
#include "generalwidget/detailwidget.h"

#include <QDebug>

using namespace GrandSearch;

PluginProxy::PluginProxy(PreviewWidget *parent)
    : QObject(parent)
    , q(parent)
{

}

void PluginProxy::updateDetailInfo(PreviewPlugin *plugin)
{
    if (plugin == nullptr || plugin != q->m_preview) {
        qWarning() << "updateDetailInfo: invaild plugin.";
        return;
    }

    if (thread() != qApp->thread()) {
        qWarning() << __FUNCTION__ << "could not be called in other thread.";
        return;
    }

    q->m_detailInfoWidget->setDetailInfoList(plugin->getAttributeDetailInfo());
}
