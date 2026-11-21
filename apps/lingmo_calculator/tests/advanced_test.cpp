// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "calculator.h"
#include "programmer.h"
#include "graph.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace Calculator;
void check(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
int main()
{
    for (const auto& [expression, expected] : std::vector<std::pair<std::string, long double>>{
        {"2^3^2",512}, {"-2^2",-4}, {"2^-3",0.125}, {"sqrt(81)+log(100)",11},
        {"sin(pi/2)",1}, {"ln(e)",1}, {"5!",120}, {"abs(-7)",7}, {"exp(0)",1}, {"acos(1)",0}}) {
        auto result = evaluate(expression);
        check(result.error == Error::None && std::abs(result.value - expected) < 1e-10L, expression.c_str());
    }
    check(std::abs(evaluate("sin(30)",AngleUnit::Degrees).value - 0.5L) < 1e-12L,"degrees");
    check(std::abs(evaluate("asin(0.5)",AngleUnit::Degrees).value - 30) < 1e-10L,"inverse degrees");
    for (const auto& expression : {"sqrt(-1)","ln(0)","acos(2)","(-2)^0.5","(-1)!"})
        check(evaluate(expression).error == Error::Domain,"domain error");
    check(evaluate("tan(90)",AngleUnit::Degrees).error == Error::Domain,"tangent pole");
    check(evaluate("x").error == Error::InvalidExpression,"x needs graph context");
    check(evaluate("x^2",AngleUnit::Radians,3).value == 9,"graph variable");
    check(evaluate("171!").error == Error::OutOfRange,"factorial bounded");
    check(evaluate("0^-1").error == Error::DivisionByZero,"negative power of zero");
    Model model;model.setAngleUnit(AngleUnit::Degrees);model.input("sin(30)");model.calculate();
    model.setAngleUnit(AngleUnit::Radians);model.recall(0);
    check(model.angleUnit()==AngleUnit::Degrees,"history angle restoration");
    model.addIntegerHistory("0xFFFFFFFFFFFFFFFF","FFFFFFFFFFFFFFFF",16);
    check(model.history()[0].integer && model.history()[0].base==16,"integer history metadata");
    check(!model.recall(0),"integer recall must not pass through floating point");
    const auto max = std::numeric_limits<uint64_t>::max();
    for (const auto& [expression, expected] : std::vector<std::pair<std::string,uint64_t>>{
        {"18446744073709551615",max},{"18446744073709551615+1",0},{"9007199254740993",9007199254740993ULL},
        {"0xFF & 0x0F",15},{"1<<63",uint64_t(1)<<63},{"~0",max},{"-1",max},{"8>>2",2},
        {"1|2^3&4",3},{"5%2",1},{"0b1010+0o7",17},{"(2+3)*4",20}}) {
        auto result = evaluateInteger(expression);
        check(result.error == Error::None && result.value == expected,expression.c_str());
    }
    check(evaluateInteger("FF+1",16).value==256,"hex base");
    check(evaluateInteger("8",8).error==Error::InvalidExpression,"invalid octal digit");
    check(evaluateInteger("1<<64").error==Error::Domain,"invalid shift");
    check(evaluateInteger("1/0").error==Error::DivisionByZero,"integer divide zero");
    check(evaluateInteger("18446744073709551616").error==Error::OutOfRange,"literal overflow");
    check(evaluateInteger(std::string(80,'~')+"0").error==Error::TooLong,"integer nesting limit");
    check(formatInteger(max,16)=="FFFFFFFFFFFFFFFF","hex precision");
    auto curve = sampleGraph("sin(x)",0,0,10,5,AngleUnit::Radians);
    check(curve.error==Error::None && curve.segments.size()>700,"sine curve");
    auto pole = sampleGraph("1/x",0.013,0,10,5,AngleUnit::Radians);
    for(const auto& segment:pole.segments) check(!(segment.from.x<0 && segment.to.x>0),"pole must not be bridged");
    check(sampleGraph("sin(",0,0,10,5,AngleUnit::Radians).error==Error::InvalidExpression,"invalid graph");
    check(sampleGraph("sqrt(-1)",0,0,10,5,AngleUnit::Radians).segments.empty(),"empty domain graph");
    std::cout << "Scientific, integer precision and graph tests passed\n";
}
