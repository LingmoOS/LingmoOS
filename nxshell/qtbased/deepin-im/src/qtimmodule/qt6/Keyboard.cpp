#include "Keyboard.h"

#include <private/qxkbcommon_p.h>

#include <QDebug>
#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformcursor.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatformintegration.h>

#include <sys/mman.h>
#include <unistd.h>

Keyboard::Keyboard(wl_keyboard *val)
    : wl::client::Keyboard(val)
    , xkb_ctx_(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    , device_(std::make_unique<QInputDevice>("dim virtual",
                                             QInputDevice::primaryKeyboard()->systemId(),
                                             QInputDevice::DeviceType::Keyboard))
#endif
{
    QObject::connect(&repeatTimer_, &QTimer::timeout, [&]() {
        // if (!focusWindow()) {
        //     // We destroyed the keyboard focus surface, but the server didn't get the message
        //     yet...
        //     // or the server didn't send an enter event first.
        //     return;
        // }
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

    // auto *window = focusWindow();
    // if (!window) {
    //     // We destroyed the keyboard focus surface, but the server didn't get the message yet...
    //     // or the server didn't send an enter event first. In either case, ignore the event.
    //     return;
    // }

    serial_ = serial;

    const bool isDown = state != WL_KEYBOARD_KEY_STATE_RELEASED;
    // if (isDown) {
    //     mParent->mQDisplay->setLastInputDevice(mParent, serial, window);
    // }

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
    QPlatformInputContext *inputContext =
        QGuiApplicationPrivate::platformIntegration()->inputContext();
    bool filtered = false;

    if (inputContext) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        QKeyEvent event(type,
                        key,
                        modifiers,
                        nativeScanCode,
                        nativeVirtualKey,
                        nativeModifiers,
                        text,
                        autorepeat,
                        count,
                        device_.get());
#else
        QKeyEvent event(type,
                        key,
                        modifiers,
                        nativeScanCode,
                        nativeVirtualKey,
                        nativeModifiers,
                        text,
                        autorepeat,
                        count);
#endif
        event.setTimestamp(timestamp);
        filtered = inputContext->filterEvent(&event);
    }

    if (!filtered) {
        auto window = qGuiApp->focusWindow();

        if (type == QEvent::KeyPress && key == Qt::Key_Menu) {
            auto cursor = window->screen()->handle()->cursor();
            if (cursor) {
                const QPoint globalPos = cursor->pos();
                const QPoint pos = window->mapFromGlobal(globalPos);
                QWindowSystemInterface::handleContextMenuEvent(window,
                                                               false,
                                                               pos,
                                                               globalPos,
                                                               modifiers);
            }
        }

        QWindowSystemInterface::handleExtendedKeyEvent(window,
                                                       timestamp,
                                                       type,
                                                       key,
                                                       modifiers,
                                                       nativeScanCode,
                                                       nativeVirtualKey,
                                                       nativeModifiers,
                                                       text,
                                                       autorepeat,
                                                       count);
    }
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
