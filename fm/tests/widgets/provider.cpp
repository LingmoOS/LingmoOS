// SPDX-License-Identifier: GPL-3.0-or-later
#include "shell/notification/notification_widgets.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/application.h"
#include "ui/window.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "base/event_loop.h"
#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
using namespace Lingmo;
void check(bool value,const char* text){if(!value){std::cerr<<text<<'\n';std::exit(1);}}
ui::View* find(ui::View* root,const std::string& name){if(root->name()==name)return root;for(const auto& child:root->children())if(auto* v=find(child.get(),name))return v;return nullptr;}
int main(int argc,char** argv){
 std::string temp="/tmp/lingmo-widget-provider-XXXXXX";check(mkdtemp(temp.data()),"temp directory");
 struct Cleanup{std::string path;~Cleanup(){std::filesystem::remove_all(path);}}cleanup{temp};
 setenv("HOME",temp.c_str(),1);setenv("XDG_CONFIG_HOME",temp.c_str(),1);
 ui::Application app(argc,argv,ui::Application::Platform::Headless);
 base::Settings settings(base::paths::lingmoConfig("filemanager"));
 const std::string layout=R"([{"kind":"calendar","size":"small","x":0,"y":0,"id":"shelf-calendar"},{"kind":"clock","size":"small","x":0,"y":0,"id":"shelf-clock"},{"kind":"worldclock","size":"medium","x":0,"y":0,"id":"shelf-world"}])";
 settings.setValue("NotificationWidgets/layout.v1",layout);settings.sync();
 const auto& provider=Shell::NotificationWidgetProvider::instance();check(provider.create&&provider.active,"provider versioned entry points");
 ui::Window window;window.resize({380,720});auto root=std::make_unique<ui::View>();auto shelf=std::unique_ptr<ui::View>(provider.create());auto* view=shelf.get();root->addChild(std::move(shelf));window.setContentView(std::move(root));window.show();
 provider.active(view,true);view->setGeometry({6,16,368,view->heightForWidth(368)});window.grab();
 auto* layer=find(view,"desktop-widgets");check(layer&&layer->children().size()==3,"shared provider loads three real widgets");
 check(layer->children()[0]->geometry().y==layer->children()[1]->geometry().y,"two small widgets share a row");
 check(layer->children()[2]->geometry().y>=192,"medium widget flows below");
 const auto saved=base::Settings(base::paths::lingmoConfig("filemanager")).stringValue("NotificationWidgets/layout.v1");
 provider.active(view,false);check(layer->children().empty(),"closing releases widget timers and subscriptions");
 check(base::Settings(base::paths::lingmoConfig("filemanager")).stringValue("NotificationWidgets/layout.v1")==saved,"closing preserves saved layout");
 provider.active(view,true);view->setGeometry({6,16,368,view->heightForWidth(368)});window.grab();check(layer->children().size()==3,"reopening restores cards");
 auto* first=layer->children()[0].get();auto p=first->mapToWindow({12,12});ui::TestInput::press(window,p);ui::TestInput::movePointer(window,{p.x,600});ui::TestInput::release(window,{p.x,600});app.loop().dispatch();window.grab();
 check(base::Settings(base::paths::lingmoConfig("filemanager")).stringValue("NotificationWidgets/layout.v1")!=saved,"drag order persists");
 settings.setValue("NotificationWidgets/visible",false);settings.sync();provider.active(view,true);check(!view->isVisible()&&layer->children().empty(),"notification visibility respected");
 settings.setValue("NotificationWidgets/visible",true);settings.sync();provider.active(view,true);view->setGeometry({6,16,368,view->heightForWidth(368)});window.grab();
 if(argc>1){std::filesystem::create_directories(argv[1]);check(window.grab().savePng(std::string(argv[1])+"/notification-widgets.png"),"provider screenshot");}
 std::cout<<"Notification widget provider passed\n";
}
