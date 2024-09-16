// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTHREADUTILS_H
#define DTHREADUTILS_H

#include <dtkcore_global.h>
#include <QObject>
#include <QSemaphore>
#include <QThread>
#include <QCoreApplication>
#include <QPointer>
#include <QDebug>

#if DTK_VERSION >= DTK_VERSION_CHECK(6, 0, 0, 0)
#include <QFuture>
#include <QPromise>
#include <QEvent>
#endif

DCORE_BEGIN_NAMESPACE

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
namespace DThreadUtil {
typedef std::function<void()> FunctionType;

class LIBDTKCORESHARED_EXPORT FunctionCallProxy : public QObject
{
    Q_OBJECT
public:
    explicit FunctionCallProxy(QThread *thread);

    static void proxyCall(QSemaphore *s, QThread *thread, QObject *target, FunctionType fun);

Q_SIGNALS:
    void callInLiveThread(QSemaphore *s, QPointer<QObject> target, FunctionType *func);
};

template <typename ReturnType>
class LIBDTKCORESHARED_EXPORT _TMP
{
public:
    inline static ReturnType runInThread(QSemaphore *s, QThread *thread, QObject *target, std::function<ReturnType()> fun)
    {
        ReturnType result;
        FunctionType proxyFun = [&result, &fun] () {
            result = fun();
        };

        FunctionCallProxy::proxyCall(s, thread, target, proxyFun);
        return result;
    }

    template <typename T>
    inline static typename std::enable_if<!std::is_base_of<QObject, T>::value, ReturnType>::type
            runInThread(QSemaphore *s, QThread *thread, T *, std::function<ReturnType()> fun)
    {
        return runInThread(s, thread, static_cast<QObject*>(nullptr), fun);
    }
};
template <>
class LIBDTKCORESHARED_EXPORT _TMP<void>
{
public:
    inline static void runInThread(QSemaphore *s, QThread *thread, QObject *target, std::function<void()> fun)
    {
        FunctionCallProxy::proxyCall(s, thread, target, fun);
    }

