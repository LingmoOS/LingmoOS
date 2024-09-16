// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QT_WAYLAND_WAYLAND
#define QT_WAYLAND_WAYLAND

#include <QtWaylandClient/private/wayland-wayland-client-protocol.h>
#include <QByteArray>
#include <QString>

QT_BEGIN_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wmissing-field-initializers")

#if !defined(Q_WAYLAND_CLIENT_WAYLAND_EXPORT)
#  if defined(QT_SHARED)
#    define Q_WAYLAND_CLIENT_WAYLAND_EXPORT Q_DECL_EXPORT
#  else
#    define Q_WAYLAND_CLIENT_WAYLAND_EXPORT
#  endif
#endif

namespace QtWayland {
    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_registry
    {
    public:
        wl_registry(struct ::wl_registry *registry, int id, int version);
        wl_registry(struct ::wl_registry *object);
        wl_registry();

        virtual ~wl_registry();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_registry *object);

        struct ::wl_registry *object() { return m_wl_registry; }
        const struct ::wl_registry *object() const { return m_wl_registry; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void *bind(uint32_t name, const struct ::wl_interface *interface, uint32_t version);

    protected:
        virtual void registry_global(uint32_t name, const QString &interface, uint32_t version);
        virtual void registry_global_remove(uint32_t name);

    private:
        void init_listener();
        static const struct wl_registry_listener m_wl_registry_listener;
        static void handle_global(
            void *data,
            struct ::wl_registry *object,
            uint32_t name,
            const char *interface,
            uint32_t version);
        static void handle_global_remove(
            void *data,
            struct ::wl_registry *object,
            uint32_t name);
        struct ::wl_registry *m_wl_registry;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_callback
    {
    public:
        wl_callback(struct ::wl_registry *registry, int id, int version);
        wl_callback(struct ::wl_callback *object);
        wl_callback();

        virtual ~wl_callback();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_callback *object);

        struct ::wl_callback *object() { return m_wl_callback; }
        const struct ::wl_callback *object() const { return m_wl_callback; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

    protected:
        virtual void callback_done(uint32_t callback_data);

    private:
        void init_listener();
        static const struct wl_callback_listener m_wl_callback_listener;
        static void handle_done(
            void *data,
            struct ::wl_callback *object,
            uint32_t callback_data);
        struct ::wl_callback *m_wl_callback;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_compositor
    {
    public:
        wl_compositor(struct ::wl_registry *registry, int id, int version);
        wl_compositor(struct ::wl_compositor *object);
        wl_compositor();

        virtual ~wl_compositor();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_compositor *object);

        struct ::wl_compositor *object() { return m_wl_compositor; }
        const struct ::wl_compositor *object() const { return m_wl_compositor; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        struct ::wl_surface *create_surface();
        struct ::wl_region *create_region();

    private:
        struct ::wl_compositor *m_wl_compositor;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_shm_pool
    {
    public:
        wl_shm_pool(struct ::wl_registry *registry, int id, int version);
        wl_shm_pool(struct ::wl_shm_pool *object);
        wl_shm_pool();

        virtual ~wl_shm_pool();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_shm_pool *object);

        struct ::wl_shm_pool *object() { return m_wl_shm_pool; }
        const struct ::wl_shm_pool *object() const { return m_wl_shm_pool; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        struct ::wl_buffer *create_buffer(int32_t offset, int32_t width, int32_t height, int32_t stride, uint32_t format);
        void destroy();
        void resize(int32_t size);

    private:
        struct ::wl_shm_pool *m_wl_shm_pool;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_shm
    {
    public:
        wl_shm(struct ::wl_registry *registry, int id, int version);
        wl_shm(struct ::wl_shm *object);
        wl_shm();

        virtual ~wl_shm();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_shm *object);

        struct ::wl_shm *object() { return m_wl_shm; }
        const struct ::wl_shm *object() const { return m_wl_shm; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum error {
            error_invalid_format = 0, // buffer format is not known
            error_invalid_stride = 1, // invalid size or stride during pool or buffer creation
            error_invalid_fd = 2 // mmapping the file descriptor failed
        };

        enum format {
            format_argb8888 = 0, // 32-bit ARGB format
            format_xrgb8888 = 1, // 32-bit RGB format
            format_c8 = 0x20203843,
            format_rgb332 = 0x38424752,
            format_bgr233 = 0x38524742,
            format_xrgb4444 = 0x32315258,
            format_xbgr4444 = 0x32314258,
            format_rgbx4444 = 0x32315852,
            format_bgrx4444 = 0x32315842,
            format_argb4444 = 0x32315241,
            format_abgr4444 = 0x32314241,
            format_rgba4444 = 0x32314152,
            format_bgra4444 = 0x32314142,
            format_xrgb1555 = 0x35315258,
            format_xbgr1555 = 0x35314258,
            format_rgbx5551 = 0x35315852,
            format_bgrx5551 = 0x35315842,
            format_argb1555 = 0x35315241,
            format_abgr1555 = 0x35314241,
            format_rgba5551 = 0x35314152,
            format_bgra5551 = 0x35314142,
            format_rgb565 = 0x36314752,
            format_bgr565 = 0x36314742,
            format_rgb888 = 0x34324752,
            format_bgr888 = 0x34324742,
            format_xbgr8888 = 0x34324258,
            format_rgbx8888 = 0x34325852,
            format_bgrx8888 = 0x34325842,
            format_abgr8888 = 0x34324241,
            format_rgba8888 = 0x34324152,
            format_bgra8888 = 0x34324142,
            format_xrgb2101010 = 0x30335258,
            format_xbgr2101010 = 0x30334258,
            format_rgbx1010102 = 0x30335852,
            format_bgrx1010102 = 0x30335842,
            format_argb2101010 = 0x30335241,
            format_abgr2101010 = 0x30334241,
            format_rgba1010102 = 0x30334152,
            format_bgra1010102 = 0x30334142,
            format_yuyv = 0x56595559,
            format_yvyu = 0x55595659,
            format_uyvy = 0x59565955,
            format_vyuy = 0x59555956,
            format_ayuv = 0x56555941,
            format_nv12 = 0x3231564e,
            format_nv21 = 0x3132564e,
            format_nv16 = 0x3631564e,
            format_nv61 = 0x3136564e,
            format_yuv410 = 0x39565559,
            format_yvu410 = 0x39555659,
            format_yuv411 = 0x31315559,
            format_yvu411 = 0x31315659,
            format_yuv420 = 0x32315559,
            format_yvu420 = 0x32315659,
            format_yuv422 = 0x36315559,
            format_yvu422 = 0x36315659,
            format_yuv444 = 0x34325559,
            format_yvu444 = 0x34325659
        };

        struct ::wl_shm_pool *create_pool(int32_t fd, int32_t size);

    protected:
        virtual void shm_format(uint32_t format);

    private:
        void init_listener();
        static const struct wl_shm_listener m_wl_shm_listener;
        static void handle_format(
            void *data,
            struct ::wl_shm *object,
            uint32_t format);
        struct ::wl_shm *m_wl_shm;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_buffer
    {
    public:
        wl_buffer(struct ::wl_registry *registry, int id, int version);
        wl_buffer(struct ::wl_buffer *object);
        wl_buffer();

        virtual ~wl_buffer();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_buffer *object);

        struct ::wl_buffer *object() { return m_wl_buffer; }
        const struct ::wl_buffer *object() const { return m_wl_buffer; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void destroy();

    protected:
        virtual void buffer_release();

    private:
        void init_listener();
        static const struct wl_buffer_listener m_wl_buffer_listener;
        static void handle_release(
            void *data,
            struct ::wl_buffer *object);
        struct ::wl_buffer *m_wl_buffer;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_data_offer
    {
    public:
        wl_data_offer(struct ::wl_registry *registry, int id, int version);
        wl_data_offer(struct ::wl_data_offer *object);
        wl_data_offer();

        virtual ~wl_data_offer();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_data_offer *object);

        struct ::wl_data_offer *object() { return m_wl_data_offer; }
        const struct ::wl_data_offer *object() const { return m_wl_data_offer; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void accept(uint32_t serial, const QString &mime_type);
        void receive(const QString &mime_type, int32_t fd);
        void destroy();

    protected:
        virtual void data_offer_offer(const QString &mime_type);

    private:
        void init_listener();
        static const struct wl_data_offer_listener m_wl_data_offer_listener;
        static void handle_offer(
            void *data,
            struct ::wl_data_offer *object,
            const char *mime_type);
        struct ::wl_data_offer *m_wl_data_offer;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_data_source
    {
    public:
        wl_data_source(struct ::wl_registry *registry, int id, int version);
        wl_data_source(struct ::wl_data_source *object);
        wl_data_source();

        virtual ~wl_data_source();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_data_source *object);

        struct ::wl_data_source *object() { return m_wl_data_source; }
        const struct ::wl_data_source *object() const { return m_wl_data_source; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void offer(const QString &mime_type);
        void destroy();

    protected:
        virtual void data_source_target(const QString &mime_type);
        virtual void data_source_send(const QString &mime_type, int32_t fd);
        virtual void data_source_cancelled();

    private:
        void init_listener();
        static const struct wl_data_source_listener m_wl_data_source_listener;
        static void handle_target(
            void *data,
            struct ::wl_data_source *object,
            const char *mime_type);
        static void handle_send(
            void *data,
            struct ::wl_data_source *object,
            const char *mime_type,
            int32_t fd);
        static void handle_cancelled(
            void *data,
            struct ::wl_data_source *object);
        struct ::wl_data_source *m_wl_data_source;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_data_device
    {
    public:
        wl_data_device(struct ::wl_registry *registry, int id, int version);
        wl_data_device(struct ::wl_data_device *object);
        wl_data_device();

        virtual ~wl_data_device();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_data_device *object);

        struct ::wl_data_device *object() { return m_wl_data_device; }
        const struct ::wl_data_device *object() const { return m_wl_data_device; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void start_drag(struct ::wl_data_source *source, struct ::wl_surface *origin, struct ::wl_surface *icon, uint32_t serial);
        void set_selection(struct ::wl_data_source *source, uint32_t serial);

    protected:
        virtual void data_device_data_offer(struct ::wl_data_offer *id);
        virtual void data_device_enter(uint32_t serial, struct ::wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct ::wl_data_offer *id);
        virtual void data_device_leave();
        virtual void data_device_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y);
        virtual void data_device_drop();
        virtual void data_device_selection(struct ::wl_data_offer *id);

    private:
        void init_listener();
        static const struct wl_data_device_listener m_wl_data_device_listener;
        static void handle_data_offer(
            void *data,
            struct ::wl_data_device *object,
            struct ::wl_data_offer *id);
        static void handle_enter(
            void *data,
            struct ::wl_data_device *object,
            uint32_t serial,
            struct ::wl_surface *surface,
            wl_fixed_t x,
            wl_fixed_t y,
            struct ::wl_data_offer *id);
        static void handle_leave(
            void *data,
            struct ::wl_data_device *object);
        static void handle_motion(
            void *data,
            struct ::wl_data_device *object,
            uint32_t time,
            wl_fixed_t x,
            wl_fixed_t y);
        static void handle_drop(
            void *data,
            struct ::wl_data_device *object);
        static void handle_selection(
            void *data,
            struct ::wl_data_device *object,
            struct ::wl_data_offer *id);
        struct ::wl_data_device *m_wl_data_device;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_data_device_manager
    {
    public:
        wl_data_device_manager(struct ::wl_registry *registry, int id, int version);
        wl_data_device_manager(struct ::wl_data_device_manager *object);
        wl_data_device_manager();

        virtual ~wl_data_device_manager();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_data_device_manager *object);

        struct ::wl_data_device_manager *object() { return m_wl_data_device_manager; }
        const struct ::wl_data_device_manager *object() const { return m_wl_data_device_manager; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        struct ::wl_data_source *create_data_source();
        struct ::wl_data_device *get_data_device(struct ::wl_seat *seat);

    private:
        struct ::wl_data_device_manager *m_wl_data_device_manager;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_shell
    {
    public:
        wl_shell(struct ::wl_registry *registry, int id, int version);
        wl_shell(struct ::wl_shell *object);
        wl_shell();

        virtual ~wl_shell();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_shell *object);

        struct ::wl_shell *object() { return m_wl_shell; }
        const struct ::wl_shell *object() const { return m_wl_shell; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        struct ::wl_shell_surface *get_shell_surface(struct ::wl_surface *surface);

    private:
        struct ::wl_shell *m_wl_shell;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_shell_surface
    {
    public:
        wl_shell_surface(struct ::wl_registry *registry, int id, int version);
        wl_shell_surface(struct ::wl_shell_surface *object);
        wl_shell_surface();

        virtual ~wl_shell_surface();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_shell_surface *object);

        struct ::wl_shell_surface *object() { return m_wl_shell_surface; }
        const struct ::wl_shell_surface *object() const { return m_wl_shell_surface; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum resize {
            resize_none = 0,
            resize_top = 1,
            resize_bottom = 2,
            resize_left = 4,
            resize_top_left = 5,
            resize_bottom_left = 6,
            resize_right = 8,
            resize_top_right = 9,
            resize_bottom_right = 10
        };

        enum transient {
            transient_inactive = 0x1 // do not set keyboard focus
        };

        enum fullscreen_method {
            fullscreen_method_default = 0, // no preference, apply default policy
            fullscreen_method_scale = 1, // scale, preserve the surface's aspect ratio and center on output
            fullscreen_method_driver = 2, // switch output mode to the smallest mode that can fit the surface, add black borders to compensate size mismatch
            fullscreen_method_fill = 3 // no upscaling, center on output and add black borders to compensate size mismatch
        };

        void pong(uint32_t serial);
        void move(struct ::wl_seat *seat, uint32_t serial);
        void resize(struct ::wl_seat *seat, uint32_t serial, uint32_t edges);
        void set_toplevel();
        void set_transient(struct ::wl_surface *parent, int32_t x, int32_t y, uint32_t flags);
        void set_fullscreen(uint32_t method, uint32_t framerate, struct ::wl_output *output);
        void set_popup(struct ::wl_seat *seat, uint32_t serial, struct ::wl_surface *parent, int32_t x, int32_t y, uint32_t flags);
        void set_maximized(struct ::wl_output *output);
        void set_title(const QString &title);
        void set_class(const QString &class_);

    protected:
        virtual void shell_surface_ping(uint32_t serial);
        virtual void shell_surface_configure(uint32_t edges, int32_t width, int32_t height);
        virtual void shell_surface_popup_done();

    private:
        void init_listener();
        static const struct wl_shell_surface_listener m_wl_shell_surface_listener;
        static void handle_ping(
            void *data,
            struct ::wl_shell_surface *object,
            uint32_t serial);
        static void handle_configure(
            void *data,
            struct ::wl_shell_surface *object,
            uint32_t edges,
            int32_t width,
            int32_t height);
        static void handle_popup_done(
            void *data,
            struct ::wl_shell_surface *object);
        struct ::wl_shell_surface *m_wl_shell_surface;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_surface
    {
    public:
        wl_surface(struct ::wl_registry *registry, int id, int version);
        wl_surface(struct ::wl_surface *object);
        wl_surface();

        virtual ~wl_surface();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_surface *object);

        struct ::wl_surface *object() { return m_wl_surface; }
        const struct ::wl_surface *object() const { return m_wl_surface; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum error {
            error_invalid_scale = 0, // buffer scale value is invalid
            error_invalid_transform = 1 // buffer transform value is invalid
        };

        void destroy();
        void attach(struct ::wl_buffer *buffer, int32_t x, int32_t y);
        void damage(int32_t x, int32_t y, int32_t width, int32_t height);
        struct ::wl_callback *frame();
        void set_opaque_region(struct ::wl_region *region);
        void set_input_region(struct ::wl_region *region);
        void commit();
        void set_buffer_transform(int32_t transform);
        void set_buffer_scale(int32_t scale);

    protected:
        virtual void surface_enter(struct ::wl_output *output);
        virtual void surface_leave(struct ::wl_output *output);

    private:
        void init_listener();
        static const struct wl_surface_listener m_wl_surface_listener;
        static void handle_enter(
            void *data,
            struct ::wl_surface *object,
            struct ::wl_output *output);
        static void handle_leave(
            void *data,
            struct ::wl_surface *object,
            struct ::wl_output *output);
        struct ::wl_surface *m_wl_surface;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_seat
    {
    public:
        wl_seat(struct ::wl_registry *registry, int id, int version);
        wl_seat(struct ::wl_seat *object);
        wl_seat();

        virtual ~wl_seat();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_seat *object);

        struct ::wl_seat *object() { return m_wl_seat; }
        const struct ::wl_seat *object() const { return m_wl_seat; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum capability {
            capability_pointer = 1, // The seat has pointer devices
            capability_keyboard = 2, // The seat has one or more keyboards
            capability_touch = 4 // The seat has touch devices
        };

        struct ::wl_pointer *get_pointer();
        struct ::wl_keyboard *get_keyboard();
        struct ::wl_touch *get_touch();

    protected:
        virtual void seat_capabilities(uint32_t capabilities);
        virtual void seat_name(const QString &name);

    private:
        void init_listener();
        static const struct wl_seat_listener m_wl_seat_listener;
        static void handle_capabilities(
            void *data,
            struct ::wl_seat *object,
            uint32_t capabilities);
        static void handle_name(
            void *data,
            struct ::wl_seat *object,
            const char *name);
        struct ::wl_seat *m_wl_seat;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_pointer
    {
    public:
        wl_pointer(struct ::wl_registry *registry, int id, int version);
        wl_pointer(struct ::wl_pointer *object);
        wl_pointer();

        virtual ~wl_pointer();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_pointer *object);

        struct ::wl_pointer *object() { return m_wl_pointer; }
        const struct ::wl_pointer *object() const { return m_wl_pointer; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum button_state {
            button_state_released = 0, // The button is not pressed
            button_state_pressed = 1 // The button is pressed
        };

        enum axis {
            axis_vertical_scroll = 0,
            axis_horizontal_scroll = 1
        };

        void set_cursor(uint32_t serial, struct ::wl_surface *surface, int32_t hotspot_x, int32_t hotspot_y);
        void release();

    protected:
        virtual void pointer_enter(uint32_t serial, struct ::wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y);
        virtual void pointer_leave(uint32_t serial, struct ::wl_surface *surface);
        virtual void pointer_motion(uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y);
        virtual void pointer_button(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
        virtual void pointer_axis(uint32_t time, uint32_t axis, wl_fixed_t value);

    private:
        void init_listener();
        static const struct wl_pointer_listener m_wl_pointer_listener;
        static void handle_enter(
            void *data,
            struct ::wl_pointer *object,
            uint32_t serial,
            struct ::wl_surface *surface,
            wl_fixed_t surface_x,
            wl_fixed_t surface_y);
        static void handle_leave(
            void *data,
            struct ::wl_pointer *object,
            uint32_t serial,
            struct ::wl_surface *surface);
        static void handle_motion(
            void *data,
            struct ::wl_pointer *object,
            uint32_t time,
            wl_fixed_t surface_x,
            wl_fixed_t surface_y);
        static void handle_button(
            void *data,
            struct ::wl_pointer *object,
            uint32_t serial,
            uint32_t time,
            uint32_t button,
            uint32_t state);
        static void handle_axis(
            void *data,
            struct ::wl_pointer *object,
            uint32_t time,
            uint32_t axis,
            wl_fixed_t value);
        struct ::wl_pointer *m_wl_pointer;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_keyboard
    {
    public:
        wl_keyboard(struct ::wl_registry *registry, int id, int version);
        wl_keyboard(struct ::wl_keyboard *object);
        wl_keyboard();

        virtual ~wl_keyboard();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_keyboard *object);

        struct ::wl_keyboard *object() { return m_wl_keyboard; }
        const struct ::wl_keyboard *object() const { return m_wl_keyboard; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum keymap_format {
            keymap_format_no_keymap = 0, // no keymap; client must understand how to interpret the raw keycode
            keymap_format_xkb_v1 = 1 // libxkbcommon compatible; to determine the xkb keycode, clients must add 8 to the key event keycode
        };

        enum key_state {
            key_state_released = 0, // key is not pressed
            key_state_pressed = 1 // key is pressed
        };

        void release();

    protected:
        virtual void keyboard_keymap(uint32_t format, int32_t fd, uint32_t size);
        virtual void keyboard_enter(uint32_t serial, struct ::wl_surface *surface, wl_array *keys);
        virtual void keyboard_leave(uint32_t serial, struct ::wl_surface *surface);
        virtual void keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
        virtual void keyboard_modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
        virtual void keyboard_repeat_info(int32_t rate, int32_t delay);

    private:
        void init_listener();
        static const struct wl_keyboard_listener m_wl_keyboard_listener;
        static void handle_keymap(
            void *data,
            struct ::wl_keyboard *object,
            uint32_t format,
            int32_t fd,
            uint32_t size);
        static void handle_enter(
            void *data,
            struct ::wl_keyboard *object,
            uint32_t serial,
            struct ::wl_surface *surface,
            wl_array *keys);
        static void handle_leave(
            void *data,
            struct ::wl_keyboard *object,
            uint32_t serial,
            struct ::wl_surface *surface);
        static void handle_key(
            void *data,
            struct ::wl_keyboard *object,
            uint32_t serial,
            uint32_t time,
            uint32_t key,
            uint32_t state);
        static void handle_modifiers(
            void *data,
            struct ::wl_keyboard *object,
            uint32_t serial,
            uint32_t mods_depressed,
            uint32_t mods_latched,
            uint32_t mods_locked,
            uint32_t group);
        static void handle_repeat_info(
            void *data,
            struct ::wl_keyboard *object,
            int32_t rate,
            int32_t delay);
        struct ::wl_keyboard *m_wl_keyboard;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_touch
    {
    public:
        wl_touch(struct ::wl_registry *registry, int id, int version);
        wl_touch(struct ::wl_touch *object);
        wl_touch();

        virtual ~wl_touch();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_touch *object);

        struct ::wl_touch *object() { return m_wl_touch; }
        const struct ::wl_touch *object() const { return m_wl_touch; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void release();

    protected:
        virtual void touch_down(uint32_t serial, uint32_t time, struct ::wl_surface *surface, int32_t id, wl_fixed_t x, wl_fixed_t y);
        virtual void touch_up(uint32_t serial, uint32_t time, int32_t id);
        virtual void touch_motion(uint32_t time, int32_t id, wl_fixed_t x, wl_fixed_t y);
        virtual void touch_frame();
        virtual void touch_cancel();

    private:
        void init_listener();
        static const struct wl_touch_listener m_wl_touch_listener;
        static void handle_down(
            void *data,
            struct ::wl_touch *object,
            uint32_t serial,
            uint32_t time,
            struct ::wl_surface *surface,
            int32_t id,
            wl_fixed_t x,
            wl_fixed_t y);
        static void handle_up(
            void *data,
            struct ::wl_touch *object,
            uint32_t serial,
            uint32_t time,
            int32_t id);
        static void handle_motion(
            void *data,
            struct ::wl_touch *object,
            uint32_t time,
            int32_t id,
            wl_fixed_t x,
            wl_fixed_t y);
        static void handle_frame(
            void *data,
            struct ::wl_touch *object);
        static void handle_cancel(
            void *data,
            struct ::wl_touch *object);
        struct ::wl_touch *m_wl_touch;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_output
    {
    public:
        wl_output(struct ::wl_registry *registry, int id, int version);
        wl_output(struct ::wl_output *object);
        wl_output();

        virtual ~wl_output();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_output *object);

        struct ::wl_output *object() { return m_wl_output; }
        const struct ::wl_output *object() const { return m_wl_output; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum subpixel {
            subpixel_unknown = 0,
            subpixel_none = 1,
            subpixel_horizontal_rgb = 2,
            subpixel_horizontal_bgr = 3,
            subpixel_vertical_rgb = 4,
            subpixel_vertical_bgr = 5
        };

        enum transform {
            transform_normal = 0,
            transform_90 = 1,
            transform_180 = 2,
            transform_270 = 3,
            transform_flipped = 4,
            transform_flipped_90 = 5,
            transform_flipped_180 = 6,
            transform_flipped_270 = 7
        };

        enum mode {
            mode_current = 0x1, // indicates this is the current mode
            mode_preferred = 0x2 // indicates this is the preferred mode
        };

    protected:
        virtual void output_geometry(int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const QString &make, const QString &model, int32_t transform);
        virtual void output_mode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);
        virtual void output_done();
        virtual void output_scale(int32_t factor);

    private:
        void init_listener();
        static const struct wl_output_listener m_wl_output_listener;
        static void handle_geometry(
            void *data,
            struct ::wl_output *object,
            int32_t x,
            int32_t y,
            int32_t physical_width,
            int32_t physical_height,
            int32_t subpixel,
            const char *make,
            const char *model,
            int32_t transform);
        static void handle_mode(
            void *data,
            struct ::wl_output *object,
            uint32_t flags,
            int32_t width,
            int32_t height,
            int32_t refresh);
        static void handle_done(
            void *data,
            struct ::wl_output *object);
        static void handle_scale(
            void *data,
            struct ::wl_output *object,
            int32_t factor);
        struct ::wl_output *m_wl_output;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_region
    {
    public:
        wl_region(struct ::wl_registry *registry, int id, int version);
        wl_region(struct ::wl_region *object);
        wl_region();

        virtual ~wl_region();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_region *object);

        struct ::wl_region *object() { return m_wl_region; }
        const struct ::wl_region *object() const { return m_wl_region; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        void destroy();
        void add(int32_t x, int32_t y, int32_t width, int32_t height);
        void subtract(int32_t x, int32_t y, int32_t width, int32_t height);

    private:
        struct ::wl_region *m_wl_region;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_subcompositor
    {
    public:
        wl_subcompositor(struct ::wl_registry *registry, int id, int version);
        wl_subcompositor(struct ::wl_subcompositor *object);
        wl_subcompositor();

        virtual ~wl_subcompositor();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_subcompositor *object);

        struct ::wl_subcompositor *object() { return m_wl_subcompositor; }
        const struct ::wl_subcompositor *object() const { return m_wl_subcompositor; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum error {
            error_bad_surface = 0 // the to-be sub-surface is invalid
        };

        void destroy();
        struct ::wl_subsurface *get_subsurface(struct ::wl_surface *surface, struct ::wl_surface *parent);

    private:
        struct ::wl_subcompositor *m_wl_subcompositor;
    };

    class Q_WAYLAND_CLIENT_WAYLAND_EXPORT wl_subsurface
    {
    public:
        wl_subsurface(struct ::wl_registry *registry, int id, int version);
        wl_subsurface(struct ::wl_subsurface *object);
        wl_subsurface();

        virtual ~wl_subsurface();

        void init(struct ::wl_registry *registry, int id, int version);
        void init(struct ::wl_subsurface *object);

        struct ::wl_subsurface *object() { return m_wl_subsurface; }
        const struct ::wl_subsurface *object() const { return m_wl_subsurface; }

        bool isInitialized() const;

        static const struct ::wl_interface *interface();

        enum error {
            error_bad_surface = 0 // wl_surface is not a sibling or the parent
        };

        void destroy();
        void set_position(int32_t x, int32_t y);
        void place_above(struct ::wl_surface *sibling);
        void place_below(struct ::wl_surface *sibling);
        void set_sync();
        void set_desync();

    private:
        struct ::wl_subsurface *m_wl_subsurface;
    };
}

QT_WARNING_POP
QT_END_NAMESPACE

#endif
