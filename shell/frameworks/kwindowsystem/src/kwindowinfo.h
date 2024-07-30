/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2007 Lubos Lunak <l.lunak@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
/*
 * kwindowinfo.h. Part of the KDE project.
 */

#ifndef KWINDOWINFO_H
#define KWINDOWINFO_H

#include <QExplicitlySharedDataPointer>
#include <QStringList>
#include <QWidgetList> //For WId
#include <kwindowsystem_export.h>

#include <netwm_def.h>

class KWindowInfoPrivate;

/**
 * This class provides information about a given X11 window. It provides the information
 * for the current state when a KWindowInfo instance gets created.
 * The instance does not get updated when the
 * window changes. To get update about window changes connect to the
 * @link KX11Extras::windowChanged windowChanged@endlink signal of KX11Extras
 * and create a new KWindowInfo instance to reflect the current state.
 *
 * KWindowInfo does not encapsulate all information about the window. One needs to
 * request which information is required by passing the appropriate NET::Property and
 * NET::Property2 flags to the constructor. Please refer to the documentation of the
 * methods to see which flags are required. This is done to limit the interaction with
 * the xserver and avoid excess roundtrips.
 *
 * KWindowInfo does nothing when running outside of X11.
 *
 * Example usage of this class illustrated by monitoring a QWidget for change of the
 * demands attention window state:
 *
 * @code
 * QWidget *widget = new QWidget(nullptr);
 * widget->show(); // ensures native window gets created
 * connect(KX11Extras::self(), &KX11Extras::KWindowSystem::windowChanged,
 *        [window](WId id, NET::Properties properties, NET::Properties2 properties2) {
 *     if (widget->winId() != winId) {
 *         return; // not our window
 *     }
 *     if (properties & NET::WMState) {
 *         // let's check whether our window is demanding attention
 *         KWindowInfo info(widget->winId(), NET::WMState);
 *         qDebug() << "Has demands attention: " << info.hasState(NET::DemandsAttention);
 *     }
 * });
 * @endcode
 */
