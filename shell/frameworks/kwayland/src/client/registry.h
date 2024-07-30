/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_REGISTRY_H
#define WAYLAND_REGISTRY_H

#include <QHash>
#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_compositor;
struct wl_data_device_manager;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_seat;
struct wl_shell;
struct wl_shm;
struct wl_subcompositor;
struct wl_text_input_manager;
struct zwp_text_input_manager_v2;
struct org_kde_kwin_appmenu_manager;
struct org_kde_kwin_fake_input;
struct org_kde_kwin_dpms_manager;
struct org_kde_kwin_shadow_manager;
struct org_kde_kwin_blur_manager;
struct org_kde_kwin_contrast_manager;
struct org_kde_kwin_slide_manager;
struct org_kde_plasma_activation_feedback;
struct org_kde_plasma_shell;
struct org_kde_plasma_virtual_desktop_management;
struct org_kde_plasma_window_management;
struct xdg_shell;
struct zxdg_shell_v6;
struct xdg_wm_base;
struct zwp_relative_pointer_manager_v1;
struct zwp_pointer_gestures_v1;
struct zwp_pointer_constraints_v1;
struct zxdg_exporter_v2;
struct zxdg_importer_v2;
struct zwp_idle_inhibit_manager_v1;
struct zxdg_output_manager_v1;
struct zxdg_decoration_manager_v1;

namespace KWayland
{
namespace Client
{
class AppMenuManager;
class Compositor;
class ConnectionThread;
class DataDeviceManager;
class DpmsManager;
class EventQueue;
class FakeInput;
class FullscreenShell;
class Idle;
class IdleInhibitManager;
class Keystate;
class RemoteAccessManager;
class Output;
class PlasmaActivationFeedback;
class PlasmaShell;
class PlasmaVirtualDesktopManagement;
class PlasmaWindowManagement;
class PointerConstraints;
class PointerGestures;
class Seat;
class ShadowManager;
class BlurManager;
class ContrastManager;
class SlideManager;
class Shell;
class ShmPool;
class ServerSideDecorationManager;
class ServerSideDecorationPaletteManager;
class SubCompositor;
class TextInputManager;
class TextInputManagerUnstableV0;
class TextInputManagerUnstableV2;
class XdgShell;
class RelativePointerManager;
class XdgExporterUnstableV2;
class XdgImporterUnstableV2;
class XdgExporter;
class XdgImporter;
class XdgOutputManager;
class XdgDecorationManager;

/**
 * @short Wrapper for the wl_registry interface.
 *
 * The purpose of this class is to manage the wl_registry interface.
 * This class supports some well-known interfaces and can create a
 * wrapper class for those.
 *
 * The main purpose is to emit signals whenever a new interface is
 * added or an existing interface is removed. For the well known interfaces
 * dedicated signals are emitted allowing a user to connect directly to the
 * signal announcing the interface it is interested in.
 *
 * To create and setup the Registry one needs to call create with either a
 * wl_display from an existing Wayland connection or a ConnectionThread instance:
 *
 * @code
 * ConnectionThread *connection; // existing connection
 * Registry registry;
 * registry.create(connection);
 * registry.setup();
 * @endcode
 *
 * The interfaces are announced in an asynchronous way by the Wayland server.
 * To initiate the announcing of the interfaces one needs to call setup.
 **/
class KWAYLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
public:
    /**
     * The well-known interfaces this Registry supports.
     * For each of the enum values the Registry is able to create a Wrapper
     * object.
     **/
    enum class Interface {
        Unknown, ///< Refers to an Unknown interface
        Compositor, ///< Refers to the wl_compositor interface
        Shell, ///< Refers to the wl_shell interface
        Seat, ///< Refers to the wl_seat interface
        Shm, ///< Refers to the wl_shm interface
        Output, ///< Refers to the wl_output interface
        SubCompositor, ///< Refers to the wl_subcompositor interface;
        DataDeviceManager, ///< Refers to the wl_data_device_manager interface
        PlasmaShell, ///< Refers to org_kde_plasma_shell interface
        PlasmaWindowManagement, ///< Refers to org_kde_plasma_window_management interface
        FakeInput, ///< Refers to org_kde_kwin_fake_input interface
        Shadow, ///< Refers to org_kde_kwin_shadow_manager interface
        Blur, ///< refers to org_kde_kwin_blur_manager interface
        Contrast, ///< refers to org_kde_kwin_contrast_manager interface
        Slide, ///< refers to org_kde_kwin_slide_manager
        Dpms, ///< Refers to org_kde_kwin_dpms_manager interface
        TextInputManagerUnstableV0, ///< Refers to wl_text_input_manager @since 5.23
        TextInputManagerUnstableV2, ///< Refers to zwp_text_input_manager_v2 @since 5.23
        XdgShellUnstableV5, ///< Refers to xdg_shell (unstable version 5) @since 5.25
        RelativePointerManagerUnstableV1, ///< Refers to zwp_relative_pointer_manager_v1 @since 5.28
        PointerGesturesUnstableV1, ///< Refers to zwp_pointer_gestures_v1 @since 5.29
        PointerConstraintsUnstableV1, ///< Refers to zwp_pointer_constraints_v1 @since 5.29
        XdgExporterUnstableV2, ///< refers to zxdg_exporter_v2 @since 5.40
        XdgImporterUnstableV2, ///< refers to zxdg_importer_v2 @since 5.40
        XdgShellUnstableV6, ///< Refers to zxdg_shell_v6 (unstable version 6) @since 5.39
        IdleInhibitManagerUnstableV1, ///< Refers to zwp_idle_inhibit_manager_v1 (unstable version 1) @since 5.41
        AppMenu, ///< Refers to org_kde_kwin_appmenu @since 5.42
        PlasmaVirtualDesktopManagement, ///< Refers to org_kde_plasma_virtual_desktop_management interface @since 5.52
        XdgOutputUnstableV1, ///< refers to zxdg_output_v1 @since 5.47
        XdgShellStable, ///< refers to xdg_wm_base @since 5.48
        XdgDecorationUnstableV1, ///< refers to zxdg_decoration_manager_v1 @since 5.54
        PlasmaActivationFeedback, ///< Refers to org_kde_plasma_activation_feedback interface, @since 5.83
    };
    explicit Registry(QObject *parent = nullptr);
    ~Registry() override;

