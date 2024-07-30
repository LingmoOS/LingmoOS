/*
    This file is part of the proxy model test suite.

    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mainwindow.h"

#include <QTabWidget>

#include "dynamictreemodel.h"

#include "breadcrumbdirectionwidget.h"
#include "breadcrumbnavigationwidget.h"
#include "breadcrumbswidget.h"
#include "checkablewidget.h"
#include "descendantpmwidget.h"
#include "selectionpmwidget.h"
// #include "statesaverwidget.h"
#include "descendantqmltree.h"
#include "proxyitemselectionwidget.h"
#include "proxymodeltestwidget.h"
#ifdef QT_QML_LIB
#include "reparentingpmwidget.h"
#endif
#include "kidentityproxymodelwidget.h"
#include "lessthanwidget.h"
#include "matchcheckingwidget.h"
#ifdef QT_QUICKWIDGETS_LIB
#include "selectioninqmlwidget.h"
#endif

MainWindow::MainWindow()
    : QMainWindow()
{
    QTabWidget *tabWidget = new QTabWidget(this);

    tabWidget->addTab(new MatchCheckingWidget(), QStringLiteral("Match Checking PM"));
    tabWidget->addTab(new DescendantProxyModelWidget(), QStringLiteral("descendant PM"));
    tabWidget->addTab(new SelectionProxyWidget(), QStringLiteral("selection PM"));
#ifdef QT_QUICKWIDGETS_LIB
    tabWidget->addTab(new SelectionInQmlWidget(), QStringLiteral("selection PM in QML"));
#endif
    tabWidget->addTab(new KIdentityProxyModelWidget(), QStringLiteral("Identity PM"));
    tabWidget->addTab(new CheckableWidget(), QStringLiteral("Checkable"));
    tabWidget->addTab(new BreadcrumbsWidget(), QStringLiteral("Breadcrumbs"));
    tabWidget->addTab(new BreadcrumbNavigationWidget(), QStringLiteral("Breadcrumb Navigation"));
    tabWidget->addTab(new BreadcrumbDirectionWidget(), QStringLiteral("Breadcrumb Direction"));
    tabWidget->addTab(new ProxyItemSelectionWidget(), QStringLiteral("Proxy Item selection"));
#ifdef QT_QML_LIB
    tabWidget->addTab(new ReparentingProxyModelWidget(), QStringLiteral("reparenting PM"));
#endif
#ifdef QT_QUICKWIDGETS_LIB
    tabWidget->addTab(new DescendantQmlTreeWidget(), QStringLiteral("QML Trees"));
#endif
    tabWidget->addTab(new LessThanWidget(), QStringLiteral("Less Than"));
    tabWidget->addTab(new ProxyModelTestWidget(), QStringLiteral("Proxy Model Test"));
    //   tabWidget->addTab(new StateSaverWidget(), "State Saver Test");

    setCentralWidget(tabWidget);
}

MainWindow::~MainWindow()
{
}

#include "moc_mainwindow.cpp"
