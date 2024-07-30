/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kdeplatformsystemtrayicon.h"
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QRect>
#include <kstatusnotifieritem.h>

#include "statusnotifierwatcher_interface.h"

SystemTrayMenu::SystemTrayMenu()
    : QPlatformMenu()
    , m_enabled(QVariant::Bool)
    , m_visible(QVariant::Bool)
    , m_separatorsCollapsible(QVariant::Bool)
    , m_tag(0)
{
}

SystemTrayMenu::~SystemTrayMenu()
{
    if (m_menu) {
        m_menu->deleteLater();
    }
}

QPlatformMenuItem *SystemTrayMenu::createMenuItem() const
{
    return new SystemTrayMenuItem();
}

QPlatformMenu *SystemTrayMenu::createSubMenu() const
{
    return new SystemTrayMenu();
}

void SystemTrayMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
    if (SystemTrayMenuItem *ours = qobject_cast<SystemTrayMenuItem *>(menuItem)) {
        bool inserted = false;
        if (SystemTrayMenuItem *oursBefore = qobject_cast<SystemTrayMenuItem *>(before)) {
            for (auto it = m_items.begin(); it != m_items.end(); ++it) {
                if (*it == oursBefore) {
                    m_items.insert(it, ours);
                    if (m_menu) {
                        m_menu->insertAction(oursBefore->action(), ours->action());
                    }
                    inserted = true;
                    break;
                }
            }
        }
        if (!inserted) {
            m_items.append(ours);
            if (m_menu) {
                m_menu->addAction(ours->action());
            }
        }
    }
}

QPlatformMenuItem *SystemTrayMenu::menuItemAt(int position) const
{
    if (position < m_items.size()) {
        return m_items.at(position);
    }
    return nullptr;
}

QPlatformMenuItem *SystemTrayMenu::menuItemForTag(quintptr tag) const
{
    auto it = std::find_if(m_items.constBegin(), m_items.constEnd(), [tag](SystemTrayMenuItem *item) {
        return item->tag() == tag;
    });
    if (it != m_items.constEnd()) {
        return *it;
    }
    return nullptr;
}

void SystemTrayMenu::removeMenuItem(QPlatformMenuItem *menuItem)
{
    if (SystemTrayMenuItem *ours = qobject_cast<SystemTrayMenuItem *>(menuItem)) {
        m_items.removeOne(ours);
        if (ours->action() && m_menu) {
            m_menu->removeAction(ours->action());
        }
    }
}

void SystemTrayMenu::setEnabled(bool enabled)
{
    m_enabled = enabled;
    if (m_menu) {
        m_menu->setEnabled(enabled);
    }
}

void SystemTrayMenu::setIcon(const QIcon &icon)
{
    m_icon = icon;
    if (m_menu) {
        m_menu->setIcon(icon);
    }
}

void SystemTrayMenu::setTag(quintptr tag)
{
    m_tag = tag;
}

void SystemTrayMenu::setText(const QString &text)
{
    m_text = text;
    if (m_menu) {
        m_menu->setTitle(text);
    }
}

void SystemTrayMenu::setVisible(bool visible)
{
    m_visible = visible;
    if (m_menu) {
        m_menu->setVisible(visible);
    }
}

void SystemTrayMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
    Q_UNUSED(menuItem)
    // nothing to do
}

void SystemTrayMenu::syncSeparatorsCollapsible(bool enable)
{
    m_separatorsCollapsible = enable;
    if (m_menu) {
        m_menu->setSeparatorsCollapsible(enable);
    }
}

quintptr SystemTrayMenu::tag() const
{
    return m_tag;
}

QMenu *SystemTrayMenu::menu()
{
    if (!m_menu) {
        createMenu();
    }
    return m_menu;
}

void SystemTrayMenu::createMenu()
{
    m_menu = new QMenu();
    connect(m_menu, &QMenu::aboutToShow, this, &QPlatformMenu::aboutToShow);
    connect(m_menu, &QMenu::aboutToHide, this, &QPlatformMenu::aboutToHide);

    if (!m_icon.isNull()) {
        m_menu->setIcon(m_icon);
    }
    if (m_menu->title() != m_text) {
        m_menu->setTitle(m_text);
    }
    if (!m_enabled.isNull()) {
        m_menu->setEnabled(m_enabled.toBool());
    }
    if (!m_visible.isNull()) {
        m_menu->setVisible(m_visible.toBool());
    }
    if (!m_separatorsCollapsible.isNull()) {
        m_menu->setSeparatorsCollapsible(m_separatorsCollapsible.toBool());
    }
    for (auto item : std::as_const(m_items)) {
        m_menu->addAction(item->action());
    }
}

SystemTrayMenuItem::SystemTrayMenuItem()
    : QPlatformMenuItem()
    , m_tag(0)
    , m_action(new QAction(this))
{
    connect(m_action, &QAction::triggered, this, &QPlatformMenuItem::activated);
    connect(m_action, &QAction::hovered, this, &QPlatformMenuItem::hovered);
}

SystemTrayMenuItem::~SystemTrayMenuItem()
{
}

void SystemTrayMenuItem::setCheckable(bool checkable)
{
    m_action->setCheckable(checkable);
}

void SystemTrayMenuItem::setChecked(bool isChecked)
{
    m_action->setChecked(isChecked);
}

