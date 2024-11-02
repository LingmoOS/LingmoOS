/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "chat.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTextCodec>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonArray>

static const QString ERNIEBOT("https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/completions");
static const QString ERNIEBOT_TURBO("https://aip.baidubce.com/rpc/2.0/ai_custom/v1/wenxinworkshop/chat/eb-instant");
static const QString GET_TOKEN_URL("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials");
class ChatPrivate: public QObject {
    friend class Chat;
    Q_OBJECT
    explicit ChatPrivate(QObject *parent = nullptr);

private Q_SLOTS:
    void onGetAccessTokenReply(QNetworkReply *reply);
    void onResponse(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_nm = nullptr;
    QString m_accessToken;
    QDateTime m_tokenGetTime;
    int m_tokenExpiresIn = 0;
    QJsonArray m_cache;
    bool m_isWaitingForToken = false;
    bool m_isWaitingForResponse = false;
    Chat *q = nullptr;

};

ChatPrivate::ChatPrivate(QObject *parent) : QObject(parent)
{
    q = qobject_cast<Chat *>(parent);
}
void ChatPrivate::onGetAccessTokenReply(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError) {
        Q_EMIT q->error(reply->errorString());
        qWarning() << reply->errorString();
        m_isWaitingForToken = false;
        return;
    }

    QJsonParseError errRpt{};
    QJsonDocument jsonDocument(QJsonDocument::fromJson(reply->readAll(), &errRpt));
    if (errRpt.error == QJsonParseError::NoError) {
        QJsonObject replyObject = jsonDocument.object();
        QString error = replyObject.value("error").toString();
        if(!error.isEmpty()) {
            qWarning() << "Get access token error:" << error << replyObject.value("error_description").toString();
            Q_EMIT q->error(replyObject.value("error_description").toString());
        } else {
            m_accessToken = replyObject.value("access_token").toString();
            m_tokenExpiresIn = replyObject.value("expires_in").toInt();
            m_tokenGetTime = QDateTime::currentDateTime();
            qDebug() << m_accessToken << m_tokenExpiresIn << m_tokenGetTime;
            connect(m_nm, &QNetworkAccessManager::finished, this, &ChatPrivate::onResponse);
            Q_EMIT q->readyToChat();
        }
    } else {
        Q_EMIT q->error(errRpt.errorString());
        qWarning() << "Parse error at GetAccessTokenReply" << errRpt.errorString();
    }
    reply->deleteLater();
    m_isWaitingForToken = false;
}


void ChatPrivate::onResponse(QNetworkReply *reply)
{
    if(reply->error() != QNetworkReply::NoError)  {
        qWarning() << reply->errorString();
        m_cache.removeLast();
        Q_EMIT q->error(reply->errorString());
    } else {
        QJsonParseError errRpt{};
        QJsonDocument jsonDocument(QJsonDocument::fromJson(reply->readAll(), &errRpt));
        if (errRpt.error == QJsonParseError::NoError) {
            QJsonObject replyObject = jsonDocument.object();
            QString error = replyObject.value("error_code").toString();
            if(!error.isEmpty()) {
                qWarning() << "Response error:" << error << replyObject.value("error_msg").toString();
                m_cache.removeLast();
                Q_EMIT q->error(replyObject.value("error_msg").toString());
            } else {
                const QString &result = replyObject.value("result").toString();
                QJsonObject tmp;
                tmp.insert("content", result);
                tmp.insert("role", "assistant");
                m_cache.append(tmp);
                Q_EMIT q->response(result);
            }
        } else {
            m_cache.removeLast();
            Q_EMIT q->error(errRpt.errorString());
            qWarning() << "Parse error at onResponse" << errRpt.errorString();
        }
    }
    m_isWaitingForResponse = false;
    reply->deleteLater();
}
Chat::Chat(QObject *parent) : QObject(parent), d(new ChatPrivate(this))
{
    d->m_nm = new QNetworkAccessManager(this);
    connect(d->m_nm, &QNetworkAccessManager::finished, d, &ChatPrivate::onResponse);
}

void Chat::newChat(const QString &clientId, const QString &clientSecret)
{
    if(d->m_isWaitingForToken) {
        return;
    }
    disconnect(d->m_nm, &QNetworkAccessManager::finished, d, &ChatPrivate::onResponse);
    QNetworkRequest request(QUrl(GET_TOKEN_URL + "&client_id=" + clientId + "&client_secret=" + clientSecret));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = d->m_nm->post(request, "");
    d->m_isWaitingForToken = true;
    QObject::connect(reply, &QNetworkReply::finished, [reply, this](){
        d->onGetAccessTokenReply(reply);
    });
}

bool Chat::isValid() const
{
    return d->m_tokenExpiresIn >= d->m_tokenGetTime.secsTo(QDateTime::currentDateTime());
}

void Chat::clear()
{
    QJsonArray tmp;
    d->m_cache.swap(tmp);
}

void Chat::talk(const QString &content)
{
    if(d->m_isWaitingForResponse) {
        qWarning() << "Waiting for response, please try later.";
        return;
    }
    if(!isValid()) {
        Q_EMIT this->expired();
        return;
    }
    QJsonObject tmp;
    tmp.insert("content", content);
    tmp.insert("role", "user");
    d->m_cache.append(tmp);

    QNetworkRequest request(ERNIEBOT + "?access_token=" + d->m_accessToken);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject body;
    body.insert("messages", d->m_cache);
    qDebug() << "post"<< QJsonDocument(body).toJson();
    d->m_nm->post(request, QJsonDocument(body).toJson());
    d->m_isWaitingForResponse = true;
}

#include "chat.moc"
