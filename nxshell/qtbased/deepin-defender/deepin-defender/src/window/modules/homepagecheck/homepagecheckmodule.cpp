// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagecheckmodule.h"
#include "window/modules/common/common.h"
#include "window/interface/moduleinterface.h"
#include "window/interface/frameproxyinterface.h"
#include <pwd.h>
#include <unistd.h>

#include <QTimer>
#include <QModelIndex>
#include <QStandardItemModel>

HomePageCheckModule::HomePageCheckModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frame)
{

}


void HomePageCheckModule::initialize()
{

}

void HomePageCheckModule::preInitialize()
{
}

const QString HomePageCheckModule::name() const
{
    return "homepage";
}

void HomePageCheckModule::active(int index)
{
    Q_UNUSED(index);
    // 首页是不可删除窗口, 所以始终保留
    if (nullptr == m_homePagecheckWidget) {
        m_homePagecheckWidget = new HomePageCheckWidget();
    }

    m_homePagecheckWidget->show();
    m_frameProxy->pushWidget(this, m_homePagecheckWidget);

}

void HomePageCheckModule::deactive()
{

}


