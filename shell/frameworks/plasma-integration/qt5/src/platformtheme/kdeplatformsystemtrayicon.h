/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KDEPLATFORMSYSTEMTRAYICON_H
#define KDEPLATFORMSYSTEMTRAYICON_H

#include <QVariant>
#include <qpa/qplatformmenu.h>
#include <qpa/qplatformsystemtrayicon.h>

class KStatusNotifierItem;
class SystemTrayMenuItem;
class QAction;
class QMenu;

class SystemTrayMenu : public QPlatformMenu
{
    Q_OBJECT
public:
    SystemTrayMenu();
    ~SystemTrayMenu() override;
    void insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before) override;
    QPlatformMenuItem *menuItemAt(int position) const override;
    QPlatformMenuItem *menuItemForTag(quintptr tag) const override;
    void removeMenuItem(QPlatformMenuItem *menuItem) override;
    void setEnabled(bool enabled) override;
    void setIcon(const QIcon &icon) override;
    void setTag(quintptr tag) override;
    void setText(const QString &text) override;
    void setVisible(bool visible) override;
    void syncMenuItem(QPlatformMenuItem *menuItem) override;
    void syncSeparatorsCollapsible(bool enable) override;
    quintptr tag() const override;
    QPlatformMenuItem *createMenuItem() const override;
    QPlatformMenu *createSubMenu() const override;

    QMenu *menu();

private:
    void createMenu();

    QString m_text;
    QIcon m_icon;
    QVariant m_enabled;
    QVariant m_visible;
    QVariant m_separatorsCollapsible;
    quintptr m_tag;
    QPointer<QMenu> m_menu;
    QList<SystemTrayMenuItem *> m_items;
};

class SystemTrayMenuItem : public QPlatformMenuItem
{
    Q_OBJECT
public:
    SystemTrayMenuItem();
    ~SystemTrayMenuItem() override;
    void setCheckable(bool checkable) override;
    void setChecked(bool isChecked) override;
    void setEnabled(bool enabled) override;
    void setFont(const QFont &font) override;
    void setIcon(const QIcon &icon) override;
    void setIsSeparator(bool isSeparator) override;
    void setMenu(QPlatformMenu *menu) override;
    void setRole(MenuRole role) override;
    void setShortcut(const QKeySequence &shortcut) override;
    void setTag(quintptr tag) override;
    void setText(const QString &text) override;
    void setVisible(bool isVisible) override;
    quintptr tag() const override;
    void setIconSize(int size) override;
    void setHasExclusiveGroup(bool hasExclusiveGroup) override;

    QAction *action() const;

private:
    quintptr m_tag;
    QAction *m_action = nullptr;
};

class KDEPlatformSystemTrayIcon : public QPlatformSystemTrayIcon
{
public:
    KDEPlatformSystemTrayIcon();
    ~KDEPlatformSystemTrayIcon() override;

    void init() override;
    void cleanup() override;
    void updateIcon(const QIcon &icon) override;
    void updateToolTip(const QString &tooltip) override;
    void updateMenu(QPlatformMenu *menu) override;
    QRect geometry() const override;
    void showMessage(const QString &title, const QString &msg, const QIcon &icon, MessageIcon iconType, int secs) override;

    bool isSystemTrayAvailable() const override;
    bool supportsMessages() const override;

    QPlatformMenu *createMenu() const override;

private:
    KStatusNotifierItem *m_sni = nullptr;
};

#endif
