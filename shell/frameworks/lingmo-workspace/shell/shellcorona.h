/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2013 Ivan Cukic <ivan.cukic@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "config-X11.h"
#include "lingmo/corona.h"

#include <QDBusArgument>
#include <QDBusContext>
#include <QDBusVariant>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include <KConfigWatcher>
#include <KPackage/Package>

class DesktopView;
class PanelView;
class QMenu;
class QScreen;
class ScreenPool;
class StrutManager;
class ShellContainmentConfig;

namespace KActivities
{
class Controller;
} // namespace KActivities

namespace KDeclarative
{
class QmlObjectSharedEngine;
} // namespace KDeclarative

namespace KScreen
{
class Output;
} // namespace KScreen

namespace Lingmo
{
class Applet;
} // namespace Lingmo

namespace KWayland
{
namespace Client
{
class LingmoWindow;
class LingmoWindowManagement;
}
}

class ShellCorona : public Lingmo::Corona, QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(QString shell READ shell WRITE setShell)
    Q_PROPERTY(int numScreens READ numScreens)
    Q_PROPERTY(PanelView *panelBeingConfigured READ panelBeingConfigured NOTIFY panelBeingConfiguredChanged)
    Q_CLASSINFO("D-Bus Interface", "org.kde.LingmoShell")

public:
    explicit ShellCorona(QObject *parent = nullptr);
    ~ShellCorona() override;

    KPackage::Package lookAndFeelPackage();
    void init();

    /**
     * Where to save global configuration that doesn't have anything to do with the scene (e.g. views)
     */
    KSharedConfig::Ptr applicationConfig();

    int numScreens() const override;
    Q_INVOKABLE QRect screenGeometry(int id) const override;
    Q_INVOKABLE QRegion availableScreenRegion(int id) const override;
    Q_INVOKABLE QRect availableScreenRect(int id) const override;

    // lingmoshellCorona's value
    QRegion _availableScreenRegion(int id) const;
    QRect _availableScreenRect(int id) const;

    Q_INVOKABLE QStringList availableActivities() const;

    PanelView *panelView(Lingmo::Containment *containment) const;
    void savePreviousWindow();
    void restorePreviousWindow();
    void clearPreviousWindow();

    // This one is a bit of an hack but are just for desktop scripting
    void insertActivity(const QString &id, const QString &plugin);

    Lingmo::Containment *setContainmentTypeForScreen(int screen, const QString &plugin);

    void removeDesktop(DesktopView *desktopView);

    /**
     * @returns a new containment associated with the specified @p activity and @p screen.
     */
    Lingmo::Containment *createContainmentForActivity(const QString &activity, int screenNum);

    ScreenPool *screenPool() const;

    QList<int> screenIds() const;

    QString defaultContainmentPlugin() const;

    static QString defaultShell();

    // Set all Desktop containments (for all activities) associated to a given screen to this new screen.
    // swapping ones that had newScreen to oldScreen if existing. Panels are ignored
    void swapDesktopScreens(int oldScreen, int newScreen);

    // Set a single containment to a new screen.
    // If it is a Desktop contaiment, swap it with the other containment that was associated with same screen and activity if existent
    void setScreenForContainment(Lingmo::Containment *containment, int screen);

    // Grab a screenshot of the contaiment if it has a view in an async fashion
    // containmentPreviewReady will be emitted when done
    // If there is no view, this will have no effect
    void grabContainmentPreview(Lingmo::Containment *containment);

    // If a containment preview has been grabbed, for this containment, return its path
    QString containmentPreviewPath(Lingmo::Containment *containment) const;

    /**
     * Whether "accent color from wallpaper" option is enabled in global settings
     */
    bool accentColorFromWallpaperEnabled() const;

    Q_INVOKABLE QVariantMap wallpaper(uint screenNum = 0);

    Q_INVOKABLE void showAddPanelContextMenu(const QPoint pos);

    PanelView *panelBeingConfigured() const;

Q_SIGNALS:
    void glInitializationFailed();
    // A preview for this containment has been rendered and saved to disk
    void containmentPreviewReady(Lingmo::Containment *containment, const QString &path);
    void accentColorFromWallpaperEnabledChanged();
    void colorChanged(const QColor &color);
    // This API is used only by autotests, do we need something else?
    void screenOrderChanged(QList<QScreen *> screens);

    void wallpaperChanged(uint screenNum);
    void panelBeingConfiguredChanged();

