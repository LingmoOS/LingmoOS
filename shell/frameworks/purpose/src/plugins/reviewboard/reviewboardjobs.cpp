/*
    SPDX-FileCopyrightText: 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "reviewboardjobs.h"
#include "debug.h"

#include <QFile>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QMimeType>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

#include <KLocalizedString>
#include <KRandom>

using namespace ReviewBoard;

QByteArray ReviewBoard::urlToData(const QUrl &url)
{
    QByteArray ret;
    if (url.isLocalFile()) {
        QFile f(url.toLocalFile());
        Q_ASSERT(f.exists());
        bool corr = f.open(QFile::ReadOnly | QFile::Text);
        Q_ASSERT(corr);
        Q_UNUSED(corr);

        ret = f.readAll();

    } else {
        // TODO: add downloading the data
    }
    return ret;
}
namespace
{
static const QByteArray m_boundary = "----------" + KRandom::randomString(42 + 13).toLatin1();

QByteArray multipartFormData(const QList<QPair<QString, QVariant>> &values)
{
    QByteArray form_data;
    for (const auto &val : values) {
        QByteArray hstr("--");
        hstr += m_boundary;
        hstr += "\r\n";
        hstr += "Content-Disposition: form-data; name=\"";
        hstr += val.first.toLatin1();
        hstr += "\"";

        // File
        if (val.second.userType() == QMetaType::QUrl) {
            QUrl path = val.second.toUrl();
            hstr += "; filename=\"" + path.fileName().toLatin1() + "\"";
            const QMimeType mime = QMimeDatabase().mimeTypeForUrl(path);
            if (!mime.name().isEmpty()) {
                hstr += "\r\nContent-Type: ";
                hstr += mime.name().toLatin1();
            }
        }
        //

        hstr += "\r\n\r\n";

        // append body
        form_data.append(hstr);
        if (val.second.userType() == QMetaType::QUrl) {
            form_data += urlToData(val.second.toUrl());
        } else {
            form_data += val.second.toByteArray();
        }
        form_data.append("\r\n");
        // EOFILE
    }

    form_data += QByteArray("--" + m_boundary + "--\r\n");

    return form_data;
}

QByteArray multipartFormData(const QVariantMap &values)
{
    QList<QPair<QString, QVariant>> vals;
    for (QVariantMap::const_iterator it = values.constBegin(), itEnd = values.constEnd(); it != itEnd; ++it) {
        vals += qMakePair<QString, QVariant>(QString(it.key()), QVariant(it.value()));
    }
    return multipartFormData(vals);
}

}

HttpCall::HttpCall(const QUrl &s,
                   const QString &apiPath,
                   const QList<QPair<QString, QString>> &queryParameters,
                   Method method,
                   const QByteArray &post,
                   bool multipart,
                   QObject *parent)
    : KJob(parent)
    , m_reply(nullptr)
    , m_post(post)
    , m_multipart(multipart)
    , m_method(method)
{
    m_requrl = s;
    m_requrl.setPath(m_requrl.path() + QLatin1Char('/') + apiPath);
    QUrlQuery query;
    for (QList<QPair<QString, QString>>::const_iterator i = queryParameters.begin(); i < queryParameters.end(); i++) {
        query.addQueryItem(i->first, i->second);
    }
    m_requrl.setQuery(query);
}

void HttpCall::start()
{
    QNetworkRequest r(m_requrl);

    if (!m_requrl.userName().isEmpty()) {
        QByteArray head = "Basic " + m_requrl.userInfo().toLatin1().toBase64();
        r.setRawHeader("Authorization", head);
    }

    if (m_multipart) {
        r.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("multipart/form-data"));
        r.setHeader(QNetworkRequest::ContentLengthHeader, QString::number(m_post.size()));
        r.setRawHeader("Content-Type", "multipart/form-data; boundary=" + m_boundary);
    }

    switch (m_method) {
    case Get:
        m_reply = m_manager.get(r);
        break;
    case Post:
        m_reply = m_manager.post(r, m_post);
        break;
    case Put:
        m_reply = m_manager.put(r, m_post);
        break;
    }
    connect(m_reply, &QNetworkReply::finished, this, &HttpCall::onFinished);

    // qCDebug(PLUGIN_REVIEWBOARD) << "starting... requrl=" << m_requrl << "post=" << m_post;
}

QVariant HttpCall::result() const
{
    Q_ASSERT(m_reply->isFinished());
    return m_result;
}

void HttpCall::onFinished()
{
    const QByteArray receivedData = m_reply->readAll();
    QJsonParseError error;
    QJsonDocument parser = QJsonDocument::fromJson(receivedData, &error);
    const QVariant output = parser.toVariant();

    if (error.error == 0) {
        m_result = output;
    } else {
        setError(1);
        setErrorText(i18n("JSON error: %1", error.errorString()));
    }

    if (output.toMap().value(QStringLiteral("stat")).toString() != QLatin1String("ok")) {
        setError(2);
        setErrorText(i18n("Request Error: %1", output.toMap().value(QStringLiteral("err")).toMap().value(QStringLiteral("msg")).toString()));
    }

    if (receivedData.size() > 10000) {
        qCDebug(PLUGIN_REVIEWBOARD) << "parsing..." << receivedData.size();
    } else {
        qCDebug(PLUGIN_REVIEWBOARD) << "parsing..." << receivedData;
    }
    emitResult();
}

NewRequest::NewRequest(const QUrl &server, const QString &projectPath, QObject *parent)
    : ReviewRequest(server, QString(), parent)
    , m_project(projectPath)
{
    m_newreq = new HttpCall(this->server(), QStringLiteral("/api/review-requests/"), {}, HttpCall::Post, "repository=" + projectPath.toLatin1(), false, this);
    connect(m_newreq, &HttpCall::finished, this, &NewRequest::done);
}

void NewRequest::start()
{
    m_newreq->start();
}

void NewRequest::done()
{
    if (m_newreq->error()) {
        qCDebug(PLUGIN_REVIEWBOARD) << "Could not create the new request" << m_newreq->errorString();
        setError(2);
        setErrorText(i18n("Could not create the new request:\n%1", m_newreq->errorString()));
    } else {
        QVariant res = m_newreq->result();
        setRequestId(res.toMap()[QStringLiteral("review_request")].toMap()[QStringLiteral("id")].toString());
        Q_ASSERT(!requestId().isEmpty());
    }

    emitResult();
}

SubmitPatchRequest::SubmitPatchRequest(const QUrl &server, const QUrl &patch, const QString &basedir, const QString &id, QObject *parent)
    : ReviewRequest(server, id, parent)
    , m_patch(patch)
    , m_basedir(basedir)
{
    QList<QPair<QString, QVariant>> vals;
    vals += QPair<QString, QVariant>(QStringLiteral("basedir"), m_basedir);
    vals += QPair<QString, QVariant>(QStringLiteral("path"), QVariant::fromValue<QUrl>(m_patch));

    m_uploadpatch = new HttpCall(this->server(),
                                 QStringLiteral("/api/review-requests/") + requestId() + QStringLiteral("/diffs/"),
                                 {},
                                 HttpCall::Post,
                                 multipartFormData(vals),
                                 true,
                                 this);
    connect(m_uploadpatch, &HttpCall::finished, this, &SubmitPatchRequest::done);
}

void SubmitPatchRequest::start()
{
    m_uploadpatch->start();
}

void SubmitPatchRequest::done()
{
    if (m_uploadpatch->error()) {
        qCWarning(PLUGIN_REVIEWBOARD) << "Could not upload the patch" << m_uploadpatch->errorString();
        setError(3);
        setErrorText(i18n("Could not upload the patch"));
    }

    emitResult();
}

ProjectsListRequest::ProjectsListRequest(const QUrl &server, QObject *parent)
    : KJob(parent)
    , m_server(server)
{
}

void ProjectsListRequest::start()
{
    requestRepositoryList(0);
}

QVariantList ProjectsListRequest::repositories() const
{
    return m_repositories;
}

void ProjectsListRequest::requestRepositoryList(int startIndex)
{
    QList<QPair<QString, QString>> repositoriesParameters;

    // In practice, the web API will return at most 200 repos per call, so just hardcode that value here
    repositoriesParameters << qMakePair(QStringLiteral("max-results"), QStringLiteral("200"));
    repositoriesParameters << qMakePair(QStringLiteral("start"), QString::number(startIndex));

    HttpCall *repositoriesCall = new HttpCall(m_server, QStringLiteral("/api/repositories/"), repositoriesParameters, HttpCall::Get, QByteArray(), false, this);
    connect(repositoriesCall, &HttpCall::finished, this, &ProjectsListRequest::done);

    repositoriesCall->start();
}

void ProjectsListRequest::done(KJob *job)
{
    // TODO error
    // TODO max iterations
    HttpCall *repositoriesCall = qobject_cast<HttpCall *>(job);
    const QMap<QString, QVariant> resultMap = repositoriesCall->result().toMap();
    const int totalResults = resultMap[QStringLiteral("total_results")].toInt();
    m_repositories << resultMap[QStringLiteral("repositories")].toList();

    if (m_repositories.count() < totalResults) {
        requestRepositoryList(m_repositories.count());
    } else {
        emitResult();
    }
}

ReviewListRequest::ReviewListRequest(const QUrl &server, const QString &user, const QString &reviewStatus, QObject *parent)
    : KJob(parent)
    , m_server(server)
    , m_user(user)
    , m_reviewStatus(reviewStatus)
{
}

void ReviewListRequest::start()
{
    requestReviewList(0);
}

QVariantList ReviewListRequest::reviews() const
{
    return m_reviews;
}

void ReviewListRequest::requestReviewList(int startIndex)
{
    QList<QPair<QString, QString>> reviewParameters;

    // In practice, the web API will return at most 200 repos per call, so just hardcode that value here
    reviewParameters << qMakePair(QStringLiteral("max-results"), QStringLiteral("200"));
    reviewParameters << qMakePair(QStringLiteral("start"), QString::number(startIndex));
    reviewParameters << qMakePair(QStringLiteral("from-user"), m_user);
    reviewParameters << qMakePair(QStringLiteral("status"), m_reviewStatus);

    HttpCall *reviewsCall = new HttpCall(m_server, QStringLiteral("/api/review-requests/"), reviewParameters, HttpCall::Get, QByteArray(), false, this);
    connect(reviewsCall, &HttpCall::finished, this, &ReviewListRequest::done);

    reviewsCall->start();
}

void ReviewListRequest::done(KJob *job)
{
    // TODO error
    // TODO max iterations
    if (job->error()) {
        qCDebug(PLUGIN_REVIEWBOARD) << "Could not get reviews list" << job->errorString();
        setError(3);
        setErrorText(i18n("Could not get reviews list"));
        emitResult();
    }

    HttpCall *reviewsCall = qobject_cast<HttpCall *>(job);
    QMap<QString, QVariant> resultMap = reviewsCall->result().toMap();
    const int totalResults = resultMap[QStringLiteral("total_results")].toInt();

    m_reviews << resultMap[QStringLiteral("review_requests")].toList();

    if (m_reviews.count() < totalResults) {
        requestReviewList(m_reviews.count());
    } else {
        emitResult();
    }
}

UpdateRequest::UpdateRequest(const QUrl &server, const QString &id, const QVariantMap &newValues, QObject *parent)
    : ReviewRequest(server, id, parent)
{
    m_req = new HttpCall(this->server(),
                         QStringLiteral("/api/review-requests/") + id + QStringLiteral("/draft/"),
                         {},
                         HttpCall::Put,
                         multipartFormData(newValues),
                         true,
                         this);
    connect(m_req, &HttpCall::finished, this, &UpdateRequest::done);
}

void UpdateRequest::start()
{
    m_req->start();
}

void UpdateRequest::done()
{
    if (m_req->error()) {
        qCWarning(PLUGIN_REVIEWBOARD) << "Could not set all metadata to the review" << m_req->errorString() << m_req->property("result");
        setError(3);
        setErrorText(i18n("Could not set metadata"));
    }

    emitResult();
}

#include "moc_reviewboardjobs.cpp"
