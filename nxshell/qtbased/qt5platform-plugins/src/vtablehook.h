// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VTABLEHOOK_H
#define VTABLEHOOK_H

#include <QObject>
#include <QSet>
#include <QDebug>
#include <QLoggingCategory>
#include "global.h"

#include <functional>

DPP_BEGIN_NAMESPACE

#ifndef QT_DEBUG
inline Q_LOGGING_CATEGORY(vtableHook, "deepin.qpa.vtableHook", QtInfoMsg);
#else
inline Q_LOGGING_CATEGORY(vtableHook, "deepin.qpa.vtableHook");
#endif

static inline quintptr toQuintptr(void *ptr)
{
    return *(quintptr*)ptr;
}

static inline int getVtableSize(quintptr **obj)
{
    quintptr *begin = *obj;
    while (true) {
      if ((int64_t)*begin == 0 or (int64_t)*begin < QSysInfo::WordSize)  // offset will grater than 8 bytes(64 bit)
          break;
       ++begin;
    }
    return begin - *obj + 2;  // for offset and rtti info
}

// http://refspecs.linux-foundation.org/cxxabi-1.83.html#vtable
class VtableHook
{
public:
    static inline quintptr *getVtableOfObject(const void *obj)
    {
        return *(quintptr**)obj;
    }

    template <typename T>
    static quintptr *getVtableOfClass()
    {
        QByteArray vtable_symbol(typeid(T).name());
        vtable_symbol.prepend("_ZTV");

        quintptr *vfptr_t1 = reinterpret_cast<quintptr*>(resolve(vtable_symbol.constData()));

        return vfptr_t1 ? adjustToEntry(vfptr_t1) : nullptr;
    }

    static int getDestructFunIndex(quintptr **obj, std::function<void(void)> destoryObjFun);
    static constexpr const QObject *getQObject(...) { return nullptr;}
    static constexpr const QObject *getQObject(const QObject *obj) { return obj;}
    static void autoCleanVtable(const void *obj);
    static bool ensureVtable(const void *obj, std::function<void(void)> destoryObjFun);
    static bool hasVtable(const void *obj);
    static void resetVtable(const void *obj);
    static quintptr resetVfptrFun(const void *obj, quintptr functionOffset);
    template <typename Fun> // for class
    static inline quintptr resetVfptrFun(Fun fun)
    {
        typedef QtPrivate::FunctionPointer<Fun> FunInfo;
        return resetVfptrFun(getVtableOfClass<typename FunInfo::Object>(), toQuintptr(&fun));
    }
    static quintptr originalFun(const void *obj, quintptr functionOffset);
    static bool forceWriteMemory(void *adr, const void *data, size_t length);
    static QFunctionPointer resolve(const char *symbol);

    template <typename T> class OverrideDestruct : public T { ~OverrideDestruct() override;};
    template <typename List1, typename List2> struct CheckCompatibleArguments { enum { value = false }; };
    template <typename List> struct CheckCompatibleArguments<List, List> { enum { value = true }; };

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, quintptr *vfptr_t2, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;

