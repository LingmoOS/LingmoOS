// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DASYNC_H
#define DASYNC_H
#include <dtkcore_global.h>

#include <QQueue>
#include <QMutex>
#include <QThread>
#include <QMutexLocker>
#include <QCoreApplication>

#include <functional>
#include <type_traits>

DCORE_BEGIN_NAMESPACE

#define GUARDED_BY(...)
#define D_THREAD_IN_MAIN() (qApp->instance() && qApp->instance()->thread() == QThread::currentThread())

// TODO: 添加 DtkCorePrivate 到 dtkcore_global.h
namespace DtkCorePrivate {
// 本类是继承实现的，只有子类方法是安全的，暂不对外提供接口
template <class T>
class DSafeQueue : public QQueue<T>
{
public:
    inline void enqueue(const T &t)
    {
        QMutexLocker lkc(&m_mtx);
        QQueue<T>::enqueue(t);
    }
    inline T dequeue()
    {
        QMutexLocker lkc(&m_mtx);
        return QQueue<T>::dequeue();
    }
    inline int size()
    {
        QMutexLocker lkc(&m_mtx);
        return QQueue<T>::size();
    }
    inline T &head()
    {
        QMutexLocker lkc(&m_mtx);
        return QQueue<T>::head();
    }
    inline const T &head() const
    {
        QMutexLocker lkc(&m_mtx);
        return QQueue<T>::head();
    }

private:
    mutable QMutex m_mtx;
};

// 内部使用，不对外提供接口
class MainWorker : public QObject
{
    Q_OBJECT
    std::function<void(void *)> m_handle;
    std::function<void(void *)> m_handleProxy;

    std::function<void(void)> m_handleV;
    std::function<void(void)> m_handleVProxy;

    bool m_dasyncDestroyed = false;
    char __padding[7];

public:
    void setDAsyncDestroyed() { m_dasyncDestroyed = true; }
    bool dasyncDestroyed() { return m_dasyncDestroyed; }

public:
    MainWorker(QObject *parent = nullptr)
        : QObject(parent)
    {
        // Ensure that QApplication is initialized
        Q_ASSERT(qApp->instance() && qApp->instance()->thread());
        moveToThread(qApp->instance()->thread());

        bool isStartInMain = D_THREAD_IN_MAIN();

        QObject::connect(this,
                         &MainWorker::sigRunInMain,
                         this,
                         &MainWorker::slotRunInMain,
                         isStartInMain ? Qt::AutoConnection : Qt::BlockingQueuedConnection);

        QObject::connect(this,
                         &MainWorker::sigRunInMainVoid,
                         this,
                         &MainWorker::slotRunInMainVoid,
                         isStartInMain ? Qt::AutoConnection : Qt::BlockingQueuedConnection);
    }

    // 1. handle arg is non void
    template <typename FUNC, typename ArgType>
    typename std::enable_if<!std::is_void<ArgType>::value>::type setHandle(FUNC &&func)
    {
        m_handle = [&](void *arg) {
            DSafeQueue<ArgType> *q = static_cast<DSafeQueue<ArgType> *>(arg);
            while (q && q->size()) {
                // 这里是 then 回调真正执行到的地方
                func(q->dequeue());
            }
        };

        m_handleProxy = [this](void *arg) {
            if (m_handle) {
                m_handle(arg);
            }
        };
    }

    // 2. handle arg is void
    template <typename FUNC, typename ArgType>
    typename std::enable_if<std::is_void<ArgType>::value>::type setHandle(FUNC &&func)
    {
        m_handleV = [&](void) {
            // 这里是 then 回调真正执行到的地方
            func();
        };

        m_handleVProxy = [this](void) {
            if (m_handleV) {
                m_handleV();
            }
        };
    }
Q_SIGNALS:
    void sigRunInMain(void *arg);
    void sigRunInMainVoid();
public Q_SLOTS:
    void slotRunInMain(void *arg)
    {
        Q_ASSERT(D_THREAD_IN_MAIN());
        if (m_handleProxy && !m_dasyncDestroyed) {
            m_handleProxy(arg);
        }
    }
    void slotRunInMainVoid(void)
    {
        Q_ASSERT(D_THREAD_IN_MAIN());
        if (m_handleVProxy && !m_dasyncDestroyed) {
            m_handleVProxy();
        }
    }
};
}  // namespace DtkCorePrivate

