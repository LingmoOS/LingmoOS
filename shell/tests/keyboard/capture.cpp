// SPDX-License-Identifier: GPL-3.0-or-later
#include "shell/keyboard/keyboard_view.h"
#include "ui/application.h"
#include "ui/theme.h"
#include "ui/window.h"
#include <filesystem>
int main(int argc,char**argv){
 if(argc!=2)return 1;std::filesystem::create_directories(argv[1]);
 Lingmo::ui::Application app(argc,argv,Lingmo::ui::Application::Platform::Headless);
 for(bool dark:{false,true})for(double scale:{1.0,1.5}){
  Lingmo::ui::Theme::setCurrent(dark?Lingmo::ui::Theme::dark():Lingmo::ui::Theme::light());
  Lingmo::ui::Window window;window.resize({960,330});window.setScale(scale);
  window.setContentView(std::make_unique<Lingmo::Shell::ScreenKeyboardView>());window.show();
  const auto name=std::string(dark?"dark":"light")+(scale==1?"-100":"-150");
  if(!window.grab().savePng(std::string(argv[1])+"/keyboard-"+name+".png"))return 2;
 }
}
