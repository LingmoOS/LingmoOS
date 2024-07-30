/*
    SPDX-FileCopyrightText: 2011 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_MACROS_H
#define NETWORKMANAGERQT_MACROS_H

#define NM_GLOBAL_STATIC_STRUCT_NAME(NAME)
typedef void (*NmCleanUpFunction)();
class NmCleanUpGlobalStatic
{
public:
    NmCleanUpFunction func;

    inline ~NmCleanUpGlobalStatic()
    {
        func();
    }
};

#define NM_GLOBAL_STATIC(TYPE, NAME) NM_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ())

/* clang-format off */
#define NM_GLOBAL_STATIC_WITH_ARGS(TYPE, NAME, ARGS)                            \
    static QBasicAtomicPointer<TYPE > _nm_static_##NAME = Q_BASIC_ATOMIC_INITIALIZER(0); \
    static bool _nm_static_##NAME##_destroyed;                                      \
    static struct NM_GLOBAL_STATIC_STRUCT_NAME(NAME)                                \
    {                                                                              \
        inline bool isDestroyed() const                                            \
        {                                                                          \
            return _nm_static_##NAME##_destroyed;                                   \
        }                                                                          \
        inline bool exists() const                                                 \
        {                                                                          \
            return _nm_static_##NAME != 0;                                          \
        }                                                                          \
        inline operator TYPE*()                                                    \
        {                                                                          \
            return operator->();                                                   \
        }                                                                          \
        inline TYPE *operator->()                                                  \
        {                                                                          \
            if (!_nm_static_##NAME) {                                               \
                if (isDestroyed()) {                                               \
                    qFatal("Fatal Error: Accessed global static '%s *%s()' after destruction. " \
                           "Defined at %s:%d", #TYPE, #NAME, __FILE__, __LINE__);  \
                }                                                                  \
                TYPE *x = new TYPE ARGS;                                           \
                if (!_nm_static_##NAME.testAndSetOrdered(0, x)                      \
                        && _nm_static_##NAME != x ) {                                   \
                    delete x;                                                      \
                } else {                                                           \
                    static NmCleanUpGlobalStatic cleanUpObject = { destroy };       \
                }                                                                  \
            }                                                                      \
            return _nm_static_##NAME;                                               \
        }                                                                          \
        inline TYPE &operator*()                                                   \
        {                                                                          \
            return *operator->();                                                  \
        }                                                                          \
        static void destroy()                                                      \
        {                                                                          \
            _nm_static_##NAME##_destroyed = true;                                   \
            TYPE *x = _nm_static_##NAME;                                            \
            _nm_static_##NAME = 0;                                                  \
            delete x;                                                              \
        }                                                                          \
    } NAME;
/* clang-format on */

#endif
