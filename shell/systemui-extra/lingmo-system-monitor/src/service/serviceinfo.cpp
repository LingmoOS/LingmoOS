#include "serviceinfo.h"

#include <QString>

class ServiceInfoData : public QSharedData
{
public:
    /**
     * @brief ServiceInfoData 构造
     */
    ServiceInfoData() {}

    /**
     * @brief 拷贝构造
     * @param other 其他服务信息数据对象
     */
    ServiceInfoData(const ServiceInfoData &other)
        : QSharedData(other)
        , m_mainPID(other.m_mainPID)
        , m_jobId(other.m_jobId)
        , m_name(other.m_name)
        , m_description(other.m_description)
        , m_loadState(other.m_loadState)
        , m_activeState(other.m_activeState)
        , m_subState(other.m_subState)
        , m_followedBy(other.m_followedBy)
        , m_serviceObjectPath(other.m_serviceObjectPath)
        , m_jobType(other.m_jobType)
        , m_jobObjectPath(other.m_jobObjectPath)
        , m_canReload(other.m_canReload)
        , m_canStart(other.m_canStart)
        , m_canStop(other.m_canStop)
        , m_state(other.m_state)
        , m_sName(other.m_sName)
        , m_strID(other.m_strID)
        , m_startupType(other.m_startupType)
    {
    }

    /**
     * @brief 赋值操作
     * @param other 其他服务信息数据对象
     * @return 赋值后的对象
     */
    ServiceInfoData &operator=(const ServiceInfoData &other)
    {
        if (this != &other) {
            m_jobId = other.m_jobId;
            m_name = other.m_name;
            m_description = other.m_description;
            m_loadState = other.m_loadState;
            m_activeState = other.m_activeState;
            m_subState = other.m_subState;
            m_followedBy = other.m_followedBy;
            m_serviceObjectPath = other.m_serviceObjectPath;
            m_jobType = other.m_jobType;
            m_jobObjectPath = other.m_jobObjectPath;
            m_canReload = other.m_canReload;
            m_canStart = other.m_canStart;
            m_canStop = other.m_canStop;
            m_mainPID = other.m_mainPID;
            m_state = other.m_state;
            m_sName = other.m_sName;
            m_strID = other.m_strID;
            m_startupType = other.m_startupType;
        }
        return *this;
    }
    ~ServiceInfoData() {}

    friend class ServiceInfo;

private:
    // 服务主进程id
    quint32 m_mainPID = 0;
    // 服务任务Id
    quint32 m_jobId = 0;
    // 服务名称
    QString m_name = "";
    // 服务描述
    QString m_description = "";
    // 服务加载状态
    QString m_loadState = "";
    // 服务活动状态
    QString m_activeState = "";
    // 服务子状态
    QString m_subState = "";
    // 服务跟随
    QString m_followedBy = "";
    // 服务对象路径
    QString m_serviceObjectPath = "";
    // 服务任务类型
    QString m_jobType = "";
    // 任务对象路径
    QString m_jobObjectPath = "";
    // 是否可重新加载
    bool m_canReload = false;
    // 是否可启动
    bool m_canStart = false;
    // 是否可停止
    bool m_canStop = false;
    // 服务状态（文件）
    QString m_state = "";
    // 服务短名称
    QString m_sName = "";
    // 服务ID
    QString m_strID = "";
    // 服务启动类型
    QString m_startupType = "";
};

ServiceInfo::ServiceInfo()
    : m_data(new ServiceInfoData)
{

}

ServiceInfo::~ServiceInfo()
{

}

// 带参构造
ServiceInfo::ServiceInfo(const QString &name,
                   const QString &description,
                   const QString &loadState,
                   const QString &activeState,
                   const QString &subState,
                   const QString &followedBy,
                   const QString &serviceObjectPath,
                   quint32 jobId,
                   const QString &jobType,
                   const QString &jobObjectPath)
    : m_data(new ServiceInfoData())
{
    m_data->m_name = name;
    m_data->m_description = description;
    m_data->m_loadState = loadState;
    m_data->m_activeState = activeState;
    m_data->m_subState = subState;
    m_data->m_followedBy = followedBy;
    m_data->m_serviceObjectPath = serviceObjectPath;
    m_data->m_jobId = jobId;
    m_data->m_jobType = jobType;
    m_data->m_jobObjectPath = jobObjectPath;
}

// 拷贝构造
ServiceInfo::ServiceInfo(const ServiceInfo &other)
    : m_data(other.m_data)
{
}