    /**
     * Releases the wl_registry interface.
     * After the interface has been released the Registry instance is no
     * longer valid and can be setup with another wl_registry interface.
     **/
    void release();
    /**
     * Destroys the data held by this Registry.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_registry interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, registry, &Registry::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Gets the registry from the @p display.
     **/
    void create(wl_display *display);
    /**
     * Gets the registry from the @p connection.
     **/
    void create(ConnectionThread *connection);
    /**
     * Finalizes the setup of the Registry.
     * After calling this method the interfaces will be announced in an asynchronous way.
     * The Registry must have been created when calling this method.
     * @see create
     **/
    void setup();

    /**
     * Sets the @p queue to use for this Registry.
     *
     * The EventQueue should be set before the Registry gets setup.
     * The EventQueue gets automatically added to all interfaces created by
     * this Registry. So that all objects are in the same EventQueue.
     *
     * @param queue The event queue to use for this Registry.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The EventQueue used by this Registry
     **/
    EventQueue *eventQueue();

    /**
     * @returns @c true if managing a wl_registry.
     **/
    bool isValid() const;
    /**
     * @returns @c true if the Registry has an @p interface.
     **/
    bool hasInterface(Interface interface) const;

    /**
     * Representation of one announced interface.
     **/
    struct AnnouncedInterface {
        /**
         * The name of the announced interface.
         **/
        quint32 name;
        /**
         * The maximum supported version of the announced interface.
         **/
        quint32 version;
    };
    /**
     * Provides name and version for the @p interface.
     *
     * The first value of the returned pair is the "name", the second value is the "version".
     * If the @p interface has not been announced, both values are set to 0.
     * If there @p interface has been announced multiple times, the last announced is returned.
     * In case one is interested in all announced interfaces, one should prefer @link interfaces(Interface) @endlink.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns name and version of the given interface
     * @since 5.5
     **/
    AnnouncedInterface interface(Interface interface) const;
    /**
     * Provides all pairs of name and version for the well-known @p interface.
     *
     * If the @p interface has not been announced, an empty vector is returned.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns All pairs of name and version of the given interface
     * @since 5.5
     **/
    QList<AnnouncedInterface> interfaces(Interface interface) const;

