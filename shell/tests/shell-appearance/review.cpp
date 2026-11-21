// SPDX-License-Identifier: BSD-3-Clause
#include "shell/theme_follower.h"
#include "shell/wallpaper/bar_backdrop.h"
#include "ui/application.h"
#include "ui/window.h"
#include "ui/painter.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <filesystem>
#include <unistd.h>
using namespace Lingmo;
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
class Sheet:public ui::View {
public:std::string data;
 void paint(ui::Painter& p)override {
  p.fillRect(localRect(),ui::Color::rgb(0x808080));
  for(int row=0;row<2;++row){p.fillRect({12,12+row*62.0,568,50},row?ui::Color::rgb(0x222833):ui::Color::rgb(0xF1F3F6));
   const std::string variant=row?"dark":"light";int col=0;
   for(const auto* name:{"battery-level-100-symbolic.svg","battery-level-50-symbolic.svg","battery-level-20-charging-symbolic.svg","audio-volume-high-symbolic.svg","network-wireless-connected-75.svg","control.svg"})p.drawSvg(data+"/statusbar/"+variant+"/"+name,{28+col++*86.0,24+row*62.0,44,26});
  }
  p.drawSvg(data+"/launchpad.svg",{250,144,80,80});
 }
};
int main(int argc,char**argv){
 std::string temp="/tmp/lingmo-shell-look-XXXXXX";check(mkdtemp(temp.data()),"temp");
 struct Cleanup{std::string path;~Cleanup(){std::filesystem::remove_all(path);}}cleanup{temp};
 setenv("HOME",temp.c_str(),1);setenv("XDG_CONFIG_HOME",temp.c_str(),1);
 ui::Application app(argc,argv,ui::Application::Platform::Headless);
 decoration::Theme decoration;
 Shell::ShellThemeFollower follower(&decoration,temp+"/services.conf");
 app.loop().dispatch();
 auto service=base::dbus::Connection::open(base::dbus::Connection::Bus::Session,&app.loop());check(bool(service),"private bus");
 base::dbus::Object object(service.get(),"/org/lingmo/Services/Appearance","org.lingmo.Services.Appearance");
 std::string icon="first-login-icons";object.addProperty("iconTheme",[&]{return icon;});object.addSignal("iconThemeChanged","");object.publish();
 service->requestName("org.lingmo.Services");
 auto wait=[&](const std::string& expected){auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);while(ui::Theme::current().iconTheme!=expected&&std::chrono::steady_clock::now()<deadline){app.loop().dispatch();std::this_thread::sleep_for(std::chrono::milliseconds(2));}check(ui::Theme::current().iconTheme==expected,"Shell did not apply service icon theme");};
 wait(icon);icon="changed-icons";object.emitSignal("iconThemeChanged");wait(icon);
 decoration.setDarkMode(true);check(ui::Theme::current().iconTheme==icon,"colour change overwrote icon theme");
 check(!Shell::BarBackdrop::prefersWhite(Shell::BarBackdrop::relativeLuminance(245,245,245)),"bright background uses light resource variant");
 check(Shell::BarBackdrop::prefersWhite(Shell::BarBackdrop::relativeLuminance(20,20,20)),"dark background uses dark resource variant");
 if(argc>2){ui::Window window;window.resize({592,240});auto sheet=std::make_unique<Sheet>();sheet->data=argv[1];window.setContentView(std::move(sheet));window.show();check(window.grab().savePng(argv[2]),"screenshot");}
 std::cout<<"Shell appearance startup and resource contrast checks passed\n";
}