class KWINDOWSYSTEM_EXPORT KWindowInfo
{
public:
    /**
     * Reads all the info about the given window.
     *
     * Only the information requested through the @p properties and @p properties2
     * parameters are fetched. Refer to the methods you are interested in to see
     * which flags to pass.
     *
     * @param window The platform specific window identifier
     * @param properties Bitmask of NET::Property
     * @param properties2 Bitmask of NET::Property2
     */
    KWindowInfo(WId window, NET::Properties properties, NET::Properties2 properties2 = NET::Properties2());
    ~KWindowInfo();
    /**
     * Returns false if this window info is not valid.
     *
     * In case the window does not exist @c false is returned.
     *
     * @param withdrawn_is_valid if true, windows in the withdrawn state
     *        (i.e. not managed) are also considered. This is usually not the case.
     */
    bool valid(bool withdrawn_is_valid = false) const;
    /**
     * Returns the window identifier.
     */
    WId win() const;
    /**
     * Returns the window's state flags.
     *
     * Requires NET::WMState passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMState);
     * if (info.valid())
     *     info.state();
     * @endcode
     *
     * @see NET::State
     */
    NET::States state() const;
    /**
     * Returns true if the window has the given state flag set.
     *
     * Requires NET::WMState passed as properties parameter to the constructor.
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMState);
     * if (info.valid())
     *     info.hasState(NET::DemandsAttention);
     * @endcode
     *
     * @see NET::State
     */
    bool hasState(NET::States s) const;
    /**
     * Returns true if the window is minimized.
     *
     * Note that it is true only if the window is truly minimized,
     * not shaded or on another virtual desktops,
     * which makes it different from mappingState() == NET::Iconic
     * or QWidget::isMinimized().
     * Requires NET::WMState and NET::XAWMState passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMState | NET::XAWMState);
     * if (info.valid())
     *     info.isMinimized();
     * @endcode
     */
    bool isMinimized() const;
    /**
     * Returns the mapping state of the window.
     *
     * Note that it's very likely that you don't want to use this function,
     * and use isOnDesktop(), isMinimized() etc. instead.
     * Requires NET::XAWMState passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::XAWMState);
     * if (info.valid())
     *     info.mappingState();
     * @endcode
     *
     * @see NET::MappingState
     * @see isOnDesktop()
     * @see isMinimzed()
     */
    NET::MappingState mappingState() const;
    /**
     * Returns the window extended (partial) strut.
     *
     * Requires NET::WM2ExtendedStrut passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2ExtendedStrut);
     * if (info.valid())
     *     info.extendedStrut();
     * @endcode
     */
    NETExtendedStrut extendedStrut() const;
    /**
     * Returns the window type of this window.
     *
     * The argument should be all window types your application supports.
     * Requires NET::WMWindowType passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMWindowType);
     * if (info.valid())
     *     info.windowType(NET::NormalMask | NET::DialogMask);
     * @endcode
     *
     * @see NET::WindowType
     * @see NET::WindowTypeMask
     */
    NET::WindowType windowType(NET::WindowTypes supported_types) const;
    /**
     * Returns the visible name of the window.
     *
     * The visible name differs from the name by including possible <2> appended
     * when there are two or more windows with the same name.
     * Requires NET::WMVisibleName passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMVisibleName);
     * if (info.valid())
     *     info.visibleName();
     * @endcode
     *
     * @see name()
     */
    QString visibleName() const;
    /**
     * Returns a visible name with state.
     *
     * This is a simple convenience function that returns the
     * visible name but with parentheses around minimized windows.
     * Requires NET::WMVisibleName, NET::WMState and NET::XAWMState passed
     * as properties parameter to the constructor.
     * @return the window name with state
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMVisibleName | NET::WMState | NET::XAWMState);
     * if (info.valid())
     *     info.visibleNameWithState();
     * @endcode
     *
     * @see visibleName()
     */
    QString visibleNameWithState() const;
    /**
     * Returns the name of the window, as specified by the application.
     *
     * The difference to visibleName() is that this is the name provided by
     * the application without any modifications by the window manager.
     * You should often use visibleName() instead.
     * Requires NET::WMName passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMName);
     * if (info.valid())
     *     info.name();
     * @endcode
     *
     * @see visibleName()
     */
    QString name() const;
    /**
     * Returns the visible name of the window that should be shown in a taskbar.
     *
     * Note that this has nothing to do with normal icons but with an "iconic"
     * representation of the window.
     * Requires NET::WMVisibleIconName passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMVisibleIconName);
     * if (info.valid())
     *     info.visibleIconName();
     * @endcode
     */
    QString visibleIconName() const;
    /**
     * Returns a visible icon name with state.
     *
     * This is a simple convenience function that returns the
     * visible iconic name but with parentheses around minimized windows.
     * Note that this has nothing to do with normal icons.
     * Requires NET::WMVisibleIconName, NET::WMState and NET::XAWMState passed
     * as properties parameter to the constructor.
     * @return the window iconic name with state
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMVisibleIconName | NET::WMState | NET::XAWMState);
     * if (info.valid())
     *     info.visibleIconNameWithState();
     * @endcode
     *
     * @see visibleIconName()
     */
    QString visibleIconNameWithState() const;
    /**
     * Returns the name of the window that should be shown in taskbar.
     *
     * Note that this has nothing to do with normal icons but with an "iconic"
     * representation of the window.
     * Requires NET::WMIconName passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMIconName);
     * if (info.valid())
     *     info.iconName();
     * @endcode
     */
    QString iconName() const;
    /**
     * Returns true if the window is on the currently active virtual desktop.
     *
     * Requires NET::WMDesktop passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMDesktop);
     * if (info.valid())
     *     info.isOnCurrentDesktop();
     * @endcode
     */
    bool isOnCurrentDesktop() const;
    /**
     * Returns true if the window is on the given virtual desktop.
     *
     * Requires NET::WMDesktop passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMDesktop);
     * if (info.valid())
     *     info.isOnDesktop(KWindowSystem::currentDesktop());
     * @endcode
     */
    bool isOnDesktop(int desktop) const;
    /**
     * Returns true if the window is on all desktops.
     *
     * A window is on all desktops if desktop() returns NET::OnAllDesktops.
     * Requires NET::WMDesktop passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMDesktop);
     * if (info.valid())
     *     info.onAllDesktops();
     * @endcode
     *
     * @see desktop()
     */
    bool onAllDesktops() const;
    /**
     * Returns the virtual desktop this window is on.
     *
     * If the window is on all desktops NET::OnAllDesktops is returned.
     * You should prefer using isOnDesktop().
     * Requires NET::WMDesktop passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMDesktop);
     * if (info.valid())
     *     info.desktop();
     * @endcode
     *
     * @see isOnDesktop()
     */
    int desktop() const;
    /**
     * Returns the list of activity UUIDs this window belongs to.
     *
     * The Plasma workspace allows the user to separate her work into
     * different activities, by assigning windows, documents etc. to
     * the specific ones. An activity is an abstract concept whose meaning
     * can differ from one user to another. Typical examples of activities
     * are "developing a KDE project", "studying the 19th century art",
     * "composing music", "lazing on a Sunday afternoon" etc.
     *
     * If the list is empty, or contains a null UUID, the window is on
     * all activities.
     *
     * Requires NET::WM2Activities passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2Activities);
     * if (info.valid())
     *     info.desktop();
     * @endcode
     *
     * @note Activities are only supported on Plasma Workspace on X11
     *
     * @since 5.0
     */
    QStringList activities() const;
    /**
     * Returns the position and size of the window contents.
     *
     * Requires NET::WMGeometry passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMGeometry);
     * if (info.valid())
     *     info.geometry();
     * @endcode
     */
    QRect geometry() const;
    /**
     * Returns the frame geometry of the window, i.e. including the window decoration.
     *
     * Requires NET::WMFrameExtents passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMFrameExtents);
     * if (info.valid())
     *     info.frameGeometry();
     * @endcode
     */
    QRect frameGeometry() const;
    /**
     * Returns the window identifier of the main window this window belongs to.
     *
     * This is the value of the WM_TRANSIENT_FOR X11 property.
     *
     * Requires NET::WM2TransientFor passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2TransientFor);
     * if (info.valid())
     *     info.transientFor();
     * @endcode
     */
    WId transientFor() const;
    /**
     * Returns the leader window for the group the window is in, if any.
     *
     * Requires NET::WM2GroupLeader passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2GroupLeader);
     * if (info.valid())
     *     info.groupLeader();
     * @endcode
     */
    WId groupLeader() const;

