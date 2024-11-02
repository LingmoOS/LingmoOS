#ifndef SERVICEFILEINFO_H
#define SERVICEFILEINFO_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QtDBus>

class ServiceFileInfoData;
class QDebug;

class ServiceFileInfo
{
public:
    ServiceFileInfo();
    /**
     * @brief 重载带参构造
     * @param name 服务文件名称
     * @param status 服务文件状态
     */
    ServiceFileInfo(const QString &name, const QString &status);

    /**
     * @brief 拷贝构造
     */
    ServiceFileInfo(const ServiceFileInfo &);

    /**
     * @brief operator = 赋值操作符
     * @return 赋值对象
     */
    ServiceFileInfo &operator=(const ServiceFileInfo &);
    ~ServiceFileInfo();

    /**
     * @brief operator == 比较当前服务文件信息与其他服务文件信息
     * @param other 其他文件信息对象
     * @return 比对结果 true为同
     */
    bool operator==(const ServiceFileInfo &other) const;

    /**
     * @brief getName 获取服务文件名称
     * @return 服务文件名称
     */
    QString getName() const;

    /**
    * @brief setName 设置服务文件名称
    * @param name 服务文件名称
    */
    void setName(const QString &name);

    /**
     * @brief getStatus 获取服务文件状态
     * @return 服务文件状态
     */
    QString getStatus() const;

    /**
     * @brief setStatus 设置服务文件状态
     * @param status 服务文件状态
     */
    void setStatus(const QString &status);

    /**
     * @brief registerMetaType 注册ServiceFileInfo元数据类型
     */
    static void registerMetaType();

    /**
     * @brief operator << 输出ServiceFileInfo对象到debug流
     * @param debug debug流对象
     * @param service 服务文件信息
     * @return debug流对象
     */
    friend QDebug &operator<<(QDebug debug, const ServiceFileInfo &service);

    /**
     * @brief operator << 输出ServiceFileInfo对象到DBus参数
     * @param argument DBus参数对象
     * @param service 服务文件信息
     * @return DBus参数对象
     */
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ServiceFileInfo &service);

    /**
     * @brief operator << 输出ServiceFileInfo对象到数据流
     * @param stream 数据流对象
     * @param service 服务文件信息
     * @return 数据流对象
     */
    friend QDataStream &operator<<(QDataStream &stream, const ServiceFileInfo &service);

    /**
     * @brief operator >> 从DBus参数对象中读取ServiceFileInfo信息
     * @param argument DBus参数对象
     * @param service 服务文件信息
     * @return DBus参数对象
     */
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceFileInfo &service);

    /**
     * @brief operator >> 从数据流对象中读取ServiceFileInfo信息
     * @param stream 数据流对象
     * @param service 服务文件信息
     * @return 数据流对象
     */
    friend const QDataStream &operator>>(QDataStream &stream, ServiceFileInfo &service);

private:
    QExplicitlySharedDataPointer<ServiceFileInfoData> m_data;
};

typedef QList<ServiceFileInfo> ServiceFileInfoList;

Q_DECLARE_METATYPE(ServiceFileInfo)
Q_DECLARE_METATYPE(ServiceFileInfoList)

/**
 * @brief operator << 输出ServiceFileInfo对象列表到DBus参数
 * @param argument DBus参数对象
 * @param list 服务文件信息列表
 * @return DBus参数对象
 */
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceFileInfoList &list);

/**
 * @brief operator >> 从DBus参数对象中读取ServiceFileInfo信息列表
 * @param argument DBus参数对象
 * @param list 服务文件信息列表
 * @return DBus参数对象
 */
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceFileInfoList &list);

#endif // SERVICEFILEINFO_H
