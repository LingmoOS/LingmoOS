// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculatorwindow.h"
#include "base/dbus.h"
#include "base/i18n.h"
#include "ui/application.h"
#include <chrono>
#include <cstdio>
#include <string>
int main(int argc,char** argv) {
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        if(arg=="--help" || arg=="-h") {std::puts("Usage: lingmo-calculator [--help] [--version]");return 0;}
        if(arg=="--version") {std::puts("Lingmo Calculator 1.0.0");return 0;}
        std::fprintf(stderr,"Unknown option: %s\n",argv[i]);return 1;
    }
    Lingmo::ui::Application app(argc,argv);
    app.setApplicationId("lingmo-calculator"); Lingmo::i18n::setDomain("lingmo-calculator");
    auto* bus=app.sessionBus();
    if(bus && !bus->requestName("org.lingmo.Calculator")) {
        Lingmo::base::dbus::Proxy existing(bus,"org.lingmo.Calculator","/Calculator","org.lingmo.Calculator");
        existing.setTimeout(std::chrono::seconds(5));
        return existing.call("activate",app.takeActivationToken()).isError()?1:0;
    }
    Calculator::CalculatorWindow calculator;
    Lingmo::base::dbus::Object service(bus,"/Calculator","org.lingmo.Calculator");
    service.addMethod("activate",[&calculator](const std::string& token){calculator.window().activate(token);});
    if(bus) service.publish();
    calculator.show(); return app.run();
}
