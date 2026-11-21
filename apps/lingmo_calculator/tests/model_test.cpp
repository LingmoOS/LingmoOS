// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculator.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace Calculator;
int checks=0;
void require(bool value,const char* message) { ++checks;if(!value){std::cerr<<message<<'\n';std::exit(1);} }
int main(){
    for(const auto& [expression,expected]:std::vector<std::pair<std::string,long double>>{{"2+3*4",14},{"(2+3)*4",20},{"-2*-3",6},{".5+0.25",.75L},{"200*10%",20},{"100+10%",100.1L},{"1e3+2",1002},{"-(2+3)",-5},{"0.1+0.2",.3L},{"10/4",2.5L},{"2--3",5}}){
        const auto result=evaluate(expression);require(result.error==Error::None,"valid expression rejected");require(std::abs(result.value-expected)<1e-12L,"wrong arithmetic result");
    }
    for(const auto& expression:{"",".","1+","(2+3","2 3","1e","abc","2**3","()"})require(evaluate(expression).error!=Error::None,"invalid expression accepted");
    require(evaluate("1/0").error==Error::DivisionByZero,"division by zero");
    require(evaluate("1e99999").error==Error::OutOfRange,"overflow");
    require(evaluate(std::string(513,'1')).error==Error::TooLong,"input limit");
    require(evaluate(std::string(80,'(')+"1"+std::string(80,')')).error==Error::TooLong,"nesting limit");
    require(format(.1L+.2L)=="0.3","decimal display");
    Model model; model.input("2+3");require(model.calculate(),"calculate");require(model.expression()=="5","result display");
    model.input("*4");require(model.calculate()&&model.expression()=="20","continue result");
    model.input("7");require(model.expression()=="7","fresh input after result");model.backspace();require(model.expression().empty(),"backspace");
    model.input("1/0");require(!model.calculate(),"error state");model.clear();model.input("4");model.negate();require(model.calculate()&&model.expression()=="-4","negate");
    require(!model.paste("2; rm -rf /")&&model.expression()=="-4","invalid paste must preserve input");
    require(model.paste(" (2 + 3) * 4 \n")&&model.calculate()&&model.expression()=="20","paste expression");
    for(int i=0;i<60;++i){model.clear();model.input(std::to_string(i));model.calculate();}
    require(model.history().size()==50,"bounded history");model.clearHistory();require(model.history().empty(),"clear history");
    model.clear(); model.input("12"); model.input("+"); model.input("*"); model.input("3");
    require(model.expression()=="12*3" && model.preview()=="36", "operator correction and preview");
    require(model.history().empty(), "preview must not add history");
    model.input("+"); require(model.preview().empty(), "incomplete preview");
    model.clear(); model.input("."); model.input("5"); model.input(".");
    require(model.expression()=="0.5", "duplicate decimal prevention");
    model.clear(); model.input("1e-3"); model.input(".");
    require(model.expression()=="1e-3", "decimal in exponent rejected");
    require(model.paste("  (12 × 3) ÷ 2 − 1  ") && model.calculate()
            && model.expression()=="17", "Unicode clipboard operators");
    require(!model.paste(" \n\t ") && model.expression()=="17", "blank paste preserves expression");
    model.clear(); model.input("1+1"); model.calculate();
    require(model.recall(1) && model.expression()=="17", "recall earlier result");
    require(model.calculation()=="(12 * 3) / 2 - 1", "recall original expression");
    model.clearHistory(); require(model.calculation()=="(12 * 3) / 2 - 1", "clear history preserves displayed calculation");
    require(!model.recall(0), "invalid recall index");
    model.input("+3"); require(model.calculate() && model.expression()=="20", "continue recalled result");
    std::cout<<checks<<" checks passed\n";
}
