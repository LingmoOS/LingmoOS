// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "airplanemodeitem.h"
#include "airplanemodecontroller.h"
#include "constants.h"
#include "tipswidget.h"

#include <DDBusSender>
#include <DGuiApplicationHelper>

#include <QDBusConnection>
#include <QIcon>
#include <QJsonDocument>
#include <QPainter>
#include <QVBoxLayout>

DGUI_USE_NAMESPACE

#define SHIFT "shift"

AirplaneModeItem::AirplaneModeItem(QWidget* parent)
    : QWidget(parent)
    , m_tipsLabel(new Dock::TipsWidget(this))
    , m_applet(new CommonApplet(this))
    , m_icon(new CommonIconButton(this))
{
    m_tipsLabel->setText(tr("Airplane mode enabled"));
    m_tipsLabel->setVisible(false);
    m_applet->setVisible(false);

    m_applet->setTitle(tr("Airplane Mode"));
    m_applet->setDccPage("network", "Airplane Mode");
    m_applet->setDescription(tr("Airplane mode settings"));
    m_applet->setIcon(QIcon::fromTheme("open-arrow"));
    m_applet->hideSettingButton();

    auto vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(m_icon, 0, Qt::AlignCenter);
    m_icon->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);
    m_icon->setStateIconMapping({
        { CommonIconButton::State::On, QPair<QString, QString>("airplanemode-on",":/airplanemode-on.svg") },
        { CommonIconButton::State::Off, QPair<QString, QString>("airplanemode-off",":/airplanemode-off.svg") }
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AirplaneModeItem::refreshIcon);
    connect(m_applet, &CommonApplet::enableChanged, AMC_PTR, &AirplaneModeController::enable);
    connect(m_applet, &CommonApplet::requestHideApplet, this, &AirplaneModeItem::requestHideApplet);
    connect(AMC_PTR, &AirplaneModeController::enabledChanged, this, [this](bool enable) {
        m_applet->setEnabled(enable);
        refreshIcon();
        Q_EMIT airplaneEnableChanged(enable);
        updateTips();
    });

    connect(AMC_PTR, &AirplaneModeController::prepareForSleep, this, [=](bool sleep) {
        if (!sleep) {
            const bool enabled = AMC_PTR->isEnabled();
            m_applet->setEnabled(enabled);
            refreshIcon();
            Q_EMIT airplaneEnableChanged(enabled);
            updateTips();
        }
    });

    m_applet->setEnabled(AMC_PTR->isEnabled());
    refreshIcon();
    updateTips();
}

QWidget* AirplaneModeItem::tipsWidget()
{
    return m_tipsLabel;
}

QWidget* AirplaneModeItem::popupApplet()
{
    return m_applet;
}

const QString AirplaneModeItem::contextMenu() const
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> shift;
    shift["itemId"] = SHIFT;
    if (AMC_PTR->isEnabled())
        shift["itemText"] = tr("Disable");
    else
        shift["itemText"] = tr("Enable");
    shift["isActive"] = true;
    items.push_back(shift);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void AirplaneModeItem::invokeMenuItem(const QString menuId, const bool checked)
{
    Q_UNUSED(menuId);
    Q_UNUSED(checked);

    if (menuId == SHIFT) {
        AMC_PTR->toggle();
    }
}

void AirplaneModeItem::refreshIcon()
{
    m_icon->setState(AMC_PTR->isEnabled() ? CommonIconButton::On : CommonIconButton::Off);
}

void AirplaneModeItem::updateTips()
{
    if (AMC_PTR->isEnabled())
        m_tipsLabel->setText(tr("Airplane mode enabled"));
    else
        m_tipsLabel->setText(tr("Airplane mode disabled"));
}

bool AirplaneModeItem::airplaneEnable()
{
    return AMC_PTR->isEnabled();
}

void AirplaneModeItem::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    const Dock::Position position = qApp->property(PROP_POSITION).value<Dock::Position>();
    if (position == Dock::Bottom || position == Dock::Top) {
        setMaximumWidth(height());
        setMaximumHeight(QWIDGETSIZE_MAX);
    } else {
        setMaximumHeight(width());
        setMaximumWidth(QWIDGETSIZE_MAX);
    }

    refreshIcon();
}
