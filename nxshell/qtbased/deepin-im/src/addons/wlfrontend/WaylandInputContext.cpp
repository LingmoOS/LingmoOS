// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WaylandInputContext.h"

#include "InputMethodKeyboardGrabV2.h"
#include "InputMethodV2.h"
#include "VirtualInputContextManager.h"
#include "common/common.h"
#include "dimcore/Dim.h"
#include "wl/client/ZwpVirtualKeyboardV1.h"

#include <linux/input.h>

#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include <sys/mman.h>

using namespace org::deepin::dim;

WaylandInputContext::WaylandInputContext(
    const std::shared_ptr<InputMethodV2> &im,
    const std::shared_ptr<wl::client::ZwpVirtualKeyboardV1> &vk,
    const std::shared_ptr<wl::client::Surface> &surface,
    const std::shared_ptr<AppMonitor> &appMonitor,
    Dim *dim)
    : VirtualInputContextGlue(nullptr, dim)
    , im_(im)
    , vk_(vk)
    , state_(std::make_unique<State>())
    , xkbContext_(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
{
    vicm_ = std::make_unique<VirtualInputContextManager>(this, appMonitor, dim);

    connect(im->qobject(),
            &InputMethodV2QObject::activate,
            this,
            &WaylandInputContext::activateCallback);
    connect(im->qobject(),
            &InputMethodV2QObject::deactivate,
            this,
            &WaylandInputContext::deactivateCallback);
    connect(im->qobject(),
            &InputMethodV2QObject::surroundingText,
            this,
            &WaylandInputContext::surroundingTextCallback);
    connect(im->qobject(),
            &InputMethodV2QObject::textChangeCause,
            this,
            &WaylandInputContext::textChangeCauseCallback);
    connect(im->qobject(),
            &InputMethodV2QObject::contentType,
            this,
            &WaylandInputContext::contentTypeCallback);
    connect(im->qobject(), &InputMethodV2QObject::done, this, &WaylandInputContext::doneCallback);
    connect(im->qobject(),
            &InputMethodV2QObject::unavailable,
            this,
            &WaylandInputContext::unavailableCallback);
}

WaylandInputContext::~WaylandInputContext() = default;

void WaylandInputContext::updatePreeditDelegate(InputContext *ic,
                                                const QString &text,
                                                int32_t cursorBegin,
                                                int32_t cursorEnd) const
{
    im_->set_preedit_string(text.toStdString().c_str(), cursorBegin, cursorEnd);
}

void WaylandInputContext::commitStringDelegate(InputContext *, const QString &text) const
{
    im_->commit_string(text.toStdString().c_str());
}

void WaylandInputContext::commitDelegate() const
{
    im_->commit(serial_);
}

void WaylandInputContext::forwardKeyDelegate(InputContext *, uint32_t keycode, bool pressed) const
{
    vk_->key(getTimestamp(),
             keycode,
             pressed ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
}

void WaylandInputContext::deleteSurroundingTextDelegate(InputContext *ic,
                                                        int offset,
                                                        unsigned int size) const
{
    //
}

void WaylandInputContext::activateCallback()
{
    pendingActivate_ = true;
}

void WaylandInputContext::deactivateCallback()
{
    pendingDeactivate_ = true;
}

void WaylandInputContext::surroundingTextCallback(const char *text,
                                                  uint32_t cursor,
                                                  uint32_t anchor)
{
    surroundingText().setText(text);
    surroundingText().setCursor(cursor);
    surroundingText().setAnchor(anchor);

    updateSurroundingTextWrapper();
}

void WaylandInputContext::textChangeCauseCallback(uint32_t cause)
{
    // TODO:
}

void WaylandInputContext::contentTypeCallback(uint32_t hint, uint32_t purpose)
{
    contentType().setHint(hint);
    contentType().setPurpose(purpose);

    updateContentTypeWrapper();
}

void WaylandInputContext::doneCallback()
{
    serial_++;

    // TODO:
    if (pendingDeactivate_) {
        pendingDeactivate_ = false;

        grab_.reset();

        // todo: release all pressed keys

        focusOutWrapper();
    }

    if (pendingActivate_) {
        pendingActivate_ = false;
        grab_.reset();

        auto *val = im_->grabKeyboard();
        if (val == nullptr) {
            // todo:
        }
        grab_ = std::make_shared<InputMethodKeyboardGrabV2>(val);
        connect(grab_->qobject(),
                &InputMethodKeyboardGrabV2QObject::keymap,
                this,
                &WaylandInputContext::keymapCallback);
        connect(grab_->qobject(),
                &InputMethodKeyboardGrabV2QObject::key,
                this,
                &WaylandInputContext::keyCallback);
        connect(grab_->qobject(),
                &InputMethodKeyboardGrabV2QObject::modifiers,
                this,
                &WaylandInputContext::modifiersCallback);
        connect(grab_->qobject(),
                &InputMethodKeyboardGrabV2QObject::repeatInfo,
                this,
                &WaylandInputContext::repeatInfoCallback);

        focusInWrapper();
    }

    InputContextEvent event(EventType::InputContextDone, delegatedInputContext());
    dim()->postEvent(event);
}

void WaylandInputContext::unavailableCallback()
{
    // TODO:
}

void WaylandInputContext::keymapCallback(uint32_t format, int32_t fd, uint32_t size)
{
    if (format == WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP) {
        return;
    }

    auto *ptr = static_cast<const char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (ptr == MAP_FAILED) {
        return;
    }

    const bool keymapChanged =
        (size != keymapData_.size() || memcmp(ptr, keymapData_.data(), size) != 0);
    if (keymapChanged) {
        std::vector<char> newKeymapData(ptr, ptr + size);
        std::swap(keymapData_, newKeymapData);

        xkbKeymap_.reset(xkb_keymap_new_from_string(xkbContext_.get(),
                                                    static_cast<const char *>(ptr),
                                                    XKB_KEYMAP_FORMAT_TEXT_V1,
                                                    XKB_KEYMAP_COMPILE_NO_FLAGS));
    }

    if (!xkbKeymap_) {
        return;
    }

    xkbState_.reset(xkb_state_new(xkbKeymap_.get()));
    if (!xkbState_) {
        xkbKeymap_.reset();
        return;
    }

    modifierMask_[static_cast<uint8_t>(Modifiers::Shift)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Shift");
    modifierMask_[static_cast<uint8_t>(Modifiers::Lock)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Lock");
    modifierMask_[static_cast<uint8_t>(Modifiers::Control)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Control");
    modifierMask_[static_cast<uint8_t>(Modifiers::Mod1)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Mod1");
    modifierMask_[static_cast<uint8_t>(Modifiers::Mod2)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Mod2");
    modifierMask_[static_cast<uint8_t>(Modifiers::Mod3)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Mod3");
    modifierMask_[static_cast<uint8_t>(Modifiers::Mod4)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Mod4");
    modifierMask_[static_cast<uint8_t>(Modifiers::Mod5)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Mod5");
    modifierMask_[static_cast<uint8_t>(Modifiers::Alt)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Alt");
    modifierMask_[static_cast<uint8_t>(Modifiers::Meta)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Meta");
    modifierMask_[static_cast<uint8_t>(Modifiers::Super)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Super");
    modifierMask_[static_cast<uint8_t>(Modifiers::Hyper)] = 1
        << xkb_keymap_mod_get_index(xkbKeymap_.get(), "Hyper");

    if (keymapChanged) {
        vk_->keymap(format, fd, size);
    }
}

void WaylandInputContext::keyCallback(uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    assert(xkbState_);

    auto *ic = delegatedInputContext();

    uint32_t code = key + XKB_HISTORICAL_OFFSET;

    xkb_keysym_t sym = xkb_state_key_get_one_sym(xkbState_.get(), code);
    InputContextKeyEvent ke(ic,
                            static_cast<uint32_t>(sym),
                            key,
                            state_->modifiers,
                            state == WL_KEYBOARD_KEY_STATE_RELEASED,
                            time);
    bool res = keyEvent(ke);
    if (!res) {
        vk_->key(getTimestamp(), key, state);
        return;
    }
}

void WaylandInputContext::modifiersCallback(uint32_t serial,
                                            uint32_t mods_depressed,
                                            uint32_t mods_latched,
                                            uint32_t mods_locked,
                                            uint32_t group)
{
    if (xkbState_) {
        xkb_state_component comp = xkb_state_update_mask(xkbState_.get(),
                                                         mods_depressed,
                                                         mods_latched,
                                                         mods_locked,
                                                         0,
                                                         0,
                                                         group);
        xkb_mod_mask_t mask = xkb_state_serialize_mods(xkbState_.get(), comp);
        state_->modifiers = 0;
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Shift)]) {
            state_->modifiers |= SHIFT_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Lock)]) {
            state_->modifiers |= LOCK_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Control)]) {
            state_->modifiers |= CONTROL_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Mod1)]) {
            state_->modifiers |= MOD1_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Mod2)]) {
            state_->modifiers |= MOD2_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Mod3)]) {
            state_->modifiers |= MOD3_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Mod4)]) {
            state_->modifiers |= MOD4_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Mod5)]) {
            state_->modifiers |= MOD5_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Alt)]) {
            state_->modifiers |= ALT_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Meta)]) {
            state_->modifiers |= META_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Super)]) {
            state_->modifiers |= SUPER_MASK;
        }
        if (mask & modifierMask_[static_cast<uint8_t>(Modifiers::Hyper)]) {
            state_->modifiers |= HYPER_MASK;
        }
    }

    if (vk_) {
        vk_->modifiers(mods_depressed, mods_latched, mods_locked, group);
    }
}

void WaylandInputContext::repeatInfoCallback(int32_t rate, int32_t delay)
{
    // TODO:
}
