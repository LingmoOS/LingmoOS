// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "desktop/widgets/widget.h"
#include "desktop/widgets/widgetgallery.h"
#include "desktop/widgets/widgetlayer.h"
#include "desktop/widgets/worldclockwidget.h"
#include "core/config.h"
#include "base/settings.h"
#include "base/i18n.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/painter.h"
#include "ui/label.h"
#include "ui/text_field.h"
#include "ui/button.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"
#include <cstdlib>
#include <clocale>
#include <libintl.h>
#include <filesystem>
#include <iostream>
#include <chrono>
#include <unistd.h>
using namespace Lingmo;
void check(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
void setReviewTheme(bool dark){auto theme=dark?ui::Theme::dark():ui::Theme::light();if(const char* font=std::getenv("WIDGET_REVIEW_FONT"))theme.fontFamily=font;ui::Theme::setCurrent(theme);}
class Backdrop:public ui::View{
public:bool dark=false;
protected:void paint(ui::Painter& p)override{p.fillRect(localRect(),dark?ui::Color::rgb(0x151631):ui::Color::rgb(0xC8CDF4));}
};
ui::View* find(ui::View* root,const std::string& name){if(root->name()==name)return root;for(const auto& child:root->children())if(auto* result=find(child.get(),name))return result;return nullptr;}
int main(int argc,char**argv){
 std::string temp="/tmp/lingmo-widget-review-XXXXXX";check(::mkdtemp(temp.data()),"temporary home");
 struct Cleanup{std::string path;~Cleanup(){std::filesystem::remove_all(path);}}cleanup{temp};
 ::setenv("HOME",temp.c_str(),1);fm::setSettingsFile(temp+"/widgets.conf");
 base::Settings settings(fm::settingsFile());
 // Deliberately sparse configured slots previously hid later cities.
 settings.setValue("WorldClock/Zone1","America/New_York");settings.setValue("WorldClock/Zone3","Asia/Shanghai");settings.sync();
 const std::string originalTz=std::getenv("TZ")?std::getenv("TZ"):"";
 const auto now=std::time(nullptr);check(fm::worldClockTime("Asia/Shanghai",now).valid,"ICU Shanghai");
 check(!fm::worldClockTime("Not/A_Real_Zone",now).valid,"invalid zone flagged");
 check((std::getenv("TZ")?std::getenv("TZ"):"")==originalTz,"world clock must not mutate TZ");
 settings.remove("WorldClock/Zone1");settings.remove("WorldClock/Zone3");settings.sync();
 if(std::getenv("WIDGET_REVIEW_LOCALE_DIR")){::setenv("LANGUAGE","zh_CN",1);::setenv("LANG","en_US.UTF-8",1);}
 ui::Application app(argc,argv,ui::Application::Platform::Headless);
 if(const char* locale=std::getenv("WIDGET_REVIEW_LOCALE_DIR")){::setenv("LANGUAGE","zh_CN",1);::setlocale(LC_ALL,"C");::setlocale(LC_ALL,"en_US.UTF-8");Lingmo::i18n::setDomain("widget-review");::bindtextdomain("widget-review",locale);::bindtextdomain("lingmo-filemanager",locale);::bind_textdomain_codeset("widget-review","UTF-8");::textdomain("widget-review");check(std::string(Lingmo::i18n::tr("Search widgets"))=="搜索小组件","Chinese translation catalog loads");}
 for(bool dark:{false,true})for(double scale:{1.0,1.5}){
  setReviewTheme(dark);
  ui::Window window;window.resize({1184,800});window.setScale(scale);
  auto background=std::make_unique<Backdrop>();background->dark=dark;
  auto* layer=background->addChild<fm::WidgetLayer>();layer->setGeometry({0,0,1184,800});layer->setWorkArea({16,16,1152,768});
  window.setContentView(std::move(background));window.show();
  layer->setDesktopWindow(&window);
  std::vector<fm::WidgetPlacement> placements;
  for(int i=0;i<12;++i){const int row=i/3,col=i%3;placements.push_back({static_cast<fm::WidgetKind>(i),fm::WidgetSize::Medium,16.0+col*384,16.0+row*192,"review-"+std::to_string(i)});}
  layer->setPlacements(placements);window.grab();
  for(auto* widget:layer->widgets())check(widget->geometry().width>0,"positive widget geometry");
  auto* todo=layer->widgets()[10];auto* input=dynamic_cast<ui::TextField*>(find(todo,"todo-input"));auto* add=dynamic_cast<ui::Button*>(find(todo,"todo-add"));
  check(input&&add&&input->isVisible(),"medium todo accepts tasks");input->setText("Review widget design");add->onClicked();window.grab();
  auto* task=find(todo,"todo-row-0");check(task&&task->geometry().width>100,"task row has usable width");
  auto* world=layer->widgets()[7];int cityCount=0;for(const auto& child:world->children())if(child->name().starts_with("world-city-")&&child->isVisible())++cityCount;
  check(cityCount==4,"medium world clock shows four cities");
  auto* first=layer->widgets()[0];const auto count=layer->widgets().size();const auto origin=first->geometry().topLeft();
  ui::TestInput::press(window,first->mapToWindow({8,8}));ui::TestInput::movePointer(window,{origin.x+10,790});ui::TestInput::release(window,{origin.x+10,790});app.loop().dispatch();
  check(layer->widgets().size()==count,"drag to bottom must not delete card");layer->moveWidget(first,origin,true);window.grab();
  if(argc>1){std::filesystem::create_directories(argv[1]);const auto suffix=std::string(dark?"dark":"light")+(scale==1?"-100":"-150");check(window.grab().savePng(std::string(argv[1])+"/all-widgets-"+suffix+".png"),"capture all");}
 }
 // Appearance policy preserves placement/content, and edit mode restores colour.
 {
  fm::WidgetLayer layer;layer.setGeometry({0,0,400,400});layer.setWorkArea({0,0,400,400});
  layer.setPlacements({{fm::WidgetKind::Calendar,fm::WidgetSize::Small,8,8,"appearance"}});
  auto before=fm::serializeWidgets(layer.placements());
  layer.setAppearance(false,0,false);check(!layer.isVisible()&&fm::serializeWidgets(layer.placements())==before,"hiding retains layout");
  layer.setAppearance(true,0,false);check(layer.monochrome(),"automatic background is monochrome");
  layer.setAppearance(true,0,true);check(!layer.monochrome(),"automatic foreground is full colour");
  layer.setAppearance(true,1,true);check(layer.monochrome(),"always monochrome");
  check(fm::widgetTheme(layer.widgets()[0]).highlight==ui::Theme::current().text.withAlpha(.72),"monochrome palette");
  layer.setEditing(true);check(!layer.monochrome(),"editing restores colour");layer.setEditing(false);
  layer.setAppearance(true,2,false);check(!layer.monochrome(),"full colour ignores focus");
 }
 // Every real widget must render at each supported size.
 {
  ui::Window window;window.resize({400,400});auto root=std::make_unique<ui::View>();auto* layer=root->addChild<fm::WidgetLayer>();
  layer->setGeometry({0,0,400,400});layer->setWorkArea({0,0,400,400});window.setContentView(std::move(root));window.show();
  for(int kind=0;kind<12;++kind)for(auto size:{fm::WidgetSize::Small,fm::WidgetSize::Medium,fm::WidgetSize::Large}){
   layer->setPlacements({{static_cast<fm::WidgetKind>(kind),size,0,0,"size-review-"+std::to_string(kind)}});window.grab();
   check(layer->widgets().size()==1,"size card creation");
  }
 }
 if(argc>1){
  setReviewTheme(true);ui::Window window;window.resize({592,208});auto root=std::make_unique<Backdrop>();root->dark=true;
  auto* layer=root->addChild<fm::WidgetLayer>();layer->setGeometry({0,0,592,208});layer->setWorkArea({16,16,560,176});
  layer->setPlacements({{fm::WidgetKind::Calendar,fm::WidgetSize::Small,16,16,"calendar"},{fm::WidgetKind::WorldClock,fm::WidgetSize::Medium,208,16,"world"}});
  window.setContentView(std::move(root));window.show();check(window.grab().savePng(std::string(argv[1])+"/calendar-world-dark.png"),"reference composition");
 }
 // Catalogue interaction: search, exact sizes, pointer capture and drop boundaries.
 for(bool dark:{false,true}) {
  setReviewTheme(dark);
  ui::Window window;window.resize({1184,800});auto root=std::make_unique<Backdrop>();root->dark=dark;
  auto* layer=root->addChild<fm::WidgetLayer>();layer->setGeometry({0,0,1184,800});layer->setWorkArea({8,8,1168,784});
  auto* gallery=root->addChild<fm::WidgetGallery>();gallery->setGeometry({0,0,1184,800});gallery->setVisible(true);
  int added=0,dropped=0;fm::WidgetSize picked=fm::WidgetSize::Small;
  gallery->onAddSized=[&](fm::WidgetKind kind,fm::WidgetSize size){++added;picked=size;layer->addWidget(kind,size);};
  gallery->onAddAt=[&](fm::WidgetKind kind,fm::WidgetSize size,ui::PointF p){++dropped;layer->addWidgetAt(kind,size,p);};
  gallery->onSidebarGeometry=[&](ui::RectF r){layer->setGalleryBlurRect(r);};
  window.setContentView(std::move(root));window.show();layer->setDesktopWindow(&window);window.grab();
  auto* search=dynamic_cast<ui::TextField*>(find(gallery,"widget-gallery:search"));check(search,"catalogue search field");
  search->setText("unfindable-widget");window.grab();check(!find(gallery,"widget-preview:small")->isVisible(),"empty search hides previews");
  search->setText("WORLD");window.grab();check(!find(gallery,"widget-category:calendar")->isVisible(),"search filters categories");
  search->setText("");gallery->selectKind(fm::WidgetKind::WorldClock);window.grab();
  auto* small=find(gallery,"widget-preview:small");auto* medium=find(gallery,"widget-preview:medium");auto* large=find(gallery,"widget-preview:large");
  check(small&&medium&&large,"three size previews");
  ui::TestInput::click(window,medium->mapToWindow({20,20}),ui::MouseButton::Right);check(added==0,"right click must not add");
  ui::TestInput::click(window,medium->mapToWindow({20,20}));check(added==1&&picked==fm::WidgetSize::Medium,"click selects exact size");
  auto p=small->mapToWindow({20,20});ui::TestInput::press(window,p);ui::TestInput::movePointer(window,{p.x+15,p.y+15});ui::TestInput::release(window,{p.x+15,p.y+15});check(dropped==0&&added==1,"drop back into catalogue cancels");
  ui::TestInput::press(window,p);ui::TestInput::movePointer(window,{24,24});ui::TestInput::release(window,{24,24});check(dropped==1,"drag preview onto desktop");
  const auto placed=layer->placements();check(placed.back().size==fm::WidgetSize::Small&&placed.back().x>=8&&placed.back().y>=8,"drop size and work area clamp");
  int notificationAdds=0;bool target=false;gallery->onTargetChanged=[&](bool value){target=value;};
  gallery->onAddSized=[&](fm::WidgetKind,fm::WidgetSize){if(target)++notificationAdds;};
  auto* targetButton=dynamic_cast<ui::Button*>(find(gallery,"widget-gallery:notification-target"));check(targetButton,"notification target button");targetButton->onClicked();check(target,"notification target selected");
  ui::TestInput::click(window,medium->mapToWindow({20,20}));check(notificationAdds==1,"add routes to notification target");
  ui::TestInput::press(window,p);ui::TestInput::movePointer(window,{24,24});ui::TestInput::release(window,{24,24});check(dropped==1,"notification target disables desktop drag");
  gallery->setNotificationTarget(false);
  gallery->focusSearch();ui::TestInput::setKeyboardFocus(window,true);int closed=0;gallery->onDone=[&]{++closed;};
  ui::TestInput::typeKey(window,ui::key::Escape);check(closed==1,"Escape exits from search field");
  auto* done=dynamic_cast<ui::Button*>(find(gallery,"widget-gallery:done"));check(done,"Done control");ui::TestInput::click(window,done->mapToWindow({20,12}));check(closed==2,"Done exits catalogue");
  if(argc>1){const auto file=std::string(argv[1])+"/widget-gallery-"+(dark?"dark":"light")+".png";check(window.grab().savePng(file),"capture catalogue");}
  for(int kind=0;kind<12;++kind){gallery->selectKind(static_cast<fm::WidgetKind>(kind));window.grab();}
  gallery->selectKind(fm::WidgetKind::Calendar);window.grab();
  if(argc>1)check(window.grab().savePng(std::string(argv[1])+"/widget-gallery-calendar-"+(dark?"dark":"light")+".png"),"calendar catalogue capture");
  ui::TestInput::setGlassAvailable(true);layer->updateBlur();window.grab();check(ui::TestInput::backgroundGlass(window).has_value(),"gallery and cards request glass");
  gallery->setVisible(false);layer->setGalleryBlurRect({});layer->setPlacements({});window.grab();check(!ui::TestInput::backgroundGlass(window).has_value(),"closing empty catalogue clears glass");ui::TestInput::setGlassAvailable(false);
 }
 std::cout<<"Desktop widget review passed\n";
}
