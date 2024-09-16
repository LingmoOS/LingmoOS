// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "powerapplet.h"
#include "constants.h"
#include "pluginitemdelegate.h"
#include "pluginlistview.h"
#include "performancemodecontroller.h"
#include <qglobal.h>

#include <DSwitchButton>
#include <DBlurEffectWidget>
#include <DFontSizeManager>

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>

#define MODE_DATA_ROLE (Dtk::UserRole+1)

DWIDGET_USE_NAMESPACE

PowerApplet::PowerApplet(QWidget *parent)
    : QWidget(parent)
    , m_titleWidget(new QWidget(this))
    , m_titleLayout(new QHBoxLayout(m_titleWidget))
    , m_titleLabel(new DLabel(QString(), m_titleWidget))
    , m_view(new PluginListView(this))
    , m_model(new QStandardItemModel(m_view))
    , m_settingButton(new JumpSettingButton(this))
    , m_batteryWidget(new QWidget(this))
    , m_batteryIcon(new CommonIconButton(m_batteryWidget))
    , m_batteryPercentage(new DLabel(QString(), m_batteryWidget))
    , m_batteryTips(new DLabel(QString(), m_batteryWidget))
    , m_minHeight(400)
{
    initUI();
    initConnect();
    initData();
}

void PowerApplet::setIcon(const QIcon &icon)
{
    m_settingButton->setIcon(icon);
}

void PowerApplet::setDescription(const QString &description)
{
    m_settingButton->setDescription(description);
}

void PowerApplet::setDccPage(const QString &module, const QString &page)
{
    m_settingButton->setDccPage(module, page);
}

void PowerApplet::initUI()
{
    setFixedWidth(Dock::DOCK_POPUP_WIDGET_WIDTH);
    setForegroundRole(QPalette::BrightText);

    auto mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 标题
    m_titleLayout->setSpacing(0);
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    m_titleLayout->addWidget(m_titleLabel);
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T6, QFont::Medium);
    m_titleLabel->setText(tr("Power")); // 电源管理
    m_titleLabel->setAlignment(Qt::AlignCenter);

    // 电池状态
    auto batteryLayout = new QVBoxLayout;
    batteryLayout->setSpacing(5);
    batteryLayout->setContentsMargins(20, 0, 0, 0);

    m_batteryIcon->setFixedSize(36, 36);

    m_batteryPercentage->setAlignment(Qt::AlignLeft);
    m_batteryPercentage->setForegroundRole(foregroundRole());
    DFontSizeManager::instance()->bind(m_batteryPercentage, DFontSizeManager::T2, QFont::DemiBold);

    m_batteryTips->setAlignment(Qt::AlignLeft);
    m_batteryTips->setForegroundRole(foregroundRole());
    m_batteryTips->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_batteryTips, DFontSizeManager::T6);

    batteryLayout->addWidget(m_batteryIcon);
    batteryLayout->addWidget(m_batteryPercentage);
    batteryLayout->addWidget(m_batteryTips);
    m_batteryWidget->setLayout(batteryLayout);

    // 列表view
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->setModel(m_model);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setCurrentIndex(QModelIndex());

    mainLayout->addWidget(m_titleWidget, 0);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_batteryWidget, 0);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_view, 1);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(m_settingButton, 0);

    installEventFilter(this);
    setLayout(mainLayout);
}

void PowerApplet::initConnect()
{
    connect(m_view, &PluginListView::clicked, this, &PowerApplet::onModeChanged);
    connect(&PerformanceModeController::ref(), &PerformanceModeController::powerModeChanged, this, &PowerApplet::onCurPowerModeChanged);
    connect(&PerformanceModeController::ref(), &PerformanceModeController::highPerformanceSupportChanged, this, &PowerApplet::onHighPerformanceSupportChanged);
    connect(m_settingButton, &JumpSettingButton::showPageRequestWasSended, this, &PowerApplet::requestHideApplet);
}