void SystemTrayMenuItem::setEnabled(bool enabled)
{
    m_action->setEnabled(enabled);
}

void SystemTrayMenuItem::setFont(const QFont &font)
{
    m_action->setFont(font);
}

void SystemTrayMenuItem::setIcon(const QIcon &icon)
{
    m_action->setIcon(icon);
}

void SystemTrayMenuItem::setIsSeparator(bool isSeparator)
{
    m_action->setSeparator(isSeparator);
}

void SystemTrayMenuItem::setMenu(QPlatformMenu *menu)
{
    if (SystemTrayMenu *ourMenu = qobject_cast<SystemTrayMenu *>(menu)) {
        m_action->setMenu(ourMenu->menu());
    }
}

void SystemTrayMenuItem::setRole(QPlatformMenuItem::MenuRole role)
{
    Q_UNUSED(role)
}

void SystemTrayMenuItem::setShortcut(const QKeySequence &shortcut)
{
    m_action->setShortcut(shortcut);
}

void SystemTrayMenuItem::setTag(quintptr tag)
{
    m_tag = tag;
}

void SystemTrayMenuItem::setText(const QString &text)
{
    m_action->setText(text);
}

void SystemTrayMenuItem::setVisible(bool isVisible)
{
    m_action->setVisible(isVisible);
}

void SystemTrayMenuItem::setIconSize(int size)
{
    Q_UNUSED(size)
}

void SystemTrayMenuItem::setHasExclusiveGroup(bool hasExclusiveGroup)
{
    if (hasExclusiveGroup) {
        if (!m_action->actionGroup()) {
            m_action->setActionGroup(new QActionGroup(m_action));
        }
    } else {
        QActionGroup *actionGroup = m_action->actionGroup();
        if (actionGroup) {
            m_action->setActionGroup(nullptr);
            delete actionGroup;
        }
    }
}

quintptr SystemTrayMenuItem::tag() const
{
    return m_tag;
}

QAction *SystemTrayMenuItem::action() const
{
    return m_action;
}

KDEPlatformSystemTrayIcon::KDEPlatformSystemTrayIcon()
    : QPlatformSystemTrayIcon()
{
}

KDEPlatformSystemTrayIcon::~KDEPlatformSystemTrayIcon()
{
}

void KDEPlatformSystemTrayIcon::init()
{
    if (!m_sni) {
        m_sni = new KStatusNotifierItem();
        m_sni->setStandardActionsEnabled(false);
        m_sni->setTitle(QApplication::applicationDisplayName());
        m_sni->setStatus(KStatusNotifierItem::Active);
        connect(m_sni, &KStatusNotifierItem::activateRequested, [this](bool active, const QPoint &pos) {
            Q_UNUSED(active)
            Q_UNUSED(pos)
            Q_EMIT activated(QPlatformSystemTrayIcon::Trigger);
        });
        connect(m_sni, &KStatusNotifierItem::secondaryActivateRequested, [this](const QPoint &pos) {
            Q_UNUSED(pos)
            Q_EMIT activated(QPlatformSystemTrayIcon::MiddleClick);
        });
    }
}

void KDEPlatformSystemTrayIcon::cleanup()
{
    delete m_sni;
    m_sni = nullptr;
}

void KDEPlatformSystemTrayIcon::updateIcon(const QIcon &icon)
{
    if (!m_sni) {
        return;
    }
    if (icon.name().isEmpty()) {
        m_sni->setIconByPixmap(icon);
        m_sni->setToolTipIconByPixmap(icon);
    } else {
        m_sni->setIconByName(icon.name());
        m_sni->setToolTipIconByName(icon.name());
    }
}

void KDEPlatformSystemTrayIcon::updateToolTip(const QString &tooltip)
{
    if (!m_sni) {
        return;
    }
    m_sni->setToolTipTitle(tooltip);
}

void KDEPlatformSystemTrayIcon::updateMenu(QPlatformMenu *menu)
{
    if (!m_sni) {
        return;
    }
    if (SystemTrayMenu *ourMenu = qobject_cast<SystemTrayMenu *>(menu)) {
        m_sni->setContextMenu(ourMenu->menu());
    }
}

QPlatformMenu *KDEPlatformSystemTrayIcon::createMenu() const
{
    return new SystemTrayMenu();
}

QRect KDEPlatformSystemTrayIcon::geometry() const
{
    // StatusNotifierItem doesn't provide the geometry
    return QRect();
}

void KDEPlatformSystemTrayIcon::showMessage(const QString &title, const QString &msg, const QIcon &icon, MessageIcon iconType, int secs)
{
    Q_UNUSED(iconType)
    if (!m_sni) {
        return;
    }
    m_sni->showMessage(title, msg, icon.name(), secs);
}

bool KDEPlatformSystemTrayIcon::isSystemTrayAvailable() const
{
    org::kde::StatusNotifierWatcher systrayHost(QStringLiteral("org.kde.StatusNotifierWatcher"),
                                                QStringLiteral("/StatusNotifierWatcher"),
                                                QDBusConnection::sessionBus());
    if (systrayHost.isValid()) {
        return systrayHost.isStatusNotifierHostRegistered();
    }

    return false;
}

bool KDEPlatformSystemTrayIcon::supportsMessages() const
{
    return true;
}
