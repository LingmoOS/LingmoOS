// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backendprocesspage.h"
#include "view/widgets/processwidget.h"
#include "view/widgets/noprocesswidget.h"

#include <QStackedLayout>

BackendProcessPage::BackendProcessPage(QWidget *parent)
    : QWidget(parent)
{
    processWidget = new ProcessWidget;
    processWidget->setIcon(QIcon::fromTheme("application-x-deb"));
    processWidget->setMainText(tr("Loading packages..."));
    processWidget->setProcessText(tr("%1/%2 loaded"));

    noProcessWidget = new NoProcessWidget;
    noProcessWidget->setActionText(tr("Initializing..."));

    allLayout = new QStackedLayout;
    allLayout->addWidget(processWidget);
    allLayout->addWidget(noProcessWidget);
    setLayout(allLayout);
}

void BackendProcessPage::setDisplayPage(DisplayMode mode)
{
    if (mode == APT_INIT) {
        allLayout->setCurrentWidget(noProcessWidget);
        noProcessWidget->start();
    } else if (mode == READ_PKG) {
        allLayout->setCurrentWidget(processWidget);
        noProcessWidget->stop();
    } else {  // PROCESS_FIN
        noProcessWidget->stop();
    }
}

void BackendProcessPage::setPkgProcessRate(int currentRate, int pkgCount)
{
    processWidget->setProgress(currentRate, pkgCount);
}
