// SPDX-License-Identifier: GPL-3.0-or-later
#include "shell/keyboard/keyboard_model.h"
#include "shell/keyboard/keyboard_view.h"
#include "input/xkb_tracker.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/label.h"
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <cstdlib>
#include <iostream>
using namespace Lingmo;
void check(bool value,const char* message){if(!value){std::cerr<<message<<'\n';std::exit(1);}}
ui::View* find(ui::View* root,const std::string& name){if(root->name()==name)return root;for(const auto& child:root->children())if(auto* v=find(child.get(),name))return v;return nullptr;}
void click(ui::Window& window,const std::string& id){window.grab();auto* key=find(window.contentView(),"keyboard:"+id);check(key,"key exists");auto r=key->geometry();ui::TestInput::click(window,{r.x+r.width/2,r.y+r.height/2});}
int main(int argc,char** argv){
    shell::ScreenKeyboardModel model;
    check(model.rows().size()==5,"five rows");check(model.toggle("shift"),"shift toggle");
    check(model.rows()[1][1].label=="Q","uppercase label");model.consumed();check(!model.selected("shift"),"one-shot shift");
    model.toggle("caps");model.consumed();check(model.rows()[1][1].label=="Q","caps persists");
    model.toggle("shift");check(model.rows()[1][1].label=="q","caps shift inversion");model.reset();
    model.toggle("ctrl");model.toggle("alt");model.consumed();check(!model.control()&&!model.alt(),"modifiers consumed");
    model.toggle("symbols");for(const auto& row:model.rows())for(const auto& key:row)check(!key.label.empty(),"symbol grid labels");
    const auto chord=shell::screenKeyboardChord(KEY_A,{KEY_LEFTSHIFT},true,false,{KEY_LEFTCTRL});
    check(chord==std::vector<uint32_t>({KEY_LEFTSHIFT,KEY_A}),"physical ctrl not released");
    check(shell::screenKeyboardChord(KEY_A,{},false,false,{KEY_A}).empty(),"held key not repeated/released");
    input::XkbTracker xkb;auto stroke=xkb.keyStroke(XKB_KEY_A);check(stroke && stroke->key==KEY_A,"uppercase maps to evdev");
    for(auto code:stroke->modifiers)xkb.updateKey(code,true);check(xkb.keysym(stroke->key)==XKB_KEY_A,"mapped modifiers type label");
    for(auto code:stroke->modifiers)xkb.updateKey(code,false);
    check(xkb.updateKeyboardLayout("de"),"German layout");stroke=xkb.keyStroke(XKB_KEY_y);check(stroke && stroke->key==KEY_Z,"layout-aware key translation");
    ui::Application app(argc,argv,ui::Application::Platform::Headless);ui::Window window;window.resize({960,330});
    auto view=std::make_unique<shell::ScreenKeyboardView>();auto* keyboard=view.get();
    uint32_t symbol=0;bool ctrl=false;keyboard->onKey=[&](uint32_t key,bool c,bool){symbol=key;ctrl=c;return true;};
    window.setContentView(std::move(view));window.show();window.grab();
    click(window,"shift");click(window,"letter-a");check(symbol==XKB_KEY_A,"shift click types capital");
    click(window,"letter-a");check(symbol==XKB_KEY_a,"next click lower");
    click(window,"ctrl");
    auto* modifiers=dynamic_cast<ui::Label*>(find(window.contentView(),"keyboard-modifiers"));
    check(modifiers && modifiers->text()=="Ctrl","modifier state visible");
    click(window,"letter-c");check(modifiers->text().empty(),"modifier hint clears after key");check(ctrl&&symbol==XKB_KEY_c,"control chord");
    click(window,"letter-v");check(!ctrl,"control not stuck");
    click(window,"backspace");check(symbol==XKB_KEY_BackSpace,"backspace");
    check(window.focusView()==nullptr,"buttons do not request SDK keyboard focus");
    keyboard->reset();window.resize({720,280});window.grab();
    for(const auto& row:model.rows())for(const auto& key:row){auto* view=find(window.contentView(),"keyboard:"+key.id);check(view,"button exists");check(view->geometry().width>0&&view->geometry().right()<=721,"key fits narrow width");}
    for(bool dark:{false,true}){
        ui::Theme::setCurrent(dark?ui::Theme::dark():ui::Theme::light());
        window.setBackgroundColor(ui::Color{0,0,0,0});keyboard->setGlassEnabled(true);window.requestLayout();auto image=window.grab();
        check(image.pixel(4,35).a<255 && image.pixel(4,100).a==255,"transparent header solid key bed");
        keyboard->setGlassEnabled(false);image=window.grab();check(image.pixel(4,35).a==255,"opaque header fallback");
    }
    std::cout<<"Screen keyboard tests passed\n";
}
