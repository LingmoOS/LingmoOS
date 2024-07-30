/*

    SPDX-FileCopyrightText: 2016 Dmitry Shachnev <mitya57@gmail.com>
    SPDX-FileContributor: The Qt Company <https://www.qt.io/licensing/>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial

*/

#include "qdbusmenubar_p.h"

QT_BEGIN_NAMESPACE

/* note: do not change these to QStringLiteral;
   we are unloaded before QtDBus is done using the strings.
 */
#define REGISTRAR_SERVICE QLatin1String("com.canonical.AppMenu.Registrar")
#define REGISTRAR_PATH QLatin1String("/com/canonical/AppMenu/Registrar")

QDBusMenuBar::QDBusMenuBar(KdePlatformTheme *platformTheme)
    : QPlatformMenuBar()
    , m_menu(new QDBusPlatformMenu())
    , m_menuAdaptor(new QDBusMenuAdaptor(m_menu))
    , m_platformTheme(platformTheme)
{
    QDBusMenuItem::registerDBusTypes();
    connect(m_menu, &QDBusPlatformMenu::propertiesUpdated, m_menuAdaptor, &QDBusMenuAdaptor::ItemsPropertiesUpdated);
    connect(m_menu, &QDBusPlatformMenu::updated, m_menuAdaptor, &QDBusMenuAdaptor::LayoutUpdated);
    connect(m_menu, SIGNAL(popupRequested(int, uint)), m_menuAdaptor, SIGNAL(ItemActivationRequested(int, uint)));
}

QDBusMenuBar::~QDBusMenuBar()
{
    if (s_globalMenuBar == this) {
        s_globalMenuBar = nullptr;
        m_platformTheme->globalMenuBarNoLongerExists();
    }

    if (this == s_menuBars.value(m_window))
        s_menuBars.remove(m_window);

    unregisterMenuBarX11(m_window);
    delete m_menuAdaptor;
    delete m_menu;
    qDeleteAll(m_menuItems);
}

QDBusPlatformMenuItem *QDBusMenuBar::menuItemForMenu(QPlatformMenu *menu)
{
    if (!menu)
        return nullptr;
    quintptr tag = menu->tag();
    const auto it = m_menuItems.constFind(tag);
    if (it != m_menuItems.cend()) {
        return *it;
    } else {
        QDBusPlatformMenuItem *item = new QDBusPlatformMenuItem;
        updateMenuItem(item, menu);
        m_menuItems.insert(tag, item);
        return item;
    }
}

void QDBusMenuBar::updateMenuItem(QDBusPlatformMenuItem *item, QPlatformMenu *menu)
{
    const QDBusPlatformMenu *ourMenu = qobject_cast<const QDBusPlatformMenu *>(menu);
    item->setText(ourMenu->text());
    item->setIcon(ourMenu->icon());
    item->setEnabled(ourMenu->isEnabled());
    item->setVisible(ourMenu->isVisible());
    item->setMenu(menu);
}

void QDBusMenuBar::insertMenu(QPlatformMenu *menu, QPlatformMenu *before)
{
    QDBusPlatformMenuItem *menuItem = menuItemForMenu(menu);
    QDBusPlatformMenuItem *beforeItem = menuItemForMenu(before);
    m_menu->insertMenuItem(menuItem, beforeItem);
    m_menu->emitUpdated();
}

void QDBusMenuBar::removeMenu(QPlatformMenu *menu)
{
    QDBusPlatformMenuItem *menuItem = menuItemForMenu(menu);
    m_menu->removeMenuItem(menuItem);
    m_menu->emitUpdated();
}

void QDBusMenuBar::syncMenu(QPlatformMenu *menu)
{
    QDBusPlatformMenuItem *menuItem = menuItemForMenu(menu);
    updateMenuItem(menuItem, menu);
}

void QDBusMenuBar::handleReparent(QWindow *newParentWindow)
{
    // if the parent is set to nullptr on our first time around,
    // this is supposed to be an app-wide menu bar
    // so we only care if nullptr == nullptr after our first round
    if (m_initted && newParentWindow == m_window) {
        return;
    }
    m_initted = true;

    QWindow *oldWindow = m_window;

    if (this == s_menuBars.value(oldWindow))
        s_menuBars.remove(oldWindow);

    unregisterMenuBarX11(m_window);
    m_window = newParentWindow;
    s_menuBars[newParentWindow] = this;

    if (newParentWindow) {
        if (s_globalMenuBar == this) {
            s_globalMenuBar = nullptr;
            m_platformTheme->globalMenuBarNoLongerExists();
        }
        if (createDBusMenuBar()) {
            registerMenuBarX11(m_window, m_objectPath);
        }
    } else if (!s_globalMenuBar) {
        s_globalMenuBar = this;
        createDBusMenuBar();
        m_platformTheme->globalMenuBarExistsNow();
    } else {
        qWarning() << "There's already a global menu bar...";
    }

    Q_EMIT windowChanged(newParentWindow, oldWindow);
}

QDBusMenuBar *QDBusMenuBar::globalMenuBar()
{
    return s_globalMenuBar;
}

QPlatformMenu *QDBusMenuBar::menuForTag(quintptr tag) const
{
    QDBusPlatformMenuItem *menuItem = m_menuItems.value(tag);
    if (menuItem)
        return const_cast<QPlatformMenu *>(menuItem->menu());
    return nullptr;
}

QPlatformMenu *QDBusMenuBar::createMenu() const
{
    return new QDBusPlatformMenu;
}

bool QDBusMenuBar::createDBusMenuBar()
{
    static uint menuBarId = 0;

    QDBusConnection connection = QDBusConnection::sessionBus();
    m_objectPath = QStringLiteral("/MenuBar/%1").arg(++menuBarId);
    if (!connection.registerObject(m_objectPath, m_menu))
        return false;

    return true;
}
void QDBusMenuBar::uncreateDBusMenuBar()
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!m_objectPath.isEmpty())
        connection.unregisterObject(m_objectPath);
}

QDBusMenuBar *QDBusMenuBar::menuBarForWindow(QWindow *window)
{
    return s_menuBars.value(window);
}

void QDBusMenuBar::registerMenuBarX11(QWindow *window, const QString &objectPath)
{
    if (!window) {
        qWarning("Cannot register window menu without window");
        return;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusMenuRegistrarInterface registrar(REGISTRAR_SERVICE, REGISTRAR_PATH, connection, window);
    QDBusPendingReply<> r = registrar.RegisterWindow(static_cast<uint>(window->winId()), QDBusObjectPath(objectPath));
    r.waitForFinished();
    if (r.isError()) {
        qWarning("Failed to register window menu, reason: %s (\"%s\")", qUtf8Printable(r.error().name()), qUtf8Printable(r.error().message()));
        connection.unregisterObject(objectPath);
    }
}

QDBusMenuBar *QDBusMenuBar::s_globalMenuBar = nullptr;
QMap<QWindow *, QDBusMenuBar *> QDBusMenuBar::s_menuBars;

void QDBusMenuBar::unregisterMenuBarX11(QWindow *window)
{
    if (!window) {
        return;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusMenuRegistrarInterface registrar(REGISTRAR_SERVICE, REGISTRAR_PATH, connection, window);
    QDBusPendingReply<> r = registrar.UnregisterWindow(static_cast<uint>(window->winId()));
    r.waitForFinished();
    if (r.isError())
        qWarning("Failed to unregister window menu, reason: %s (\"%s\")", qUtf8Printable(r.error().name()), qUtf8Printable(r.error().message()));
}

QT_END_NAMESPACE