    /**
     * Returns the class component of the WM_CLASS X11 property for the window.
     *
     * Requires NET::WM2WindowClass passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2WindowClass);
     * if (info.valid())
     *     info.windowClassClass();
     * @endcode
     */
    QByteArray windowClassClass() const;

    /**
     * Returns the name component of the WM_CLASS X11 property for the window.
     *
     * Requires NET::WM2WindowClass passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2WindowClass);
     * if (info.valid())
     *     info.windowClassName();
     * @endcode
     */
    QByteArray windowClassName() const;

    /**
     * Returns the WM_WINDOW_ROLE X11 property for the window.
     *
     * Requires NET::WM2WindowRole passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2WindowRole);
     * if (info.valid())
     *     info.windowRole();
     * @endcode
     */
    QByteArray windowRole() const;

    /**
     * Returns the WM_CLIENT_MACHINE property for the window.
     *
     * Requires NET::WM2ClientMachine passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2ClientMachine);
     * if (info.valid())
     *     info.clientMachine();
     * @endcode
     */
    QByteArray clientMachine() const;

    /**
     * Returns true if the given action is currently supported for the window.
     *
     * The supported actions are set by the window manager and
     * can differ depending on the window manager.
     * Requires NET::WM2AllowedActions passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2AllowedActions);
     * if (info.valid())
     *     info.actionSupported(NET::ActionClose);
     * @endcode
     */
    bool actionSupported(NET::Action action) const;

    /**
     * Returns the desktop file name of the window's application if present.
     *
     * This is either the base name without full path and without file extension of the
     * desktop file for the window's application (e.g. "org.kde.foo").
     *
     * If the application's desktop file name is not at a standard location it should be
     * the full path to the desktop file name (e.g. "/opt/kde/share/org.kde.foo.desktop").
     *
     * Requires NET::WM2DesktopFileName passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2DesktopFileName);
     * if (info.valid())
     *     info.desktopFileName();
     * @endcode
     *
     * @since 5.29
     **/
    QByteArray desktopFileName() const;

    /**
     * Returns the GTK application id of the window if present.
     *
     * This is comparable to desktopFileName.
     *
     * Requires NET::WM2GTKApplicationId passed as properties2 parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), 0, NET::WM2GTKApplicationId);
     * if (info.valid())
     *     info.gtkApplicationId();
     * @endcode
     *
     * @since 5.91
     **/
    QByteArray gtkApplicationId() const;

    /**
     * Returns the process ID of the window's application if present.
     *
     * Requires NET::WMPid passed as properties parameter to the constructor.
     *
     * @code
     * QWidget *window = new QWidget(nullptr);
     * window->show();
     * KWindowInfo info(window->winId(), NET::WMPid);
     * if (info.valid())
     *     info.pid();
     * @endcode
     *
     * @since 5.29
     */
    int pid() const;

    /**
     * Returns service name of a window's application menu if present.
     *
     * Requires NET::WMPid passed as properties parameter to the constructor.
     *
     * @since 5.69
     */
    QByteArray applicationMenuServiceName() const;

    /**
     * Returns object path of a window's application menu if present.
     *
     * Requires NET::WMPid passed as properties parameter to the constructor.
     *
     * @since 5.69
     */
    QByteArray applicationMenuObjectPath() const;

    /**
     * Copy constructor.
     */
    KWindowInfo(const KWindowInfo &);
    /**
     * Assignment operator.
     */
    KWindowInfo &operator=(const KWindowInfo &);

private:
    bool KWINDOWSYSTEM_NO_EXPORT icccmCompliantMappingState() const;
    bool KWINDOWSYSTEM_NO_EXPORT allowedActionsSupported() const;

    QExplicitlySharedDataPointer<KWindowInfoPrivate> d;
};

#endif // multiple inclusion guard
