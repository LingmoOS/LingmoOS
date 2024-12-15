// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include <memory>

using namespace std;

namespace DCalendar {

template <class T>
class DSingleton
{
public:
    static inline T *instance()
    {
        static T *_instance = new T;
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

} // namespace DCalendar
