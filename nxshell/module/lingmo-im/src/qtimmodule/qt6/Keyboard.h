#ifndef DIM_KEYBOARD_H
#define DIM_KEYBOARD_H

#include "common/common.h"
#include "wl/client/Keyboard.h"

#include <xkbcommon/xkbcommon.h>

#include <QEvent>
#include <QList>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#  include <QInputDevice>
#endif

#include <memory>

class Keyboard : public wl::client::Keyboard
{
public:
    explicit Keyboard(wl_keyboard *val);
    ~Keyboard();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool sameDevice(const QInputDevice *device) { return device_.get() == device; }
#endif

protected:
    void wl_keyboard_keymap(uint32_t format, int32_t fd, uint32_t size) override;
    void wl_keyboard_enter(uint32_t serial,
                           struct wl_surface *surface,
                           struct wl_array *keys) override;
    void wl_keyboard_leave(uint32_t serial, struct wl_surface *surface) override;
    void wl_keyboard_key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state) override;
    void wl_keyboard_modifiers(uint32_t serial,
                               uint32_t mods_depressed,
                               uint32_t mods_latched,
                               uint32_t mods_locked,
                               uint32_t group) override;
    void wl_keyboard_repeat_info(int32_t rate, int32_t delay) override;

private:
    int keysymToQtKey(xkb_keysym_t keysym,
                      Qt::KeyboardModifiers modifiers,
                      xkb_state *state,
                      xkb_keycode_t code);
    void handleKey(ulong timestamp,
                   QEvent::Type type,
                   int key,
                   Qt::KeyboardModifiers modifiers,
                   quint32 nativeScanCode,
                   quint32 nativeVirtualKey,
                   quint32 nativeModifiers,
                   const QString &text,
                   bool autorepeat = false,
                   ushort count = 1);
    bool createDefaultKeymap();

private:
    std::unique_ptr<struct xkb_context, Deleter<xkb_context_unref>> xkb_ctx_;
    std::unique_ptr<struct xkb_keymap, Deleter<xkb_keymap_unref>> xkb_keymap_;
    std::unique_ptr<struct xkb_state, Deleter<xkb_state_unref>> xkb_state_;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    std::unique_ptr<QInputDevice> device_;
#endif

    uint32_t serial_ = 0;

    enum wl_keyboard_keymap_format keymapFormat_ = WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1;

    Qt::KeyboardModifiers modifiers_;
    uint32_t nativeModifiers_;

    struct repeatKey
    {
        int key = 0;
        uint32_t code = 0;
        uint32_t time = 0;
        QString text;
        Qt::KeyboardModifiers modifiers;
        uint32_t nativeVirtualKey = 0;
        uint32_t nativeModifiers = 0;
    } repeatKey_;

    QTimer repeatTimer_;
    int repeatRate = 25;
    int repeatDelay = 400;
};

#endif // !DIM_KEYBOARD_H
