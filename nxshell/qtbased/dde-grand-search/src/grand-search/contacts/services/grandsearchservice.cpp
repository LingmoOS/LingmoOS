// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "grandsearchservice_p.h"
#include "grandsearchservice.h"
#include "gui/mainwindow.h"

#include <QDebug>

using namespace GrandSearch;

GrandSearchServicePrivate::GrandSearchServicePrivate(MainWindow *mainWindow, GrandSearchService *parent)
    : q_p(parent)
    , m_mainWindow(mainWindow)
{

}

GrandSearchService::GrandSearchService(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , d_p(new GrandSearchServicePrivate(mainWindow, this))
{
    connect(d_p->m_mainWindow, &MainWindow::visibleChanged, this, &GrandSearchService::VisibleChanged);
}

GrandSearchService::~GrandSearchService()
{

}

bool GrandSearchService::IsVisible() const
{
    Q_ASSERT(d_p->m_mainWindow);

    bool isvisble = d_p->m_mainWindow->isVisible();
    qDebug() << "get by dbus and isvisible:" << isvisble;
    return isvisble;
}

void GrandSearchService::SetVisible(const bool visible)
{
    Q_ASSERT(d_p->m_mainWindow);

    // todo 二阶段：根据情况，可能需要设置为隐藏，而不能直接退出
    if (d_p->m_mainWindow->isVisible() == visible)
        return;
    if (!visible) {
        qInfo() << "set visible is:" << visible <<",and exit it.";
        d_p->m_mainWindow->close();
    } else {
        d_p->m_mainWindow->setVisible(visible);
    }
}