    /**
     * @name Low-level bind methods for global interfaces.
     **/
    ///@{
    /**
     * Binds the wl_compositor with @p name and @p version.
     * If the @p name does not exist or is not for the compositor interface,
     * @c null will be returned.
     *
     * Prefer using createCompositor instead.
     * @see createCompositor
     **/
    wl_compositor *bindCompositor(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shell with @p name and @p version.
     * If the @p name does not exist or is not for the shell interface,
     * @c null will be returned.
     *
     * Prefer using createShell instead.
     * @see createShell
     **/
    wl_shell *bindShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_seat with @p name and @p version.
     * If the @p name does not exist or is not for the seat interface,
     * @c null will be returned.
     *
     * Prefer using createSeat instead.
     * @see createSeat
     **/
    wl_seat *bindSeat(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shm with @p name and @p version.
     * If the @p name does not exist or is not for the shm interface,
     * @c null will be returned.
     *
     * Prefer using createShmPool instead.
     * @see createShmPool
     **/
    wl_shm *bindShm(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_outputdevice with @p name and @p version.
     * If the @p name does not exist or is not for the outputdevice interface,
     * @c null will be returned.
     **/
    wl_output *bindOutput(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_subcompositor with @p name and @p version.
     * If the @p name does not exist or is not for the subcompositor interface,
     * @c null will be returned.
     *
     * Prefer using createSubCompositor instead.
     * @see createSubCompositor
     **/
    wl_subcompositor *bindSubCompositor(uint32_t name, uint32_t version) const;

    /**
     * Binds the wl_data_device_manager with @p name and @p version.
     * If the @p name does not exist or is not for the data device manager interface,
     * @c null will be returned.
     *
     * Prefer using createDataDeviceManager instead.
     * @see createDataDeviceManager
     **/
    wl_data_device_manager *bindDataDeviceManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_shell with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma shell interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaShell instead.
     * @see createPlasmaShell
     * @since 5.4
     **/
    org_kde_plasma_shell *bindPlasmaShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_activation_feedback with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma activation manager interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaActivationFeedback instead.
     * @see createPlasmaActivationFeedback
     * @since 5.83
     **/
    org_kde_plasma_activation_feedback *bindPlasmaActivationFeedback(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_virtual_desktop_management with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma Virtual desktop interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaShell instead.
     * @see createPlasmaShell
     * @since 5.52
     **/
    org_kde_plasma_virtual_desktop_management *bindPlasmaVirtualDesktopManagement(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_window_management with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma window management interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaWindowManagement instead.
     * @see createPlasmaWindowManagement
     * @since 5.46
     **/
    org_kde_plasma_window_management *bindPlasmaWindowManagement(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_fake_input with @p name and @p version.
     * If the @p name does not exist or is not for the fake input interface,
     * @c null will be returned.
     *
     * Prefer using createFakeInput instead.
     * @see createFakeInput
     * @since 5.4
     **/
    org_kde_kwin_fake_input *bindFakeInput(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_shadow_manager with @p name and @p version.
     * If the @p name does not exist or is not for the shadow manager interface,
     * @c null will be returned.
     *
     * Prefer using createShadowManager instead.
     * @see createShadowManager
     * @since 5.4
     **/
    org_kde_kwin_shadow_manager *bindShadowManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_blur_manager with @p name and @p version.
     * If the @p name does not exist or is not for the blur manager interface,
     * @c null will be returned.
     *
     * Prefer using createBlurManager instead.
     * @see createBlurManager
     * @since 5.5
     **/
    org_kde_kwin_blur_manager *bindBlurManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_contrast_manager with @p name and @p version.
     * If the @p name does not exist or is not for the contrast manager interface,
     * @c null will be returned.
     *
     * Prefer using createContrastManager instead.
     * @see createContrastManager
     * @since 5.5
     **/
    org_kde_kwin_contrast_manager *bindContrastManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_slide_manager with @p name and @p version.
     * If the @p name does not exist or is not for the slide manager interface,
     * @c null will be returned.
     *
     * Prefer using createSlideManager instead.
     * @see createSlideManager
     * @since 5.5
     **/
    org_kde_kwin_slide_manager *bindSlideManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_dpms_manager with @p name and @p version.
     * If the @p name does not exist or is not for the dpms manager interface,
     * @c null will be returned.
     *
     * Prefer using createDpmsManager instead.
     * @see createDpmsManager
     * @since 5.5
     **/
    org_kde_kwin_dpms_manager *bindDpmsManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_text_input_manager with @p name and @p version.
     * If the @p name does not exist or is not for the text input interface in unstable version 0,
     * @c null will be returned.
     *
     * Prefer using createTextInputManager instead.
     * @see createTextInputManager
     * @since 5.23
     **/
    wl_text_input_manager *bindTextInputManagerUnstableV0(uint32_t name, uint32_t version) const;
    /**
     * Binds the zwp_text_input_manager_v2 with @p name and @p version.
     * If the @p name does not exist or is not for the text input interface in unstable version 2,
     * @c null will be returned.
     *
     * Prefer using createTextInputManager instead.
     * @see createTextInputManager
     * @since 5.23
     **/
    zwp_text_input_manager_v2 *bindTextInputManagerUnstableV2(uint32_t name, uint32_t version) const;
    /**
     * Binds the xdg_shell (unstable version 5) with @p name and @p version.
     * If the @p name does not exist or is not for the xdg shell interface in unstable version 5,
     * @c null will be returned.
     *
     * Prefer using createXdgShell instead.
     * @see createXdgShell
     * @since 5.25
     **/
    xdg_shell *bindXdgShellUnstableV5(uint32_t name, uint32_t version) const;
    /**
     * Binds the zxdg_shell_v6 (unstable version 6) with @p name and @p version.
     * If the @p name does not exist or is not for the xdg shell interface in unstable version 5,
     * @c null will be returned.
     *
     * Prefer using createXdgShell instead.
     * @see createXdgShell
     * @since 5.39
     **/
    zxdg_shell_v6 *bindXdgShellUnstableV6(uint32_t name, uint32_t version) const;
    /**
     * Binds the zxdg_shell_v6 (unstable version 6) with @p name and @p version.
     * If the @p name does not exist or is not for the xdg shell interface in unstable version 5,
     * @c null will be returned.
     *
     * Prefer using createXdgShell instead.
     * @see createXdgShell
     * @since 5.39
     **/
    xdg_wm_base *bindXdgShellStable(uint32_t name, uint32_t version) const;
    /**
     * Binds the zwp_relative_pointer_manager_v1 with @p name and @p version.
     * If the @p name does not exist or is not for the relative pointer interface in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createRelativePointerManager instead.
     * @see createRelativePointerManager
     * @since 5.28
     **/
    zwp_relative_pointer_manager_v1 *bindRelativePointerManagerUnstableV1(uint32_t name, uint32_t version) const;
    /**
     * Binds the zwp_pointer_gestures_v1 with @p name and @p version.
     * If the @p name does not exist or is not for the pointer gestures interface in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createPointerGestures instead.
     * @see createPointerGestures
     * @since 5.29
     **/
    zwp_pointer_gestures_v1 *bindPointerGesturesUnstableV1(uint32_t name, uint32_t version) const;
    /**
     * Binds the zwp_pointer_constraints_v1 with @p name and @p version.
     * If the @p name does not exist or is not for the pointer constraints interface in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createPointerConstraints instead.
     * @see createPointerConstraints
     * @since 5.29
     **/
    zwp_pointer_constraints_v1 *bindPointerConstraintsUnstableV1(uint32_t name, uint32_t version) const;

    /**
     * Binds the zxdg_exporter_v2 with @p name and @p version.
     * If the @p name does not exists or isnot for the exporter
     * extension in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createXdgExporter
     * @since 5.40
     */
    zxdg_exporter_v2 *bindXdgExporterUnstableV2(uint32_t name, uint32_t version) const;

    /**
     * Binds the zxdg_importer_v2 with @p name and @p version.
     * If the @p name does not exists or isnot for the importer
     * extension in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createXdgImporter
     * @since 5.40
     */
    zxdg_importer_v2 *bindXdgImporterUnstableV2(uint32_t name, uint32_t version) const;

    /**
     * Binds the zwp_idle_inhibit_manager_v1 with @p name and @p version.
     * If the @p name does not exists or is not for the idle inhibit manager in unstable version 1,
     * @c null will be returned.
     *
     * Prefer using createIdleInhibitManager
     * @since 5.41
     */
    zwp_idle_inhibit_manager_v1 *bindIdleInhibitManagerUnstableV1(uint32_t name, uint32_t version) const;

    /**
     * Binds the org_kde_kwin_appmenu_manager with @p name and @p version.
     * If the @p name does not exist or is not for the appmenu manager interface,
     * @c null will be returned.
     *
     * Prefer using createAppMenuManager instead.
     * @see createAppMenuManager
     * @since 5.42
     **/
    org_kde_kwin_appmenu_manager *bindAppMenuManager(uint32_t name, uint32_t version) const;

    /**
     * Binds the zxdg_output_v1 with @p name and @p version.
     * If the @p name does not exist,
     * @c null will be returned.
     *
     * Prefer using createXdgOutputManager instead.
     * @see createXdgOutputManager
     * @since 5.47
     **/
    zxdg_output_manager_v1 *bindXdgOutputUnstableV1(uint32_t name, uint32_t version) const;

    /**
     * Binds the zxdg_decoration_manager_v1 with @p name and @p version.
     * If the @p name does not exist,
     * @c null will be returned.
     *
     * Prefer using createXdgDecorationManager instead.
     * @see createXdgDecorationManager
     * @since 5.54
     **/
    zxdg_decoration_manager_v1 *bindXdgDecorationUnstableV1(uint32_t name, uint32_t version) const;

    ///@}

    /**
     * @name Convenient factory methods for global objects.
     **/
    ///@{
    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Compositor *createCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Seat and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_seat interface,
     * the returned Seat will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_seat interface to bind
     * @param version The version or the wl_seat interface to use
     * @param parent The parent for Seat
     *
     * @returns The created Seat.
     **/
    Shell *createShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Seat *createSeat(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ShmPool and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_shm interface,
     * the returned ShmPool will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_shm interface to bind
     * @param version The version or the wl_shm interface to use
     * @param parent The parent for ShmPool
     *
     * @returns The created ShmPool.
     **/
    ShmPool *createShmPool(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a SubCompositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_subcompositor interface,
     * the returned SubCompositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_subcompositor interface to bind
     * @param version The version or the wl_subcompositor interface to use
     * @param parent The parent for SubCompositor
     *
     * @returns The created SubCompositor.
     **/
    SubCompositor *createSubCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an Output and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_output interface,
     * the returned Output will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_output interface to bind
     * @param version The version or the wl_output interface to use
     * @param parent The parent for Output
     *
     * @returns The created Output.
     **/
    Output *createOutput(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a DataDeviceManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_data_device_manager interface,
     * the returned DataDeviceManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_data_device_manager interface to bind
     * @param version The version or the wl_data_device_manager interface to use
     * @param parent The parent for DataDeviceManager
     *
     * @returns The created DataDeviceManager.
     **/
    DataDeviceManager *createDataDeviceManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaShell and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_shell interface,
     * the returned PlasmaShell will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_shell interface to bind
     * @param version The version or the org_kde_plasma_shell interface to use
     * @param parent The parent for PlasmaShell
     *
     * @returns The created PlasmaShell.
     * @since 5.4
     **/
    PlasmaShell *createPlasmaShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaActivationFeedback and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_activation_feedback interface,
     * the returned PlasmaActivationFeedback will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_activation_feedback interface to bind
     * @param version The version or the org_kde_plasma_activation_feedback interface to use
     * @param parent The parent for PlasmaActivationFeedback
     *
     * @returns The created PlasmaActivationFeedback.
     * @since 5.83
     **/
    PlasmaActivationFeedback *createPlasmaActivationFeedback(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaVirtualDesktopManagement and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_virtual_desktop_management interface,
     * the returned VirtualDesktop will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_virtual_desktop_management interface to bind
     * @param version The version or the org_kde_plasma_virtual_desktop_management interface to use
     * @param parent The parent for PlasmaShell
     *
     * @returns The created PlasmaShell.
     * @since 5.52
     **/
    PlasmaVirtualDesktopManagement *createPlasmaVirtualDesktopManagement(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaWindowManagement and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_window_management interface,
     * the returned PlasmaWindowManagement will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_window_management interface to bind
     * @param version The version or the org_kde_plasma_window_management interface to use
     * @param parent The parent for PlasmaWindowManagement
     *
     * @returns The created PlasmaWindowManagement.
     * @since 5.4
     **/
    PlasmaWindowManagement *createPlasmaWindowManagement(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a FakeInput and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_fake_input interface,
     * the returned FakeInput will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_fake_input interface to bind
     * @param version The version or the org_kde_kwin_fake_input interface to use
     * @param parent The parent for FakeInput
     *
     * @returns The created FakeInput.
     * @since 5.4
     **/
    FakeInput *createFakeInput(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ShadowManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_shadow_manager interface,
     * the returned ShadowManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_shadow_manager interface to bind
     * @param version The version or the org_kde_kwin_shadow_manager interface to use
     * @param parent The parent for ShadowManager
     *
     * @returns The created ShadowManager.
     * @since 5.4
     **/
    ShadowManager *createShadowManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a BlurManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_blur_manager interface,
     * the returned BlurManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_blur_manager interface to bind
     * @param version The version or the org_kde_kwin_blur_manager interface to use
     * @param parent The parent for BlurManager
     *
     * @returns The created BlurManager.
     * @since 5.5
     **/
    BlurManager *createBlurManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ContrastManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_contrast_manager interface,
     * the returned ContrastManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_contrast_manager interface to bind
     * @param version The version or the org_kde_kwin_contrast_manager interface to use
     * @param parent The parent for ContrastManager
     *
     * @returns The created ContrastManager.
     * @since 5.5
     **/
    ContrastManager *createContrastManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a SlideManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_slide_manager interface,
     * the returned SlideManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_slide_manager interface to bind
     * @param version The version or the org_kde_kwin_slide_manager interface to use
     * @param parent The parent for SlideManager
     *
     * @returns The created SlideManager.
     * @since 5.5
     **/
    SlideManager *createSlideManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a DpmsManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_dpms_manager interface,
     * the returned DpmsManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_dpms_manager interface to bind
     * @param version The version or the org_kde_kwin_dpms_manager interface to use
     * @param parent The parent for DpmsManager
     *
     * @returns The created DpmsManager.
     * @since 5.5
     **/
    DpmsManager *createDpmsManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a TextInputManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li wl_text_input_manager
     * @li zwp_text_input_manager_v2
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @param name The name of the interface to bind
     * @param version The version of the interface to use
     * @param parent The parent for the TextInputManager
     *
     * @returns The created TextInputManager
     * @since 5.23
     **/
    TextInputManager *createTextInputManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an XdgShell and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li xdg_shell (Unstable version 5)
     *
     * If @p name is for one of the supported interfaces the corresponding shell will be created,
     * otherwise @c null will be returned.
     *
     * @param name The name of the interface to bind
     * @param version The version of the interface to use
     * @param parent The parent for the XdgShell
     *
     * @returns The created XdgShell
     * @since 5.25
     **/
    XdgShell *createXdgShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a RelativePointerManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zwp_relative_pointer_manager_v1
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @param name The name of the interface to bind
     * @param version The version of the interface to use
     * @param parent The parent for the RelativePointerManager
     *
     * @returns The created RelativePointerManager
     * @since 5.28
     **/
    RelativePointerManager *createRelativePointerManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PointerGestures and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zwp_pointer_gestures_v1
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @param name The name of the interface to bind
     * @param version The version of the interface to use
     * @param parent The parent for the PointerGestures
     *
     * @returns The created PointerGestures
     * @since 5.29
     **/
    PointerGestures *createPointerGestures(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PointerConstraints and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zwp_pointer_constraints_v1
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @param name The name of the interface to bind
     * @param version The version of the interface to use
     * @param parent The parent for the PointerConstraints
     *
     * @returns The created PointerConstraints
     * @since 5.29
     **/
    PointerConstraints *createPointerConstraints(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates an XdgExporter and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zxdg_exporter_v2
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @returns The created XdgExporter
     * @since 5.40
     */
    XdgExporter *createXdgExporter(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates an XdgImporter and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zxdg_importer_v2
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @returns The created XdgImporter
     * @since 5.40
     */
    XdgImporter *createXdgImporter(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates an IdleInhibitManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * This factory method supports the following interfaces:
     * @li zwp_idle_inhibit_manager_v1
     *
     * If @p name is for one of the supported interfaces the corresponding manager will be created,
     * otherwise @c null will be returned.
     *
     * @returns The created IdleInhibitManager
     * @since 5.41
     */
    IdleInhibitManager *createIdleInhibitManager(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates a AppMenuManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_appmenu_manager interface,
     * the returned AppMenuManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_appmenu_manager interface to bind
     * @param version The version or the org_kde_kwin_appmenu_manager interface to use
     * @param parent The parent for AppMenuManager
     *
     * @returns The created AppMenuManager.
     * @since 5.42
     **/
    AppMenuManager *createAppMenuManager(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates an XdgOutputManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the zxdg_output_manager_v1 interface,
     * the returned XdgOutputManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the zxdg_output_manager_v1 interface to bind
     * @param version The version or the zxdg_output_manager_v1 interface to use
     * @param parent The parent for XdgOuptutManager
     *
     * @returns The created XdgOuptutManager.
     * @since 5.47
     **/
    XdgOutputManager *createXdgOutputManager(quint32 name, quint32 version, QObject *parent = nullptr);

    /**
     * Creates an XdgDecorationManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the zxdg_decoration_manager_v1 interface,
     * the returned XdgDecorationManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the zxdg_decoration_manager_v1 interface to bind
     * @param version The version or the zxdg_decoration_manager_v1 interface to use
     * @param parent The parent for XdgDecorationManager
     *
     * @returns The created XdgDecorationManager.
     * @since 5.54
     **/
    XdgDecorationManager *createXdgDecorationManager(quint32 name, quint32 version, QObject *parent = nullptr);

    ///@}

    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry *();
    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry *() const;
    /**
     * @returns access to the low-level Wayland @c wl_registry
     **/
    wl_registry *registry();

Q_SIGNALS:
    /**
     * @name Interface announced signals.
     **/
    ///@{
    /**
     * Emitted whenever a wl_compositor interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void compositorAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_seat interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void seatAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shm interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shmAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_subcompositor interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void subCompositorAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_output interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void outputAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_data_device_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void dataDeviceManagerAnnounced(quint32 name, quint32 version);

    /**
     * Emitted whenever a org_kde_plasma_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void plasmaShellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_activation_feedback interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.83
     **/
    void plasmaActivationFeedbackAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_virtual_desktop_management interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.52
     **/
    void plasmaVirtualDesktopManagementAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_window_management interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void plasmaWindowManagementAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_fake_input interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void fakeInputAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_shadow_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void shadowAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_blur_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void blurAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_contrast_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void contrastAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_slide_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void slideAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_dpms_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void dpmsAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_text_input_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.23
     **/
    void textInputManagerUnstableV0Announced(quint32 name, quint32 version);
    /**
     * Emitted whenever a zwp_text_input_manager_v2 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.23
     **/
    void textInputManagerUnstableV2Announced(quint32 name, quint32 version);
    /**
     * Emitted whenever a xdg_shell (unstable version 5) interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.25
     **/
    void xdgShellUnstableV5Announced(quint32 name, quint32 version);
    /**
     * Emitted whenever a zxdg_shell_v6 (unstable version 6) interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.25
     **/
    void xdgShellUnstableV6Announced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zwp_relative_pointer_manager_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.28
     **/
    void relativePointerManagerUnstableV1Announced(quint32 name, quint32 version);
    /**
     * Emitted whenever a zwp_pointer_gestures_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.29
     **/
    void pointerGesturesUnstableV1Announced(quint32 name, quint32 version);
    /**
     * Emitted whenever a zwp_pointer_constraints_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.29
     **/
    void pointerConstraintsUnstableV1Announced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zxdg_exporter_v2 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.40
     */
    void exporterUnstableV2Announced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zxdg_importer_v2 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.40
     */
    void importerUnstableV2Announced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zwp_idle_inhibit_manager_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.41
     */
    void idleInhibitManagerUnstableV1Announced(quint32 name, quint32 version);

    /**
     * Emitted whenever a org_kde_kwin_appmenu_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.42
     */
    void appMenuAnnounced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zxdg_output_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.47
     */
    void xdgOutputAnnounced(quint32 name, quint32 version);

    /**
     * Emitted whenever a xdg_wm_base (stable xdg shell) interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.48
     **/
    void xdgShellStableAnnounced(quint32 name, quint32 version);

    /**
     * Emitted whenever a zxdg_decoration_manager_v1 interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.54
     **/
    void xdgDecorationAnnounced(quint32 name, quint32 version);

    ///@}

    /**
     * @name Interface removed signals.
     **/
    ///@{
    /**
     * Emitted whenever a wl_compositor interface gets removed.
     * @param name The name for the removed interface
     **/
    void compositorRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void shellRemoved(quint32 name);
    /**
     * Emitted whenever a wl_seat interface gets removed.
     * @param name The name for the removed interface
     **/
    void seatRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shm interface gets removed.
     * @param name The name for the removed interface
     **/
    void shmRemoved(quint32 name);
    /**
     * Emitted whenever a wl_subcompositor interface gets removed.
     * @param name The name for the removed interface
     **/
    void subCompositorRemoved(quint32 name);
    /**
     * Emitted whenever a wl_output interface gets removed.
     * @param name The name for the removed interface
     **/
    void outputRemoved(quint32 name);
    /**
     * Emitted whenever a wl_data_device_manager interface gets removed.
     * @param name The name for the removed interface
     **/
    void dataDeviceManagerRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_shell interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void plasmaShellRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_activation_feedback interface gets removed.
     * @param name The name for the removed interface
     * @since 5.83
     **/
    void plasmaActivationFeedbackRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_virtual_desktop_management interface gets removed.
     * @param name The name for the removed interface
     * @since 5.52
     **/
    void plasmaVirtualDesktopManagementRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_window_management interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void plasmaWindowManagementRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_fake_input interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void fakeInputRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_shadow_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void shadowRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_blur_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void blurRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_contrast_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void contrastRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_slide_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void slideRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_dpms_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void dpmsRemoved(quint32 name);
    /**
     * Emitted whenever a wl_text_input_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.23
     **/
    void textInputManagerUnstableV0Removed(quint32 name);
    /**
     * Emitted whenever a zwp_text_input_manager_v2 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.23
     **/
    void textInputManagerUnstableV2Removed(quint32 name);
    /**
     * Emitted whenever an xdg_shell (unstable version 5) interface gets removed.
     * @param name The name for the removed interface
     * @since 5.25
     **/
    void xdgShellUnstableV5Removed(quint32 name);
    /**
     * Emitted whenever an xdg_shell (unstable version 5) interface gets removed.
     * @param name The name for the removed interface
     * @since 5.25
     **/
    void xdgShellUnstableV6Removed(quint32 name);
    /**
     * Emitted whenever a zwp_relative_pointer_manager_v1 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.28
     **/
    void relativePointerManagerUnstableV1Removed(quint32 name);
    /**
     * Emitted whenever a zwp_pointer_gestures_v1 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.29
     **/
    void pointerGesturesUnstableV1Removed(quint32 name);
    /**
     * Emitted whenever a zwp_pointer_constraints_v1 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.29
     **/
    void pointerConstraintsUnstableV1Removed(quint32 name);

    /**
     * Emitted whenever a zxdg_exporter_v2 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.40
     **/
    void exporterUnstableV2Removed(quint32 name);

    /**
     * Emitted whenever a zxdg_importer_v2 interface gets removed.
     * @param name The name for the removed interface
     * @since 5.40
     **/
    void importerUnstableV2Removed(quint32 name);

    /**
     * Emitted whenever a zwp_idle_inhibit_manager_v1 interface gets removed.
     * @param name The name of the removed interface
     * @since 5.41
     **/
    void idleInhibitManagerUnstableV1Removed(quint32 name);

    /**
     * Emitted whenever a org_kde_kwin_appmenu_manager gets removed.
     * @param name The name of the removed interface
     * @since 5.42
     **/
    void appMenuRemoved(quint32 name);

    /**
     * Emitted whenever a zxdg_output_v1 gets removed.
     * @param name The name of the removed interface
     * @since 5.47
     **/
    void xdgOutputRemoved(quint32 name);
    /**
     * Emitted whenever an xdg_wm_base (stable xdgshell) interface gets removed.
     * @param name The name for the removed interface
     * @since 5.48
     **/
    void xdgShellStableRemoved(quint32 name);

    /**
     * Emitted whenever a zxdg_decoration_manager_v1 gets removed.
     * @param name The name of the removed interface
     * @since 5.54
     **/
    void xdgDecorationRemoved(quint32 name);

    ///@}
    /**
     * Generic announced signal which gets emitted whenever an interface gets
     * announced.
     *
     * This signal is emitted before the dedicated signals are handled. If one
     * wants to know about one of the well-known interfaces use the dedicated
     * signals instead. Especially the bind methods might fail before the dedicated
     * signals are emitted.
     *
     * @param interface The interface (e.g. wl_compositor) which is announced
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void interfaceAnnounced(QByteArray interface, quint32 name, quint32 version);
    /**
     * Generic removal signal which gets emitted whenever an interface gets removed.
     *
     * This signal is emitted after the dedicated signals are handled.
     *
     * @param name The name for the removed interface
     **/
    void interfaceRemoved(quint32 name);
    /**
     * Emitted when the Wayland display is done flushing the initial interface
     * callbacks, announcing wl_display properties. This can be used to compress
     * events. Note that this signal is emitted only after announcing interfaces,
     * such as outputs, but not after receiving callbacks of interface properties,
     * such as the output's geometry, modes, etc..
     * This signal is emitted from the wl_display_sync callback.
     **/
    void interfacesAnnounced();

Q_SIGNALS:
    /*
     * Emitted when the registry has been destroyed rather than released
     */
    void registryDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
