// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "clipboarditem.h"
#include "constants.h"
#include "clipboardcontroller.h"

#include <DGuiApplicationHelper>

#include <QDBusConnection>
#include <QIcon>
#include <QJsonDocument>
#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>

DGUI_USE_NAMESPACE

#define SHIFT "shift"

ClipboardItem::ClipboardItem(QWidget* parent)
    : QWidget(parent)
    , m_tipsLabel(new TipsWidget(this))
    , m_icon(new CommonIconButton(this))
{
    init();
}

void ClipboardItem::init()
{
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setText(tr("Clipboard"));

    m_icon->setFixedSize(Dock::DOCK_PLUGIN_ITEM_FIXED_SIZE);
    m_icon->setIcon(QIcon::fromTheme("clipboard"));
    m_icon->setClickable(true);

    auto vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(m_icon, 0, Qt::AlignCenter);

    refreshIcon();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ClipboardItem::refreshIcon);
    connect(m_icon, &CommonIconButton::clicked, this, [] {
        ClipboardController::ref().toggle();
    });
}

QWidget* ClipboardItem::tipsWidget()
{
    return m_tipsLabel;
}

const QString ClipboardItem::contextMenu() const
{
    QList<QVariant> items;
    items.reserve(1);

    QMap<QString, QVariant> shift;
    shift["itemId"] = SHIFT;
    shift["itemText"] = tr("Open");
    shift["isActive"] = true;
    items.push_back(shift);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void ClipboardItem::invokeMenuItem(const QString menuId, const bool checked)
{
    Q_UNUSED(menuId);
    Q_UNUSED(checked);

    if (menuId == SHIFT) {
        ClipboardController::ref().toggle();
        Q_EMIT requestHideApplet();
    }
}

void ClipboardItem::refreshIcon()
{
    m_icon->setState(CommonIconButton::Default);
}

void ClipboardItem::resizeEvent(QResizeEvent* e)
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