class DAsyncState : public QObject
{
    Q_OBJECT
public:
    explicit DAsyncState(QObject *parent = nullptr) noexcept
        : QObject(parent)
    {
    }
    enum AsyncTaskState {
        NotReady = 0x00,            // initial state
        Ready = 0x02,               // deffered = false
        Running = 0x04,             // thread started
        Pending = Ready | Running,  // condition wait
        Cancel = 0x08,              // set thread canceled
        WaitFinished = 0x10,        // wiaitForFinished
        Finished = 0x20,            // thread exit
        Forever = 0x30,             // TODO: DAsync<void, xxx>::post execute forever
    };
    Q_DECLARE_FLAGS(AsyncTaskStatus, AsyncTaskState)
};

// Template classes not supported by Q_OBJECT, so class MainWorker is independent
template <typename DataTypeIn, typename DataTypeOut>
class D_DECL_DEPRECATED DAsync : public QObject
{
    class Helper;

    std::mutex m_mtxIn;
    std::condition_variable m_cvIn;

    std::mutex m_mtxForWaitTask;
    std::condition_variable m_cvForWaitTask;

    class Guard
    {
        DAsync *m_as;
        // 如果 DAsync 已经析构了，工作线程还没结束
        // DAsync 中的有些数据就不能在 guard 的析构里面访问了
        bool m_dasDestructed = false;

    public:
        bool destructed() { return m_dasDestructed; }
        void setDestructed() { m_dasDestructed = true; }

    public:
        explicit Guard(DAsync *as) noexcept
            : m_as(as)
        {
            m_as->m_status.setFlag(DAsyncState::Ready);
            m_as->m_status.setFlag(DAsyncState::Finished, false);  // 防止重入
        }
        ~Guard()
        {
            if (destructed()) {
                return;
            }
            m_as->m_threadGuard = nullptr;
            m_as->m_status.setFlag(DAsyncState::Finished);
            m_as->m_status.setFlag(DAsyncState::Ready, false);  // 防止重入
            if (m_as->m_status.testFlag(DAsyncState::WaitFinished)) {
                m_as->m_cvForWaitTask.notify_one();
            }
            setPending(false);
        }
        void setPending(bool isPending)
        {
            if (!destructed()) {
                m_as->m_status.setFlag(DAsyncState::Pending, isPending);
            }
        }
    };
    Guard *m_threadGuard = nullptr;

    /*
     * m_QueueIn 的作用是存储 PostData 传进来的数据
     * m_QueueOut 的作用是将 post 处理完的结果暂存起来然后传入到 then 中
     * 在 emitHelper 中调用 post 进来的任务，然后将结果传到主线程中处理
     * 数据传递使用 void * 做转换，对于复合类型避免了使用 qRegisterMetaType
     */
    template <typename T, typename Enable = void>
    struct DataQueueType
    {
        DtkCorePrivate::DSafeQueue<T> m_queue;
    };
    template <class T>
    struct DataQueueType<T, typename std::enable_if<std::is_void<T>::value>::type>
    {
    };
    using DataInQueue = DataQueueType<DataTypeIn>;
    using DataOutQueue = DataQueueType<DataTypeOut>;
    // Queue 中处理完的结果经由 m_QueueIn 变量暂存，然后经由 signal、slot 传给 then 中的回调函数做参数
    DataInQueue m_QueueIn;
    DataOutQueue m_QueueOut;

    // 存储不同类型的输入函数
    template <typename T1, typename T2, typename Enable1 = void, typename Enable2 = void>
    struct FuncType
    {
    };
    template <typename T1, typename T2>
    struct FuncType<T1,
                    T2,
                    typename std::enable_if<std::is_void<T1>::value>::type,
                    typename std::enable_if<std::is_void<T2>::value>::type>
    {
        std::function<void(void)> cbp;
    };
    template <typename T1, typename T2>
    struct FuncType<T1,
                    T2,
                    typename std::enable_if<!std::is_void<T1>::value>::type,
                    typename std::enable_if<!std::is_void<T2>::value>::type>
    {
        std::function<T2(T1)> cbp;
    };
    template <typename T1, typename T2>
    struct FuncType<T1,
                    T2,
                    typename std::enable_if<std::is_void<T1>::value>::type,
                    typename std::enable_if<!std::is_void<T2>::value>::type>
    {
        std::function<T2(void)> cbp;
    };
    template <typename T1, typename T2>
    struct FuncType<T1,
                    T2,
                    typename std::enable_if<!std::is_void<T1>::value>::type,
                    typename std::enable_if<std::is_void<T2>::value>::type>
    {
        std::function<void(T1)> cbp;
    };

