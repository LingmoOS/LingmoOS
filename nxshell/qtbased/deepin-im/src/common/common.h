// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMON_COMMON_H_
#define COMMON_COMMON_H_

#include <stdint.h>
#include <time.h>

constexpr int XKB_HISTORICAL_OFFSET = 8;

enum class Modifiers : uint8_t {
    Shift,
    Lock,
    Control,
    Mod1,
    Mod2,
    Mod3,
    Mod4,
    Mod5,
    Alt,
    Meta,
    Super,
    Hyper,

    CNT,
};

enum ModifiersMask {
    SHIFT_MASK = 1 << 0,
    LOCK_MASK = 1 << 1,
    CONTROL_MASK = 1 << 2,
    MOD1_MASK = 1 << 3,
    MOD2_MASK = 1 << 4,
    MOD3_MASK = 1 << 5,
    MOD4_MASK = 1 << 6,
    MOD5_MASK = 1 << 7,
    ALT_MASK = 1 << 8,
    META_MASK = 1 << 9,
    SUPER_MASK = 1 << 10,
    HYPER_MASK = 1 << 11,
};

template<typename T>
inline constexpr bool always_false_v = false;

template<auto Func>
class Deleter
{
public:
    template<typename T>
    void operator()(T *ptr) const
    {
        if (ptr) {
            Func(ptr);
        }
    }
};

template<auto F>
struct CallbackWrapper;

template<typename C, typename R, typename... Args, R (C::*F)(Args...)>
struct CallbackWrapper<F>
{
    static R func(void *userdata, Args... args)
    {
        auto *p = static_cast<C *>(userdata);
        return (p->*F)(args...);
    }
};

static int32_t getTimestamp()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return time.tv_sec * 1000 + time.tv_nsec / (1000 * 1000);
}

#endif // COMMON_COMMON_H_