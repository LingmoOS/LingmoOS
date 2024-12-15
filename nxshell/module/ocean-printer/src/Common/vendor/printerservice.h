// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICE_H
#define SERVICE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>

#define SD_KEY_from "from"
#define SD_KEY_make_model "describe"
#define SD_KEY_driver "driver"
#define SD_KEY_excat "excat"
#define SD_KEY_ppd "ppd"
#define SD_KEY_sid "sid"
#define SD_KEY_ver "version"
#define SD_KEY_debver "deb_version"
#define SD_KEY_code "client_code"
#define SD_KEY_mfg "MFG"
#define SD_KEY_mdl "MDL"
#define SD_KEY_ieeeid "ieee1284_id"
#define SD_KEY_suc "success"
#define SD_KEY_rid "id"
#define SD_KEY_reason "reason"
#define SD_KEY_feedback "feedback"
#define SD_KEY_detail "detail"
#define SD_KEY_package "package"
#define SD_KEY_recommended "is_recommend"

class PrinterServerInterface : public QObject
{
    Q_OBJECT

public:
    void getFromServer();
    void postToServer();

signals:
    void signalDone(int, QByteArray);

protected:
    PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject *parent = nullptr);

private:
    QNetworkReply *get_request(const QString &path);
    QNetworkReply *post_request(const QString &path, const QJsonObject &obj);

    QString m_url;
    QJsonObject m_args;

    friend class PrinterService;
};

class PrinterService : public QObject
{
    Q_OBJECT

public:
    static PrinterService *getInstance();

    bool isInvaild();

    PrinterServerInterface *feedbackResult(int solution_id, bool success,
                                           const QString &reason = "", const QString &feedback = "", int record_id = 0);
    PrinterServerInterface *searchDriverSolution(const QString &manufacturer, const QString &model,
                                                 const QString &ieee1284_id = "");


protected:
    PrinterService();

private:
    QString m_osVersion;
    QString m_hostname;
    unsigned short m_port;
    QString m_urlPrefix;
    QString m_version;
    QString m_code;
    QString m_urlDriver;
};

#define g_printerServer PrinterService::getInstance()

#endif
