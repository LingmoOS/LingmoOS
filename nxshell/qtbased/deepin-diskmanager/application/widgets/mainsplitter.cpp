// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "mainsplitter.h"
#include "devicelistwidget.h"
#include "infoshowwidget.h"

MainSplitter::MainSplitter(DWidget *parent)
    : DSplitter(parent)
{
    setFrameShape(QFrame::NoFrame);

    initui();
    initConnection();
}

void MainSplitter::initui()
{
    setHandleWidth(1);
    setChildrenCollapsible(false); //  子部件不可拉伸到 0

    m_deviceList = new DeviceListWidget(this);
    m_infoShow = new InfoShowWidget(this);

    setMouseTracking(true);
//    setStretchFactor(1, 1);

    m_infoShow->setFrameShape(QFrame::NoFrame);

    addWidget(m_deviceList);
    addWidget(m_infoShow);
    setStretchFactor(0, 1);
    setStretchFactor(1, 4);
}

void MainSplitter::initConnection()
{
}
