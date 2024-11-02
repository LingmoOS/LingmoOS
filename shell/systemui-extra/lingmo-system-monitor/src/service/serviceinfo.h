#ifndef SERVICEINFO_H
#define SERVICEINFO_H

#include <QExplicitlySharedDataPointer>
#include <QList>
#include <QtDBus>

class QString;
class ServiceInfoData;

class ServiceInfo
{
public:
    /**
     * @brief ServiceInfo 构造
     */
    ServiceInfo();

    /**
     * @brief ServiceInfo 服务信息构造重载
     * @param name  服务名称
     * @param description 服务描述
     * @param loadState 加载状态
     * @param activeState 活动状态
     * @param subState 子状态
     * @param followedBy 跟随
     * @param serviceObjectPath 服务对象路径
     * @param jobId 服务任务id
     * @param jobType 服务任务类型
     * @param jobObjectPath 服务任务对象路径
     */
    ServiceInfo(const QString &name,
                const QString &description,
                const QString &loadState,
                const QString &activeState,
                const QString &subState,
                const QString &followedBy,
                const QString &serviceObjectPath,
                quint32 jobId,
                const QString &jobType,
                const QString &jobObjectPath);

    /**
     * @brief 拷贝构造
     */
    ServiceInfo(const ServiceInfo &);

    /**
     * @brief operator = 赋值操作
     * @return 赋值对象
     */
    ServiceInfo &operator=(const ServiceInfo &);
    ~ServiceInfo();

    /**
     * @brief operator == 与另一个服务信息对象对比
     * @param other 其他服务信息对象
     * @return 对比结果 true为同
     */
    bool operator==(const ServiceInfo &other) const;

    /**
     * @brief getName 获取服务名称
     * @return 服务名称
     */
    QString getName() const;

    /**
    * @brief setName 设置服务名称
    * @param 服务名称
    */
    void setName(const QString &name);

    /**
     * @brief getDescription 获取服务描述
     * @return 服务描述
     */
    QString getDescription() const;

    /**
     * @brief setDescription 设置服务描述
     * @param description 服务描述
     */
    void setDescription(const QString &description);

    /**
     * @brief getLoadState 获取服务加载状态
     * @return 服务加载状态
     */
    QString getLoadState() const;

    /**
    * @brief setLoadState 设置服务加载状态
    * @param loadState 服务加载状态
    */
    void setLoadState(const QString &loadState);

    /**
     * @brief getActiveState 获取服务活动状态
     * @return 服务活动状态
     */
    QString getActiveState() const;

    /**
    * @brief setActiveState 设置服务活动状态
    * @param activeState 服务活动状态
    */
    void setActiveState(const QString &activeState);

    /**
     * @brief getSubState 获取服务子状态
     * @return 服务子状态
     */
    QString getSubState() const;

    /**
     * @brief setSubState 设置服务子状态
     * @param subState 服务子状态
     */
    void setSubState(const QString &subState);

    /**
     * @brief getFollowedBy 获取服务跟随
     * @return 服务跟随
     */
    QString getFollowedBy() const;

    /**
     * @brief setFollowedBy 设置服务跟随
     * @param followedBy 服务跟随
     */
    void setFollowedBy(const QString &followedBy);

    /**
     * @brief getServiceObjectPath 获取服务对象路径
     * @return 服务对象路径
     */
    QString getServiceObjectPath() const;

    /**
     * @brief setServiceObjectPath 设置服务对象路径
     * @param serviceObjectPath 服务对象路径
     */
    void setServiceObjectPath(const QString &serviceObjectPath);

    /**
     * @brief getMainPID 获取服务主进程id
     * @return 服务主进程id
     */
    quint32 getMainPID() const;

    /**
     * @brief setMainPID 设置服务主进程id
     * @param mainPID 服务主进程id
     */
    void setMainPID(quint32 mainPID);

    /**
     * @brief getJobId 获取服务任务id
     * @return 服务任务id
     */
    quint32 getJobId() const;

    /**
     * @brief setJobId 设置服务任务id
     * @param jobId 服务任务id
     */
    void setJobId(quint32 jobId);

    /**
     * @brief getJobType 获取服务任务类型
     * @return 服务任务类型
     */
    QString getJobType() const;

    /**
     * @brief setJobType 设置服务任务类型
     * @param jobType 服务任务类型
     */
    void setJobType(const QString &jobType);

