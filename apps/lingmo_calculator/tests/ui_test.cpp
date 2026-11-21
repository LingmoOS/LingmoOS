// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculatorwindow.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/label.h"
#include "ui/keys.h"
#include "ui/theme.h"
#include "ui/popup_glass.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace Lingmo::ui;
using Calculator::Mode;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
View* find(View* root,const std::string& name)
{
    if(root->name()==name)return root;
    for(const auto& child:root->children())if(auto* result=find(child.get(),name))return result;
    return nullptr;
}
void click(Window& window,const std::string& name)
{
    window.grab();
    auto* view=find(window.contentView(),name);check(view,"missing view");
    PointF point{view->geometry().width/2,view->geometry().height/2};
    for(auto* node=view;node;node=node->parent())point+=node->geometry().topLeft();
    TestInput::click(window,point);
}
int main(int argc,char** argv)
{
    Application app(argc,argv,Application::Platform::Headless);
    Calculator::CalculatorWindow calculator;calculator.show();auto& window=calculator.window();window.grab();
    check(find(window.contentView(),"local-menu")->isVisible(),"local fallback without global registrar");
    check(!find(window.contentView(),"mode-menu"),"no space-consuming mode row");
    auto* title=find(window.contentView(),"window-title");check(title,"title exists");
    check(std::abs(title->geometry().x+title->geometry().width/2-window.size().width/2)<0.01,"title centered on entire window");
    TestInput::setKeyboardFocus(window,true);
    TestInput::typeText(window,"2+3*4");TestInput::typeKey(window,key::Return);
    check(calculator.model().expression()=="14","standard keyboard");
    calculator.clear();click(window,"key-7");TestInput::typeKey(window,key::Return);
    check(calculator.model().expression()=="7" && calculator.model().completed(),"enter after button focus");
    TestInput::typeText(window,"+8");TestInput::typeKey(window,key::KeypadEnter);
    check(calculator.model().expression()=="15","continued result");
    window.resize({480,650});window.grab();const auto size=window.size();
    calculator.toggleHistory();window.grab();
    check(window.size().width==size.width && window.size().height==size.height,"history must not resize window");
    check(!find(window.contentView(),"key-7")->isVisible(),"history hides keypad");
    check(find(window.contentView(),"history-list")->isVisible(),"history page visible");
    check(find(window.contentView(),"history-back")->isVisible(),"history has dedicated back action");
    click(window,"history-0");window.grab();
    check(!calculator.historyVisible() && calculator.model().expression()=="15","history recall returns to calculator");
    check(find(window.contentView(),"key-7")->isVisible(),"keypad restored");
    calculator.setMode(Mode::Scientific);calculator.clear();calculator.input("sqrt(81)+log(100)");calculator.calculate();
    check(calculator.model().expression()=="11","scientific mode");
    calculator.toggleAngleUnit();calculator.clear();calculator.input("sin(30)");calculator.calculate();
    check(calculator.model().expression()=="0.5","degree mode");
    calculator.setMode(Mode::Graph);window.grab();
    check(find(window.contentView(),"graph-page")->isVisible(),"graph visible");
    auto* field=dynamic_cast<TextField*>(find(window.contentView(),"graph-expression"));check(field,"graph field");
    field->setText("x^2");click(window,"graph-plot");
    auto* range=dynamic_cast<Label*>(find(window.contentView(),"graph-range"));const auto original=range->text();
    click(window,"graph-zoom-in");check(range->text()!=original,"graph zoom");
    calculator.setMode(Mode::Programmer);window.grab();
    auto* integer=dynamic_cast<TextField*>(find(window.contentView(),"programmer-expression"));check(integer,"integer field");
    integer->setText("18446744073709551615");click(window,"integer-equals");click(window,"base-16");
    check(integer->text()=="FFFFFFFFFFFFFFFF","base conversion exact");
    click(window,"base-10");check(integer->text()=="18446744073709551615","decimal roundtrip exact");
    calculator.toggleHistory();window.grab();
    click(window,"history-0");window.grab();
    check(calculator.mode()==Mode::Programmer && integer->text()=="18446744073709551615","programmer history exact recall");
    click(window,"base-2");window.grab();
    check(!find(window.contentView(),"integer-7")->isEnabled(),"binary digit restriction");
    calculator.setMode(Mode::Standard);window.grab();
    check(window.size().width==480 && window.size().height==650,"restore mode size");
    calculator.setMode(Mode::Programmer);click(window,"base-10");
    calculator.toggleHistory();window.grab();calculator.toggleHistory();window.grab();
    check(calculator.mode()==Mode::Programmer && integer->text()=="18446744073709551615","history preserves programmer page state");
    // A headless compositor can expose the capability without rendering a backdrop.
    // Verify requested material and alpha separation, not a simulated glass screenshot.
    calculator.setMode(Mode::Standard);
    for (bool dark : {false, true}) {
        Theme::setCurrent(dark ? Theme::dark() : Theme::light());
        TestInput::setGlassAvailable(true);
        window.requestLayout();
        auto image = window.grab();
        const auto material = TestInput::backgroundGlass(window);
        check(material.has_value(), "SDK glass material requested");
        check(material->tintRed == popupGlass(dark).tintRed, "glass follows theme");
        check(image.pixel(8, 100).a < 255, "display region exposes glass");
        check(image.pixel(8, 400).a == 255, "keypad remains opaque");
        calculator.setMode(Mode::Scientific); image = window.grab();
        check(image.pixel(8, 100).a < 255 && image.pixel(8, 300).a == 255,
              "scientific function keypad remains opaque");
        calculator.setMode(Mode::Programmer); image = window.grab();
        check(image.pixel(8, 300).a < 255 && image.pixel(8, 450).a == 255,
              "programmer conversions above opaque keypad");
        calculator.toggleHistory(); image = window.grab();
        check(image.pixel(8, 60).a < 255 && image.pixel(8, 150).a == 255,
              "history header glass and solid list body");
        calculator.setMode(Mode::Standard);
        TestInput::setGlassAvailable(false); window.requestLayout(); image = window.grab();
        check(!TestInput::backgroundGlass(window), "glass cleared when unavailable");
        check(image.pixel(8, 100).a == 255, "opaque fallback on unsupported compositors");
    }
    std::cout<<"Title, pages, modes, glass and SDK interactions passed\n";
}
