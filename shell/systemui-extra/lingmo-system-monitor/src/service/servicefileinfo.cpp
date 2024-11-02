#include "servicefileinfo.h"

#include <QString>
#include <QDebug>

class ServiceFileInfoData : public QSharedData
{
public:
    // 构造
    ServiceFileInfoData() {}

    // 拷贝构造
    ServiceFileInfoData(const ServiceFileInfoData &other)
        : QSharedData(other)
        , m_name(other.m_name)
        , m_status(other.m_status)
    {
    }

    // 赋值操作
    ServiceFileInfoData &operator=(const ServiceFileInfoData &other)
    {
        if (this != &other) {
            m_name = other.m_name;
            m_status = other.m_status;
        }
        return *this;
    }
    ~ServiceFileInfoData() {}

    friend class ServiceFileInfo;

private:
    // 服务文件名称
    QString m_name = "";
    // 服务文件状态
    QString m_status = "";
};

ServiceFileInfo::ServiceFileInfo()
    : m_data(new ServiceFileInfoData())
{

}

// 重载带参构造
ServiceFileInfo::ServiceFileInfo(const QString &name, const QString &status)
    : m_data(new ServiceFileInfoData())
{
    m_data->m_name = name;
    m_data->m_status = status;
}

// 拷贝构造
ServiceFileInfo::ServiceFileInfo(const ServiceFileInfo &other)
    : m_data(other.m_data)
{
}

// 赋值操作符重载
ServiceFileInfo &ServiceFileInfo::operator=(const ServiceFileInfo &other)
{
    if (this != &other)
        m_data.operator = (other.m_data);
    return *this;
}

ServiceFileInfo::~ServiceFileInfo() {}

// 比较当前服务文件信息与其他服务文件信息
bool ServiceFileInfo::operator==(const ServiceFileInfo &other) const
{
    return m_data->m_name == other.getName() && m_data->m_status == other.getStatus();
}

// 获取服务文件名称
QString ServiceFileInfo::getName() const
{
    return m_data->m_name;
}

// 设置服务文件名称
void ServiceFileInfo::setName(const QString &name)
{
    m_data->m_name = name;
}

// 获取服务文件状态
QString ServiceFileInfo::getStatus() const
{
    return m_data->m_status;
}

// 设置服务文件状态
void ServiceFileInfo::setStatus(const QString &status)
{
    m_data->m_status = status;
}

// 注册ServiceFileInfo元数据类型
void ServiceFileInfo::registerMetaType()
{
    qRegisterMetaType<ServiceFileInfo>("ServiceFileInfo");
    qDBusRegisterMetaType<ServiceFileInfo>();
    qRegisterMetaType<ServiceFileInfoList>("ServiceFileInfoList");
    qDBusRegisterMetaType<ServiceFileInfoList>();
}

// 输出ServiceFileInfo对象到debug流
QDebug &operator<<(QDebug &debug, const ServiceFileInfo &service)
{
    debug << service.getName() << service.getStatus();
    return debug;
}

// 输出ServiceFileInfo对象到DBus参数
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceFileInfo &service)
{
    argument.beginStructure();
    argument << service.getName() << service.getStatus();
    argument.endStructure();
    return argument;
}

// 输出ServiceFileInfo对象到数据流
QDataStream &operator<<(QDataStream &stream, const ServiceFileInfo &service)
{
    stream << service.getName() << service.getStatus();
    return stream;
}

// 从DBus参数对象中读取ServiceFileInfo信息
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceFileInfo &service)
{
    QString name, status;
    argument.beginStructure();
    argument >> name >> status;
    service.setName(name);
    service.setStatus(status);
    argument.endStructure();
    return argument;
}

// 从数据流对象中读取ServiceFileInfo信息
const QDataStream &operator>>(QDataStream &stream, ServiceFileInfo &service)
{
    QString name, status;
    stream >> name >> status;
    service.setName(name);
    service.setStatus(status);
    return stream;
}

// 输出ServiceFileInfo对象列表到DBus参数
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceFileInfoList &list)
{
    argument.beginArray(qMetaTypeId<ServiceFileInfoList>());
    foreach (const ServiceFileInfo &service, list) {
        argument << service;
    }
    argument.endArray();
    return argument;
}

// 从DBus参数对象中读取ServiceFileInfo信息列表
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceFileInfoList &list)
{
    argument.beginArray();
    while (!argument.atEnd()) {
        ServiceFileInfo service;
        argument >> service;
        list << service;
    }
    argument.endArray();
    return argument;
}

