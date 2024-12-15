// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "workspacewidget.h"
#include "workspacewidget_p.h"
#include "cooperationstatewidget.h"
#include "devicelistwidget.h"
#include "firsttipwidget.h"
#include "common/commonutils.h"

#include <QMouseEvent>
#include <QRegularExpression>
#include <QTimer>
#include <QToolButton>
#include <QToolTip>
#include <QFile>

#include <gui/utils/cooperationguihelper.h>

using namespace cooperation_core;

WorkspaceWidgetPrivate::WorkspaceWidgetPrivate(WorkspaceWidget *qq)
    : q(qq),
      sortFilterWorker(new SortFilterWorker),
      workThread(new QThread)
{
    sortFilterWorker->moveToThread(workThread.data());
    workThread->start();
}

WorkspaceWidgetPrivate::~WorkspaceWidgetPrivate()
{
    sortFilterWorker->stop();
    workThread->quit();
    workThread->wait();
}

void WorkspaceWidgetPrivate::initUI()
{
    tipWidget = new FirstTipWidget(q);
    tipWidget->setVisible(false);

    searchEdit = new CooperationSearchEdit(q);
    searchEdit->setContentsMargins(10, 0, 10, 0);

    searchEdit->setPlaceholderText(tr("Please enter the device ip/name of the collaborator"));
    searchEdit->setPlaceHolder(tr("Please enter the device ip/name of the collaborator"));
    stackedLayout = new QStackedLayout;

    deviceLabel = new QLabel(tr("Nearby Device"));
    deviceLabel->setContentsMargins(20, 0, 10, 0);
    CooperationGuiHelper::setAutoFont(deviceLabel, 14, QFont::Normal);
    QHBoxLayout *hLayout = new QHBoxLayout;
    refreshBtn = new CooperationIconButton();
    refreshBtn->setIconSize(QSize(16, 16));
#ifdef __linux__
    refreshBtn->setIcon(QIcon::fromTheme("refresh_tip"));
    refreshBtn->setFlat(true);
#else
    refreshBtn->setIcon(QIcon(":/icons/lingmo/builtin/texts/refresh_tip_14px.svg"));
    refreshBtn->setStyleSheet("QToolButton {"
                              "background-color: rgba(0,0,0,0.15);"
                              "border-radius: 8px;"
                              "}");
#endif
    refreshBtn->setToolTip(tr("Re-scan for devices"));
    refreshBtn->setFixedSize(24, 24);
    connect(refreshBtn, &QPushButton::clicked, q, &WorkspaceWidget::refresh);

    hLayout->addWidget(deviceLabel);
    hLayout->addWidget(refreshBtn);
    hLayout->setSpacing(0);
    hLayout->setAlignment(Qt::AlignLeft);

    lfdWidget = new LookingForDeviceWidget(q);
    nnWidget = new NoNetworkWidget(q);
    nrWidget = new NoResultWidget(q);
    nrWidget->setContentsMargins(10, 0, 10, 0);
    dlWidget = new DeviceListWidget(q);
    dlWidget->setContentsMargins(10, 0, 10, 0);

    stackedLayout->addWidget(lfdWidget);
    stackedLayout->addWidget(nnWidget);
    stackedLayout->addWidget(nrWidget);
    stackedLayout->addWidget(dlWidget);
    stackedLayout->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 15, 0, 0);
#ifndef linux
    mainLayout->addSpacing(50);
    mainLayout->addWidget(searchEdit, 0, Qt::AlignHCenter);
#else
    mainLayout->addWidget(searchEdit);
#endif

    mainLayout->addWidget(tipWidget);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(hLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(stackedLayout);
    q->setLayout(mainLayout);
}

void WorkspaceWidgetPrivate::initConnect()
{
    connect(searchEdit, &CooperationSearchEdit::returnPressed, this, &WorkspaceWidgetPrivate::onSearchDevice);
    connect(searchEdit, &CooperationSearchEdit::textChanged, this, &WorkspaceWidgetPrivate::onSearchValueChanged);
    connect(this, &WorkspaceWidgetPrivate::devicesAoceand, sortFilterWorker.data(), &SortFilterWorker::addDevice, Qt::QueuedConnection);
    connect(this, &WorkspaceWidgetPrivate::devicesRemoved, sortFilterWorker.data(), &SortFilterWorker::removeDevice, Qt::QueuedConnection);
    connect(this, &WorkspaceWidgetPrivate::filterDevice, sortFilterWorker.data(), &SortFilterWorker::filterDevice, Qt::QueuedConnection);
    connect(this, &WorkspaceWidgetPrivate::clearDevice, sortFilterWorker.data(), &SortFilterWorker::clear, Qt::QueuedConnection);
    connect(sortFilterWorker.data(), &SortFilterWorker::sortFilterResult, this, &WorkspaceWidgetPrivate::onSortFilterResult, Qt::QueuedConnection);
    connect(sortFilterWorker.data(), &SortFilterWorker::filterFinished, this, &WorkspaceWidgetPrivate::onFilterFinished, Qt::QueuedConnection);
    connect(sortFilterWorker.data(), &SortFilterWorker::deviceRemoved, this, &WorkspaceWidgetPrivate::onDeviceRemoved, Qt::QueuedConnection);
    connect(sortFilterWorker.data(), &SortFilterWorker::deviceUpdated, this, &WorkspaceWidgetPrivate::onDeviceUpdated, Qt::QueuedConnection);
    connect(sortFilterWorker.data(), &SortFilterWorker::deviceMoved, this, &WorkspaceWidgetPrivate::onDeviceMoved, Qt::QueuedConnection);
}

