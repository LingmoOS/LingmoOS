// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dndmodeitem.h"
#include "commonapplet.h"
#include "constants.h"
#include "dndmodecontroller.h"
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
#define SETTINGS "settings"

DndModeItem::DndModeItem(QWidget* parent)
    : QWidget(parent)
    , m_tipsLabel(new Dock::TipsWidget(this))
    , m_applet(new CommonApplet(this))
    , m_icon(new CommonIconButton(this))
{
    init();
}

void DndModeItem::init()
{
    m_tipsLabel->setVisible(false);

    m_applet->setVisible(false);
    m_applet->setDccPage("notification", "SystemNotify");
    m_applet->setTitle(tr("DND Mode"));
    m_applet->setDescription(tr("DND mode settings"));
    m_applet->setIcon(QIcon::fromTheme("open-arrow"));

    m_icon->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);
    m_icon->setStateIconMapping({ { CommonIconButton::State::On, QPair<QString, QString>("dnd-mode-on", ":/dnd-mode-on.svg") },
        { CommonIconButton::State::Off, QPair<QString, QString>("dnd-mode-off", ":/dnd-mode-off.svg") } });

    auto vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setMargin(0);
    vLayout->addWidget(m_icon, 0, Qt::AlignCenter);

    m_applet->setEnabled(DndModeController::ref().isDndModeEnabled());
    refreshIcon();
    updateTips();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DndModeItem::refreshIcon);
    connect(m_applet, &CommonApplet::enableChanged, &DndModeController::ref(), &DndModeController::enable);
    connect(m_applet, &CommonApplet::requestHideApplet, this, &DndModeItem::requestHideApplet);
    connect(&DndModeController::ref(), &DndModeController::dndModeChanged, this, [this](bool enable) {
        m_applet->setEnabled(enable);
        refreshIcon();
        updateTips();
    });
}

QWidget* DndModeItem::tipsWidget()
{
    return m_tipsLabel;
}

QWidget* DndModeItem::popupApplet()
{
    return m_applet;
}

const QString DndModeItem::contextMenu() const
{
    QList<QVariant> items;
    items.reserve(2);

    QMap<QString, QVariant> shift;
    shift["itemId"] = SHIFT;
    if (DndModeController::ref().isDndModeEnabled())
        shift["itemText"] = tr("Disable");
    else
        shift["itemText"] = tr("Enable");
    shift["isActive"] = true;
    items.push_back(shift);

    QMap<QString, QVariant> settings;
    settings["itemId"] = SETTINGS;
    settings["itemText"] = tr("DND mode settings");
    settings["isActive"] = true;
    items.push_back(settings);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DndModeItem::invokeMenuItem(const QString menuId, const bool checked)
{
    Q_UNUSED(menuId);
    Q_UNUSED(checked);

    if (menuId == SHIFT) {
        DndModeController::ref().toggle();
    } else if (menuId == SETTINGS) {
        DDBusSender()
            .service("com.deepin.dde.ControlCenter")
            .interface("com.deepin.dde.ControlCenter")
            .path("/com/deepin/dde/ControlCenter")
            .method(QString("ShowPage"))
            .arg(QString("notification"))
            .arg(QString("SystemNotify"))
            .call();

        Q_EMIT requestHideApplet();
    }
}

void DndModeItem::refreshIcon()
{
    m_icon->setState(DndModeController::ref().isDndModeEnabled() ? CommonIconButton::On : CommonIconButton::Off);
}

void DndModeItem::updateTips()
{
    if (DndModeController::ref().isDndModeEnabled())
        m_tipsLabel->setText(tr("DND mode enabled"));
    else
        m_tipsLabel->setText(tr("DND mode disabled"));
}

bool DndModeItem::airplaneEnable()
{
    return DndModeController::ref().isDndModeEnabled();
}

void DndModeItem::resizeEvent(QResizeEvent* e)
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
