/*
    SPDX-FileCopyrightText: 2011 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef MM_MACROS_H
#define MM_MACROS_H

#define MM_GLOBAL_STATIC_STRUCT_NAME(NAME)
typedef void (*MmCleanUpFunction)();
class MmCleanUpGlobalStatic
{
public:
    MmCleanUpFunction func;

    inline ~MmCleanUpGlobalStatic()
    {
        func();
    }
};

#define MM_GLOBAL_STATIC(TYPE, NAME) MM_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ())

/* clang-format off */
#define MM_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                            \
static QBasicAtomicPointer<TYPE > _mm_static_##NAME = Q_BASIC_ATOMIC_INITIALIZER(0); \
static bool _mm_static_##NAME##_destroyed;                                      \
static struct MM_GLOBAL_STATIC_STRUCT_NAME(NAME)                                \
{                                                                              \
    inline bool isDestroyed() const                                            \
    {                                                                          \
        return _mm_static_##NAME##_destroyed;                                   \
    }                                                                          \
    inline bool exists() const                                                 \
    {                                                                          \
        return _mm_static_##NAME != 0;                                          \
    }                                                                          \
    inline operator TYPE*()                                                    \
    {                                                                          \
        return operator->();                                                   \
    }                                                                          \
    inline TYPE *operator->()                                                  \
    {                                                                          \
        if (!_mm_static_##NAME) {                                               \
            if (isDestroyed()) {                                               \
                qFatal("Fatal Error: Accessed global static '%s *%s()' after destruction. " \
                       "Defined at %s:%d", #TYPE, #NAME, __FILE__, __LINE__);  \
            }                                                                  \
            TYPE *x = new TYPE ARGS;                                           \
            if (!_mm_static_##NAME.testAndSetOrdered(0, x)                      \
                && _mm_static_##NAME != x ) {                                   \
                delete x;                                                      \
            } else {                                                           \
                static MmCleanUpGlobalStatic cleanUpObject = { destroy };       \
            }                                                                  \
        }                                                                      \
        return _mm_static_##NAME;                                               \
    }                                                                          \
    inline TYPE &operator*()                                                   \
    {                                                                          \
        return *operator->();                                                  \
    }                                                                          \
    static void destroy()                                                      \
    {                                                                          \
        _mm_static_##NAME##_destroyed = true;                                   \
        TYPE *x = _mm_static_##NAME;                                            \
        _mm_static_##NAME = 0;                                                  \
        delete x;                                                              \
    }                                                                          \
} NAME;
/* clang-format on */

#endif
