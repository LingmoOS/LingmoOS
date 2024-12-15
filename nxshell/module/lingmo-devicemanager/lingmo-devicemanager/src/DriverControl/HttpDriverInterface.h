// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HTTPDRIVERINTERFACE_H
#define HTTPDRIVERINTERFACE_H

#include "MacroDefinition.h"

#include <QObject>
#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QUrl>

#include <mutex>

class HttpDriverInterface : public QObject
{
    Q_OBJECT
public:
    inline static HttpDriverInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        HttpDriverInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new HttpDriverInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }
    void getRequest(DriverInfo *driverInfo);

    bool convertJsonToDeviceList(QString strJson, QList<RepoDriverInfo> &lstDriverInfo);
protected:
    explicit HttpDriverInterface(QObject* parent = nullptr);
    virtual ~HttpDriverInterface();
    QString getRequestJson(QString strUrl);//从仓库接口查询获取json字符串

    QString getRequestBoard(QString strManufacturer = "", QString strModels = "", int iClassP = 0, int iClass = 0);//板卡设备用
    QString getRequestPrinter(QString strDebManufacturer = "", QString strDesc = "");//打印机用
    QString getRequestCamera(QString strDesc = "");//图像设备
    void checkDriverInfo(QString strJson, DriverInfo *driverInfo);

private:
    int packageInstall(const QString& package_name, const QString& version);
    QString getOsBuild();
public:
    signals:
    void sigRequestFinished(bool sucess, QString msg);

private:
    QString strJsonDriverInfo;
    static std::atomic<HttpDriverInterface *> s_Instance;
    static std::mutex                         m_mutex;
};

#endif // HTTPDRIVERINTERFACE_H
