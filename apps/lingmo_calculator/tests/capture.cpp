// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculatorwindow.h"
#include "ui/application.h"
#include "ui/theme.h"
#include <filesystem>
int main(int argc,char**argv)
{
    if(argc!=2)return 1;
    std::filesystem::create_directories(argv[1]);
    Lingmo::ui::Application app(argc,argv,Lingmo::ui::Application::Platform::Headless);
    for(bool dark:{false,true})for(double scale:{1.0,1.5}) {
        Lingmo::ui::Theme::setCurrent(dark?Lingmo::ui::Theme::dark():Lingmo::ui::Theme::light());
        Calculator::CalculatorWindow calculator;calculator.window().setScale(scale);calculator.show();
        const std::string suffix=std::string(dark?"dark":"light")+(scale==1.0?"-100":"-150");
        auto save=[&](const std::string& name){return calculator.window().grab().savePng(std::string(argv[1])+"/"+name+"-"+suffix+".png");};
        calculator.input("(128+64)*2");calculator.calculate();if(!save("calculator"))return 2;
        calculator.input("+16");calculator.calculate();calculator.toggleHistory();if(!save("history"))return 2;
        calculator.setMode(Calculator::Mode::Scientific);calculator.clear();calculator.input("sqrt(81)+log(100)");calculator.calculate();if(!save("scientific"))return 2;
        calculator.setMode(Calculator::Mode::Graph);if(!save("graph"))return 2;
        calculator.setMode(Calculator::Mode::Programmer);if(!save("programmer"))return 2;
    }
}