    std::mutex m_mtxFunc;
    FuncType<DataTypeIn, DataTypeOut> m_func GUARDED_BY(m_mtxFunc);
    DAsyncState::AsyncTaskStatus m_status;

public:
    explicit DAsync(QObject *parent = nullptr) noexcept
        : QObject(parent)
        , m_func({nullptr})
        , m_status(DAsyncState::NotReady)
    {
        m_mainWorker = new DtkCorePrivate::MainWorker();
        m_helper = new Helper(this, this);
    }
    ~DAsync()
    {
        if (m_threadGuard) {
            m_threadGuard->setDestructed();
        }
        m_status.setFlag(DAsyncState::Cancel);
        if (m_status.testFlag(DAsyncState::Pending)) {
            m_cvIn.notify_one();
        }
        if (m_mainWorker) {
            m_mainWorker->setDAsyncDestroyed();
            m_mainWorker->deleteLater();
            m_mainWorker = nullptr;
        }
    }

private:
    // 1. input void & emit void
    template <typename PostInType, typename EmitInType>
    typename std::enable_if<std::is_void<PostInType>::value && std::is_void<EmitInType>::value>::type emitHelper()
    {
        m_func.cbp();
        Q_EMIT m_mainWorker->sigRunInMainVoid();
    }
    // 2. input non void & emit non void
    template <typename PostInType, typename EmitInType>
    typename std::enable_if<!std::is_void<PostInType>::value && !std::is_void<EmitInType>::value>::type emitHelper()
    {
        m_QueueOut.m_queue.enqueue(m_func.cbp(m_QueueIn.m_queue.dequeue()));
        Q_EMIT m_mainWorker->sigRunInMain(static_cast<void *>(&(m_QueueOut.m_queue)));
    }
    // 3. input non void & emit void
    template <typename PostInType, typename EmitInType>
    typename std::enable_if<!std::is_void<PostInType>::value && std::is_void<EmitInType>::value>::type emitHelper()
    {
        m_func.cbp(m_QueueIn.m_queue.dequeue());
        Q_EMIT m_mainWorker->sigRunInMainVoid();
    }
    // 4. input void & emit non void
    template <typename PostInType, typename EmitInType>
    typename std::enable_if<std::is_void<PostInType>::value && !std::is_void<EmitInType>::value>::type emitHelper()
    {
        m_QueueOut.m_queue.enqueue(m_func.cbp());
        Q_EMIT m_mainWorker->sigRunInMain(static_cast<void *>(&(m_QueueOut.m_queue)));
    }

public:
    void startUp()
    {
        if (m_status.testFlag(DAsyncState::Cancel)) {
            return;
        }
        m_helper->start();
    }
    void cancelAll()
    {
        m_status.setFlag(DAsyncState::Cancel);
        if (m_status.testFlag(DAsyncState::Pending)) {
            m_cvIn.notify_one();
        }
    }
    bool isFinished() { return m_status.testFlag(DAsyncState::Finished); }
    /*
     * 不能在 QTimer 中使用 waitForFinished，防止阻塞主线程
     * 也不能在主线程执行前使用 waitForFinished()
     * 它的默认参数为 true，等同于 waitForFinished(false) +
     * cancelAll, 如果调用了后者， 会一直阻塞等待任务，直到
     * cancelAll 被调用之后 waitForFinished 才会在任务完成完
     * 成后退出，此时就可以删除DAsync了。最好的管理方式还是采用
     * QObject 的内存托管。主线程中使用，可以采用托管的方式，
     * 任务结束只要调用 cancelAll + isFinished 轮询判断就行了，
     * DAsync 的工作线程就会在完成后自动退出。
     */
    void waitForFinished(bool cancelAllWorks = true)
    {
        Q_ASSERT(!D_THREAD_IN_MAIN());
        if (cancelAllWorks) {
            cancelAll();
        }
        if (!m_status.testFlag(DAsyncState::Finished)) {
            if (m_status.testFlag(DAsyncState::Pending)) {
                m_cvIn.notify_one();
            }
            m_status.setFlag(DAsyncState::WaitFinished);
            std::unique_lock<std::mutex> lck(m_mtxForWaitTask);
            m_cvForWaitTask.wait(lck);
        }
    }
    // 输入数据不是 void 类型则依赖于 m_QueueIn
    template <typename FUNC, typename InputType = DataTypeIn>
    typename std::enable_if<!std::is_void<InputType>::value, Helper *>::type post(FUNC &&func)
    {
        m_func.cbp = std::forward<FUNC>(func);
        if (m_postProxy) {
            return m_helper;
        }
        m_postProxy = [this]() {
            std::thread thread([this] {
                if (m_status.testFlag(DAsyncState::Cancel)) {
                    return;
                }
                Guard guard(this);
                m_threadGuard = &guard;

                std::unique_lock<std::mutex> lck(m_mtxIn);
                while (true) {
                    while (!m_status.testFlag(DAsyncState::Ready) || !m_QueueIn.m_queue.size()) {
                        guard.setPending(true);
                        // 定时查询 flag，防止睡死的情况发生
                        m_cvIn.wait_for(lck, std::chrono::milliseconds(200));
                        if (guard.destructed() || m_status.testFlag(DAsyncState::Cancel)) {
                            return;
                        }
                    }
                    guard.setPending(false);

                    while (m_func.cbp && m_QueueIn.m_queue.size()) {
                        emitHelper<DataTypeIn, DataTypeOut>();
                    }
                }
            });
            thread.detach();
        };

        return m_helper;
    }

