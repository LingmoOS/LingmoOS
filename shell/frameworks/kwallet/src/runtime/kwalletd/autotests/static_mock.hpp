/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef __STATIC_MOCK__H__
#define __STATIC_MOCK__H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>

/*
 * Details
 */

#define COMPTIME_HASH(x) details::fnv1a64_hash<sizeof(x) - 2>(x)

namespace details
{
template<size_t I>
constexpr uint64_t fnv1a64_hash(const char *str)
{
    return (fnv1a64_hash<I - 1>(str) ^ static_cast<uint64_t>(str[I])) * 0x100000001b3;
}

template<>
constexpr uint64_t fnv1a64_hash<size_t(-1)>(const char *)
{
    return 0xcbf29ce484222325;
}

template<bool Enable, size_t N, typename... ArgsT>
struct ElementType_ {
    using type = typename std::tuple_element<N, std::tuple<ArgsT...>>::type;
};

template<size_t N, typename... ArgsT>
struct ElementType_<false, N, ArgsT...> {
    using type = std::nullptr_t;
};

template<size_t N, typename... ArgsT>
struct ElementType : details::ElementType_<(N < sizeof...(ArgsT)), N, ArgsT...> {
};

template<typename ReturnT, typename T, typename... ArgsT>
struct FunctionTraitsImpl {
    using ReturnType = ReturnT;

    template<size_t ArgNum>
    using ArgT = typename ElementType<ArgNum, ArgsT...>::type;

    static constexpr size_t arity = sizeof...(ArgsT);
};

template<typename T>
struct FunctionTraits {
};

template<typename ReturnT, typename T, typename... ArgsT>
struct FunctionTraits<ReturnT (T::*)(ArgsT...)> : FunctionTraitsImpl<ReturnT, T, ArgsT...> {
};

template<typename ReturnT, typename T, typename... ArgsT>
struct FunctionTraits<ReturnT (T::*)(ArgsT...) const> : FunctionTraitsImpl<ReturnT, T, ArgsT...> {
};

template<typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (*)(ArgsT...)> : FunctionTraitsImpl<ReturnT, void, ArgsT...> {
};

template<typename, uint64_t, typename T>
static T &returnStaticStorage()
{
    static T value;
    return value;
}

template<typename MemberF, uint64_t NameHash, typename T>
struct ReturnStaticStorageHelper {
    using TT = typename std::decay<T>::type;

    static T get()
    {
        return returnStaticStorage<MemberF, NameHash, TT>();
    }
    template<typename V>
    static void setValue(V &&v)
    {
        returnStaticStorage<MemberF, NameHash, TT>() = std::forward<V>(v);
    }
};

template<typename MemberF, uint64_t NameHash>
struct ReturnStaticStorageHelper<MemberF, NameHash, void> {
    static void get()
    {
    }
    template<typename V>
    static void setValue(V)
    {
    }
};

template<typename T>
struct DefaultCtorOrNullForVoid {
    using TT = typename std::decay<T>::type;
    static TT get()
    {
        return TT();
    }
};

template<>
struct DefaultCtorOrNullForVoid<void> {
    static std::nullptr_t get()
    {
        return nullptr;
    }
};

template<typename MemberF, uint64_t NameHash, typename... ArgsT>
struct FuncImplHelper {
    using RetT = typename FunctionTraits<MemberF>::ReturnType;

    static typename FunctionTraits<MemberF>::ReturnType call(ArgsT... args)
    {
        if (func())
            return func()(args...);
        else
            return ReturnStaticStorageHelper<MemberF, NameHash, typename FunctionTraits<MemberF>::ReturnType>::get();
    }

    static std::function<RetT(ArgsT...)> &func()
    {
        static std::function<RetT(ArgsT...)> holder;
        return holder;
    }
};

template<typename MemberF, uint64_t NameHash, size_t I = 0, typename... ArgsT>
struct FuncImpl : std::conditional<(I < FunctionTraits<MemberF>::arity),
                                   FuncImpl<MemberF, NameHash, I + 1, ArgsT..., typename FunctionTraits<MemberF>::template ArgT<I>>,
                                   FuncImplHelper<MemberF, NameHash, ArgsT...>>::type {
};

} // namespace details

#define MM_MEMBER_TRAITS(NAME) details::FunctionTraits<decltype(&NAME)>