void WorkspaceWidgetPrivate::onSearchValueChanged(const QString &text)
{
    if (currentPage == WorkspaceWidget::kNoNetworkWidget)
        return;

    dlWidget->clear();
    Q_EMIT filterDevice(text);
}

void WorkspaceWidgetPrivate::onSearchDevice()
{
    QRegularExpression ipPattern("^(10\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}|172\\.(1[6-9]|2[0-9]|3[0-1])\\.\\d{1,3}\\.\\d{1,3}|192\\.168\\.\\d{1,3}\\.\\d{1,3})$");
    QString ip = searchEdit->text();
    if (!ipPattern.match(ip).hasMatch())
        return;

    q->switchWidget(WorkspaceWidget::kLookignForDeviceWidget);
    emit q->search(ip);
}

void WorkspaceWidgetPrivate::onSortFilterResult(int index, const DeviceInfoPointer info)
{
    q->switchWidget(WorkspaceWidget::kDeviceListWidget);
    dlWidget->insertItem(index, info);
}

void WorkspaceWidgetPrivate::onFilterFinished()
{
    if (dlWidget->itemCount() == 0) {
        if (searchEdit->text().isEmpty()) {
            stackedLayout->setCurrentIndex(currentPage);
            return;
        }

        q->switchWidget(WorkspaceWidget::kNoResultWidget);
    }
}

void WorkspaceWidgetPrivate::onDeviceRemoved(int index)
{
    dlWidget->removeItem(index);
    if (dlWidget->itemCount() == 0)
        q->switchWidget(WorkspaceWidget::kNoResultWidget);
}

void WorkspaceWidgetPrivate::onDeviceUpdated(int index, const DeviceInfoPointer info)
{
    dlWidget->updateItem(index, info);
}

void WorkspaceWidgetPrivate::onDeviceMoved(int from, int to, const DeviceInfoPointer info)
{
    dlWidget->updateItem(from, info);
    dlWidget->moveItem(from, to);
}

WorkspaceWidget::WorkspaceWidget(QWidget *parent)
    : QWidget(parent),
      d(new WorkspaceWidgetPrivate(this))
{
    d->initUI();
    d->initConnect();
}

int WorkspaceWidget::itemCount()
{
    return d->dlWidget->itemCount();
}

void WorkspaceWidget::switchWidget(PageName page)
{
    if (d->currentPage == page || page == kUnknownPage)
        return;

    if (page == kDeviceListWidget) {
        d->deviceLabel->setVisible(true);
        d->refreshBtn->setVisible(true);
    } else {
        d->deviceLabel->setVisible(false);
        d->refreshBtn->setVisible(false);
    }

    if (page == kLookignForDeviceWidget) {
        d->lfdWidget->seAnimationtEnabled(true);
        d->tipWidget->setVisible(false);

    } else {
        if (qApp->property("onlyTransfer").toBool() || !QFile(lingmo_cross::CommonUitls::tipConfPath()).exists())
            d->tipWidget->setVisible(true);
        d->lfdWidget->seAnimationtEnabled(false);
    }

    d->currentPage = page;
    d->stackedLayout->setCurrentIndex(page);
}

void WorkspaceWidget::addDeviceInfos(const QList<DeviceInfoPointer> &infoList)
{
    Q_EMIT d->devicesAoceand(infoList);
}

void WorkspaceWidget::removeDeviceInfos(const QString &ip)
{
    Q_EMIT d->devicesRemoved(ip);
}

void WorkspaceWidget::addDeviceOperation(const QVariantMap &map)
{
    d->dlWidget->addItemOperation(map);
}

DeviceInfoPointer WorkspaceWidget::findDeviceInfo(const QString &ip)
{
    return d->dlWidget->findDeviceInfo(ip);
}

void WorkspaceWidget::clear()
{
    d->dlWidget->clear();
    Q_EMIT d->clearDevice();
}

void WorkspaceWidget::setFirstStartTip(bool visible)
{
    d->tipWidget->setVisible(visible);
}

bool WorkspaceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *widget = childAt(mouseEvent->pos());
            if (widget) {
                widget->setFocus();
            }
        }
    }
    return QWidget::event(event);
}