// 赋值操作
ServiceInfo &ServiceInfo::operator=(const ServiceInfo &other)
{
    if (this != &other)
        m_data.operator = (other.m_data);
    return *this;
}

// 比较当前ServiceInfo与另一个ServiceInfo
bool ServiceInfo::operator==(const ServiceInfo &other) const
{
    return (m_data->m_name == other.getName() && m_data->m_description == other.getDescription() &&
           m_data->m_loadState == other.getLoadState() &&
           m_data->m_activeState == other.getActiveState() && m_data->m_subState == other.getSubState() &&
           m_data->m_followedBy == other.getFollowedBy() &&
           m_data->m_serviceObjectPath == other.getServiceObjectPath() && m_data->m_jobId == other.getJobId() &&
           m_data->m_jobType == other.getJobType() && m_data->m_jobObjectPath == other.getJobObjectPath());
}

// 获取服务名称
QString ServiceInfo::getName() const
{
    return m_data->m_name;
}

// 设置服务名称
void ServiceInfo::setName(const QString &name)
{
    m_data->m_name = name;
}

// 获取服务描述
QString ServiceInfo::getDescription() const
{
    return m_data->m_description;
}

// 设置服务描述
void ServiceInfo::setDescription(const QString &description)
{
    m_data->m_description = description;
}

// 获取服务加载状态
QString ServiceInfo::getLoadState() const
{
    return m_data->m_loadState;
}

// 设置服务加载状态
void ServiceInfo::setLoadState(const QString &loadState)
{
    m_data->m_loadState = loadState;
}

// 获取服务活动状态
QString ServiceInfo::getActiveState() const
{
    return m_data->m_activeState;
}

// 设置服务活动状态
void ServiceInfo::setActiveState(const QString &activeState)
{
    m_data->m_activeState = activeState;
}

// 获取服务子状态
QString ServiceInfo::getSubState() const
{
    return m_data->m_subState;
}

// 设置服务子状态
void ServiceInfo::setSubState(const QString &subState)
{
    m_data->m_subState = subState;
}

// 获取服务跟随
QString ServiceInfo::getFollowedBy() const
{
    return m_data->m_followedBy;
}

// 设置服务跟随
void ServiceInfo::setFollowedBy(const QString &followedBy)
{
    m_data->m_followedBy = followedBy;
}

// 获取服务对象路径
QString ServiceInfo::getServiceObjectPath() const
{
    return m_data->m_serviceObjectPath;
}

// 设置服务对象路径
void ServiceInfo::setServiceObjectPath(const QString &serviceObjectPath)
{
    m_data->m_serviceObjectPath = serviceObjectPath;
}

// 获取服务主进程ID
quint32 ServiceInfo::getMainPID() const
{
    return m_data->m_mainPID;
}

// 设置服务主进程ID
void ServiceInfo::setMainPID(quint32 mainPID)
{
    m_data->m_mainPID = mainPID;
}

// 获取服务任务id
quint32 ServiceInfo::getJobId() const
{
    return m_data->m_jobId;
}

// 设置服务任务id
void ServiceInfo::setJobId(quint32 jobId)
{
    m_data->m_jobId = jobId;
}

// 获取服务任务类型
QString ServiceInfo::getJobType() const
{
    return m_data->m_jobType;
}

// 设置服务任务类型
void ServiceInfo::setJobType(const QString &jobType)
{
    m_data->m_jobType = jobType;
}

// 获取服务任务对象路径
QString ServiceInfo::getJobObjectPath() const
{
    return m_data->m_jobObjectPath;
}

// 设置服务任务对象路径
void ServiceInfo::setJobObjectPath(const QString &jobObjectPath)
{
    m_data->m_jobObjectPath = jobObjectPath;
}

// 获取服务是否可重新加载
bool ServiceInfo::getIsCanReload() const
{
    return m_data->m_canReload;
}

// 设置服务是否可重新加载
void ServiceInfo::setIsCanReload(bool canReload)
{
    m_data->m_canReload = canReload;
}

// 获取服务是否可启动
bool ServiceInfo::getIsCanStart() const
{
    return m_data->m_canStart;
}

// 设置服务是否可启动
void ServiceInfo::setIsCanStart(bool canStart)
{
    m_data->m_canStart = canStart;
}

// 获取服务是否可停止
bool ServiceInfo::getIsCanStop() const
{
    return m_data->m_canStop;
}

// 设置服务是否可停止
void ServiceInfo::setIsCanStop(bool canStop)
{
    m_data->m_canStop = canStop;
}

// 获取服务状态
QString ServiceInfo::getState() const
{
    return m_data->m_state;
}