    template <typename T>
    inline static typename std::enable_if<!std::is_base_of<QObject, T>::value, void>::type
            runInThread(QSemaphore *s, QThread *thread, T *, std::function<void()> fun)
    {
        return runInThread(s, thread, static_cast<QObject*>(nullptr), fun);
    }
};

template <typename Fun, typename... Args>
inline auto runInThread(QSemaphore *s, QThread *thread, QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return _TMP<decltype(fun(args...))>::runInThread(s, thread, target, std::bind(fun, std::forward<Args>(args)...));
}
template <typename Fun, typename... Args>
inline auto runInThread(QSemaphore *s, QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInThread(s, thread, nullptr, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QSemaphore *s, QThread *thread, QObject *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, target, std::bind(fun, obj, std::forward<Args>(args)...));
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QSemaphore *s, QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return _TMP<typename QtPrivate::FunctionPointer<Fun>::ReturnType>::runInThread(s, thread, obj, std::bind(fun, obj, std::forward<Args>(args)...));
}

template <typename Fun, typename... Args>
inline auto runInThread(QThread *thread, QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    QSemaphore s;

    return runInThread(&s, thread, target, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline auto runInThread(QThread *thread, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInThread(thread, nullptr, fun, std::forward<Args>(args)...);
}
template <typename T, typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QThread *thread, T *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    QSemaphore s;

    return runInThread(&s, thread, target, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInThread(QThread *thread, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return runInThread(thread, obj, obj, fun, std::forward<Args>(args)...);
}

template <typename Fun, typename... Args>
inline auto runInMainThread(QObject *target, Fun fun, Args&&... args) -> decltype(fun(args...))
{
    if (!QCoreApplication::instance()) {
        return fun(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), target, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline auto runInMainThread(Fun fun, Args&&... args) -> decltype(fun(args...))
{
    return runInMainThread(nullptr, fun, std::forward<Args>(args)...);
}

template <typename T, typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInMainThread(T *target, typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    if (!QCoreApplication::instance()) {
        return (obj->*fun)(std::forward<Args>(args)...);
    }

    return runInThread(QCoreApplication::instance()->thread(), target, obj, fun, std::forward<Args>(args)...);
}
template <typename Fun, typename... Args>
inline typename QtPrivate::FunctionPointer<Fun>::ReturnType
        runInMainThread(typename QtPrivate::FunctionPointer<Fun>::Object *obj, Fun fun, Args&&... args)
{
    return runInMainThread(obj, obj, fun, std::forward<Args>(args)...);
}
}
#else
class LIBDTKCORESHARED_EXPORT DThreadUtils final
{
    friend class Caller;
public:
    explicit DThreadUtils(QThread *thread);
    ~DThreadUtils();

    static DThreadUtils &gui();

    QThread *thread() const noexcept;

    template <typename Func, typename... Args>
    inline auto run(QObject *context,typename QtPrivate::FunctionPointer<Func>::Object *obj, Func fun, Args &&...args)
    {
        return call(context, fun, *obj, std::forward<Args>(args)...);
    }
    template <typename Func, typename... Args>
    inline auto run(typename QtPrivate::FunctionPointer<Func>::Object *obj, Func fun, Args &&...args)
    {
        if constexpr (std::is_base_of<QObject, typename QtPrivate::FunctionPointer<Func>::Object>::value) {
            return call(obj, fun, *obj, std::forward<Args>(args)...);
        } else {
            return call(static_cast<QObject *>(nullptr), fun, *obj, std::forward<Args>(args)...);
        }
    }
    template <typename Func, typename... Args>
    inline QFuture<std::invoke_result_t<std::decay_t<Func>, Args...>> run(QObject *context, Func fun, Args &&...args)
    {
        return call(context, fun, std::forward<Args>(args)...);
    }
    template <typename Func, typename... Args>
    inline QFuture<std::invoke_result_t<std::decay_t<Func>, Args...>> run(Func fun, Args &&...args)
    {
        return call(static_cast<QObject *>(nullptr), fun, std::forward<Args>(args)...);
    }
    template <typename... T>
    inline decltype(auto) exec(T &&...args)
    {
        auto future = run(std::forward<T>(args)...);
        if (!thread()->isRunning()) {
            qWarning() << "The target thread is not running, maybe lead to deadlock.";
        }
        future.waitForFinished();
        if constexpr (std::is_same_v<decltype(future), QFuture<void>>) {
            return;
        } else {
            return future.result();
        }
    }

private:
    class AbstractCallEvent : public QEvent
    {
    public:
        AbstractCallEvent(QEvent::Type type)
            : QEvent(type)
        {
        }
        virtual void call() = 0;
    };

    template <typename Func, typename... Args>
    class Q_DECL_HIDDEN CallEvent : public AbstractCallEvent
    {
        using FunInfo = QtPrivate::FunctionPointer<std::decay_t<Func>>;
        using ReturnType = std::invoke_result_t<std::decay_t<Func>, Args...>;

    public:
        CallEvent(QEvent::Type type, Func &&fun, Args &&...args)
            : AbstractCallEvent(type)
            , function(std::forward<Func>(fun))
            , arguments(std::forward<Args>(args)...)
        {
        }

        QEvent *clone() const override { return nullptr; }

        void call() override
        {
            if (promise.isCanceled()) {
                return;
            }

            if (contextChecker == context) {
                promise.start();
#ifndef QT_NO_EXCEPTIONS
                try {
#endif
                    if constexpr (std::is_void_v<ReturnType>) {
                        std::apply(function, arguments);
                    } else {
                        promise.addResult(std::apply(function, arguments));
                    }
#ifndef QT_NO_EXCEPTIONS
                } catch (...) {
                    promise.setException(std::current_exception());
                }
#endif
                promise.finish();
            } else {
                promise.start();
                promise.setException(std::make_exception_ptr(std::runtime_error("The context object is destroyed.")));
                promise.finish();
            }
        }

        Func function;
        const std::tuple<Args...> arguments;
        QPromise<ReturnType> promise;

        QObject *context{nullptr};
        QPointer<QObject> contextChecker;
    };

    template <typename Func, typename... Args>
    inline auto call(QObject *context, Func fun, Args &&...args)
    {
        using FuncInfo = QtPrivate::FunctionPointer<std::decay_t<Func>>;
        using ReturnType = std::invoke_result_t<std::decay_t<Func>, Args...> ;

        if constexpr (FuncInfo::IsPointerToMemberFunction) {
            static_assert(std::is_same_v<std::decay_t<typename QtPrivate::List<Args...>::Car>, typename FuncInfo::Object>,
                          "The obj and function are not compatible.");
            static_assert(
                QtPrivate::CheckCompatibleArguments<typename QtPrivate::List<Args...>::Cdr, typename FuncInfo::Arguments>::value,
                "The args and function are not compatible.");
        } else if constexpr (FuncInfo::ArgumentCount != -1) {
            static_assert(QtPrivate::CheckCompatibleArguments<QtPrivate::List<Args...>, typename FuncInfo::Arguments>::value,
                          "The args and function are not compatible.");
        } else {  // for lambda and impl operator()
            static_assert(std::is_invocable_r_v<ReturnType, Func, Args...>,
                          "The callable object can't invoke with supplied args");
        }

        QPromise<ReturnType> promise;
        auto future = promise.future();

        if (Q_UNLIKELY(QThread::currentThread() == m_thread)) {
            promise.start();
            if constexpr (std::is_void_v<ReturnType>) {
                std::invoke(fun, std::forward<Args>(args)...);
            } else {
                promise.addResult(std::invoke(fun, std::forward<Args>(args)...));
            }
            promise.finish();
        } else {
            auto event = new CallEvent<Func, Args...>(eventType, std::move(fun), std::forward<Args>(args)...);
            event->promise = std::move(promise);
            event->context = context;
            event->contextChecker = context;

            QCoreApplication::postEvent(ensureThreadContextObject(), event);
        }

        return future;
    }

    QObject *ensureThreadContextObject();

    static inline QEvent::Type eventType;
    QThread *m_thread;
    QAtomicPointer<QObject> threadContext;
};
#endif // version macro end
DCORE_END_NAMESPACE
#endif // protect macro end
