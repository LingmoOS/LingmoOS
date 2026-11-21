// SPDX-License-Identifier: GPL-3.0-or-later
#include "accelerators/accelerator_table.h"
#include "accelerators/accelerator_delegate.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Lingmo::Accelerators;
void check(bool ok){if(!ok)std::exit(1);}
int main(){
 for(const auto& table:{std::vector<AcceleratorData>(kAcceleratorData.begin(),kAcceleratorData.end()),std::vector<AcceleratorData>(kCommandAcceleratorData.begin(),kCommandAcceleratorData.end())}){
  int count=0;for(const auto& key:table)if(key.keysym==XKB_KEY_k&&key.modifiers==(MetaDown|ControlDown)&&key.triggerOnPress){check(key.action==AcceleratorAction::ToggleScreenKeyboard);++count;}check(count==1);
 }
 check(std::find(kActionsAllowedAtLockScreen.begin(),kActionsAllowedAtLockScreen.end(),AcceleratorAction::ToggleScreenKeyboard)==kActionsAllowedAtLockScreen.end());
 check(acceleratorActionFromName("toggle-screen-keyboard")==AcceleratorAction::ToggleScreenKeyboard);
 Lingmo::base::EventLoop loop;auto* bus=Lingmo::base::dbus::Connection::session();check(bus);
 bool called=false;Lingmo::base::dbus::Object object(bus,"/org/lingmo/ScreenKeyboard","org.lingmo.ScreenKeyboard");
 object.addMethod("toggle",[&]{called=true;});check(object.publish()&&bus->requestName("org.lingmo.ScreenKeyboard"));
 DBusAcceleratorDelegate delegate;delegate.toggleScreenKeyboard();
 auto end=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(!called&&std::chrono::steady_clock::now()<end){pollfd fd{loop.fd(),POLLIN,0};::poll(&fd,1,10);loop.dispatch();}
 check(called);std::cout<<"Keyboard shortcut contract passed\n";
}
