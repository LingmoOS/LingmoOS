/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kmainwindow.h"

#include "kmainwindow_p.h"
#ifdef QT_DBUS_LIB
#include "kmainwindowiface_p.h"
#endif
#include "khelpmenu.h"
#include "ktoolbar.h"
#include "ktoolbarhandler_p.h"
#include "ktooltiphelper.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFile>
#include <QList>
#include <QMenuBar>
#include <QObject>
#ifndef QT_NO_SESSIONMANAGER
#include <QSessionManager>
#endif
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#ifdef QT_DBUS_LIB
#include <QDBusConnection>
#endif

#include <KAboutData>
#include <KConfig>
#include <KConfigGroup>
#include <KConfigGui>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

//#include <cctype>

static const char WINDOW_PROPERTIES[] = "WindowProperties";

static QMenuBar *internalMenuBar(KMainWindow *mw)
{
    return mw->findChild<QMenuBar *>(QString(), Qt::FindDirectChildrenOnly);
}

static QStatusBar *internalStatusBar(KMainWindow *mw)
{
    return mw->findChild<QStatusBar *>(QString(), Qt::FindDirectChildrenOnly);
}

/**

 * Listens to resize events from QDockWidgets. The KMainWindow
 * settings are set as dirty, as soon as at least one resize
 * event occurred. The listener is attached to the dock widgets
 * by dock->installEventFilter(dockResizeListener) inside
 * KMainWindow::event().
 */
class DockResizeListener : public QObject
{
    Q_OBJECT
public:
    DockResizeListener(KMainWindow *win);
    ~DockResizeListener() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    KMainWindow *const m_win;
};

DockResizeListener::DockResizeListener(KMainWindow *win)
    : QObject(win)
    , m_win(win)
{
}

DockResizeListener::~DockResizeListener()
{
}

bool DockResizeListener::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Resize:
    case QEvent::Move:
    case QEvent::Show:
    case QEvent::Hide:
        m_win->k_ptr->setSettingsDirty(KMainWindowPrivate::CompressCalls);
        break;

    default:
        break;
    }

    return QObject::eventFilter(watched, event);
}

KMWSessionManager::KMWSessionManager()
{
#ifndef QT_NO_SESSIONMANAGER
    connect(qApp, &QGuiApplication::saveStateRequest, this, &KMWSessionManager::saveState);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &KMWSessionManager::commitData);
#endif
}

KMWSessionManager::~KMWSessionManager()
{
}

void KMWSessionManager::saveState(QSessionManager &sm)
{
#ifndef QT_NO_SESSIONMANAGER
    KConfigGui::setSessionConfig(sm.sessionId(), sm.sessionKey());

    KConfig *config = KConfigGui::sessionConfig();
    const auto windows = KMainWindow::memberList();
    if (!windows.isEmpty()) {
        // According to Jochen Wilhelmy <digisnap@cs.tu-berlin.de>, this
        // hook is useful for better document orientation
        windows.at(0)->saveGlobalProperties(config);
    }

    int n = 0;
    for (KMainWindow *mw : windows) {
        n++;
        mw->savePropertiesInternal(config, n);
    }

    KConfigGroup group(config, "Number");
    group.writeEntry("NumberOfWindows", n);

    // store new status to disk
    config->sync();

    // generate discard command for new file
    QString localFilePath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + config->name();
    if (QFile::exists(localFilePath)) {
        QStringList discard;
        discard << QStringLiteral("rm");
        discard << localFilePath;
        sm.setDiscardCommand(discard);
    }
#else
    Q_UNUSED(sm)
#endif // QT_NO_SESSIONMANAGER
}

