// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAYLANDINPUTCONTEXT_H
#define WAYLANDINPUTCONTEXT_H

#include "VirtualInputContextGlue.h"
#include "common/common.h"

#include <dimcore/InputContext.h>
#include <xkbcommon/xkbcommon.h>

#include <memory>

namespace wl {
namespace client {
class ZwpVirtualKeyboardV1;
class Surface;
} // namespace client
} // namespace wl

namespace org {
namespace deepin {
namespace dim {

class InputMethodV2;
class InputMethodKeyboardGrabV2;
class InputPopupSurfaceV2;
class AppMonitor;

class State
{
public:
    uint32_t modifiers = 0;
    uint32_t group = 0;
};

class WaylandInputContext : public VirtualInputContextGlue
{
    Q_OBJECT

public:
    explicit WaylandInputContext(const std::shared_ptr<InputMethodV2> &im,
                                 const std::shared_ptr<wl::client::ZwpVirtualKeyboardV1> &vk,
                                 const std::shared_ptr<wl::client::Surface> &surface,
                                 const std::shared_ptr<AppMonitor> &appMonitor,
                                 Dim *dim);
    ~WaylandInputContext() override;

    InputMethodV2 *getInputMethodV2() { return im_.get(); }

protected:
    void updatePreeditDelegate(InputContext *ic,
                               const QString &text,
                               int32_t cursorBegin,
                               int32_t cursorEnd) const override;
    void commitStringDelegate(InputContext *, const QString &text) const override;
    void forwardKeyDelegate(InputContext *, uint32_t keycode, bool pressed) const override;
    void commitDelegate() const override;
    void deleteSurroundingTextDelegate(InputContext *ic,
                                       int offset,
                                       unsigned int size) const override;

private:
    void activateCallback();
    void deactivateCallback();
    void surroundingTextCallback(const char *text, uint32_t cursor, uint32_t anchor);
    void textChangeCauseCallback(uint32_t cause);
    void contentTypeCallback(uint32_t hint, uint32_t purpose);
    void doneCallback();
    void unavailableCallback();

    void keymapCallback(uint32_t format, int32_t fd, uint32_t size);
    void keyCallback(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiersCallback(uint32_t serial,
                           uint32_t mods_depressed,
                           uint32_t mods_latched,
                           uint32_t mods_locked,
                           uint32_t group);
    void repeatInfoCallback(int32_t rate, int32_t delay);

private:
    std::shared_ptr<InputMethodV2> im_;
    std::shared_ptr<InputMethodKeyboardGrabV2> grab_;
    std::shared_ptr<wl::client::ZwpVirtualKeyboardV1> vk_;

    std::shared_ptr<AppMonitor> appMonitor_;
    std::unique_ptr<VirtualInputContextManager> vicm_;

    uint32_t serial_ = 1;
    std::unique_ptr<State> state_;

    std::vector<char> keymapData_;
    std::unique_ptr<xkb_context, Deleter<xkb_context_unref>> xkbContext_;
    std::unique_ptr<xkb_keymap, Deleter<xkb_keymap_unref>> xkbKeymap_;
    std::unique_ptr<xkb_state, Deleter<xkb_state_unref>> xkbState_;

    bool pendingDeactivate_ = false;
    bool pendingActivate_ = false;

    uint32_t modifierMask_[static_cast<uint8_t>(Modifiers::CNT)];
};

} // namespace dim
} // namespace deepin
} // namespace org

#endif // !WAYLANDINPUTCONTEXT_H
