// SPDX-License-Identifier: GPL-3.0-or-later
#include "ui/appearance.h"
#include "ui/theme.h"
#include "ui/application.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include <chrono>
#include <iostream>
#include <thread>
using namespace Lingmo;
int main(int argc,char**argv){
 ui::Application app(argc,argv,ui::Application::Platform::Headless);
 auto connection=base::dbus::Connection::open(base::dbus::Connection::Bus::Session,&app.loop());
 if(!connection)return 1;
 ui::AppearanceWatcher watcher(*connection);watcher.waitForSettings();
 // Export the service only after the initial GetAll has failed.
 auto service=base::dbus::Connection::open(base::dbus::Connection::Bus::Session,&app.loop());
 base::dbus::Object object(service.get(),"/org/lingmo/Services/Appearance","org.lingmo.Services.Appearance");
 object.addProperty("iconTheme",[]{return std::string("startup-test-icons");});
 object.addProperty("isDarkMode",[]{return true;});
 object.publish();service->requestName("org.lingmo.Services");
 const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);
 while(ui::Theme::current().iconTheme!="startup-test-icons"&&std::chrono::steady_clock::now()<deadline){app.loop().dispatch();std::this_thread::sleep_for(std::chrono::milliseconds(2));}
 if(ui::Theme::current().iconTheme!="startup-test-icons"||!ui::Theme::current().darkMode){std::cerr<<"Late appearance service did not apply\n";return 1;}
 std::cout<<"Late appearance service applies without changing theme manually\n";
}