void KMWSessionManager::commitData(QSessionManager &sm)
{
#ifndef QT_NO_SESSIONMANAGER
    if (!sm.allowsInteraction()) {
        return;
    }

    /*
       Purpose of this exercise: invoke queryClose() without actually closing the
       windows, because
       - queryClose() may contain session management code, so it must be invoked
       - actually closing windows may quit the application - cf.
         QGuiApplication::quitOnLastWindowClosed()
       - quitting the application and thus closing the session manager connection
         violates the X11 XSMP protocol.
         The exact requirement of XSMP that would be broken is,
         in the description of the client's state machine:

           save-yourself-done: (changing state is forbidden)

         Closing the session manager connection causes a state change.
         Worst of all, that is a real problem with ksmserver - it will not save
         applications that quit on their own in state save-yourself-done.
     */
    const auto windows = KMainWindow::memberList();
    for (KMainWindow *window : windows) {
        if (window->testAttribute(Qt::WA_WState_Hidden)) {
            continue;
        }
        QCloseEvent e;
        QApplication::sendEvent(window, &e);
        if (!e.isAccepted()) {
            sm.cancel();
            return;
        }
    }
#else
    Q_UNUSED(sm)
#endif // QT_NO_SESSIONMANAGER
}

Q_GLOBAL_STATIC(KMWSessionManager, ksm)
Q_GLOBAL_STATIC(QList<KMainWindow *>, sMemberList)

KMainWindow::KMainWindow(QWidget *parent, Qt::WindowFlags f)
    : QMainWindow(parent, f)
    , k_ptr(new KMainWindowPrivate)
{
    Q_D(KMainWindow);

    d->init(this);
}

KMainWindow::KMainWindow(KMainWindowPrivate &dd, QWidget *parent, Qt::WindowFlags f)
    : QMainWindow(parent, f)
    , k_ptr(&dd)
{
    Q_D(KMainWindow);

    d->init(this);
}

void KMainWindowPrivate::init(KMainWindow *_q)
{
    q = _q;

    // Not needed in Qt6 (and doesn't exist at all)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
#endif
#endif

    q->setAnimated(q->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, q));

    q->setAttribute(Qt::WA_DeleteOnClose);

    helpMenu = nullptr;

    // actionCollection()->setWidget( this );
#if 0
    QObject::connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)),
                     q, SLOT(_k_slotSettingsChanged(int)));
#endif

#ifndef QT_NO_SESSIONMANAGER
    // force KMWSessionManager creation
    ksm();
#endif

    sMemberList()->append(q);

    // Set the icon theme fallback to breeze (if not already set)
    // Most of our apps use "lots" of icons that most of the times
    // are only available with breeze, we still honour the user icon
    // theme but if the icon is not found there, we go to breeze
    // since it's almost sure it'll be there.
    // This should be done as soon as possible (preferably via
    // Q_COREAPP_STARTUP_FUNCTION), but as for now it cannot be done too soon
    // as at that point QPlatformTheme is not instantiated yet and breaks the
    // internal status of QIconLoader (see QTBUG-74252).
    // See also discussion at https://phabricator.kde.org/D22488
    // TODO: remove this once we depend on Qt 5.15.1, where this is fixed
    if (QIcon::fallbackThemeName().isEmpty()) {
        QIcon::setFallbackThemeName(QStringLiteral("breeze"));
    }

    // If application is translated, load translator information for use in
    // KAboutApplicationDialog or other getters. The context and messages below
    // both must be exactly as listed, and are forced to be loaded from the
    // application's own message catalog instead of kxmlgui's.
    KAboutData aboutData(KAboutData::applicationData());
    if (aboutData.translators().isEmpty()) {
        aboutData.setTranslator(i18ndc(nullptr, "NAME OF TRANSLATORS", "Your names"), //
                                i18ndc(nullptr, "EMAIL OF TRANSLATORS", "Your emails"));

        KAboutData::setApplicationData(aboutData);
    }

    settingsDirty = false;
    autoSaveSettings = false;
    autoSaveWindowSize = true; // for compatibility
    // d->kaccel = actionCollection()->kaccel();
    settingsTimer = nullptr;
    sizeTimer = nullptr;

    dockResizeListener = new DockResizeListener(_q);
    letDirtySettings = true;

    sizeApplied = false;
    suppressCloseEvent = false;

    qApp->installEventFilter(KToolTipHelper::instance());
}