void PowerApplet::initData()
{
    const auto &modeList = PerformanceModeController::ref().modeList();
    QIcon icon;
    for (const auto &pair : modeList) {
        if (pair.first == PERFORMANCE) {
            icon = QIcon::fromTheme("status-efficient-mode");
        } else if (pair.first == BALANCE) {
            icon = QIcon::fromTheme("status-balanced-mode");
        } else if (pair.first == POWERSAVE) {
            icon = QIcon::fromTheme("status-energy-saving-mode");
        } else if (pair.first == BALANCEPERFORMANCE) {
            icon = QIcon::fromTheme("status-performance-mode");
        }
        PluginItem *item = new PluginItem(icon, pair.second);
        item->setData(pair.first, MODE_DATA_ROLE);
        m_model->appendRow(item);
    }
    resizeApplet();
    onHighPerformanceSupportChanged(PerformanceModeController::ref().highPerformanceSupported());
    onCurPowerModeChanged(PerformanceModeController::ref().getCurrentPowerMode());
}

bool PowerApplet::eventFilter(QObject *watcher, QEvent *event)
{
    if (event->type() == QEvent::Resize || event->type() == QEvent::Show) {
        resizeApplet();
    }

    return QWidget::eventFilter(watcher, event);
}

void PowerApplet::setMinHeight(int minHeight)
{
    m_minHeight = minHeight;
    resizeApplet();
}

void PowerApplet::resizeApplet()
{
    const int listViewHeight = m_model->rowCount() * (m_view->getItemHeight() + m_view->getItemSpacing()) - m_view->getItemSpacing();
    int totalHeight = m_titleLabel->height() + m_batteryWidget->height() + listViewHeight + m_settingButton->height();

    resize(width(), qMin(Dock::DOCK_POPUP_WIDGET_MAX_HEIGHT, qMax(totalHeight, m_minHeight)));
}

void PowerApplet::onModeChanged(const QModelIndex &index)
{
    QStandardItem *item = m_model->item(index.row(), index.column());
    if (!item) {
        return;
    }
    PerformanceModeController::ref().setPowerMode(item->data(MODE_DATA_ROLE).toString());
}

void PowerApplet::onCurPowerModeChanged(const QString &curPowerMode)
{
    int row_count = m_model->rowCount();
    for (int i = 0; i < row_count; ++i) {
        auto item = dynamic_cast<PluginItem *>(m_model->item(i, 0));
        if (!item) {
            continue;
        }
        if (item->data(MODE_DATA_ROLE).toString() == curPowerMode) {
            item->updateState(PluginItemState::ConnectedOnlyPrompt);
        } else {
            item->updateState(PluginItemState::NoState);
        }
    }
}

void PowerApplet::onHighPerformanceSupportChanged(const bool isSupport)
{
    int row_count = m_model->rowCount();
    if (!isSupport) {
        int cur_place = m_view->currentIndex().row();
        for (int i = 0; i < row_count; ++i) {
            QStandardItem *items = m_model->item(i, 0);
            if (items->data(MODE_DATA_ROLE).toString() == PERFORMANCE) {
                m_model->removeRow(i);

                if (cur_place == i || cur_place < 0) {
                    m_view->clicked(m_model->index(0, 0));
                }
                break;
            }
        }
    } else {
        bool findHighPerform = false;
        for (int i = 0; i < row_count; ++i) {
            QStandardItem *items = m_model->item(i, 0);
            if (items->data(MODE_DATA_ROLE).toString() == PERFORMANCE) {
                findHighPerform = true;
                break;
            }
        }
        if (!findHighPerform) {
            QList<QPair<QString, QString>> modeList = PerformanceModeController::ref().modeList();
            for (const auto &pair : modeList) {
                if (pair.first == PERFORMANCE) {
                    PluginItem *item = new PluginItem(QIcon::fromTheme("performance"), pair.second);
                    item->setData(PERFORMANCE, MODE_DATA_ROLE);
                    m_model->insertRow(0, item);
                    break;
                }
            }
        }
    }
}

void PowerApplet::refreshBatteryIcon(const QString &icon)
{
    m_batteryIcon->setIcon(icon, ":/batteryicons/resources/batteryicons/" + icon + ".svg");
}

void PowerApplet::refreshBatteryPercentage(const QString &percentage, const QString &tips)
{
    m_batteryPercentage->setText(percentage);
    m_batteryTips->setText(tips);
}
