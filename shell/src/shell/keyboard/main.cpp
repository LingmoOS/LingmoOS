// Copyright 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "base/dbus.h"
#include "base/event_loop.h"
#include <chrono>
#include <cstdio>
#include <string>
int main(int argc,char** argv){
 std::string method="toggle";
 if(argc>2){std::fprintf(stderr,"Usage: lingmo-screen-keyboard [--show|--hide|--help]\n");return 1;}
 if(argc==2){const std::string argument=argv[1];
  if(argument=="--help"){std::puts("Usage: lingmo-screen-keyboard [--show|--hide]");return 0;}
  if(argument=="--show")method="show";else if(argument=="--hide")method="hide";else return 1;
 }
 Lingmo::base::EventLoop loop;auto* bus=Lingmo::base::dbus::Connection::session(&loop);if(!bus)return 1;
 Lingmo::base::dbus::Proxy service(bus,"org.lingmo.ScreenKeyboard","/org/lingmo/ScreenKeyboard","org.lingmo.ScreenKeyboard");
 service.setTimeout(std::chrono::seconds(3));const auto reply=service.call(method);
 if(reply.isError()){std::fprintf(stderr,"Screen keyboard unavailable: %s\n",reply.error().message.c_str());return 1;}
 return 0;
}