public Q_SLOTS:
    /**
     * Request saving applicationConfig on disk, it's event compressed, not immediate
     */
    void requestApplicationConfigSync();

    /**
     * Cycle through all panels
     */
    void slotCyclePanelFocus();

    /**
     * Sets the shell that the corona should display
     */
    void setShell(const QString &shell);

    /**
     * Gets the currently shown shell
     */
    QString shell() const;

    /// DBUS methods
    void toggleDashboard();
    void setDashboardShown(bool show);
    void toggleActivityManager();
    void toggleWidgetExplorer();
    QString evaluateScript(const QString &string);
    void setWallpaper(const QString &wallpaperPlugin, const QVariantMap &parameters, uint screenNum = 0);
    void activateLauncherMenu();
    QRgb color() const;

    QByteArray dumpCurrentLayoutJS() const;

    /**
     * loads the shell layout from a look and feel package,
     * resetting it to the default layout exported in the
     * look and feel package
     */
    void loadLookAndFeelDefaultLayout(const QString &layout);

    Lingmo::Containment *addPanel(const QString &plugin);

    void nextActivity();
    void previousActivity();
    void stopCurrentActivity();

    void setTestModeLayout(const QString &layout)
    {
        m_testModeLayout = layout;
    }

    int panelCount() const
    {
        return m_panelViews.count();
    }

    void refreshCurrentShell();

protected Q_SLOTS:
    /**
     * Loads the layout and performs the needed checks
     */
    void load();

    /**
     * Unloads everything
     */
    void unload();

    /**
     * Loads the default (system wide) layout for this user
     **/
    void loadDefaultLayout() override;

    /**
     * Execute any update script
     */
    void processUpdateScripts();

    int screenForContainment(const Lingmo::Containment *containment) const override;

    void showAlternativesForApplet(Lingmo::Applet *applet);

private Q_SLOTS:
    void createWaitingPanels();
    void handleContainmentAdded(Lingmo::Containment *c);
    void syncAppConfig();
    void checkActivities();
    void currentActivityChanged(const QString &newActivity);
    void activityAdded(const QString &id);
    void activityRemoved(const QString &id);
    void checkAddPanelAction();
    void addPanel();
    void addPanel(QAction *action);
    void populateAddPanelsMenu();

    void addOutput(QScreen *screen);

    void panelContainmentDestroyed(QObject *cont);
    void handleScreenRemoved(QScreen *screen);
    void handleScreenOrderChanged(QList<QScreen *> screens);

    void activateTaskManagerEntry(int index);

private:
    void sanitizeScreenLayout(const QString &configFileName);
    void updateStruts();
    void configurationChanged(const QString &path);
    DesktopView *desktopForScreen(QScreen *screen) const;
    void setupWaylandIntegration();
    void executeSetupPlasmoidScript(Lingmo::Containment *containment, Lingmo::Applet *applet);
    void checkAllDesktopsUiReady(bool ready);
    void activateLauncherMenu(const QString &screenName);
    void handleColorRequestedFromDBus(const QDBusMessage &msg);

#ifndef NDEBUG
    void screenInvariants() const;
#endif

    KSharedConfig::Ptr m_config;
    QString m_configPath;

    // Accent color setting
    KConfigWatcher::Ptr m_accentColorConfigWatcher;
    bool m_accentColorFromWallpaperEnabled = false;
    QMetaObject::Connection m_fakeColorRequestConn;

    ScreenPool *m_screenPool;
    QString m_shell;
    KActivities::Controller *m_activityController;
    QMap<const Lingmo::Containment *, PanelView *> m_panelViews;
    // map from QScreen to desktop view
    QHash<int, DesktopView *> m_desktopViewForScreen;
    QHash<const Lingmo::Containment *, int> m_pendingScreenChanges;
    KConfigGroup m_desktopDefaultsConfig;
    KConfigGroup m_lnfDefaultsConfig;
    QList<Lingmo::Containment *> m_waitingPanels;
    QHash<QString, QString> m_activityContainmentPlugins;
    QAction *m_addPanelAction;
    std::unique_ptr<QMenu> m_addPanelsMenu;
    KPackage::Package m_lookAndFeelPackage;

#if HAVE_X11
    WId m_previousWId = 0;
#endif
    bool m_blockRestorePreviousWindow = false;

    QTimer m_waitingPanelsTimer;
    QTimer m_appConfigSyncTimer;
#ifndef NDEBUG
    QTimer m_invariantsTimer;
#endif
    // For getting the active window on Wayland
    KWayland::Client::LingmoWindowManagement *m_waylandWindowManagement = nullptr;
    QPointer<KWayland::Client::LingmoWindow> m_previousLingmoWindow;
    bool m_closingDown : 1;
    bool m_screenReorderInProgress = false;
    QString m_testModeLayout;
    Lingmo::Applet *m_showingAlternatives = nullptr;

    StrutManager *m_strutManager;
    QPointer<ShellContainmentConfig> m_shellContainmentConfig;
    friend class ShellTest;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, QColor &color);
QDBusArgument &operator<<(QDBusArgument &argument, const QColor &color);
