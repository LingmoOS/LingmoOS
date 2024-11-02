#ifndef SERVICEDATAWORKER_H
#define SERVICEDATAWORKER_H

#include "serviceinfo.h"

#include <QObject>
#include <QList>

class ServiceDataWorker : public QObject
{
    Q_OBJECT
public:
    explicit ServiceDataWorker(QObject *parent = nullptr);

public slots:
    /**
     * @brief refreshServiceList 刷新服务列表
     */
    void refreshServiceList();

signals:
    /**
     * @brief serviceListReady 刷新后的服务列表
     * @param list 服务列表
     */
    void serviceListReady(ServiceInfoList list);

private:
    /**
     * @brief readServiceDescriptionFromServiceFile 从服务文件中读取服务描述信息
     * @param path 服务文件路径
     * @return 服务描述信息
     */
    static QString readServiceDescriptionFromServiceFile(const QString &path);
};

#endif // SERVICEDATAWORKER_H
