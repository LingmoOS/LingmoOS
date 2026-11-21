// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "globalmenu.h"
#include "base/event_loop.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <poll.h>
using namespace Lingmo::base;
using namespace std::chrono_literals;
void require(bool ok,const char* message){if(!ok){std::cerr<<message<<'\n';std::exit(1);}}
void wait(EventLoop& loop,std::function<bool()> ready){
    auto deadline=std::chrono::steady_clock::now()+3s;
    while(!ready() && std::chrono::steady_clock::now()<deadline){pollfd fd{loop.fd(),POLLIN,0};::poll(&fd,1,10);loop.dispatch();}
    require(ready(),"D-Bus operation timed out");
}
int main(){
    EventLoop loop;
    auto* bus=dbus::Connection::session(&loop);require(bus,"session bus missing");
    auto client=dbus::Connection::open(dbus::Connection::Bus::Session,&loop);require(bool(client),"client bus missing");
    const std::string registrarName="com.canonical.AppMenu.Registrar";
    dbus::Object registrar(client.get(),"/com/canonical/AppMenu/Registrar",registrarName);
    int registered=0,unregistered=0;bool rejectRegistration=false;uint32_t registeredId=0;std::string registeredPath;
    registrar.addMethod("RegisterWindow", "uo", "", [&](dbus::MethodCall& call) {
        uint32_t id = 0; dbus::ObjectPath path; call.read(id, path);
        ++registered; registeredId = id; registeredPath = path.path;
        if (rejectRegistration) call.replyError("com.canonical.AppMenu.Error", "Registration rejected");
        else call.reply();
    });
    registrar.addMethod("UnregisterWindow",[&](uint32_t){++unregistered;});
    require(registrar.publish()&&client->requestName(registrarName),"registrar publish failed");
    int actions=0; bool enabled=true;bool visible=true;bool checked=false;
    {
        Calculator::GlobalMenu menu(bus,[&]{
            Calculator::GlobalMenu::Entry action;action.label="Calculate";action.action=[&]{++actions;};action.enabled=enabled;action.visible=visible;action.shortcut={"Return"};action.checkable=true;action.checked=checked;
            Calculator::GlobalMenu::Entry top;top.label="Calculation";top.children.push_back(action);return std::vector{top};
        },{});
        require(!menu.isAvailable(), "fallback must remain before registration acknowledgement");
        wait(loop,[&]{return registered==1 && menu.isAvailable();});require(registeredId!=0,"application registered as desktop");require(registeredPath==Calculator::GlobalMenu::kPath,"wrong menu path");
        dbus::Proxy proxy(client.get(),bus->uniqueName(),Calculator::GlobalMenu::kPath,"com.canonical.dbusmenu");
        bool done=false;uint32_t revision=0;
        using Layout=std::tuple<int32_t,dbus::VariantMap,std::vector<dbus::Variant>>;
        Layout layout;
        auto call=proxy.callAsync("GetLayout",[&](const dbus::Reply& reply){require(reply.read(revision,layout),"invalid layout reply");done=true;},int32_t(0),int32_t(-1),std::vector<std::string>{});
        wait(loop,[&]{return done;});require(std::get<2>(layout).size()==1,"missing top-level menu");
        const auto top=std::get<2>(layout)[0].value<Layout>();require(std::get<0>(top)==1,"unstable menu id");
        require(std::get<2>(top).size()==1,"missing menu action");
        const auto leaf=std::get<2>(top)[0].value<Layout>();require(std::get<1>(leaf).at("shortcut").value<std::vector<std::vector<std::string>>>()[0][0]=="Return","missing shortcut");
        require(std::get<1>(leaf).at("toggle-type").value<std::string>()=="radio","radio menu type");
        checked=true;menu.update();done=false;
        call=proxy.callAsync("GetProperty",[&](const dbus::Reply& reply){
            require(reply.value<dbus::Variant>()->value<int32_t>()==1,"radio state update");done=true;
        },int32_t(2),std::string("toggle-state"));
        wait(loop,[&]{return done;});
        proxy.send("Event",int32_t(2),std::string("clicked"),dbus::Variant(std::string()),uint32_t(0));wait(loop,[&]{return actions==1;});
        enabled=false;menu.update();done=false;
        call=proxy.callAsync("Event",[&](const dbus::Reply& reply){require(!reply.isError(),"event error");done=true;},int32_t(2),std::string("clicked"),dbus::Variant(std::string()),uint32_t(0));
        wait(loop,[&]{return done;});loop.dispatch();require(actions==1,"disabled action invoked");
        enabled=true;visible=false;menu.update();done=false;
        call=proxy.callAsync("Event",[&](const dbus::Reply&){done=true;},int32_t(2),std::string("clicked"),dbus::Variant(std::string()),uint32_t(0));wait(loop,[&]{return done;});loop.dispatch();require(actions==1,"hidden action invoked");
        client->releaseName(registrarName);
        // Losing the registrar restores the in-window menu.
        wait(loop,[&]{return !menu.isAvailable();});
        require(client->requestName(registrarName),"registrar restart failed");wait(loop,[&]{return registered>=2 && menu.isAvailable();});
    }
    wait(loop,[&]{return unregistered==1;});
    rejectRegistration = true;
    {
        Calculator::GlobalMenu rejected(bus, [] { return std::vector<Calculator::GlobalMenu::Entry>{}; }, {});
        wait(loop, [&] { return registered >= 3; });
        bool completed = false;
        dbus::Proxy probe(client.get(), bus->uniqueName(), Calculator::GlobalMenu::kPath, "com.canonical.dbusmenu");
        auto call = probe.callAsync("AboutToShow", [&](const dbus::Reply&) { completed = true; }, int32_t(0));
        wait(loop, [&] { return completed; });
        require(!rejected.isAvailable(), "failed registration must retain local menu");
    }
    std::cout<<"Global menu registration, layout, actions and reconnection passed\n";
}
