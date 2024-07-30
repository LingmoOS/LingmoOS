/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef __TESTHELPERS_HPP__
#define __TESTHELPERS_HPP__

#include <../kwalletfreedesktopservice.h>
#include <QTest>
#include <string>
#include <tuple>
#include <vector>

template<typename InputT, typename OutputT>
using Testset = std::vector<std::pair<InputT, OutputT>>;

template<typename T, typename Enable = void>
struct EasyFormater {
    std::string operator()(const T &v) const
    {
        return std::string(v.toStdString());
    }
};

template<typename T>
struct EasyFormater<T, typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type> {
    std::string operator()(T v) const
    {
        return std::to_string(v);
    }
};

template<>
struct EasyFormater<FdoUniqueLabel> {
    std::string operator()(const FdoUniqueLabel &v) const
    {
        return "{" + v.label.toStdString() + ", " + std::to_string(v.copyId) + "}";
    }
};

template<>
struct EasyFormater<EntryLocation> {
    std::string operator()(const EntryLocation &v) const
    {
        return "{" + v.folder.toStdString() + ", " + v.key.toStdString() + "}";
    }
};

template<bool Reverse>
struct TestsetCmpHelper {
    template<typename F, typename T>
    bool cmp(F &&function, const T &pair)
    {
        return function(pair.first) == pair.second;
    }
    template<typename F, typename T>
    std::string format(F &&function, const T &pair)
    {
        using RetT = decltype(function(pair.first));
        return EasyFormater<typename T::first_type>()(pair.first) + " (evaluates to " + EasyFormater<RetT>()(function(pair.first)) + ") not equal with "
            + EasyFormater<typename T::second_type>()(pair.second);
    }
};
template<>
struct TestsetCmpHelper<true> {
    template<typename F, typename T>
    bool cmp(F &&function, const T &pair)
    {
        return function(pair.second) == pair.first;
    }
    template<typename F, typename T>
    std::string format(F &&function, const T &pair)
    {
        using RetT = decltype(function(pair.second));
        return EasyFormater<typename T::second_type>()(pair.second) + " (evaluates to " + EasyFormater<RetT>()(function(pair.second)) + ") not equal with "
            + EasyFormater<typename T::first_type>()(pair.first);
    }
};

template<bool Reverse, typename F, typename I, typename O>
void runTestsetTmpl(F &&function, const Testset<I, O> &labelMap)
{
    for (auto &pair : labelMap) {
        bool ok = TestsetCmpHelper<Reverse>().cmp(function, pair);
        if (!ok) {
            std::string str = TestsetCmpHelper<Reverse>().format(function, pair);
            QVERIFY2(ok, str.c_str());
        }
    }
}

template<typename F, typename I, typename O>
void runTestset(F &&function, const Testset<I, O> &labelMap)
{
    return runTestsetTmpl<false>(std::forward<F>(function), labelMap);
}

template<typename F, typename I, typename O>
void runRevTestset(F &&function, const Testset<I, O> &labelMap)
{
    return runTestsetTmpl<true>(std::forward<F>(function), labelMap);
}

#endif // __TESTHELPERS_HPP__
