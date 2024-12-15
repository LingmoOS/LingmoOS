// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HTTPDRIVERINTERFACE_H
#define HTTPDRIVERINTERFACE_H
#include "commonfunction.h"

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
    /**
     * @brief checkDriverInfo:解析打印机cups第三方库获取的信息
     * @param driverInfo:解析得到的map信息
     * @return 返回值，true：需要更新 false：不需要更新
     */
    bool checkDriverInfo(DriverInfo &driverInfo);

protected:
    explicit HttpDriverInterface(QObject *parent = nullptr);
    virtual ~HttpDriverInterface();

    bool getDriverInfoFromJson(QString strJson, QList<RepoDriverInfo> &lstDriverInfo);//返回值true：转换成功 false：失败
    QString getRequestJson(QString strUrl);//从仓库接口查询获取json字符串
    QString getRequestBoard(QString strManufacturer = "", QString strProducts = "", int iClassP = 0, int iClass = 0);//板卡设备用
    QString getRequestPrinter(QString strDebManufacturer = "", QString strDesc = "");//打印机用
    QString getRequestCamera(QString strDesc = "");//图像设备

    bool checkDriverInfo(QString strJson, DriverInfo &driverInfo);
    bool isPkgInstalled(QString strPkgName, QString strVersion);
public:
signals:
    void sigRequestFinished(bool sucess, QString msg);

private:
    QNetworkAccessManager qnam;
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> reply;

    //QList<strDriverInfo> lstDriverInfo {};
    QString strJsonDriverInfo;

    static std::atomic<HttpDriverInterface *> s_Instance;
    static std::mutex                         m_mutex;
};

#endif // HTTPDRIVERINTERFACE_H
