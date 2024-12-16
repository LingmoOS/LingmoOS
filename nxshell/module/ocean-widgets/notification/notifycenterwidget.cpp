// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifycenterwidget.h"
#include "notification/persistence.h"
#include "notification/constants.h"
#include "notification/iconbutton.h"
#include "notification/button.h"
#include "style.h"
#include "helper.hpp"

#include <QDesktopWidget>
#include <QBoxLayout>
#include <QDBusInterface>
#include <QPalette>
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QMenu>
#include <QStackedLayout>
#include <QPushButton>

#include <DLabel>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DIconTheme>

DWIDGET_USE_NAMESPACE

NotifyCenterWidget::NotifyCenterWidget(AbstractPersistence *database, QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_notifyWidget(new NotifyWidget(this, database))
    , m_wmHelper(DWindowManagerHelper::instance())
{
    setBlurEnabled(false);
    initUI();
    initConnections();

    CompositeChanged();
}

void NotifyCenterWidget::initUI()
{
    m_notifyWidget->setAccessibleName("NotifyWidget");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0, 0, 0, 0);

    m_headWidget = new QWidget;
    m_headWidget->setAccessibleName("HeadWidget");
    m_headWidget->setFixedSize(Notify::CenterWidth, Notify::CenterTitleHeight);

    title_label = new DLabel;
    title_label->setAccessibleName("TitleLabel");
    QFont font = title_label->font();
    font.setBold(true);
    font.setWeight(QFont::Black);
    title_label->setFont(font);
    title_label->setText(tr("Notification Center"));
    title_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    title_label->setForegroundRole(QPalette::BrightText);
    DFontSizeManager::instance()->bind(title_label, DFontSizeManager::T3);

    m_toggleNotificationFolding = new CicleIconButton(nullptr);
    m_toggleNotificationFolding->setAccessibleName("ToggleNotificationFolding");
    m_toggleNotificationFolding->setFixedSize(UI::Panel::buttonSize);
    m_toggleNotificationFolding->setIcon(DDciIcon::fromTheme("arrow_ordinary_up"));
    connect(m_toggleNotificationFolding, &CicleIconButton::clicked, this, &NotifyCenterWidget::collapesNotificationFolding);
    updateToggleNotificationFoldingButtonVisible();

    m_settingBtn = new CicleIconButton(nullptr);
    m_settingBtn->setIcon(DDciIcon::fromTheme("notify_more"));
    m_settingBtn->setAccessibleName("SettingButton");
    m_settingBtn->setFixedSize(UI::Panel::buttonSize);
    connect(m_settingBtn, &CicleIconButton::clicked, this, &NotifyCenterWidget::showSettingMenu);

    m_clearButton = new CicleIconButton;
    m_clearButton->setAccessibleName("ClearButton");
    m_clearButton->setIcon(DIconTheme::findQIcon("notify_clear"));
    m_clearButton->setFixedSize(UI::Panel::buttonSize);
    m_clearButton->setVisible(false);
//    m_clearButton->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *head_Layout = new QHBoxLayout;
    head_Layout->setMargin(0);
    head_Layout->addWidget(title_label, 0, Qt::AlignLeft | Qt::AlignBottom);
    head_Layout->addStretch();
    head_Layout->addWidget(m_toggleNotificationFolding, 0, Qt::AlignRight | Qt::AlignTop);
    head_Layout->addWidget(m_settingBtn, 0, Qt::AlignRight | Qt::AlignTop);
    head_Layout->addWidget(m_clearButton, 0, Qt::AlignRight | Qt::AlignTop);
    m_headWidget->setLayout(head_Layout);

    auto bottomTipView = new QWidget();
    m_bottomTipLayout = new QStackedLayout(bottomTipView);

    m_expandRemaining = new TransparentButton();
    m_expandRemaining->setFlat(true);
    m_expandRemaining->setAccessibleName("ExpandRemainingButton");
    connect(m_expandRemaining, &QPushButton::clicked, this, &NotifyCenterWidget::expandNotificationFolding);
    m_bottomTipLayout->addWidget(m_expandRemaining);

    m_noNotifyLabel = new DLabel();
    m_noNotifyLabel->setText(tr("No new notifications"));
    m_noNotifyLabel->setAccessibleName("NoNotifyLabel");
    DFontSizeManager::instance()->bind(m_noNotifyLabel, DFontSizeManager::T9);
    {
        auto pt = m_noNotifyLabel->palette();
        auto background = pt.brush(QPalette::WindowText).color();
        background.setAlpha(UI::Panel::noNotifyLabelFontAlpha);
        pt.setBrush(QPalette::WindowText, background);
        m_noNotifyLabel->setPalette(pt);
    }
    m_bottomTipLayout->addWidget(m_noNotifyLabel);

    connect(m_notifyWidget->model(), &NotifyModel::modelReset, this, &NotifyCenterWidget::updateDisplayOfRemainingNotification);
    connect(m_notifyWidget->view(), &NotifyListView::lastItemCreated, this, &NotifyCenterWidget::updateTabFocus);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(Notify::CenterMargin, Notify::CenterMargin, 0, 0);
    mainLayout->addWidget(m_headWidget);
    mainLayout->addWidget(m_notifyWidget);
    mainLayout->addWidget(bottomTipView, 0, Qt::AlignHCenter);

    setLayout(mainLayout);

    connect(m_clearButton, &CicleIconButton::clicked, this, [this]() {
        m_notifyWidget->model()->removeAllData();
    });
    connect(m_notifyWidget->model(), &NotifyModel::appCountChanged, this, &NotifyCenterWidget::updateDisplayOfRemainingNotification);

    connect(m_notifyWidget->model(), &NotifyModel::appCountChanged, this, &NotifyCenterWidget::updateClearButtonVisible, Qt::QueuedConnection);

    refreshTheme();

    collapesNotificationFolding();

    updateDisplayOfRemainingNotification();
    updateClearButtonVisible();
}

void NotifyCenterWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &NotifyCenterWidget::refreshTheme);
    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &NotifyCenterWidget::CompositeChanged, Qt::QueuedConnection);
    connect(m_notifyWidget->model(), &NotifyModel::fullCollapsedChanged, this, &NotifyCenterWidget::updateToggleNotificationFoldingButtonVisible);
    connect(m_notifyWidget->model(), &NotifyModel::appCountChanged, this, &NotifyCenterWidget::updateToggleNotificationFoldingButtonVisible);
}

void NotifyCenterWidget::refreshTheme()
{
    QPalette pa = title_label->palette();
    pa.setBrush(QPalette::WindowText, pa.brightText());
    title_label->setPalette(pa);
}

void NotifyCenterWidget::CompositeChanged()
{
    m_hasComposite = m_wmHelper->hasComposite();
}

void NotifyCenterWidget::updateDisplayOfRemainingNotification()
{
    if (!hasAppNotification()) {
        if (m_bottomTipLayout->parentWidget()->isHidden())
            m_bottomTipLayout->parentWidget()->show();
        m_bottomTipLayout->setCurrentWidget(m_noNotifyLabel);
    } else {
        const int rowCount = m_notifyWidget->model()->remainNotificationCount();
        if (rowCount > 0) {
            if (m_bottomTipLayout->parentWidget()->isHidden()) {
                m_bottomTipLayout->parentWidget()->show();
                m_expandRemaining->show();
            }
            m_expandRemaining->setText(tr("%1 more notifications").arg(QString::number(rowCount)));
            m_bottomTipLayout->setCurrentWidget(m_expandRemaining);
        } else {
            m_bottomTipLayout->parentWidget()->hide();
        }
    }
}

void NotifyCenterWidget::updateTabFocus()
{
    qDebug() << "updateTabFocus()";

    QWidget::setTabOrder(this, m_toggleNotificationFolding);
    QWidget::setTabOrder(m_toggleNotificationFolding, m_settingBtn);
    QWidget::setTabOrder(m_settingBtn, m_clearButton);
    QWidget::setTabOrder(m_clearButton, m_notifyWidget->view());
    if (auto w = m_notifyWidget->view()->lastItemView()) {
        QWidget::setTabOrder(w, m_expandRemaining);
    } else {
        if (!m_bottomTipLayout->parentWidget()->isHidden())
            QWidget::setTabOrder(m_notifyWidget->view(), m_expandRemaining);
    }
}

void NotifyCenterWidget::expandNotificationFolding()
{
    expandNotificationFoldingImpl(true);
}

void NotifyCenterWidget::expandNotificationFoldingImpl(const bool refreshData)
{
    if (refreshData)
        m_notifyWidget->model()->expandData();
    m_expandRemaining->hide();
}

void NotifyCenterWidget::collapesNotificationFolding()
{
    collapesNotificationFoldingImpl(true);
}

void NotifyCenterWidget::collapesNotificationFoldingImpl(const bool refreshData)
{
    if (refreshData)
        m_notifyWidget->model()->collapseData();
    m_expandRemaining->show();
}

void NotifyCenterWidget::showNotificationModuleOfControlCenter()
{
    Helper::instance()->showNotificationModuleOfControlCenter();
}

void NotifyCenterWidget::updateClearButtonVisible()
{
    const bool changed = (m_clearButton->isVisible() != hasAppNotification());
    if (changed)
        m_clearButton->setVisible(!m_clearButton->isVisible());
}

void NotifyCenterWidget::updateToggleNotificationFoldingButtonVisible()
{
    if (hasAppNotification() && !m_notifyWidget->model()->fullCollapsed()) {
        m_toggleNotificationFolding->show();
    } else {
        m_toggleNotificationFolding->hide();
    }
}

bool NotifyCenterWidget::hasAppNotification() const
{
    return m_notifyWidget->model()->rowCount() > 0;
}

void NotifyCenterWidget::showSettingMenu()
{
    QMenu *menu = new QMenu(this);
    do {
        QAction *action = menu->addAction(tr("Notification settings"));
        action->setCheckable(true);

        connect(action, &QAction::triggered, this, &NotifyCenterWidget::showNotificationModuleOfControlCenter);
    } while (false);

    m_settingBtn->setDown(true);
    menu->exec(m_settingBtn->mapToGlobal(QPoint(0, m_settingBtn->geometry().bottom() + UI::Panel::settingSpacingBetweenMenu)));
    menu->deleteLater();
    m_settingBtn->setDown(false);
}
