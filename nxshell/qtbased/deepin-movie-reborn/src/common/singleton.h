// Copyright (C) 2016 ~ 2018 Wuhan Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <memory>

using namespace std;

namespace DMovie
{

template <class T>
class DSingleton
{
public:
    static inline T *instance()
    {
        static T*  _instance = new T;
        return _instance;
    }

protected:
    DSingleton(void) {}
    ~DSingleton(void) {}
    DSingleton(const DSingleton &) {}
    DSingleton &operator= (const DSingleton &)
    {
        return *this;
    }
};

}
