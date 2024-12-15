// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extendworker.h"

#include "searchplugin/pluginliaison.h"

using namespace GrandSearch;

ExtendWorker::ExtendWorker(const QString &name, QObject *parent)
    : ProxyWorker(name, parent)
{
    m_timeout.setInterval(25 * 1000);
    m_timeout.setSingleShot(true);
    connect(&m_timeout, &QTimer::timeout, this, [this](){
        qDebug() << m_name << "working time out.";
        onWorkFinished({});
    });
}

ExtendWorker::~ExtendWorker()
{

}

bool ExtendWorker::setService(const QString &service, const QString &address, const QString &interface, const QString &ver)
{
    if (Q_UNLIKELY(address.isEmpty()
            || service.isEmpty() || interface.isEmpty()
            || ver.isEmpty()))
        return false;

    if (m_liaison)
        return false;

    //创建通信模块
    auto liaison = new PluginLiaison(this);
    if (!liaison->init(service, address, interface, ver, name())) {
        qWarning() << "fail to create PluginLiaison: " << service << address << interface << ver;
        delete liaison;
        return false;
    }

    m_liaison = liaison;

    //插件返回数据解析完毕，使用队列切入主线程，信号会来自子线程，也会来自主线程
    connect(m_liaison, &PluginLiaison::searchFinished, this, &ExtendWorker::onWorkFinished,Qt::QueuedConnection);
    //dbus服务就绪尝试调用,在working函数中,可能服务还未启动,因此需在服务启动后再次尝试调用
    connect(m_liaison, &PluginLiaison::ready, this, &ExtendWorker::tryWorking, Qt::QueuedConnection);
    return true;
}

void ExtendWorker::setContext(const QString &context)
{
    m_context = context;
}

bool ExtendWorker::isAsync() const
{
    return true;
}

bool ExtendWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    QString id = *static_cast<QString *>(context);
    if (!m_liaison || id.isEmpty() || m_context.isEmpty()) {
        m_status.storeRelease(Completed);
        return false;
    }

    m_taskID = id;

    //服务是否可用
    QMutexLocker lk(&m_callMtx);
    if (m_liaison->isVaild()) {
        m_callSerach = Called;
        lk.unlock();

        //直接调用
        qDebug() << m_name << "working, search.";
        if (m_liaison->search(m_taskID, m_context)) {
            return true;
        } else {
            m_status.storeRelease(Completed);
            return false;
        }
    }

    //等待服务启动后再调用
    qDebug() << m_name << "working, wait starting.";
    m_callSerach = WaitCall;

    //主线程中启动超时
    QMetaObject::invokeMethod(&m_timeout, "start", Qt::QueuedConnection);
    return true;
}

void ExtendWorker::terminate()
{
    m_status.storeRelease(Terminated);

    if (m_liaison)
        m_liaison->stop(m_taskID);
}

ProxyWorker::Status ExtendWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool ExtendWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

MatchedItemMap ExtendWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    MatchedItemMap items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    return items;
}

void ExtendWorker::tryWorking()
{
    Q_ASSERT(m_liaison);

    //! 由dbus服务启动信号触发，在主线程执行
    QMutexLocker lk(&m_callMtx);
    if (m_callSerach != WaitCall) {
        qDebug() << m_name << "service started, but called.";
        return;
    }
    m_callSerach = Called;
    m_timeout.stop();
    lk.unlock();

    qInfo() << m_name << "service started, search.";
    if (!m_liaison->search(m_taskID, m_context)) {
        //发起搜索失败
        m_status.storeRelease(Completed);

        //发送结束信号
        emit asyncFinished(this);
    }
}

void ExtendWorker::onWorkFinished(const MatchedItemMap &ret)
{
    m_status.storeRelease(Completed);
    QMutexLocker lk(&m_mtx);
    m_items = ret;
    lk.unlock();

    if (!ret.isEmpty())
        emit unearthed(this);

    qDebug() << name() << "work finished" << ret.size();
    //异步搜索结束
    emit asyncFinished(this);
}
