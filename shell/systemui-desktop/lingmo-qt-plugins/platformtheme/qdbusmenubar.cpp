#include "qdbusmenubar_p.h"


QT_BEGIN_NAMESPACE

/* note: do not change these to QStringLiteral;
   we are unloaded before QtDBus is done using the strings.
 */
#define REGISTRAR_SERVICE QLatin1String("com.canonical.AppMenu.Registrar")
#define REGISTRAR_PATH QLatin1String("/com/canonical/AppMenu/Registrar")

QDBusMenuBar::QDBusMenuBar()
    : QPlatformMenuBar()
    , m_menu(new QDBusPlatformMenu())
    , m_menuAdaptor(new QDBusMenuAdaptor(m_menu))
{
    QDBusMenuItem::registerDBusTypes();
    connect(m_menu, &QDBusPlatformMenu::propertiesUpdated,
            m_menuAdaptor, &QDBusMenuAdaptor::ItemsPropertiesUpdated);
    connect(m_menu, &QDBusPlatformMenu::updated,
            m_menuAdaptor, &QDBusMenuAdaptor::LayoutUpdated);

    // This signal is new in Qt 5.8 but distros might have backported it, hence a runtime look-up
    if (m_menu->metaObject()->indexOfSignal("popupRequested(int,uint)") != -1) {
        connect(m_menu, SIGNAL(popupRequested(int,uint)), m_menuAdaptor, SIGNAL(ItemActivationRequested(int,uint)));
    }
}

QDBusMenuBar::~QDBusMenuBar()
{
    unregisterMenuBar();
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
    if (newParentWindow == m_window) {
        return;
    }

    QWindow *oldWindow = m_window;

    unregisterMenuBar();
    m_window = newParentWindow;

    if (newParentWindow) {
        registerMenuBar();
    }

    emit windowChanged(newParentWindow, oldWindow);
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

void QDBusMenuBar::registerMenuBar()
{
    static uint menuBarId = 0;

    if (!m_window) {
        qWarning("Cannot register window menu without window");
        return;
    }

    QDBusConnection connection = QDBusConnection::sessionBus();
    m_objectPath = QStringLiteral("/MenuBar/%1").arg(++menuBarId);
    if (!connection.registerObject(m_objectPath, m_menu))
        return;

    QDBusMenuRegistrarInterface registrar(REGISTRAR_SERVICE, REGISTRAR_PATH, connection, this);
    QDBusPendingReply<> r = registrar.RegisterWindow(static_cast<uint>(window()->winId()), QDBusObjectPath(m_objectPath));
    r.waitForFinished();
    if (r.isError()) {
        qWarning("Failed to register window menu, reason: %s (\"%s\")",
                 qUtf8Printable(r.error().name()), qUtf8Printable(r.error().message()));
        connection.unregisterObject(m_objectPath);
    }
}

void QDBusMenuBar::unregisterMenuBar()
{
    QDBusConnection connection = QDBusConnection::sessionBus();

    if (m_window) {
        QDBusMenuRegistrarInterface registrar(REGISTRAR_SERVICE, REGISTRAR_PATH, connection, this);
        QDBusPendingReply<> r = registrar.UnregisterWindow(static_cast<uint>(window()->winId()));
        r.waitForFinished();
        if (r.isError())
            qWarning("Failed to unregister window menu, reason: %s (\"%s\")",
                     qUtf8Printable(r.error().name()), qUtf8Printable(r.error().message()));
    }

    if (!m_objectPath.isEmpty())
        connection.unregisterObject(m_objectPath);
}

QT_END_NAMESPACE
