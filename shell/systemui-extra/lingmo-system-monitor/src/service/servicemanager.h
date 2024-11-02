#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include "serviceinfo.h"
#include "kerror.h"

#include <QList>
#include <mutex>
#include <thread>
#include <QObject>

class ServiceManager;
class ServiceDataWorker;
class QThread;

class DelayUpdateTimer : public QObject
{
    Q_OBJECT

public:
    explicit DelayUpdateTimer(ServiceManager *mgr, QObject *parent = nullptr);

public:
    void start(const QString &path);

private:
    int m_cnt = 0;
    QTimer *m_timer = nullptr;
    ServiceManager *m_mgr = nullptr;
};

class ServiceManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ServiceManager)

public:
    static ServiceManager *instance();

    void updateServiceList();

    static bool isSysVInitEnabled(const QString &sname);

    static QString getServiceStartupMode(const QString &id, const QString &state);

    static bool isServiceNoOp(const QString &state);

    static bool isFinalState(QString strActiveState);

    static bool isActiveState(QString strActiveState);

    static bool isServiceAutoStartup(const QString &id, const QString &state);

Q_SIGNALS:
    // 请求刷新服务列表信号
    void updateList();
    // 服务信息列表更新信号
    void serviceListUpdated(const ServiceInfoList &list);
    // 单个服务信息更新信号
    void serviceSingleUpdated(const ServiceInfo &service);

public:
    // 更新单个服务信息
    ServiceInfo updateServiceInfo(const QString &opath);
    QString normalizeServiceId(const QString &id, const QString &param = "");

public Q_SLOTS:
    // 服务管理接口
    KError startService(const QString &id, const QString &param = "");
    KError stopService(const QString &id);
    KError restartService(const QString &id, const QString &param = "");
    KError setServiceStartupMode(const QString &id, bool autoStart);

private:
    explicit ServiceManager(QObject *parent = nullptr);
    ~ServiceManager();

    // 异步刷新服务列表过程的线程
    QThread m_workerThread;
    ServiceDataWorker *m_worker = nullptr;

    //原子单例实例
    static std::atomic<ServiceManager *> m_instance;
    static std::mutex m_mutex;
};

#endif // SERVICEMANAGER_H