    template <typename FUNC, typename InputType = DataTypeIn>
    typename std::enable_if<std::is_void<InputType>::value, Helper *>::type post(FUNC &&func)
    {
        {
            std::lock_guard<std::mutex> lckFunc(m_mtxFunc);
            m_func.cbp = std::forward<FUNC>(func);
        }
        if (m_postProxy) {
            return m_helper;
        }
        m_postProxy = [this]() {
            std::thread thread([this] {
                if (m_status.testFlag(DAsyncState::Cancel)) {
                    return;
                }
                Guard guard(this);
                m_threadGuard = &guard;

                std::unique_lock<std::mutex> lck(m_mtxIn);
                while (true) {
                    if (!m_status.testFlag(DAsyncState::Ready)) {
                        guard.setPending(true);
                        // 定时查询 flag，防止睡死的情况发生
                        m_cvIn.wait_for(lck, std::chrono::milliseconds(200));
                        if (guard.destructed() || m_status.testFlag(DAsyncState::Cancel)) {
                            return;
                        }
                    }
                    guard.setPending(false);

                    if (m_func.cbp) {
                        std::lock_guard<std::mutex> lckFunc(m_mtxFunc);
                        emitHelper<DataTypeIn, DataTypeOut>();
                        m_func.cbp = nullptr;  // reset
                    }
                }
            });
            thread.detach();
        };

        return m_helper;
    }

    // only support DAsync<non void type, ...>
    template <typename InputType = DataTypeIn>
    typename std::enable_if<!std::is_void<InputType>::value>::type postData(const InputType &data)
    {
        if (Q_UNLIKELY(!m_status.testFlag(DAsyncState::Cancel))) {
            m_QueueIn.m_queue.enqueue(data);
            if (m_status.testFlag(DAsyncState::Pending)) {
                m_cvIn.notify_one();
            }
        }
    }

private:
    std::function<void()> m_postProxy;
    class Helper : public QObject
    {
        DAsync *m_async;

    public:
        explicit Helper(DAsync *async, QObject *parent = nullptr) noexcept
            : QObject(parent)
            , m_async(async)
        {
        }

        template <typename FUNC>
        Helper *then(FUNC &&func)
        {
            m_async->m_mainWorker->template setHandle<FUNC, DataTypeOut>(std::forward<FUNC>(func));
            return this;
        }
        // 仅启动，非阻塞
        void start(bool immediately = true)
        {
            if (m_async->m_postProxy) {
                m_async->m_postProxy();
            }
            if (!immediately) {
                m_async->m_status.setFlag(DAsyncState::Ready, false);
            } else {
                m_async->m_status.setFlag(DAsyncState::Ready);
                if (m_async->m_status.testFlag(DAsyncState::Pending)) {
                    m_async->m_cvIn.notify_one();
                }
            }
        }
    };

    Helper *m_helper = nullptr;
    DtkCorePrivate::MainWorker *m_mainWorker = nullptr;
};

DCORE_END_NAMESPACE
#endif  // DASYNC_H
