// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include <gtest/gtest.h>
#include <linux/input-event-codes.h>
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/keyboard.h"
#include "input/keyboard_delegate.h"
#include "input/seat.h"
#include "input/session_gate.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/keyboard/screen_keyboard.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "ui/embedded_window.h"
#include "ui/test_input.h"
#include "wm/window.h"
#include "wm/window_manager.h"
using namespace Lingmo;
namespace {
class Recorder : public input::KeyboardDelegate {
public:
 compositor::Surface* target=nullptr;
 std::vector<std::pair<uint32_t,bool>> keys;
 bool canAcceptKeyboardEventsForSurface(compositor::Surface* surface) const override{return surface==target;}
 void onKeyboardEnter(compositor::Surface*,const PressedKeys&) override{}
 void onKeyboardLeave(compositor::Surface*) override{}
 uint32_t onKeyboardKey(input::Timestamp,uint32_t key,bool pressed) override{keys.emplace_back(key,pressed);return 0;}
 void onKeyboardModifiers(const input::KeyboardModifiers&) override{}
 void onKeyRepeatSettingsChanged(bool,std::chrono::milliseconds,std::chrono::milliseconds) override{}
 void onKeyboardLayoutUpdated(const std::string&) override{}
};
class BlockedGate : public input::SessionGate {
public:
 bool restrictsInput() const override{return true;}
 scene::Node* inputRoot() const override{return nullptr;}
 bool allowsFocus(const compositor::Surface*) const override{return false;}
 bool allowsActivation() const override{return false;}
};
class ScreenKeyboardTest : public testing::Test {
protected:
 scene::Scene scene;
 scene::Node* surfaceContainer=scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
 compositor::Compositor compositor{surfaceContainer};
 Lingmo::Testsupport::FakeRenderer renderer;
 input::Seat seat{&compositor};
 input::InputManager input{&seat};
 wm::WindowManager manager{&compositor,&seat};
 std::shared_ptr<compositor::ShmPool> pool=Lingmo::Testsupport::makePool(1<<22);
 std::unique_ptr<shell::Shell> shell;
 std::unique_ptr<compositor::Surface> surface;
 shell::ScreenKeyboard* keyboard=nullptr;
 wm::Window* application=nullptr;
 void SetUp() override {
  const base::Rect output(0,0,1440,960);
  scene.setSize(output.size());manager.setOutputBounds(output);input.setBounds(output.size());input.setEventSink(&manager);
  shell::ShellEnvironment environment{&compositor,&manager,&seat};
  shell=std::make_unique<shell::Shell>(environment);
  surface=compositor.createSurface();
  surface->attach(pool->createBuffer(base::Size(400,300),0,1600,render::PixelFormat::Bgrx8888,nullptr),{});
  surface->commit();compositor.updateTextures(renderer);
  application=manager.createWindow(surface.get(),{.requestedSize=base::Size(400,300),.applicationId="test-app"});
  application->setBounds(base::Rect(100,100,400,300));application->activate();
  keyboard=shell->addComponent(std::make_unique<shell::ScreenKeyboard>(shell.get()));
 }
 void TearDown() override {seat.setSessionGate(nullptr);shell.reset();seat.shutdown();}
};
}
TEST_F(ScreenKeyboardTest, PanelDoesNotStealApplicationFocus) {
 auto recorder=std::make_unique<Recorder>();auto* events=recorder.get();events->target=surface.get();
 input::Keyboard clientKeyboard(std::move(recorder),&seat);
 ASSERT_EQ(seat.focusedSurface(),surface.get());keyboard->show();compositor.updateTextures(renderer);
 ASSERT_TRUE(keyboard->visible());EXPECT_EQ(seat.focusedSurface(),surface.get());
 const auto r=keyboard->window()->geometry();
 input.onPointerMotionAbsolute(std::chrono::steady_clock::now(),{double(r.x()+100),double(r.y()+100)});
 input.onPointerButton(std::chrono::steady_clock::now(),BTN_LEFT,true);
 input.onPointerButton(std::chrono::steady_clock::now(),BTN_LEFT,false);
 EXPECT_EQ(seat.focusedSurface(),surface.get());EXPECT_TRUE(seat.clientPressedKeys().empty());
 EXPECT_FALSE(events->keys.empty());
 if(events->keys.size()>=2){EXPECT_TRUE(events->keys.front().second);EXPECT_FALSE(events->keys.back().second);}
 keyboard->hide();EXPECT_FALSE(keyboard->visible());
}
TEST_F(ScreenKeyboardTest, LockedSessionRejectsShowAndHidesOpenPanel) {
 keyboard->show();ASSERT_TRUE(keyboard->visible());
 BlockedGate gate;seat.setSessionGate(&gate);
 EXPECT_TRUE(Lingmo::Test::runUntil([&]{return !keyboard->visible();}));
 keyboard->show();EXPECT_FALSE(keyboard->visible());
 seat.setSessionGate(nullptr);
}