static bool endsWithHashNumber(const QString &s)
{
    for (int i = s.length() - 1; i > 0; --i) {
        if (s[i] == QLatin1Char('#') && i != s.length() - 1) {
            return true; // ok
        }
        if (!s[i].isDigit()) {
            break;
        }
    }
    return false;
}

static inline bool isValidDBusObjectPathCharacter(const QChar &c)
{
    ushort u = c.unicode();
    /* clang-format off */
    return (u >= QLatin1Char('a') && u <= QLatin1Char('z'))
        || (u >= QLatin1Char('A') && u <= QLatin1Char('Z'))
        || (u >= QLatin1Char('0') && u <= QLatin1Char('9'))
        || (u == QLatin1Char('_')) || (u == QLatin1Char('/'));
    /* clang-format off */
}

void KMainWindowPrivate::polish(KMainWindow *q)
{
    // Set a unique object name. Required by session management, window management, and for the dbus interface.
    QString objname;
    QString s;
    int unusedNumber = 1;
    const QString name = q->objectName();
    bool startNumberingImmediately = true;
    bool tryReuse = false;
    if (name.isEmpty()) {
        // no name given
        objname = QStringLiteral("MainWindow#");
    } else if (name.endsWith(QLatin1Char('#'))) {
        // trailing # - always add a number  - KWin uses this for better grouping
        objname = name;
    } else if (endsWithHashNumber(name)) {
        // trailing # with a number - like above, try to use the given number first
        objname = name;
        tryReuse = true;
        startNumberingImmediately = false;
    } else {
        objname = name;
        startNumberingImmediately = false;
    }

    s = objname;
    if (startNumberingImmediately) {
        s += QLatin1Char('1');
    }

    for (;;) {
        const QList<QWidget *> list = qApp->topLevelWidgets();
        bool found = false;
        for (QWidget *w : list) {
            if (w != q && w->objectName() == s) {
                found = true;
                break;
            }
        }
        if (!found) {
            break;
        }
        if (tryReuse) {
            objname = name.left(name.length() - 1); // lose the hash
            unusedNumber = 0; // start from 1 below
            tryReuse = false;
        }
        s.setNum(++unusedNumber);
        s = objname + s;
    }
    q->setObjectName(s);
    if (!q->window() || q->window() == q) {
        q->winId(); // workaround for setWindowRole() crashing, and set also window role, just in case TT
        q->setWindowRole(s); // will keep insisting that object name suddenly should not be used for window role
    }

    dbusName = QLatin1Char('/') + QCoreApplication::applicationName() + QLatin1Char('/');
    dbusName += q->objectName().replace(QLatin1Char('/'), QLatin1Char('_'));
    // Clean up for dbus usage: any non-alphanumeric char should be turned into '_'
    for (QChar &c : dbusName) {
        if (!isValidDBusObjectPathCharacter(c)) {
            c = QLatin1Char('_');
        }
    }

#ifdef QT_DBUS_LIB
    /* clang-format off */
    constexpr auto opts = QDBusConnection::ExportScriptableSlots
                          | QDBusConnection::ExportScriptableProperties
                          | QDBusConnection::ExportNonScriptableSlots
                          | QDBusConnection::ExportNonScriptableProperties
                          | QDBusConnection::ExportAdaptors;
    /* clang-format on */
    QDBusConnection::sessionBus().registerObject(dbusName, q, opts);
#endif
}

void KMainWindowPrivate::setSettingsDirty(CallCompression callCompression)
{
    if (!letDirtySettings) {
        return;
    }

    settingsDirty = true;
    if (autoSaveSettings) {
        if (callCompression == CompressCalls) {
            if (!settingsTimer) {
                settingsTimer = new QTimer(q);
                settingsTimer->setInterval(500);
                settingsTimer->setSingleShot(true);
                QObject::connect(settingsTimer, &QTimer::timeout, q, &KMainWindow::saveAutoSaveSettings);
            }
            settingsTimer->start();
        } else {
            q->saveAutoSaveSettings();
        }
    }
}

