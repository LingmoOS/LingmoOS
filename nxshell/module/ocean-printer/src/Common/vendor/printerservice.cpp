// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printerservice.h"
#include "common.h"
#include "config.h"
#include "zsettings.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <DLog>

static QNetworkAccessManager g_networkManager;

DCORE_USE_NAMESPACE
PrinterServerInterface::PrinterServerInterface(const QString &url, const QJsonObject &obj, QObject *parent)
    : QObject(parent)
{
    m_url = url;
    m_args = obj;
}

void PrinterServerInterface::postToServer()
{
    QNetworkReply *reply = post_request(m_url, m_args);

    /*QNetworkReply默认超时时间太长，这里暂定设置为10s*/
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    if (timer.isActive()) {
        /*正常返回*/
        timer.stop();
    } else {
        /*timeout*/
        disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        qCWarning(COMMONMOUDLE) << "timeout(10s)";
    }
    if (reply->error() != QNetworkReply::NoError)
        qCWarning(COMMONMOUDLE) << reply->error();
    QByteArray data = "";
    if (reply->isOpen())
        data = reply->readAll();
    emit signalDone(reply->error(), data);
    reply->deleteLater();
}

QNetworkReply *PrinterServerInterface::post_request(const QString &path, const QJsonObject &obj)
{
    QUrl url = path;
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = g_networkManager.post(request,
                                                 QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return reply;
}

PrinterService *PrinterService::getInstance()
{
    static PrinterService *instance = nullptr;
    if (nullptr == instance)
        instance = new PrinterService();

    return instance;
}

bool PrinterService::isInvaild()
{
    return m_osVersion.split("-").count() != 2;
}

PrinterService::PrinterService()
{
    m_osVersion = g_Settings->getOSVersion();
    m_urlDriver = g_Settings->getDriverPlatformUrl() +
                  QString("?arch=%1&system=%2&majorVersion=%3&minorVersion=%4")
                          .arg(g_Settings->getSystemArch())
                          .arg(g_Settings->getSysInfo())
                          .arg(g_Settings->majorVersion())
                          .arg(g_Settings->minorVersion());
    if (isInvaild())
        return;

    m_hostname = g_Settings->getHostName();
    m_port = g_Settings->getHostPort();
    m_version = g_Settings->getClientVersion();
    m_code = g_Settings->getClientCode();

    QStringList osargs = m_osVersion.split("-");
}

void PrinterServerInterface::getFromServer()
{
    QNetworkReply *reply = get_request(m_url);

    /* QNetworkReply默认超时时间太长，这里暂定设置为10s */
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    timer.start(10000);
    loop.exec();

    if (timer.isActive()) {
        /* 正常返回 */
        timer.stop();
    } else {
        /* timeout */
        disconnect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        qCWarning(COMMONMOUDLE) << "timeout(10s)";
    }
    if (reply->error() != QNetworkReply::NoError)
        qCWarning(COMMONMOUDLE) << reply->error();
    QByteArray data = "";
    if (reply->isOpen())
        data = reply->readAll();
    emit signalDone(reply->error(), data);
    reply->deleteLater();
}

QNetworkReply *PrinterServerInterface::get_request(const QString &path)
{
   QUrl url = path;
   QNetworkRequest request(url);
   request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

   QNetworkReply *reply = g_networkManager.get(request);
   return reply;
}

PrinterServerInterface *PrinterService::searchDriverSolution(const QString &manufacturer,
                                                             const QString &model, const QString &ieee1284_id)
{
    QJsonObject obj;

    QString info = QString("&deb_manufacturer=%1&desc=%1 %2").arg(manufacturer).arg(model);
    if (manufacturer == "HP" || manufacturer == "Hewlett-Packard") {
        info = QString("&deb_manufacturer=HP&desc=HP %1").arg(model);
    }
    QString ieee1284 = ieee1284_id.toUtf8().toBase64();
    QString urlDriver = m_urlDriver + info + "&ieee1284=" + ieee1284;
    qCDebug(COMMONMOUDLE) << "Request urlinfo: " << urlDriver;
    PrinterServerInterface *reply = new PrinterServerInterface(urlDriver, obj);

    return reply;
}

PrinterServerInterface *PrinterService::feedbackResult(int solution_id, bool success,
                                                       const QString &reason, const QString &feedback, int record_id)
{
    QJsonObject obj = {
        QPair<QString, QJsonValue>(SD_KEY_sid, solution_id),
        QPair<QString, QJsonValue>(SD_KEY_suc, success),
    };
    if (record_id)
        obj.insert(SD_KEY_rid, record_id);

    QJsonObject detail;
    if (!reason.isEmpty())
        detail.insert(SD_KEY_reason, reason);
    if (!feedback.isEmpty())
        detail.insert(SD_KEY_feedback, feedback);
    if (!detail.isEmpty())
        obj.insert(SD_KEY_detail, detail);

    if (isInvaild())
        return nullptr;

    // todo: source changed
    return nullptr;
}