        //compilation error if the arguments does not match.
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<typename FunInfo1::Arguments, typename FunInfo2::Arguments>::value),
                          "Function1 and Function2 arguments are not compatible.");
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<QtPrivate::List<typename FunInfo1::ReturnType>, QtPrivate::List<typename FunInfo2::ReturnType>>::value),
                          "Function1 and Function2 return type are not compatible..");

        //! ({code}) in the form of a code is to eliminate - Wstrict - aliasing build warnings
        quintptr fun1_offset = toQuintptr(&fun1);
        quintptr fun2_offset = toQuintptr(&fun2);

        if (fun1_offset < 0 || fun1_offset > UINT_LEAST16_MAX)
            return false;

        quintptr *vfun = vfptr_t1 + fun1_offset / sizeof(quintptr);

        // if the fun2 is not virtual function
        if (fun2_offset <= UINT_LEAST16_MAX) {
            fun2_offset = *(vfptr_t2 + fun2_offset / sizeof(quintptr));
        }

        if (forceWrite)
            return forceWriteMemory(vfun, &fun2_offset, sizeof(fun2_offset));

        *vfun = fun2_offset;

        return true;
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1,
                      const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;}; //TODO: we can use std::has_virtual_destructor

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        quintptr *vfptr_t1 = getVtableOfObject(t1);
        quintptr *vfptr_t2 = getVtableOfObject(t2);

        bool ok = overrideVfptrFun(vfptr_t1, fun1, vfptr_t2, fun2, false);

        if (!ok) {
            // 恢复旧环境
            resetVtable(t1);
        }

        return ok;
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, const typename QtPrivate::FunctionPointer<Fun2>::Object *t2, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        quintptr *vfptr_t1 = getVtableOfClass<typename FunInfo1::Object>();

        if (!vfptr_t1) {
            abort();
        }

        quintptr *vfptr_t2 = getVtableOfObject(t2);

        return overrideVfptrFun(vfptr_t1, fun1, vfptr_t2, fun2, true);
    }

    template<typename Func> struct FunctionPointer { };
    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...)>
    {
        typedef QtPrivate::List<Obj*, Args...> Arguments;
    };
    template<class Obj, typename Ret, typename... Args> struct FunctionPointer<Ret (Obj::*) (Args...) const>
    {
        typedef QtPrivate::List<Obj*, Args...> Arguments;
    };
    template<typename Fun1, typename Fun2>
    static typename std::enable_if<QtPrivate::FunctionPointer<Fun2>::ArgumentCount >= 0, bool>::type
            overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        typedef QtPrivate::FunctionPointer<Fun2> FunInfo2;

        Q_STATIC_ASSERT(!FunInfo2::IsPointerToMemberFunction);
        //compilation error if the arguments does not match.
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<typename FunctionPointer<Fun1>::Arguments, typename FunInfo2::Arguments>::value),
                          "Function1 and Function2 arguments are not compatible.");
        Q_STATIC_ASSERT_X((CheckCompatibleArguments<QtPrivate::List<typename FunInfo1::ReturnType>, QtPrivate::List<typename FunInfo2::ReturnType>>::value),
                          "Function1 and Function2 return type are not compatible..");

        //! ({code}) in the form of a code is to eliminate - Wstrict - aliasing build warnings
        quintptr fun1_offset = toQuintptr(&fun1);
        quintptr fun2_offset = toQuintptr(&fun2);

        if (fun1_offset < 0 || fun1_offset > UINT_LEAST16_MAX)
            return false;

        quintptr *vfun = vfptr_t1 + fun1_offset / sizeof(quintptr);

        if (forceWrite)
            return forceWriteMemory(vfun, &fun2_offset, sizeof(fun2_offset));

        *vfun = fun2_offset;

        return true;
    }

    template<typename StdFun, typename Func> struct StdFunWrap {};
    template<typename StdFun, class Obj, typename Ret, typename... Args>
    struct StdFunWrap<StdFun, Ret (Obj::*) (Args...)> {
        typedef std::function<Ret(Obj*, Args...)> StdFunType;
        static inline StdFunType fun(StdFunType f, bool check = true) {
            static StdFunType fun = f;
            static bool initialized = false;
            if (initialized && check) {
                qCWarning(vtableHook, "The StdFunWrap is dirty! Don't use std::bind(use lambda functions).");
            }
            initialized = true;
            return fun;
        }
        static Ret call(Obj *o, Args... args) {
            return fun(call, false)(o, std::forward<Args>(args)...);
        }
    };
    template<typename StdFun, class Obj, typename Ret, typename... Args>
    struct StdFunWrap<StdFun, Ret (Obj::*) (Args...) const> : StdFunWrap<StdFun, Ret (Obj::*) (Args...)>{};

    template<typename Fun1, typename Fun2>
    static inline typename std::enable_if<QtPrivate::FunctionPointer<Fun2>::ArgumentCount == -1, bool>::type
            overrideVfptrFun(quintptr *vfptr_t1, Fun1 fun1, Fun2 fun2, bool forceWrite)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        const int FunctorArgumentCount = QtPrivate::ComputeFunctorArgumentCount<Fun2, typename FunctionPointer<Fun1>::Arguments>::Value;

        Q_STATIC_ASSERT_X((FunctorArgumentCount >= 0),
                          "Function1 and Function2 arguments are not compatible.");
        const int Fun2ArgumentCount = (FunctorArgumentCount >= 0) ? FunctorArgumentCount : 0;
        typedef typename QtPrivate::FunctorReturnType<Fun2, typename QtPrivate::List_Left<typename FunctionPointer<Fun1>::Arguments, Fun2ArgumentCount>::Value>::Value Fun2ReturnType;

        Q_STATIC_ASSERT_X((QtPrivate::AreArgumentsCompatible<Fun2ReturnType, typename FunInfo1::ReturnType>::value),
                          "Function1 and Function2 return type are not compatible.");

        StdFunWrap<Fun2, Fun1>::fun(fun2);
        return overrideVfptrFun(vfptr_t1, fun1, StdFunWrap<Fun2, Fun1>::call, forceWrite);
    }

    /*!
     * \fn template<typename Fun1, typename Fun2> static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1, Fun2 fun2)
     *
     * \note 重载多继承类中的多个虚函数时，fun1务必标记成一个类名的函数。否则可能出现内存泄露的情况
     * \note 例如 class A 继承于 B，C，D，当需要重载B中的foo1，C中的foo2时，以下函数的fun1需要统一标记为&A::foo1和&A::foo2
     * \note 因为如果分开写为&B::foo1和&C::foo2的话，指针转换内部会记录多张虚表，重载多份析构函数，可能导致最开始的部分无法正常析构！
     */
    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *t1, Fun1 fun1, Fun2 fun2) //THIS
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        // 检查析构函数是否为虚
        class OverrideDestruct : public FunInfo1::Object { ~OverrideDestruct() override;};

        if (!ensureVtable((void*)t1, std::bind(&_destory_helper<typename FunInfo1::Object>, t1))) {
            return false;
        }

        bool ok = overrideVfptrFun(getVtableOfObject(t1), fun1, fun2, false);

        if (!ok) {
            // 恢复旧环境
            resetVtable(t1);
        }

        return true;
    }

    template<typename Class, typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, Fun2 fun2)
    {
        quintptr *vfptr_t1 = getVtableOfClass<Class>();

        if (!vfptr_t1) {
            abort();
        }

        return overrideVfptrFun(vfptr_t1, fun1, fun2, true);
    }

    template<typename Fun1, typename Fun2>
    static bool overrideVfptrFun(Fun1 fun1, Fun2 fun2)
    {
        typedef QtPrivate::FunctionPointer<Fun1> FunInfo1;
        return overrideVfptrFun<typename FunInfo1::Object>(fun1, fun2);
    }

    template<typename Fun1>
    static bool resetVfptrFun(const typename QtPrivate::FunctionPointer<Fun1>::Object *obj, Fun1 fun)
    {
        return resetVfptrFun((void*)obj, toQuintptr(&fun)) > 0;
    }

    template<typename Fun>
    static Fun originalFun(const typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun)
    {
        quintptr o_fun = originalFun((void*)obj, toQuintptr(&fun));

        return *reinterpret_cast<Fun*>(o_fun);
    }

    template<typename Fun, typename... Args>
    static typename QtPrivate::FunctionPointer<Fun>::ReturnType
    callOriginalFun(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
    {
        quintptr fun_offset = toQuintptr(&fun);

        class _ResetVFun
        {
        public:
            ~_ResetVFun() {
                *(vfptr + offset / sizeof(quintptr)) = oldFun;
            }
            quintptr *vfptr = nullptr;
            quint16 offset = 0;
            quintptr oldFun = 0;
        };

        _ResetVFun rvf;

        rvf.vfptr = *(quintptr**)(obj);
        rvf.offset = fun_offset;
        rvf.oldFun = resetVfptrFun((void*)obj, fun_offset);

        if (!rvf.oldFun) {
            qCWarning(vtableHook) << "Reset the function failed, object address:" << static_cast<void *>(obj);
            abort();
        }

        // call
        return (obj->*fun)(std::forward<Args>(args)...);
    }

private:
    static bool copyVtable(quintptr **obj);
    static bool clearGhostVtable(const void *obj);
    static void clearAllGhostVtable();

    template <typename T>
    static T adjustToTop(T obj)  // vtableTop: vtable start address, Usually refers to offset_to_top
    {
        // this function should'n be called when entry is parent entry
        using fundamentalType = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;
        return obj - static_cast<fundamentalType>(2);  // vtable start address  = vtable entry - 2
    }

    template <typename T>
    static T adjustToEntry(T obj)  // vtableEntry: is located after rtti in the virtual table
    {
        // this function should'n be called when entry is parent entry
        using fundamentalType = typename std::remove_cv<typename std::remove_pointer<T>::type>::type;
        return obj + static_cast<fundamentalType>(2);  // vtable entry = vtable start address + 2
    }
    
    template<typename T>
    static void _destory_helper(const T *obj) {
        delete obj;
    }

    static QMap<quintptr**, quintptr*> objToOriginalVfptr;
    static QMap<const void*, quintptr*> objToGhostVfptr;
    static QMap<const void*, quintptr> objDestructFun;
};

DPP_END_NAMESPACE

#define HookReset VtableHook::resetVfptrFun
#define HookCall VtableHook::callOriginalFun
#define HookOverride VtableHook::overrideVfptrFun

#endif // VTABLEHOOK_H