void KMainWindowPrivate::setSizeDirty()
{
    if (autoSaveWindowSize) {
        if (!sizeTimer) {
            sizeTimer = new QTimer(q);
            sizeTimer->setInterval(500);
            sizeTimer->setSingleShot(true);
            QObject::connect(sizeTimer, &QTimer::timeout, q, [this]() {
                _k_slotSaveAutoSaveSize();
            });
        }
        sizeTimer->start();
    }
}

KMainWindow::~KMainWindow()
{
    sMemberList()->removeAll(this);
    delete static_cast<QObject *>(k_ptr->dockResizeListener); // so we don't get anymore events after k_ptr is destroyed
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
QMenu *KMainWindow::helpMenu(const QString &aboutAppText, bool showWhatsThis)
{
    Q_D(KMainWindow);
    if (!d->helpMenu) {
        if (aboutAppText.isEmpty()) {
            d->helpMenu = new KHelpMenu(this, KAboutData::applicationData(), showWhatsThis);
        } else {
            d->helpMenu = new KHelpMenu(this, aboutAppText, showWhatsThis);
        }

        if (!d->helpMenu) {
            return nullptr;
        }
    }

    return d->helpMenu->menu();
}

QMenu *KMainWindow::customHelpMenu(bool showWhatsThis)
{
    Q_D(KMainWindow);
    if (!d->helpMenu) {
        d->helpMenu = new KHelpMenu(this, QString(), showWhatsThis);
        connect(d->helpMenu, &KHelpMenu::showAboutApplication, this, &KMainWindow::showAboutApplication);
    }

    return d->helpMenu->menu();
}
#endif

bool KMainWindow::canBeRestored(int number)
{
    KConfig *config = KConfigGui::sessionConfig();
    if (!config) {
        return false;
    }

    KConfigGroup group(config, "Number");
    // TODO KF6: we should use 0 as the default value, not 1
    // See also https://bugs.kde.org/show_bug.cgi?id=427552
    const int n = group.readEntry("NumberOfWindows", 1);
    return number >= 1 && number <= n;
}

const QString KMainWindow::classNameOfToplevel(int number)
{
    KConfig *config = KConfigGui::sessionConfig();
    if (!config) {
        return QString();
    }

    KConfigGroup group(config, QByteArray(WINDOW_PROPERTIES).append(QByteArray::number(number)).constData());
    if (!group.hasKey("ClassName")) {
        return QString();
    } else {
        return group.readEntry("ClassName");
    }
}

bool KMainWindow::restore(int number, bool show)
{
    if (!canBeRestored(number)) {
        return false;
    }
    KConfig *config = KConfigGui::sessionConfig();
    if (readPropertiesInternal(config, number)) {
        if (show) {
            KMainWindow::show();
        }
        return false;
    }
    return false;
}

void KMainWindow::setCaption(const QString &caption)
{
    setPlainCaption(caption);
}

void KMainWindow::setCaption(const QString &caption, bool modified)
{
    QString title = caption;
    if (!title.contains(QLatin1String("[*]")) && !title.isEmpty()) { // append the placeholder so that the modified mechanism works
        title.append(QLatin1String(" [*]"));
    }
    setPlainCaption(title);
    setWindowModified(modified);
}

void KMainWindow::setPlainCaption(const QString &caption)
{
    setWindowTitle(caption);
}

void KMainWindow::appHelpActivated()
{
    Q_D(KMainWindow);
    if (!d->helpMenu) {
        d->helpMenu = new KHelpMenu(this);
        if (!d->helpMenu) {
            return;
        }
    }
    d->helpMenu->appHelpActivated();
}

void KMainWindow::closeEvent(QCloseEvent *e)
{
    Q_D(KMainWindow);
    if (d->suppressCloseEvent) {
        e->accept();
        return;
    }

    // Save settings if auto-save is enabled, and settings have changed
    if (d->settingsTimer && d->settingsTimer->isActive()) {
        d->settingsTimer->stop();
        saveAutoSaveSettings();
    }
    if (d->sizeTimer && d->sizeTimer->isActive()) {
        d->sizeTimer->stop();
        d->_k_slotSaveAutoSaveSize();
    }
    // Delete the marker that says we don't want to restore the position of the
    // next-opened instance; now that a window is closing, we do want to do this
    if (d->autoSaveGroup.isValid()) {
        d->getValidStateConfig(d->autoSaveGroup).deleteEntry("RestorePositionForNextInstance");
    }
    d->_k_slotSaveAutoSavePosition();

    if (queryClose()) {
        // widgets will start destroying themselves at this point and we don't
        // want to save state anymore after this as it might be incorrect
        d->autoSaveSettings = false;
        d->letDirtySettings = false;
        e->accept();
    } else {
        e->ignore(); // if the window should not be closed, don't close it
    }

#ifndef QT_NO_SESSIONMANAGER
    // If saving session, we are processing a fake close event, and might get the real one later.
    if (e->isAccepted() && qApp->isSavingSession()) {
        d->suppressCloseEvent = true;
    }
#endif
}

bool KMainWindow::queryClose()
{
    return true;
}

void KMainWindow::saveGlobalProperties(KConfig *)
{
}

void KMainWindow::readGlobalProperties(KConfig *)
{
}

void KMainWindow::savePropertiesInternal(KConfig *config, int number)
{
    Q_D(KMainWindow);
    const bool oldASWS = d->autoSaveWindowSize;
    d->autoSaveWindowSize = true; // make saveMainWindowSettings save the window size

    KConfigGroup cg(config, QByteArray(WINDOW_PROPERTIES).append(QByteArray::number(number)).constData());

    // store objectName, className, Width and Height  for later restoring
    // (Only useful for session management)
    cg.writeEntry("ObjectName", objectName());
    cg.writeEntry("ClassName", metaObject()->className());

    saveMainWindowSettings(cg); // Menubar, statusbar and Toolbar settings.

    cg = KConfigGroup(config, QByteArray::number(number).constData());
    saveProperties(cg);

    d->autoSaveWindowSize = oldASWS;
}

void KMainWindow::saveMainWindowSettings(KConfigGroup &cg)
{
    Q_D(KMainWindow);
    // qDebug(200) << "KMainWindow::saveMainWindowSettings " << cg.name();

    KConfigGroup stateConfig = d->getValidStateConfig(cg);
    // Called by session management - or if we want to save the window size anyway
    if (d->autoSaveWindowSize) {
        KWindowConfig::saveWindowSize(windowHandle(), stateConfig);
        KWindowConfig::saveWindowPosition(windowHandle(), stateConfig);
    }

    // One day will need to save the version number, but for now, assume 0
    // Utilise the QMainWindow::saveState() functionality.
    const QByteArray state = saveState();
    stateConfig.writeEntry("State", state.toBase64());

    QStatusBar *sb = internalStatusBar(this);
    if (sb) {
        if (!cg.hasDefault("StatusBar") && !sb->isHidden()) {
            cg.revertToDefault("StatusBar");
        } else {
            cg.writeEntry("StatusBar", sb->isHidden() ? "Disabled" : "Enabled");
        }
    }

    QMenuBar *mb = internalMenuBar(this);
    if (mb) {
        if (!cg.hasDefault("MenuBar") && !mb->isHidden()) {
            cg.revertToDefault("MenuBar");
        } else {
            cg.writeEntry("MenuBar", mb->isHidden() ? "Disabled" : "Enabled");
        }
    }

    if (!autoSaveSettings() || cg.name() == autoSaveGroup()) {
        // TODO should be cg == d->autoSaveGroup, to compare both kconfig and group name
        if (!cg.hasDefault("ToolBarsMovable") && !KToolBar::toolBarsLocked()) {
            cg.revertToDefault("ToolBarsMovable");
        } else {
            cg.writeEntry("ToolBarsMovable", KToolBar::toolBarsLocked() ? "Disabled" : "Enabled");
        }
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    const auto toolBars = this->toolBars();
    for (KToolBar *toolbar : toolBars) {
        QByteArray groupName("Toolbar");
        // Give a number to the toolbar, but prefer a name if there is one,
        // because there's no real guarantee on the ordering of toolbars
        groupName += (toolbar->objectName().isEmpty() ? QByteArray::number(n) : QByteArray(" ").append(toolbar->objectName().toUtf8()));

        KConfigGroup toolbarGroup(&cg, groupName.constData());
        toolbar->saveSettings(toolbarGroup);
        n++;
    }
}

bool KMainWindow::readPropertiesInternal(KConfig *config, int number)
{
    Q_D(KMainWindow);

    const bool oldLetDirtySettings = d->letDirtySettings;
    d->letDirtySettings = false;

    if (number == 1) {
        readGlobalProperties(config);
    }

    // in order they are in toolbar list
    KConfigGroup cg(config, QByteArray(WINDOW_PROPERTIES).append(QByteArray::number(number)).constData());

    // restore the object name (window role)
    if (cg.hasKey("ObjectName")) {
        setObjectName(cg.readEntry("ObjectName"));
    }

    d->sizeApplied = false; // since we are changing config file, reload the size of the window
    // if necessary. Do it before the call to applyMainWindowSettings.
    applyMainWindowSettings(cg); // Menubar, statusbar and toolbar settings.

    KConfigGroup grp(config, QByteArray::number(number).constData());
    readProperties(grp);

    d->letDirtySettings = oldLetDirtySettings;

    return true;
}

void KMainWindow::applyMainWindowSettings(const KConfigGroup &_cg)
{
    Q_D(KMainWindow);
    // qDebug(200) << "KMainWindow::applyMainWindowSettings " << cg.name();

    KConfigGroup cg = _cg;
    d->migrateStateDataIfNeeded(cg);

    QWidget *focusedWidget = QApplication::focusWidget();

    const bool oldLetDirtySettings = d->letDirtySettings;
    d->letDirtySettings = false;

    KConfigGroup stateConfig = d->getValidStateConfig(cg);

    if (!d->sizeApplied && (!window() || window() == this)) {
        winId(); // ensure there's a window created
        // Set the window's size from the existing widget geometry to respect the
        // implicit size when there is no saved geometry in the config file for
        // KWindowConfig::restoreWindowSize() to restore
        // TODO: remove once QTBUG-40584 is fixed; see below
        windowHandle()->setWidth(width());
        windowHandle()->setHeight(height());
        KWindowConfig::restoreWindowSize(windowHandle(), stateConfig);
        // NOTICE: QWindow::setGeometry() does NOT impact the backing QWidget geometry even if the platform
        // window was created -> QTBUG-40584. We therefore copy the size here.
        // TODO: remove once this was resolved in QWidget QPA
        resize(windowHandle()->size());
        d->sizeApplied = true;

        // Let the user opt out of KDE apps remembering window sizes if they
        // find it annoying or it doesn't work for them due to other bugs.
        KSharedConfigPtr config = KSharedConfig::openConfig();
        KConfigGroup group(config, "General");
        if (group.readEntry("AllowKDEAppsToRememberWindowPositions", true)) {
            if (stateConfig.readEntry("RestorePositionForNextInstance", true)) {
                KWindowConfig::restoreWindowPosition(windowHandle(), stateConfig);
                // Save the fact that we now don't want to restore position
                // anymore; if we did, the next instance would completely cover
                // the existing one
                stateConfig.writeEntry("RestorePositionForNextInstance", false);
            }
        }
    }

    QStatusBar *sb = internalStatusBar(this);
    if (sb) {
        QString entry = cg.readEntry("StatusBar", "Enabled");
        sb->setVisible(entry != QLatin1String("Disabled"));
    }

    QMenuBar *mb = internalMenuBar(this);
    if (mb) {
        QString entry = cg.readEntry("MenuBar", "Enabled");
        mb->setVisible(entry != QLatin1String("Disabled"));
    }

    if (!autoSaveSettings() || cg.name() == autoSaveGroup()) { // TODO should be cg == d->autoSaveGroup, to compare both kconfig and group name
        QString entry = cg.readEntry("ToolBarsMovable", "Disabled");
        KToolBar::setToolBarsLocked(entry == QLatin1String("Disabled"));
    }

    int n = 1; // Toolbar counter. toolbars are counted from 1,
    const auto toolBars = this->toolBars();
    for (KToolBar *toolbar : toolBars) {
        QByteArray groupName("Toolbar");
        // Give a number to the toolbar, but prefer a name if there is one,
        // because there's no real guarantee on the ordering of toolbars
        groupName += (toolbar->objectName().isEmpty() ? QByteArray::number(n) : QByteArray(" ").append(toolbar->objectName().toUtf8()));

        KConfigGroup toolbarGroup(&cg, groupName.constData());
        toolbar->applySettings(toolbarGroup);
        n++;
    }

    if (stateConfig.hasKey("State")) {
        QByteArray state;
        state = stateConfig.readEntry("State", state);
        state = QByteArray::fromBase64(state);
        // One day will need to load the version number, but for now, assume 0
        restoreState(state);
    }

    if (focusedWidget) {
        focusedWidget->setFocus();
    }

    d->settingsDirty = false;
    d->letDirtySettings = oldLetDirtySettings;
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KMainWindow::restoreWindowSize(const KConfigGroup &cg)
{
    KWindowConfig::restoreWindowSize(windowHandle(), cg);
}
#endif

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KMainWindow::saveWindowSize(KConfigGroup &cg) const
{
    KWindowConfig::saveWindowSize(windowHandle(), cg);
}
#endif

void KMainWindow::setSettingsDirty()
{
    Q_D(KMainWindow);
    d->setSettingsDirty();
}

bool KMainWindow::settingsDirty() const
{
    Q_D(const KMainWindow);
    return d->settingsDirty;
}

void KMainWindow::setAutoSaveSettings(const QString &groupName, bool saveWindowSize)
{
    setAutoSaveSettings(KConfigGroup(KSharedConfig::openConfig(), groupName), saveWindowSize);
}

void KMainWindow::setAutoSaveSettings(const KConfigGroup &group, bool saveWindowSize)
{
    // We re making a little assumption that if you want to save the window
    // size, you probably also want to save the window position too
    // This avoids having to re-implement a new version of
    // KMainWindow::setAutoSaveSettings that handles these cases independently
    Q_D(KMainWindow);
    d->autoSaveSettings = true;
    d->autoSaveGroup = group;
    d->autoSaveWindowSize = saveWindowSize;

    if (!saveWindowSize && d->sizeTimer) {
        d->sizeTimer->stop();
    }

    // Now read the previously saved settings
    applyMainWindowSettings(d->autoSaveGroup);
}

void KMainWindow::resetAutoSaveSettings()
{
    Q_D(KMainWindow);
    d->autoSaveSettings = false;
    if (d->settingsTimer) {
        d->settingsTimer->stop();
    }
}

bool KMainWindow::autoSaveSettings() const
{
    Q_D(const KMainWindow);
    return d->autoSaveSettings;
}

QString KMainWindow::autoSaveGroup() const
{
    Q_D(const KMainWindow);
    return d->autoSaveSettings ? d->autoSaveGroup.name() : QString();
}

KConfigGroup KMainWindow::autoSaveConfigGroup() const
{
    Q_D(const KMainWindow);
    return d->autoSaveSettings ? d->autoSaveGroup : KConfigGroup();
}

void KMainWindow::setStateConfigGroup(const QString &configGroup)
{
    Q_D(KMainWindow);
    d->m_stateConfigGroup = KSharedConfig::openStateConfig()->group(configGroup);
}

KConfigGroup KMainWindow::stateConfigGroup() const
{
    Q_D(const KMainWindow);
    return d->m_stateConfigGroup;
}

void KMainWindow::saveAutoSaveSettings()
{
    Q_D(KMainWindow);
    Q_ASSERT(d->autoSaveSettings);
    // qDebug(200) << "KMainWindow::saveAutoSaveSettings -> saving settings";
    saveMainWindowSettings(d->autoSaveGroup);
    d->autoSaveGroup.sync();
    d->autoSaveStateGroup().sync();
    d->settingsDirty = false;
}

bool KMainWindow::event(QEvent *ev)
{
    Q_D(KMainWindow);
    switch (ev->type()) {
#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    case QEvent::Move:
#endif
    case QEvent::Resize:
        d->setSizeDirty();
        break;
    case QEvent::Polish:
        d->polish(this);
        break;
    case QEvent::ChildPolished: {
        QChildEvent *event = static_cast<QChildEvent *>(ev);
        QDockWidget *dock = qobject_cast<QDockWidget *>(event->child());
        KToolBar *toolbar = qobject_cast<KToolBar *>(event->child());
        QMenuBar *menubar = qobject_cast<QMenuBar *>(event->child());
        if (dock) {
            connect(dock, &QDockWidget::dockLocationChanged, this, &KMainWindow::setSettingsDirty);
            connect(dock, &QDockWidget::topLevelChanged, this, &KMainWindow::setSettingsDirty);

            // there is no signal emitted if the size of the dock changes,
            // hence install an event filter instead
            dock->installEventFilter(d->dockResizeListener);
        } else if (toolbar) {
            // there is no signal emitted if the size of the toolbar changes,
            // hence install an event filter instead
            toolbar->installEventFilter(d->dockResizeListener);
        } else if (menubar) {
            // there is no signal emitted if the size of the menubar changes,
            // hence install an event filter instead
            menubar->installEventFilter(d->dockResizeListener);
        }
        break;
    }
    case QEvent::ChildRemoved: {
        QChildEvent *event = static_cast<QChildEvent *>(ev);
        QDockWidget *dock = qobject_cast<QDockWidget *>(event->child());
        KToolBar *toolbar = qobject_cast<KToolBar *>(event->child());
        QMenuBar *menubar = qobject_cast<QMenuBar *>(event->child());
        if (dock) {
            disconnect(dock, &QDockWidget::dockLocationChanged, this, &KMainWindow::setSettingsDirty);
            disconnect(dock, &QDockWidget::topLevelChanged, this, &KMainWindow::setSettingsDirty);
            dock->removeEventFilter(d->dockResizeListener);
        } else if (toolbar) {
            toolbar->removeEventFilter(d->dockResizeListener);
        } else if (menubar) {
            menubar->removeEventFilter(d->dockResizeListener);
        }
        break;
    }
    default:
        break;
    }
    return QMainWindow::event(ev);
}

bool KMainWindow::hasMenuBar()
{
    return internalMenuBar(this);
}

void KMainWindowPrivate::_k_slotSettingsChanged(int category)
{
    Q_UNUSED(category);

    // This slot will be called when the style KCM changes settings that need
    // to be set on the already running applications.

    // At this level (KMainWindow) the only thing we need to restore is the
    // animations setting (whether the user wants builtin animations or not).

    q->setAnimated(q->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, q));
}

void KMainWindowPrivate::_k_slotSaveAutoSaveSize()
{
    if (autoSaveGroup.isValid()) {
        KWindowConfig::saveWindowSize(q->windowHandle(), autoSaveStateGroup());
    }
}

void KMainWindowPrivate::_k_slotSaveAutoSavePosition()
{
    if (autoSaveGroup.isValid()) {
        KWindowConfig::saveWindowPosition(q->windowHandle(), autoSaveStateGroup());
    }
}

KToolBar *KMainWindow::toolBar(const QString &name)
{
    QString childName = name;
    if (childName.isEmpty()) {
        childName = QStringLiteral("mainToolBar");
    }

    KToolBar *tb = findChild<KToolBar *>(childName);
    if (tb) {
        return tb;
    }

    KToolBar *toolbar = new KToolBar(childName, this); // non-XMLGUI toolbar
    return toolbar;
}

QList<KToolBar *> KMainWindow::toolBars() const
{
    QList<KToolBar *> ret;

    const auto theChildren = children();
    for (QObject *child : theChildren) {
        if (KToolBar *toolBar = qobject_cast<KToolBar *>(child)) {
            ret.append(toolBar);
        }
    }

    return ret;
}

QList<KMainWindow *> KMainWindow::memberList()
{
    return *sMemberList();
}

QString KMainWindow::dbusName() const
{
    Q_D(const KMainWindow);

    return d->dbusName;
}

#include "kmainwindow.moc"
#include "moc_kmainwindow.cpp"
