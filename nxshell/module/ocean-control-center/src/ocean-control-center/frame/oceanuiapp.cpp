// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "oceanuiapp.h"

namespace oceanuiV25 {
static OceanUIApp *oceanuiApp = nullptr;

OceanUIApp *OceanUIApp::instance()
{
    return oceanuiApp;
}

OceanUIApp::OceanUIApp(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!oceanuiApp);
    oceanuiApp = this;
}

OceanUIApp::~OceanUIApp() { }

int OceanUIApp::width() const
{
    return 0;
}

int OceanUIApp::height() const
{
    return 0;
}

OceanUIObject *OceanUIApp::root() const
{
    return nullptr;
}

OceanUIObject *OceanUIApp::activeObject() const
{
    return nullptr;
}

OceanUIObject *OceanUIApp::object(const QString &)
{
    return nullptr;
}

void OceanUIApp::addObject(OceanUIObject *) { }

void OceanUIApp::removeObject(OceanUIObject *) { }

void OceanUIApp::removeObject(const QString &) { }

void OceanUIApp::showPage(const QString &) { }

void OceanUIApp::showPage(OceanUIObject *, const QString &) { }

QWindow *OceanUIApp::mainWindow() const
{
    return nullptr;
}

QAbstractItemModel *OceanUIApp::navModel() const
{
    return nullptr;
}

QSortFilterProxyModel *OceanUIApp::searchModel() const
{
    return nullptr;
}

} // namespace oceanuiV25