    /**
     * @brief getJobObjectPath 获取服务任务对象路径
     * @return 服务任务对象路径
     */
    QString getJobObjectPath() const;

    /**
     * @brief setJobObjectPath 设置服务任务对象路径
     * @param jobObjectPath 服务任务对象路径
     */
    void setJobObjectPath(const QString &jobObjectPath);

    /**
     * @brief getIsCanReload 获取服务是否可重新加载
     * @return 服务是否可重新加载
     */
    bool getIsCanReload() const;

    /**
     * @brief setIsCanReload 设置服务是否可重新加载
     * @param canReload 服务是否可重新加载
     */
    void setIsCanReload(bool canReload);

    /**
     * @brief getIsCanStart 获取服务是否可启动
     * @return 服务是否可启动
     */
    bool getIsCanStart() const;

    /**
     * @brief setIsCanStart 设置服务是否可启动
     * @param canStart 服务是否可启动
     */
    void setIsCanStart(bool canStart);

    /**
     * @brief getIsCanStop 获取服务是否可停止
     * @return 服务是否可停止
     */
    bool getIsCanStop() const;

    /**
     * @brief setIsCanStop 设置服务是否可停止
     * @param canStop 服务是否可停止
     */
    void setIsCanStop(bool canStop);

    /**
     * @brief getState 获取服务状态
     * @return 服务状态
     */
    QString getState() const;

    /**
     * @brief setState 设置服务状态
     * @param state 服务状态
     */
    void setState(QString state);

    /**
     * @brief getSName 获取服务短名称
     * @return 服务短名称
     */
    QString getSName() const;

    /**
     * @brief setSName 设置服务短名称
     * @param sName 服务短名称
     */
    void setSName(QString sName);

    /**
     * @brief getID 获取服务ID
     * @return 服务ID
     */
    QString getID() const;

    /**
     * @brief setID 设置服务ID
     * @param strID 服务ID
     */
    void setID(QString strID);

    /**
     * @brief getStartupType 获取服务启动类型
     * @return 服务启动类型
     */
    QString getStartupType() const;

    /**
     * @brief setStartupType 设置服务启动类型
     * @param strStartupType 服务启动类型
     */
    void setStartupType(QString strStartupType);

    /**
     * @brief registerMetaType 注册ServiceInfo元数据类型
     */
    static void registerMetaType();

    /**
     * @brief operator << 输出ServiceInfo信息到debug流
     * @param debug debug流
     * @param unit ServiceInfo对象
     * @return debug流
     */
    friend QDebug &operator<<(QDebug &debug, const ServiceInfo &service);

    /**
     * @brief operator << 转换服务信息至dbus参数
     * @param argument dbus参数
     * @param service 服务信息
     * @return dbus参数
     */
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ServiceInfo &service);

    /**
     * @brief operator << 输出ServiceInfo信息到数据流
     * @param stream 数据流
     * @param service 服务信息
     * @return 数据流
     */
    friend QDataStream &operator<<(QDataStream &stream, const ServiceInfo &service);

    /**
     * @brief operator >> 转换dbus参数至服务信息
     * @param argument dbus参数
     * @param service 服务信息
     * @return dbus参数
     */
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceInfo &service);

    /**
     * @brief operator >> 从数据流读取信息到ServiceInfo
     * @param stream 数据流
     * @param service 服务信息
     * @return 数据流
     */
    friend const QDataStream &operator>>(QDataStream &stream, ServiceInfo &service);

private:
    QExplicitlySharedDataPointer<ServiceInfoData> m_data;
};

typedef QList<ServiceInfo> ServiceInfoList;

Q_DECLARE_METATYPE(ServiceInfo)
Q_DECLARE_METATYPE(ServiceInfoList)

/**
 * @brief operator << 转换服务信息列表至dbus参数
 * @param argument dbus参数
 * @param list 服务信息列表
 * @return dbus参数
 */
QDBusArgument &operator<<(QDBusArgument &argument, const ServiceInfoList &list);

/**
 * @brief operator >> 转换dbus参数至服务信息列表
 * @param argument dbus参数
 * @param list 服务信息列表
 * @return dbus参数
 */
const QDBusArgument &operator>>(const QDBusArgument &argument, ServiceInfoList &list);

#endif // SERVICEINFO_H
