#include "Keyboard.h"

#include <private/qxkbcommon_p.h>

#include <QDebug>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatformintegration.h>

#include <sys/mman.h>
#include <unistd.h>

Keyboard::Keyboard(wl_keyboard *val, DimIMContextWaylandGlobal *global)
    : wl::client::Keyboard(val)
    , xkb_ctx_(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
    , global_(global)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    , device_(std::make_unique<QInputDevice>("dim virtual",
                                             QInputDevice::primaryKeyboard()->systemId(),
                                             QInputDevice::DeviceType::Keyboard))
#endif
{
    QObject::connect(&repeatTimer_, &QTimer::timeout, [&]() {
        repeatTimer_.setInterval(1000 / repeatRate);
        handleKey(repeatKey_.time,
                  QEvent::KeyRelease,
                  repeatKey_.key,
                  repeatKey_.modifiers,
                  repeatKey_.code,
                  repeatKey_.nativeVirtualKey,
                  repeatKey_.nativeModifiers,
                  repeatKey_.text,
                  true);
        handleKey(repeatKey_.time,
                  QEvent::KeyPress,
                  repeatKey_.key,
                  repeatKey_.modifiers,
                  repeatKey_.code,
                  repeatKey_.nativeVirtualKey,
                  repeatKey_.nativeModifiers,
                  repeatKey_.text,
                  true);
    });
}

Keyboard::~Keyboard() { }

void Keyboard::wl_keyboard_keymap(uint32_t format, int32_t fd, uint32_t size)
{
    keymapFormat_ = static_cast<enum wl_keyboard_keymap_format>(format);

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        qWarning() << "unknown keymap format:" << format;
        close(fd);
        return;
    }

    char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (map_str == MAP_FAILED) {
        close(fd);
        return;
    }

    xkb_keymap_.reset(xkb_keymap_new_from_string(xkb_ctx_.get(),
                                                 map_str,
                                                 XKB_KEYMAP_FORMAT_TEXT_V1,
                                                 XKB_KEYMAP_COMPILE_NO_FLAGS));
    QXkbCommon::verifyHasLatinLayout(xkb_keymap_.get());

    munmap(map_str, size);
    close(fd);

    if (xkb_keymap_) {
        xkb_state_.reset(xkb_state_new(xkb_keymap_.get()));
    } else {
        xkb_state_.reset();
    }
}

void Keyboard::wl_keyboard_enter(uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
}

void Keyboard::wl_keyboard_leave(uint32_t serial, struct wl_surface *surface) { }

void Keyboard::wl_keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    if (keymapFormat_ != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1
        && keymapFormat_ != WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP) {
        qWarning() << Q_FUNC_INFO << "unknown keymap format:" << keymapFormat_;
        return;
    }

    serial_ = serial;

    const bool isDown = state != WL_KEYBOARD_KEY_STATE_RELEASED;


    if (keymapFormat_ == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        if ((!xkb_keymap_ || !xkb_state_) && !createDefaultKeymap()) {
            return;
        }

        auto code = key + 8; // map to wl_keyboard::keymap_format::keymap_format_xkb_v1

        xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state_.get(), code);

        int qtkey = keysymToQtKey(sym, modifiers_, xkb_state_.get(), code);
        QString text = QXkbCommon::lookupString(xkb_state_.get(), code);

        QEvent::Type type = isDown ? QEvent::KeyPress : QEvent::KeyRelease;
        handleKey(time, type, qtkey, modifiers_, code, sym, nativeModifiers_, text);

        if (state == WL_KEYBOARD_KEY_STATE_PRESSED
            && xkb_keymap_key_repeats(xkb_keymap_.get(), code) && repeatRate > 0) {
            repeatKey_.key = qtkey;
            repeatKey_.code = code;
            repeatKey_.time = time;
            repeatKey_.text = text;
            repeatKey_.modifiers = modifiers_;
            repeatKey_.nativeModifiers = nativeModifiers_;
            repeatKey_.nativeVirtualKey = sym;
            repeatTimer_.setInterval(repeatDelay);
            repeatTimer_.start();
        } else if (repeatKey_.code == code) {
            repeatTimer_.stop();
        }
    } else if (keymapFormat_ == WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP) {
        // raw scan code
        return;
    }
}

void Keyboard::wl_keyboard_modifiers(uint32_t serial,
                                     uint32_t mods_depressed,
                                     uint32_t mods_latched,
                                     uint32_t mods_locked,
                                     uint32_t group)
{
    if (xkb_state_)
        xkb_state_update_mask(xkb_state_.get(),
                              mods_depressed,
                              mods_latched,
                              mods_locked,
                              0,
                              0,
                              group);
    nativeModifiers_ = mods_depressed | mods_latched | mods_locked;
}

void Keyboard::wl_keyboard_repeat_info(int32_t rate, int32_t delay) { }

int Keyboard::keysymToQtKey(xkb_keysym_t keysym,
                            Qt::KeyboardModifiers modifiers,
                            xkb_state *state,
                            xkb_keycode_t code)
{
    return QXkbCommon::keysymToQtKey(keysym, modifiers, state, code);
}

void Keyboard::handleKey(ulong timestamp,
                         QEvent::Type type,
                         int key,
                         Qt::KeyboardModifiers modifiers,
                         quint32 nativeScanCode,
                         quint32 nativeVirtualKey,
                         quint32 nativeModifiers,
                         const QString &text,
                         bool autorepeat,
                         ushort count)
{
    if (!global_->current)
        return;

    auto state = type == QEvent::KeyPress ? true : false;

#if GTK_CHECK_VERSION(4,0,0)
#else
    GdkEvent *gdk_event = gdk_event_new(state ? GDK_KEY_PRESS : GDK_KEY_RELEASE);
    GdkEventKey *gdk_event_key = reinterpret_cast<GdkEventKey *>(gdk_event);

    DimIMContext *contextWayland = DIM_IM_CONTEXT(global_->current);

    gdk_event_key->type = state ? GDK_KEY_PRESS : GDK_KEY_RELEASE;
    gdk_event_key->window = g_object_ref(contextWayland->window);
    gdk_event_key->send_event = TRUE;
    gdk_event_key->time = timestamp;
    gdk_event_key->keyval = key;
    gdk_event_key->length = 1;
    gdk_event_key->string = nullptr;
    gdk_event_key->state = modifiers;

    gdk_event_key->send_event = TRUE;
    gdk_event_key->time = timestamp;

    auto handled = gtk_im_context_filter_keypress(contextWayland->slave, &gdk_event->key);
    gdk_event_free(gdk_event);

    if (!handled) {
        g_debug("failed to handle keypress");
    }
#endif
}

bool Keyboard::createDefaultKeymap()
{
    struct xkb_rule_names names;
    names.rules = "evdev";
    names.model = "pc105";
    names.layout = "us";
    names.variant = "";
    names.options = "";

    xkb_keymap_.reset(
        xkb_keymap_new_from_names(xkb_ctx_.get(), &names, XKB_KEYMAP_COMPILE_NO_FLAGS));
    if (xkb_keymap_) {
        xkb_state_.reset(xkb_state_new(xkb_keymap_.get()));
    }

    if (!xkb_keymap_ || !xkb_state_) {
        qWarning("failed to create default keymap");
        return false;
    }

    return true;
}