// 设置服务状态
void ServiceInfo::setState(QString state)
{
    m_data->m_state = state;
}

// 获取服务短名称
QString ServiceInfo::getSName() const
{
    return m_data->m_sName;
}

// 设置服务短名称
void ServiceInfo::setSName(QString sName)
{
    m_data->m_sName = sName;
}

// 获取服务ID
QString ServiceInfo::getID() const
{
    return m_data->m_strID;
}

// 设置服务ID
void ServiceInfo::setID(QString strID)
{
    m_data->m_strID = strID;
}

// getStartupType 获取服务启动类型
QString ServiceInfo::getStartupType() const
{
    return m_data->m_startupType;
}

// 设置服务启动类型
void ServiceInfo::setStartupType(QString strStartupType)
{
    m_data->m_startupType = strStartupType;
}

// 注册ServiceInfo元数据类型
void ServiceInfo::registerMetaType()
{
    qRegisterMetaType<ServiceInfo>("ServiceInfo");
    qDBusRegisterMetaType<ServiceInfo>();
    qRegisterMetaType<ServiceInfoList>("ServiceInfoList");
    qDBusRegisterMetaType<ServiceInfoList>();
}

// 输出ServiceInfo信息到debug流
QDebug &operator<<(QDebug &debug, const ServiceInfo &service)
{
    debug << service.getName() << service.getDescription() << service.getLoadState() << service.getActiveState()
          << service.getSubState() << service.getFollowedBy() << service.getServiceObjectPath() << service.getMainPID()
          << service.getJobId() << service.getJobType() << service.getJobObjectPath() << service.getIsCanReload()
          << service.getIsCanStart() << service.getIsCanStop();
    return debug;
}

// 转换服务信息至dbus参数
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceInfo &service)
{
    argument.beginStructure();
    argument << service.getName() << service.getDescription() << service.getLoadState()
             << service.getActiveState() << service.getSubState() << service.getFollowedBy()
             << service.getServiceObjectPath() << service.getJobId() << service.getJobType()
             << service.getJobObjectPath();
    argument.endStructure();
    return argument;
}

// 输出ServiceInfo信息到数据流
QDataStream &operator<<(QDataStream &stream, const ServiceInfo &service)
{
    stream << service.getName() << service.getDescription() << service.getLoadState()
           << service.getActiveState() << service.getSubState() << service.getFollowedBy()
           << service.getServiceObjectPath() << service.getJobId() << service.getJobType()
           << service.getJobObjectPath();
    return stream;
}

// 转换dbus参数至服务信息
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceInfo &service)
{
    QString name, description, loadState, activeState, subState, followedBy, serviceObjectPath,
            jobType, jobObjectPath;
    quint32 jobId;
    argument.beginStructure();
    argument >> name >> description >> loadState >> activeState >> subState >> followedBy >>
             serviceObjectPath >> jobId >> jobType >> jobObjectPath;
    service.setName(name);
    service.setDescription(description);
    service.setLoadState(loadState);
    service.setActiveState(activeState);
    service.setSubState(subState);
    service.setFollowedBy(followedBy);
    service.setServiceObjectPath(serviceObjectPath);
    service.setJobId(jobId);
    service.setJobType(jobType);
    service.setJobObjectPath(jobObjectPath);
    argument.endStructure();
    return argument;
}

// 从数据流读取信息到ServiceInfo
const QDataStream &operator>>(QDataStream &stream, ServiceInfo &service)
{
    QString name, description, loadState, activeState, subState, followedBy, serviceObjectPath,
            jobType, jobObjectPath;
    quint32 jobId;
    stream >> name >> description >> loadState >> activeState >> subState >> followedBy >>
           serviceObjectPath >> jobId >> jobType >> jobObjectPath;
    service.setName(name);
    service.setDescription(description);
    service.setLoadState(loadState);
    service.setActiveState(activeState);
    service.setSubState(subState);
    service.setFollowedBy(followedBy);
    service.setServiceObjectPath(serviceObjectPath);
    service.setJobId(jobId);
    service.setJobType(jobType);
    service.setJobObjectPath(jobObjectPath);
    return stream;
}

// 转换服务信息列表至dbus参数
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceInfoList &list)
{
    argument.beginArray(qMetaTypeId<ServiceInfo>());
    foreach (const ServiceInfo &service, list) {
        argument << service;
    }
    argument.endArray();
    return argument;
}

// 转换dbus参数至服务信息列表
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceInfoList &list)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        ServiceInfo service;
        argument >> service;
        list << service;
    }
    argument.endArray();
    return argument;
}