#define MM_MEMBER_ARG_0(NAME)
#define MM_MEMBER_ARG_1(NAME) typename details::FunctionTraits<NAME>::ArgT<0> _0
#define MM_MEMBER_ARG_2(NAME) MM_MEMBER_ARG_1(NAME), typename details::FunctionTraits<NAME>::ArgT<1> _1
#define MM_MEMBER_ARG_3(NAME) MM_MEMBER_ARG_2(NAME), typename details::FunctionTraits<NAME>::ArgT<2> _2
#define MM_MEMBER_ARG_4(NAME) MM_MEMBER_ARG_3(NAME), typename details::FunctionTraits<NAME>::ArgT<3> _3
#define MM_MEMBER_ARG_5(NAME) MM_MEMBER_ARG_4(NAME), typename details::FunctionTraits<NAME>::ArgT<4> _4
#define MM_MEMBER_ARG_6(NAME) MM_MEMBER_ARG_5(NAME), typename details::FunctionTraits<NAME>::ArgT<5> _5
#define MM_MEMBER_ARG_7(NAME) MM_MEMBER_ARG_6(NAME), typename details::FunctionTraits<NAME>::ArgT<6> _6

#define MM_MEMBER_ARGNAME_0(NAME)
#define MM_MEMBER_ARGNAME_1(NAME) _0
#define MM_MEMBER_ARGNAME_2(NAME) MM_MEMBER_ARGNAME_1(NAME), _1
#define MM_MEMBER_ARGNAME_3(NAME) MM_MEMBER_ARGNAME_2(NAME), _2
#define MM_MEMBER_ARGNAME_4(NAME) MM_MEMBER_ARGNAME_3(NAME), _3
#define MM_MEMBER_ARGNAME_5(NAME) MM_MEMBER_ARGNAME_4(NAME), _4
#define MM_MEMBER_ARGNAME_6(NAME) MM_MEMBER_ARGNAME_5(NAME), _5
#define MM_MEMBER_ARGNAME_7(NAME) MM_MEMBER_ARGNAME_6(NAME), _6

#define MM_LINE_NAME(prefix) MM_JOIN_NAME(prefix, __LINE__)
#define MM_JOIN_NAME(NAME, LINE) MM_JOIN_NAME_1(NAME, LINE)
#define MM_JOIN_NAME_1(NAME, LINE) NAME##LINE

