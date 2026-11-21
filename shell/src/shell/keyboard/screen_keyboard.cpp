// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "screen_keyboard.h"
#include "keyboard_view.h"
#include "base/dbus.h"
#include "core/main_loop.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/application.h"
#include "ui/theme.h"
#include "wm/window.h"
#include <algorithm>
#include <chrono>
namespace Lingmo::Shell {
ScreenKeyboard::ScreenKeyboard(Shell* shell):m_shell(shell),m_guard(core::mainLoop()) {
    m_window=std::make_unique<ShellWindow>(shell,wm::WindowRole::Panel);
    m_window->setFocusPolicy(wm::FocusPolicy::NoFocus);
    m_window->setPanelBehavior(wm::PanelBehavior::StayAbove);
    m_window->setProtectedFromControl(true);
    auto view=std::make_unique<ScreenKeyboardView>();m_view=view.get();
    m_view->setGlassEnabled(true);
    m_view->onHide=[this]{hide();};
    m_view->onKey=[this](uint32_t symbol,bool control,bool alt){return send(symbol,control,alt);};
    m_window->setContentView(std::move(view));
    m_window->window()->setBackgroundColor(ui::Color{0,0,0,0});
    m_window->window()->setTranslucentSubpixelText(false);
    m_window->onCloseRequested=[this]{hide();};
    m_guard.setInterval(std::chrono::milliseconds(100));
    m_guard.onTimeout=[this]{
        if(!allowed()){hide();return;}
        const auto* focused=m_shell->seat()->focusedSurface();
        if(focused!=m_lastFocus){m_lastFocus=focused;m_view->reset();place();}
        if(m_glassDark!=int(ui::Theme::current().darkMode))place();
    };
    m_subscriptions.push_back(shell->outputsChanged.connect([this]{if(visible())place();}));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this]{m_view->reset();}));
}
ScreenKeyboard::~ScreenKeyboard(){
    m_guard.stop();m_subscriptions.clear();
    if(m_service && m_shell->bus())m_shell->bus()->releaseName("org.lingmo.ScreenKeyboard");
}
bool ScreenKeyboard::allowed() const {
    const auto* seat=m_shell->seat();
    return seat && (!seat->sessionGate() || !seat->sessionGate()->restrictsInput());
}
bool ScreenKeyboard::visible() const{return m_window->isShown();}
bool ScreenKeyboard::registerService(){
    auto* bus=m_shell->bus();if(!bus)return false;
    if(m_service)return true;
    auto object=std::make_unique<base::dbus::Object>(bus,"/org/lingmo/ScreenKeyboard","org.lingmo.ScreenKeyboard");
    // Only visibility is exposed; key injection is deliberately not a D-Bus API.
    object->addMethod("show",[this]{show();});object->addMethod("hide",[this]{hide();});object->addMethod("toggle",[this]{toggle();});
    object->addProperty("visible",[this]{return visible();});
    if(!object->publish() || !bus->requestName("org.lingmo.ScreenKeyboard",base::dbus::Connection::DoNotQueue))return false;
    m_service=std::move(object);return true;
}
void ScreenKeyboard::place(){
    const auto* output=m_shell->output({});
    if(auto* seat=m_shell->seat())if(auto* focused=seat->focusedSurface())
        if(auto* target=m_shell->windowManager()->windowForSurface(focused)) {
            const auto bounds=target->bounds();
            output=m_shell->outputAt(base::Point(bounds.x()+bounds.width()/2,bounds.y()+bounds.height()/2));
        }
    if(!output)return;
    const auto area=m_shell->workArea(*output);
    const int w=std::min(960,std::max(1,area.width()-24));
    const int h=std::min(330,std::max(1,area.height()-24));
    m_window->setOutput(output->name);
    m_window->setGeometry(base::Rect(area.x()+(area.width()-w)/2,area.y()+area.height()-h-12,w,h));
    const bool dark=ui::Theme::current().darkMode;
    m_glassDark=int(dark);
    m_window->setBackgroundBlur(base::Rect(0,0,w,h),16,dark?popupGlass(.17,.17,.19,.6):popupGlass(1,1,1,.6));
}
void ScreenKeyboard::show(){
    if(!allowed() || visible())return;
    m_lastFocus=m_shell->seat()->focusedSurface();
    m_view->reset();place();m_window->show();m_guard.start();
    if(m_service)m_service->emitPropertiesChanged({"visible"});
}
void ScreenKeyboard::hide(){
    m_guard.stop();m_view->reset();if(!visible())return;
    m_window->hide();if(m_service)m_service->emitPropertiesChanged({"visible"});
}
void ScreenKeyboard::toggle(){if(visible())hide();else show();}
bool ScreenKeyboard::send(uint32_t symbol,bool control,bool alt){
    if(!visible() || !allowed())return false;
    auto* seat=m_shell->seat();auto* target=seat->focusedSurface();
    if(target!=m_lastFocus){m_lastFocus=target;m_view->reset();return false;}
    if(!target || target==m_window->surface() || seat->focusIsBorrowed() || !seat->allowsFocus(target))return false;
    const auto stroke=seat->clientXkbTracker()->keyStroke(symbol);if(!stroke)return false;
    std::vector<uint32_t> held;for(const auto& [code,state]:seat->clientPressedKeys())held.push_back(code);
    const auto chord=screenKeyboardChord(stroke->key,stroke->modifiers,control,alt,held);
    if(chord.empty())return false;
    // Seat::tapKeys enters the existing IME filter and releases only our chord.
    // No clipboard replacement, no shortcut execution and no input-method takeover.
    seat->tapKeys(std::chrono::steady_clock::now(),chord);
    return true;
}
}
