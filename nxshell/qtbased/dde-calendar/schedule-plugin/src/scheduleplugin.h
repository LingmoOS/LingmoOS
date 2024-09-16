// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCHEDULEPLUGIN_H
#define SCHEDULEPLUGIN_H

#include "interface/systemsemanticplugin.h"
#include "interface/reply.h"

#include <QMutex>

class Scheduleplugin : public QObject
    , public IServicePlugin
{
    Q_OBJECT
    Q_INTERFACES(IServicePlugin)
    Q_PLUGIN_METADATA(IID PluginInterface_iid)
public:
    explicit Scheduleplugin(QObject *parent = nullptr);
    ~Scheduleplugin() override;

    int init() override;
    void uninit() override;

    QStringList getSupportService() override;
    IService *createService(const QString &service) override;
    void releaseService(IService *service) override;
    bool needRunInDifferentThread() override;
    /**
     * @brief isAutoDetectSession   是否自动检测多轮对话
     * @return          true 检测  false不检测由插件设置
     */
    bool isAutoDetectSession() override;
public slots:
    void slotSendMessage(Reply reply);

private:
    QSet<IService *> m_ServiceSet;
    QMutex m_ServiceLock;
};

#endif // SCHEDULEPLUGIN_H
