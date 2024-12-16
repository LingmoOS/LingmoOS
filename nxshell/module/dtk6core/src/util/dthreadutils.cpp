// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dthreadutils.h"

DCORE_BEGIN_NAMESPACE

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
namespace DThreadUtil {
FunctionCallProxy::FunctionCallProxy(QThread *thread)
{
    qRegisterMetaType<QPointer<QObject>>();

    connect(this, &FunctionCallProxy::callInLiveThread, this, [] (QSemaphore *s, QPointer<QObject> target, FunctionType *func) {
        if (Q_LIKELY(target)) {
            (*func)();
        } else {
            qWarning() << "DThreadUtils::runInThread:" << "The target object is destoryed";
        }

        s->release();
    }, Qt::QueuedConnection);
    connect(thread, &QThread::finished, this, [this] {
        qWarning() << "DThreadUtils::runInThread:" << sender() << "the thread finished";
    }, Qt::DirectConnection);
}

void FunctionCallProxy::proxyCall(QSemaphore *s, QThread *thread, QObject *target, FunctionType fun)
{
    if (QThread::currentThread() == thread)
        return fun();

    FunctionCallProxy proxy(thread);
    proxy.moveToThread(thread);

    // 如果线程未开启事件循环，且不是主线程，则需要给出严重警告信息，因为可能会导致死锁
    if (thread->loopLevel() <= 0 && (!QCoreApplication::instance() || thread != QCoreApplication::instance()->thread())) {
        qCritical() << Q_FUNC_INFO << thread << ", the thread no event loop";
    }

    proxy.callInLiveThread(s, target ? target : &proxy, &fun);
    s->acquire();
}

}
#else
class Q_DECL_HIDDEN Caller : public QObject
{
public:
    explicit Caller()
        : QObject()
    {
    }

    bool event(QEvent *event) override
    {
        if (event->type() == DThreadUtils::eventType) {
            auto ev = static_cast<DThreadUtils::AbstractCallEvent *>(event);
            ev->call();
            return true;
        }

        return QObject::event(event);
    }
};

DThreadUtils::DThreadUtils(QThread *thread)
    : m_thread(thread)
    , threadContext(nullptr)
{
}

DThreadUtils::~DThreadUtils()
{
    delete threadContext.loadRelaxed();
}

DThreadUtils &DThreadUtils::gui()
{
    static auto global = DThreadUtils(QCoreApplication::instance()->thread());
    return global;
}

QThread *DThreadUtils::thread() const noexcept
{
    return m_thread;
}

QObject *DThreadUtils::ensureThreadContextObject()
{
    QObject *context;
    if (!threadContext.loadRelaxed()) {
        context = new Caller();
        context->moveToThread(m_thread);
        if (!threadContext.testAndSetRelaxed(nullptr, context)) {
            context->moveToThread(nullptr);
            delete context;
        }
    }

    context = threadContext.loadRelaxed();
    Q_ASSERT(context);

    return context;
}
#endif
DCORE_END_NAMESPACE
