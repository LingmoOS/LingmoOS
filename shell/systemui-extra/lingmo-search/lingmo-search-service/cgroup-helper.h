#ifndef CGROUPHELPER_H
#define CGROUPHELPER_H
#include <QString>
#include <QtDBus/QDBusInterface>
#include <memory>
#include <functional>
class CgroupHelper
{
private:
    /**
     * @brief getUserPathV2 获取cgroup分组下的用户服务分组绝对路径
     * @param groupPath: 手动调用时传入cgroup根路径"/sys/fs/cgroup"
     * @return user@xxx.service的绝对路径，xxx为用户uid
     */
    static QString getUserPathV2(QString groupPath);
    /**
     * @brief getPath 获取自定义cgroup分组的相对路径
     * @return 自定义cgroup分组的相对路径
     */
    static QString getPath();
    /**
     * @brief getCgroupVerson 获取cgroup的版本，与内核版本有关
     * @return cgroup版本，1：cgroupV1  2：cgroupV2  -1:获取版本错误
     */
    static int getCgroupVerson();
    /**
     * @brief createProcessGroup 创建自定义cgroup分组
     * @return 自定义cgroup分组的相对路径
     */
    static bool createProcessGroup(QDBusInterface *interface);
    /**
     * @brief moveProcessToGroup 将进程pid添加到指定cgroup分组下
     * @param interface: 用于调用lingmo-ProcessManagerDaemon的DBus接口
     * @return true 成功 false 失败
     */
    static bool moveProcessToGroup(QDBusInterface *interface);
    /**
     * @brief setProcessGroupCPULimit 根据cgroup版本设置CPU管控
     * @param interface: 用于调用lingmo-ProcessManagerDaemon的DBus接口
     * @return true 成功 false 失败
     */
    static bool setProcessGroupCPULimit(QDBusInterface *interface);
    /**
     * @brief addProcessManagerWhiteList 将进程ID添加到lingmo-process-manager的白名单中
     *        防止被lingmo-procsss-manager管控而产生冲突
     * @param interface: 用于调用lingmo-ProcessManagerDaemon的DBus接口
     * @return true 成功 false 失败
     */
    static bool addProcessManagerWhiteList();
    /**
     * @brief setProcessGroupCPULimitV2 cgroupV2版本限制进程的CPU占用
     * @param interface: 用于调用lingmo-ProcessManagerDaemon的DBus接口
     * @return true 成功 false 失败
     */
    static bool setProcessGroupCPULimitV2(QDBusInterface *interface);
    /**
     * @brief setProcessGroupCPULimitV1 cgroupV1版本限制进程的CPU占用
     * @param interface: 用于调用lingmo-ProcessManagerDaemon的DBus接口
     * @return true 成功 false 失败
     */
    static bool setProcessGroupCPULimitV1(QDBusInterface *interface);
    //用于调用lingmo-ProcessManagerDaemon的DBus接口的智能指针
    using CgroupUniquePtr = std::unique_ptr<QDBusInterface, std::function<void(QDBusInterface *)>>;
public:
    /**
     * @brief setServiceCpuLimit 对外接口，为lingmo-search-service设置CPU管控
     * @return true 成功 false 失败
     */
    static bool setServiceCpuLimit();
};

#endif // CGROUPHELPER_H