#define MOCK_FUNCTION_RES_RAW_TYPE(CLASS, NAME, TYPE, ARGS_COUNT, INIT_VALUE, ...)                                                                             \
    details::FunctionTraits<TYPE>::ReturnType CLASS::NAME(MM_MEMBER_ARG_##ARGS_COUNT(TYPE)) __VA_ARGS__                                                        \
    {                                                                                                                                                          \
        return details::FuncImpl<TYPE, COMPTIME_HASH(#CLASS "::" #NAME)>::call(MM_MEMBER_ARGNAME_##ARGS_COUNT(TYPE));                                          \
    }                                                                                                                                                          \
    static int MM_LINE_NAME(_init_res_##CLASS##NAME##ARGS_COUNT) = []() {                                                                                      \
        details::ReturnStaticStorageHelper<TYPE, COMPTIME_HASH(#CLASS "::" #NAME), details::FunctionTraits<TYPE>::ReturnType>::setValue(INIT_VALUE);           \
        return 0;                                                                                                                                              \
    }()

/*
 * Interface
 */

/*
 * Defines implementation for the function with specified return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * INIT_VALUE - the new result of the function
 * ...        - the optional const qualifier (must be empty if member function is non-const)
 */
#define MOCK_FUNCTION_RES(CLASS, NAME, ARGS_COUNT, INIT_VALUE, ...)                                                                                            \
    MOCK_FUNCTION_RES_RAW_TYPE(CLASS, NAME, decltype(&CLASS::NAME), ARGS_COUNT, INIT_VALUE, __VA_ARGS__)

/*
 * Defines implementation for the function with default-constructed return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * ...        - the optional const qualifier (must be empty if member function is non-const)
 */
#define MOCK_FUNCTION(CLASS, NAME, ARGS_COUNT, ...)                                                                                                            \
    MOCK_FUNCTION_RES(CLASS,                                                                                                                                   \
                      NAME,                                                                                                                                    \
                      ARGS_COUNT,                                                                                                                              \
                      details::DefaultCtorOrNullForVoid<details::FunctionTraits<decltype(&CLASS::NAME)>::ReturnType>::get(),                                   \
                      __VA_ARGS__)

/*
 * Defines implementation for the overloaded function with specified return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * INIT_VALUE - the new result of the function
 * ...        - the signature of the overloaded function
 */
#define MOCK_FUNCTION_OVERLOADED_RES(CLASS, NAME, ARGS_COUNT, INIT_VALUE, ...)                                                                                 \
    MOCK_FUNCTION_RES_RAW_TYPE(CLASS, NAME, decltype((__VA_ARGS__) & CLASS::NAME), ARGS_COUNT, INIT_VALUE, )

/*
 * Defines implementation for the overloaded const member function with specified return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * INIT_VALUE - the new result of the function
 * ...        - the signature of the overloaded function
 */
#define MOCK_FUNCTION_OVERLOADED_RES_CONST(CLASS, NAME, ARGS_COUNT, INIT_VALUE, ...)                                                                           \
    MOCK_FUNCTION_RES_RAW_TYPE(CLASS, NAME, decltype((__VA_ARGS__) & CLASS::NAME), ARGS_COUNT, INIT_VALUE, const)

/*
 * Defines implementation for the overloaded function with default-constructed return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * ...        - the signature of the overloaded function
 */
#define MOCK_FUNCTION_OVERLOADED(CLASS, NAME, ARGS_COUNT, ...)                                                                                                 \
    MOCK_FUNCTION_OVERLOADED_RES(CLASS,                                                                                                                        \
                                 NAME,                                                                                                                         \
                                 ARGS_COUNT,                                                                                                                   \
                                 details::DefaultCtorOrNullForVoid<details::FunctionTraits<decltype((__VA_ARGS__) & CLASS::NAME)>::ReturnType>::get(),         \
                                 __VA_ARGS__)

/*
 * Defines implementation for the overloaded const member function with default-constructed return value
 *
 * CLASS      - the class name
 * NAME       - the function name
 * ARGS_COUNT - the count of function arguments
 * ...        - the signature of the overloaded function
 */
#define MOCK_FUNCTION_OVERLOADED_CONST(CLASS, NAME, ARGS_COUNT, ...)                                                                                           \
    MOCK_FUNCTION_OVERLOADED_RES_CONST(CLASS,                                                                                                                  \
                                       NAME,                                                                                                                   \
                                       ARGS_COUNT,                                                                                                             \
                                       details::DefaultCtorOrNullForVoid<details::FunctionTraits<decltype((__VA_ARGS__) & CLASS::NAME)>::ReturnType>::get(),   \
                                       __VA_ARGS__)

/*
 * Sets return value for the specified function
 *
 * FULL_NAME - the full name of the function (e.g. SomeClass::functionName)
 * ...       - the value to be returned (or arguments to constructor)
 */
#define SET_FUNCTION_RESULT(FULL_NAME, ...)                                                                                                                    \
    details::ReturnStaticStorageHelper<decltype(&FULL_NAME), COMPTIME_HASH(#FULL_NAME), details::FunctionTraits<decltype(&FULL_NAME)>::ReturnType>::setValue(  \
        __VA_ARGS__)

/*
 * Sets return value for the specified overloaded function
 *
 * FULL_NAME - the full name of the function (e.g. SomeClass::functionName)
 * VALUE     - the value to be returned
 * ...       - the signature of the overloaded function
 */
#define SET_FUNCTION_RESULT_OVERLOADED(FULL_NAME, VALUE, ...)                                                                                                  \
    details::ReturnStaticStorageHelper<decltype((__VA_ARGS__) & FULL_NAME),                                                                                    \
                                       COMPTIME_HASH(#FULL_NAME),                                                                                              \
                                       details::FunctionTraits<decltype((__VA_ARGS__) & FULL_NAME)>::ReturnType>::setValue(VALUE)

/*
 * Sets implementation for the specified function
 *
 * FULL_NAME - the full name of the function (e.g. SomeClass::functionName)
 * ...       - the lambda or function for implementation
 */
#define SET_FUNCTION_IMPL(FULL_NAME, ...) details::FuncImpl<decltype(&FULL_NAME), COMPTIME_HASH(#FULL_NAME)>::func() = __VA_ARGS__

/*
 * Sets implementation for the specified overloaded function
 *
 * FULL_NAME - the full name of the function (e.g. SomeClass::functionName)
 * FUNC_TYPE - the signature of the overloaded function (must be alias if it contains commas)
 * ...       - the lambda or function for implementation
 */
#define SET_FUNCTION_IMPL_OVERLOADED(FULL_NAME, FUNC_TYPE, ...)                                                                                                \
    details::FuncImpl<decltype((FUNC_TYPE) & FULL_NAME), COMPTIME_HASH(#FULL_NAME)>::func() = __VA_ARGS__

#endif // __STATIC_MOCK__H__
