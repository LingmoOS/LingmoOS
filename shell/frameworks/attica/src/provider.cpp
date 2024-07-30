/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>
    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "provider.h"

#include "accountbalance.h"
#include "accountbalanceparser.h"
#include "achievementparser.h"
#include "activity.h"
#include "activityparser.h"
#include "buildservice.h"
#include "buildservicejob.h"
#include "buildservicejoboutput.h"
#include "buildservicejoboutputparser.h"
#include "buildservicejobparser.h"
#include "buildserviceparser.h"
#include "categoryparser.h"
#include "commentparser.h"
#include "config.h"
#include "content.h"
#include "contentparser.h"
#include "distributionparser.h"
#include "downloaditem.h"
#include "downloaditemparser.h"
#include "event.h"
#include "eventparser.h"
#include "folder.h"
#include "folderparser.h"
#include "forumparser.h"
#include "homepagetype.h"
#include "homepagetypeparser.h"
#include "knowledgebaseentry.h"
#include "knowledgebaseentryparser.h"
#include "licenseparser.h"
#include "messageparser.h"
#include "person.h"
#include "personparser.h"
#include "platformdependent_v2.h"
#include "postfiledata.h"
#include "postjob.h"
#include "privatedata.h"
#include "privatedataparser.h"
#include "project.h"
#include "projectparser.h"
#include "publisher.h"
#include "publisherfield.h"
#include "publisherfieldparser.h"
#include "publisherparser.h"
#include "remoteaccount.h"
#include "remoteaccountparser.h"
#include "topic.h"
#include "topicparser.h"
#include "version.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThreadStorage>
#include <QUrlQuery>

using namespace Attica;

class Q_DECL_HIDDEN Provider::Private : public QSharedData
{
public:
    QUrl m_baseUrl;
    QUrl m_icon;
    QString m_name;
    QString m_credentialsUserName;
    QString m_credentialsPassword;
    QString m_personVersion;
    QString m_friendVersion;
    QString m_messageVersion;
    QString m_achievementVersion;
    QString m_activityVersion;
    QString m_contentVersion;
    QString m_fanVersion;
    QString m_forumVersion;
    QString m_knowledgebaseVersion;
    QString m_eventVersion;
    QString m_commentVersion;
    QString m_registerUrl;
    PlatformDependent *m_internals;
    QString m_additionalAgentInformation;

    Private()
        : m_internals(nullptr)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
        , m_baseUrl(other.m_baseUrl)
        , m_name(other.m_name)
        , m_credentialsUserName(other.m_credentialsUserName)
        , m_credentialsPassword(other.m_credentialsPassword)
        , m_personVersion(other.m_personVersion)
        , m_friendVersion(other.m_friendVersion)
        , m_messageVersion(other.m_messageVersion)
        , m_achievementVersion(other.m_achievementVersion)
        , m_activityVersion(other.m_activityVersion)
        , m_contentVersion(other.m_contentVersion)
        , m_fanVersion(other.m_fanVersion)
        , m_forumVersion(other.m_forumVersion)
        , m_knowledgebaseVersion(other.m_knowledgebaseVersion)
        , m_eventVersion(other.m_eventVersion)
        , m_commentVersion(other.m_commentVersion)
        , m_registerUrl(other.m_registerUrl)
        , m_internals(other.m_internals)
        , m_additionalAgentInformation(other.m_additionalAgentInformation)
    {
    }

    Private(PlatformDependent *internals,
            const QUrl &baseUrl,
            const QString &name,
            const QUrl &icon,
            const QString &person,
            const QString &friendV,
            const QString &message,
            const QString &achievement,
            const QString &activity,
            const QString &content,
            const QString &fan,
            const QString &forum,
            const QString &knowledgebase,
            const QString &event,
            const QString &comment,
            const QString &registerUrl,
            const QString &additionalAgentInformation)
        : m_baseUrl(baseUrl)
        , m_icon(icon)
        , m_name(name)
        , m_personVersion(person)
        , m_friendVersion(friendV)
        , m_messageVersion(message)
        , m_achievementVersion(achievement)
        , m_activityVersion(activity)
        , m_contentVersion(content)
        , m_fanVersion(fan)
        , m_forumVersion(forum)
        , m_knowledgebaseVersion(knowledgebase)
        , m_eventVersion(event)
        , m_commentVersion(comment)
        , m_registerUrl(registerUrl)
        , m_internals(internals)
        , m_additionalAgentInformation(additionalAgentInformation)
    {
        if (m_baseUrl.isEmpty()) {
            return;
        }
        QString user;
        QString pass;
        if (m_internals->hasCredentials(m_baseUrl) && m_internals->loadCredentials(m_baseUrl, user, pass)) {
            m_credentialsUserName = user;
            m_credentialsPassword = pass;
        }
    }

    ~Private()
    {
    }
};

Provider::Provider()
    : d(new Private)
{
}

Provider::Provider(const Provider &other)
    : d(other.d)
{
}

Provider::Provider(PlatformDependent *internals,
                   const QUrl &baseUrl,
                   const QString &name,
                   const QUrl &icon,
                   const QString &person,
                   const QString &friendV,
                   const QString &message,
                   const QString &achievement,
                   const QString &activity,
                   const QString &content,
                   const QString &fan,
                   const QString &forum,
                   const QString &knowledgebase,
                   const QString &event,
                   const QString &comment)
    : d(new Private(internals,
                    baseUrl,
                    name,
                    icon,
                    person,
                    friendV,
                    message,
                    achievement,
                    activity,
                    content,
                    fan,
                    forum,
                    knowledgebase,
                    event,
                    comment,
                    QString(),
                    QString()))
{
}

Provider::Provider(PlatformDependent *internals,
                   const QUrl &baseUrl,
                   const QString &name,
                   const QUrl &icon,
                   const QString &person,
                   const QString &friendV,
                   const QString &message,
                   const QString &achievement,
                   const QString &activity,
                   const QString &content,
                   const QString &fan,
                   const QString &forum,
                   const QString &knowledgebase,
                   const QString &event,
                   const QString &comment,
                   const QString &registerUrl)
    : d(new Private(internals,
                    baseUrl,
                    name,
                    icon,
                    person,
                    friendV,
                    message,
                    achievement,
                    activity,
                    content,
                    fan,
                    forum,
                    knowledgebase,
                    event,
                    comment,
                    registerUrl,
                    QString()))
{
}

Provider::Provider(PlatformDependent *internals,
                   const QUrl &baseUrl,
                   const QString &name,
                   const QUrl &icon,
                   const QString &person,
                   const QString &friendV,
                   const QString &message,
                   const QString &achievement,
                   const QString &activity,
                   const QString &content,
                   const QString &fan,
                   const QString &forum,
                   const QString &knowledgebase,
                   const QString &event,
                   const QString &comment,
                   const QString &registerUrl,
                   const QString &additionalAgentInformation)
    : d(new Private(internals,
                    baseUrl,
                    name,
                    icon,
                    person,
                    friendV,
                    message,
                    achievement,
                    activity,
                    content,
                    fan,
                    forum,
                    knowledgebase,
                    event,
                    comment,
                    registerUrl,
                    additionalAgentInformation))
{
}

Provider &Provider::operator=(const Attica::Provider &other)
{
    d = other.d;
    return *this;
}

Provider::~Provider()
{
}

QUrl Provider::baseUrl() const
{
    return d->m_baseUrl;
}

bool Provider::isValid() const
{
    return d->m_baseUrl.isValid();
}

bool Provider::isEnabled() const
{
    if (!isValid()) {
        return false;
    }

    return d->m_internals->isEnabled(d->m_baseUrl);
}

void Provider::setEnabled(bool enabled)
{
    if (!isValid()) {
        return;
    }

    d->m_internals->enableProvider(d->m_baseUrl, enabled);
}

void Provider::setAdditionalAgentInformation(const QString &additionalInformation)
{
    d->m_additionalAgentInformation = additionalInformation;
}

QString Provider::additionalAgentInformation() const
{
    return d->m_additionalAgentInformation;
}

QString Provider::name() const
{
    return d->m_name;
}

QUrl Attica::Provider::icon() const
{
    return d->m_icon;
}

bool Provider::hasCredentials()
{
    if (!isValid()) {
        return false;
    }

    return d->m_internals->hasCredentials(d->m_baseUrl);
}

bool Provider::hasCredentials() const
{
    if (!isValid()) {
        return false;
    }

    return d->m_internals->hasCredentials(d->m_baseUrl);
}

bool Provider::loadCredentials(QString &user, QString &password)
{
    if (!isValid()) {
        return false;
    }

    if (d->m_internals->loadCredentials(d->m_baseUrl, user, password)) {
        d->m_credentialsUserName = user;
        d->m_credentialsPassword = password;
        return true;
    }
    return false;
}

bool Provider::saveCredentials(const QString &user, const QString &password)
{
    if (!isValid()) {
        return false;
    }

    d->m_credentialsUserName = user;
    d->m_credentialsPassword = password;
    return d->m_internals->saveCredentials(d->m_baseUrl, user, password);
}

PostJob *Provider::checkLogin(const QString &user, const QString &password)
{
    if (!isValid()) {
        return nullptr;
    }

    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String("login"), user);
    postParameters.insert(QLatin1String("password"), password);

    return new PostJob(d->m_internals, createRequest(QLatin1String("person/check")), postParameters);
}

ItemJob<Config> *Provider::requestConfig()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("config"));
    return doRequestConfig(url);
}

PostJob *Provider::registerAccount(const QString &id, const QString &password, const QString &mail, const QString &firstName, const QString &lastName)
{
    if (!isValid()) {
        return nullptr;
    }

    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String("login"), id);
    postParameters.insert(QLatin1String("password"), password);
    postParameters.insert(QLatin1String("firstname"), firstName);
    postParameters.insert(QLatin1String("lastname"), lastName);
    postParameters.insert(QLatin1String("email"), mail);

    return new PostJob(d->m_internals, createRequest(QLatin1String("person/add")), postParameters);
}

const QString &Provider::getRegisterAccountUrl() const
{
    return d->m_registerUrl;
}

ItemJob<Person> *Provider::requestPerson(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("person/data/") + id);
    return doRequestPerson(url);
}

ItemJob<Person> *Provider::requestPersonSelf()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("person/self"));
    return doRequestPerson(url);
}

ItemJob<AccountBalance> *Provider::requestAccountBalance()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("person/balance"));
    return doRequestAccountBalance(url);
}

ListJob<Person> *Provider::requestPersonSearchByName(const QString &name)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("person/data"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("name"), name);
    url.setQuery(q);
    return doRequestPersonList(url);
}

ListJob<Person> *Provider::requestPersonSearchByLocation(qreal latitude, qreal longitude, qreal distance, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("person/data"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("latitude"), QString::number(latitude));
    q.addQueryItem(QStringLiteral("longitude"), QString::number(longitude));
    if (distance > 0.0) {
        q.addQueryItem(QStringLiteral("distance"), QString::number(distance));
    }
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestPersonList(url);
}

ListJob<Person> *Provider::requestFriends(const QString &id, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("friend/data/") + id);
    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("page"), QString::number(page));
    q.addQueryItem(QLatin1String("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestPersonList(url);
}

ListJob<Person> *Provider::requestSentInvitations(int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("friend/sentinvitations"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestPersonList(url);
}

ListJob<Person> *Provider::requestReceivedInvitations(int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("friend/receivedinvitations"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestPersonList(url);
}

ListJob<Achievement> *Provider::requestAchievements(const QString &contentId, const QString &achievementId, const QString &userId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("achievements/content/") + contentId + achievementId);
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("user_id"), userId);
    url.setQuery(q);
    return doRequestAchievementList(url);
}

ItemPostJob<Achievement> *Provider::addNewAchievement(const QString &contentId, const Achievement &newAchievement)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    int i = 0;
    int j = 0;

    postParameters.insert(QLatin1String("name"), newAchievement.name());
    postParameters.insert(QLatin1String("description"), newAchievement.description());
    postParameters.insert(QLatin1String("explanation"), newAchievement.explanation());
    postParameters.insert(QLatin1String("points"), QString::number(newAchievement.points()));
    postParameters.insert(QLatin1String("image"), newAchievement.image().toLocalFile());
    const auto dependenciesList = newAchievement.dependencies();
    for (const QString &dependency : dependenciesList) {
        postParameters.insert(QString::fromLatin1("dependencies[%1]").arg(QString::number(i++)), dependency);
    }

    postParameters.insert(QLatin1String("type"), Achievement::achievementTypeToString(newAchievement.type()));
    const auto optionsList = newAchievement.options();
    for (const QString &option : optionsList) {
        postParameters.insert(QString::fromLatin1("options[%1]").arg(QString::number(j++)), option);
    }

    postParameters.insert(QLatin1String("steps"), QString::number(newAchievement.steps()));
    postParameters.insert(QLatin1String("visibility"), Achievement::achievementVisibilityToString(newAchievement.visibility()));

    return new ItemPostJob<Achievement>(d->m_internals, createRequest(QLatin1String("achievements/content/") + contentId), postParameters);
}

PutJob *Provider::editAchievement(const QString &contentId, const QString &achievementId, const Achievement &achievement)
{
    if (!isValid()) {
        return nullptr;
    }

    if (!dynamic_cast<Attica::PlatformDependentV2 *>(d->m_internals)) {
        return nullptr;
    }

    StringMap postParameters;
    int i = 0;
    int j = 0;

    postParameters.insert(QLatin1String("name"), achievement.name());
    postParameters.insert(QLatin1String("description"), achievement.description());
    postParameters.insert(QLatin1String("explanation"), achievement.explanation());
    postParameters.insert(QLatin1String("points"), QString::number(achievement.points()));
    postParameters.insert(QLatin1String("image"), achievement.image().toLocalFile());
    const auto dependenciesList = achievement.dependencies();
    for (const QString &dependency : dependenciesList) {
        postParameters.insert(QString::fromLatin1("dependencies[%1]").arg(QString::number(i++)), dependency);
    }

    postParameters.insert(QLatin1String("type"), Achievement::achievementTypeToString(achievement.type()));
    const auto optionsList = achievement.options();
    for (const QString &option : optionsList) {
        postParameters.insert(QString::fromLatin1("options[%1]").arg(QString::number(j++)), option);
    }

    postParameters.insert(QLatin1String("steps"), QString::number(achievement.steps()));
    postParameters.insert(QLatin1String("visibility"), Achievement::achievementVisibilityToString(achievement.visibility()));

    return new ItemPutJob<Achievement>(d->m_internals, createRequest(QLatin1String("achievement/content/") + contentId + achievementId), postParameters);
}

DeleteJob *Provider::deleteAchievement(const QString &contentId, const QString &achievementId)
{
    if (!isValid()) {
        return nullptr;
    }

    if (!dynamic_cast<Attica::PlatformDependentV2 *>(d->m_internals)) {
        return nullptr;
    }

    return new ItemDeleteJob<Achievement>(d->m_internals, createRequest(QLatin1String("achievements/progress/") + contentId + achievementId));
}

PostJob *Provider::setAchievementProgress(const QString &id, const QVariant &progress, const QDateTime &timestamp)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;

    postParameters.insert(QLatin1String("progress"), progress.toString());
    postParameters.insert(QLatin1String("timestamp"), timestamp.toString());

    return new ItemPostJob<Achievement>(d->m_internals, createRequest(QLatin1String("achievements/progress/") + id), postParameters);
}

DeleteJob *Provider::resetAchievementProgress(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    if (!dynamic_cast<Attica::PlatformDependentV2 *>(d->m_internals)) {
        return nullptr;
    }

    return new ItemDeleteJob<Achievement>(d->m_internals, createRequest(QLatin1String("achievements/progress/") + id));
}

ListJob<Activity> *Provider::requestActivities()
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request activity";
    QUrl url = createUrl(QLatin1String("activity"));
    return doRequestActivityList(url);
}

ListJob<Project> *Provider::requestProjects()
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request projects";
    QUrl url = createUrl(QLatin1String("buildservice/project/list"));
    return new ListJob<Project>(d->m_internals, createRequest(url));
}

ItemJob<Project> *Provider::requestProject(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/project/get/") + id);
    // qCDebug(ATTICA) << url;
    return new ItemJob<Project>(d->m_internals, createRequest(url));
}

QMap<QString, QString> projectPostParameters(const Project &project)
{
    QMap<QString, QString> postParameters;

    if (!project.name().isEmpty()) {
        postParameters.insert(QLatin1String("name"), project.name());
    }
    if (!project.summary().isEmpty()) {
        postParameters.insert(QLatin1String("summary"), project.summary());
    }
    if (!project.description().isEmpty()) {
        postParameters.insert(QLatin1String("description"), project.description());
    }
    if (!project.url().isEmpty()) {
        postParameters.insert(QLatin1String("url"), project.url());
    }
    if (!project.developers().isEmpty()) {
        postParameters.insert(QLatin1String("developers"), project.developers().join(QLatin1Char('\n')));
    }
    if (!project.version().isEmpty()) {
        postParameters.insert(QLatin1String("version"), project.version());
    }
    if (!project.license().isEmpty()) {
        postParameters.insert(QLatin1String("license"), project.license());
    }
    if (!project.requirements().isEmpty()) {
        postParameters.insert(QLatin1String("requirements"), project.requirements());
    }
    // The specfile generator expects an empty string parameter if it is supposed to regenerate the spec file
    // So we need to check for nullity here as opposed to an empty string.
    if (!project.specFile().isNull()) {
        postParameters.insert(QLatin1String("specfile"), project.specFile());
    }
    return postParameters;
}

PostJob *Provider::createProject(const Project &project)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/project/create")), projectPostParameters(project));
}

PostJob *Provider::editProject(const Project &project)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/project/edit/") + project.id()), projectPostParameters(project));
}

PostJob *Provider::deleteProject(const Project &project)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/project/delete/") + project.id()), projectPostParameters(project));
}

ItemJob<BuildService> *Provider::requestBuildService(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/buildservices/get/") + id);
    return new ItemJob<BuildService>(d->m_internals, createRequest(url));
}

ItemJob<Publisher> *Provider::requestPublisher(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request publisher" << id;
    QUrl url = createUrl(QLatin1String("buildservice/publishing/getpublisher/") + id);
    return new ItemJob<Publisher>(d->m_internals, createRequest(url));
}

PostJob *Provider::savePublisherField(const Project &project, const PublisherField &field)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("fields[0][name]"), field.name());
    postParameters.insert(QLatin1String("fields[0][fieldtype]"), field.type());
    postParameters.insert(QLatin1String("fields[0][data]"), field.data());

    QString url = QLatin1String("buildservice/publishing/savefields/") + project.id();
    // qCDebug(ATTICA) << "saving field values...";
    return new PostJob(d->m_internals, createRequest(url), postParameters);
}

PostJob *Provider::publishBuildJob(const BuildServiceJob &buildjob, const Publisher &publisher)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("dummyparameter"), QLatin1String("dummyvalue"));

    QString url = QLatin1String("buildservice/publishing/publishtargetresult/") + buildjob.id() + QLatin1Char('/') + publisher.id();
    // qCDebug(ATTICA) << "pub'ing";
    return new PostJob(d->m_internals, createRequest(url), postParameters);
}

// Buildservices and their jobs
ItemJob<BuildServiceJobOutput> *Provider::requestBuildServiceJobOutput(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/jobs/getoutput/") + id);
    // qCDebug(ATTICA) << url;
    return new ItemJob<BuildServiceJobOutput>(d->m_internals, createRequest(url));
}

ItemJob<BuildServiceJob> *Provider::requestBuildServiceJob(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/jobs/get/") + id);
    // qCDebug(ATTICA) << url;
    return new ItemJob<BuildServiceJob>(d->m_internals, createRequest(url));
}

QMap<QString, QString> buildServiceJobPostParameters(const BuildServiceJob &buildjob)
{
    QMap<QString, QString> postParameters;

    if (!buildjob.name().isEmpty()) {
        postParameters.insert(QLatin1String("name"), buildjob.name());
    }
    if (!buildjob.projectId().isEmpty()) {
        postParameters.insert(QLatin1String("projectid"), buildjob.projectId());
    }
    if (!buildjob.target().isEmpty()) {
        postParameters.insert(QLatin1String("target"), buildjob.target());
    }
    if (!buildjob.buildServiceId().isEmpty()) {
        postParameters.insert(QLatin1String("buildservice"), buildjob.buildServiceId());
    }

    return postParameters;
}

PostJob *Provider::cancelBuildServiceJob(const BuildServiceJob &job)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("dummyparameter"), QLatin1String("dummyvalue"));
    // qCDebug(ATTICA) << "b....................b";
    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/jobs/cancel/") + job.id()), postParameters);
}

PostJob *Provider::createBuildServiceJob(const BuildServiceJob &job)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    // A postjob won't be run without parameters.
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert(QLatin1String("dummyparameter"), QLatin1String("dummyvalue"));
    // qCDebug(ATTICA) << "Creating new BSJ on" << job.buildServiceId();
    return new PostJob(
        d->m_internals,
        createRequest(QLatin1String("buildservice/jobs/create/") + job.projectId() + QLatin1Char('/') + job.buildServiceId() + QLatin1Char('/') + job.target()),
        postParameters);
}

ListJob<BuildService> *Provider::requestBuildServices()
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request projects";
    QUrl url = createUrl(QLatin1String("buildservice/buildservices/list"));
    return new ListJob<BuildService>(d->m_internals, createRequest(url));
}

ListJob<Publisher> *Provider::requestPublishers()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/publishing/getpublishingcapabilities"));
    // qCDebug(ATTICA) << "request publishers" << url;
    return new ListJob<Publisher>(d->m_internals, createRequest(url));
}

ListJob<BuildServiceJob> *Provider::requestBuildServiceJobs(const Project &project)
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request projects";
    QUrl url = createUrl(QLatin1String("buildservice/jobs/list/") + project.id());
    return new ListJob<BuildServiceJob>(d->m_internals, createRequest(url));
}

ListJob<RemoteAccount> *Provider::requestRemoteAccounts()
{
    if (!isValid()) {
        return nullptr;
    }

    // qCDebug(ATTICA) << "request remoteaccounts";
    QUrl url = createUrl(QLatin1String("buildservice/remoteaccounts/list/"));
    return new ListJob<RemoteAccount>(d->m_internals, createRequest(url));
}

PostJob *Provider::createRemoteAccount(const RemoteAccount &account)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    // A postjob won't be run without parameters.
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert(QLatin1String("login"), account.login());
    postParameters.insert(QLatin1String("password"), account.password());
    postParameters.insert(QLatin1String("type"), account.type());
    postParameters.insert(QLatin1String("typeid"), account.remoteServiceId()); // FIXME: remoteserviceid?
    postParameters.insert(QLatin1String("data"), account.data());
    // qCDebug(ATTICA) << "Creating new Remoteaccount" << account.id() << account.login() << account.password();
    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/remoteaccounts/add")), postParameters);
}

PostJob *Provider::editRemoteAccount(const RemoteAccount &account)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    // A postjob won't be run without parameters.
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert(QLatin1String("login"), account.login());
    postParameters.insert(QLatin1String("password"), account.password());
    postParameters.insert(QLatin1String("type"), account.type());
    postParameters.insert(QLatin1String("typeid"), account.remoteServiceId()); // FIXME: remoteserviceid?
    postParameters.insert(QLatin1String("data"), account.data());
    // qCDebug(ATTICA) << "Creating new Remoteaccount" << account.id() << account.login() << account.password();
    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/remoteaccounts/edit/") + account.id()), postParameters);
}

ItemJob<RemoteAccount> *Provider::requestRemoteAccount(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/remoteaccounts/get/") + id);
    // qCDebug(ATTICA) << url;
    return new ItemJob<RemoteAccount>(d->m_internals, createRequest(url));
}

PostJob *Provider::deleteRemoteAccount(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    return new PostJob(d->m_internals, createRequest(QLatin1String("buildservice/remoteaccounts/remove/") + id), postParameters);
}

PostJob *Provider::uploadTarballToBuildService(const QString &projectId, const QString &fileName, const QByteArray &payload)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("buildservice/project/uploadsource/") + projectId);
    // qCDebug(ATTICA) << "Up'ing tarball" << url << projectId << fileName << payload;
    PostFileData postRequest(url);
    postRequest.addFile(fileName, payload, QLatin1String("application/octet-stream"), QLatin1String("source"));
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

// Activity

PostJob *Provider::postActivity(const QString &message)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("message"), message);
    return new PostJob(d->m_internals, createRequest(QLatin1String("activity")), postParameters);
}

PostJob *Provider::inviteFriend(const QString &to, const QString &message)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("message"), message);
    return new PostJob(d->m_internals, createRequest(QLatin1String("friend/invite/") + to), postParameters);
}

PostJob *Provider::approveFriendship(const QString &to)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("friend/approve/") + to));
}

PostJob *Provider::declineFriendship(const QString &to)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("friend/decline/") + to));
}

PostJob *Provider::cancelFriendship(const QString &to)
{
    if (!isValid()) {
        return nullptr;
    }

    return new PostJob(d->m_internals, createRequest(QLatin1String("friend/cancel/") + to));
}

PostJob *Provider::postLocation(qreal latitude, qreal longitude, const QString &city, const QString &country)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("latitude"), QString::number(latitude));
    postParameters.insert(QLatin1String("longitude"), QString::number(longitude));
    postParameters.insert(QLatin1String("city"), city);
    postParameters.insert(QLatin1String("country"), country);
    return new PostJob(d->m_internals, createRequest(QLatin1String("person/self")), postParameters);
}

ListJob<Folder> *Provider::requestFolders()
{
    if (!isValid()) {
        return nullptr;
    }

    return doRequestFolderList(createUrl(QLatin1String("message")));
}

ListJob<Message> *Provider::requestMessages(const Folder &folder)
{
    if (!isValid()) {
        return nullptr;
    }

    return doRequestMessageList(createUrl(QLatin1String("message/") + folder.id()));
}

ListJob<Message> *Provider::requestMessages(const Folder &folder, Message::Status status)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("message/") + folder.id());
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("status"), QString::number(status));
    url.setQuery(q);
    return doRequestMessageList(url);
}

ItemJob<Message> *Provider::requestMessage(const Folder &folder, const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    return new ItemJob<Message>(d->m_internals, createRequest(QLatin1String("message/") + folder.id() + QLatin1Char('/') + id));
}

PostJob *Provider::postMessage(const Message &message)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("message"), message.body());
    postParameters.insert(QLatin1String("subject"), message.subject());
    postParameters.insert(QLatin1String("to"), message.to());
    return new PostJob(d->m_internals, createRequest(QLatin1String("message/2")), postParameters);
}

ListJob<Category> *Provider::requestCategories()
{
    if (!isValid()) {
        return nullptr;
    }

    const QUrl url = createUrl(QLatin1String("content/categories"));

    // Thread-local cache of categories requests. They are fairly slow and block startup
    static QThreadStorage<QHash<QUrl, ListJob<Category> *>> reqs;
    ListJob<Category> *job = reqs.localData().value(url);
    if (!job) {
        job = new ListJob<Category>(d->m_internals, createRequest(url));
        QObject::connect(job, &BaseJob::finished, [url] {
            reqs.localData().remove(url);
        });
        reqs.localData().insert(url, job);
    }
    return job;
}

ListJob<License> *Provider::requestLicenses()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/licenses"));
    ListJob<License> *job = new ListJob<License>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Distribution> *Provider::requestDistributions()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/distributions"));
    ListJob<Distribution> *job = new ListJob<Distribution>(d->m_internals, createRequest(url));
    return job;
}

ListJob<HomePageType> *Provider::requestHomePageTypes()
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/homepages"));
    ListJob<HomePageType> *job = new ListJob<HomePageType>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Content> *Provider::searchContents(const Category::List &categories, const QString &search, SortMode sortMode, uint page, uint pageSize)
{
    return searchContents(categories, QString(), Distribution::List(), License::List(), search, sortMode, page, pageSize);
}

ListJob<Content> *
Provider::searchContentsByPerson(const Category::List &categories, const QString &person, const QString &search, SortMode sortMode, uint page, uint pageSize)
{
    return searchContents(categories, person, Distribution::List(), License::List(), search, sortMode, page, pageSize);
}

ListJob<Content> *Provider::searchContents(const Category::List &categories,
                                           const QString &person,
                                           const Distribution::List &distributions,
                                           const License::List &licenses,
                                           const QString &search,
                                           SortMode sortMode,
                                           uint page,
                                           uint pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("content/data"));
    QUrlQuery q(url);
    QStringList categoryIds;
    categoryIds.reserve(categories.count());
    for (const Category &category : categories) {
        categoryIds.append(category.id());
    }
    q.addQueryItem(QStringLiteral("categories"), categoryIds.join(QLatin1Char('x')));

    QStringList distributionIds;
    for (const Distribution &distribution : distributions) {
        distributionIds.append(QString::number(distribution.id()));
    }
    q.addQueryItem(QStringLiteral("distribution"), distributionIds.join(QLatin1Char(',')));

    QStringList licenseIds;
    for (const License &license : licenses) {
        licenseIds.append(QString::number(license.id()));
    }
    q.addQueryItem(QStringLiteral("license"), licenseIds.join(QLatin1Char(',')));

    if (!person.isEmpty()) {
        q.addQueryItem(QStringLiteral("user"), person);
    }

    q.addQueryItem(QStringLiteral("search"), search);
    QString sortModeString;
    switch (sortMode) {
    case Newest:
        sortModeString = QLatin1String("new");
        break;
    case Alphabetical:
        sortModeString = QLatin1String("alpha");
        break;
    case Rating:
        sortModeString = QLatin1String("high");
        break;
    case Downloads:
        sortModeString = QLatin1String("down");
        break;
    }

    if (!sortModeString.isEmpty()) {
        q.addQueryItem(QStringLiteral("sortmode"), sortModeString);
    }

    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));

    url.setQuery(q);
    ListJob<Content> *job = new ListJob<Content>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<Content> *Provider::requestContent(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/data/") + id);
    ItemJob<Content> *job = new ItemJob<Content>(d->m_internals, createRequest(url));
    return job;
}

ItemPostJob<Content> *Provider::addNewContent(const Category &category, const Content &cont)
{
    if (!isValid() || !category.isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/add"));
    StringMap pars(cont.attributes());

    pars.insert(QLatin1String("type"), category.id());
    pars.insert(QLatin1String("name"), cont.name());

    // qCDebug(ATTICA) << "Parameter map: " << pars;

    return new ItemPostJob<Content>(d->m_internals, createRequest(url), pars);
}

ItemPostJob<Content> *Provider::editContent(const Category &updatedCategory, const QString &contentId, const Content &updatedContent)
{
    if (!isValid()) {
        return nullptr;
    }

    // FIXME I get a server error message here, though the name of the item is changed
    QUrl url = createUrl(QLatin1String("content/edit/") + contentId);
    StringMap pars(updatedContent.attributes());

    pars.insert(QLatin1String("type"), updatedCategory.id());
    pars.insert(QLatin1String("name"), updatedContent.name());

    // qCDebug(ATTICA) << "Parameter map: " << pars;

    return new ItemPostJob<Content>(d->m_internals, createRequest(url), pars);
}

/*
PostJob* Provider::setDownloadFile(const QString& contentId, QIODevice* payload)
{
    QUrl url = createUrl("content/uploaddownload/" + contentId);
    PostFileData postRequest(url);
    // FIXME mime type
    //postRequest.addFile("localfile", payload, "application/octet-stream");
    postRequest.addFile("localfile", payload, "image/jpeg");
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}
*/

PostJob *Provider::deleteContent(const QString &contentId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/delete/") + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob *Provider::setDownloadFile(const QString &contentId, const QString &fileName, const QByteArray &payload)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/uploaddownload/") + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    // FIXME mime type
    postRequest.addFile(fileName, payload, QLatin1String("application/octet-stream"));
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob *Provider::deleteDownloadFile(const QString &contentId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/deletedownload/") + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob *Provider::setPreviewImage(const QString &contentId, const QString &previewId, const QString &fileName, const QByteArray &image)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/uploadpreview/") + contentId + QLatin1Char('/') + previewId);

    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    postRequest.addArgument(QLatin1String("previewid"), previewId);
    // FIXME mime type
    postRequest.addFile(fileName, image, QLatin1String("application/octet-stream"));

    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob *Provider::deletePreviewImage(const QString &contentId, const QString &previewId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/deletepreview/") + contentId + QLatin1Char('/') + previewId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    postRequest.addArgument(QLatin1String("previewid"), previewId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob *Provider::voteForContent(const QString &contentId, uint rating)
{
    if (!isValid()) {
        return nullptr;
    }

    // according to OCS API, the rating is 0..100
    if (rating > 100) {
        qWarning() << "Rating cannot be superior to 100, fallback to 100.";
        rating = 100;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("vote"), QString::number(rating));
    // qCDebug(ATTICA) << "vote: " << QString::number(rating);
    return new PostJob(d->m_internals, createRequest(QLatin1String("content/vote/") + contentId), postParameters);
}

PostJob *Provider::becomeFan(const QString &contentId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("fan/add/") + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String("contentid"), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

ListJob<Person> *Provider::requestFans(const QString &contentId, uint page, uint pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("fan/data/") + contentId);
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("contentid"), contentId);
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);
    ListJob<Person> *job = new ListJob<Person>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Forum> *Provider::requestForums(uint page, uint pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("forum/list"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestForumList(url);
}

ListJob<Topic> *
Provider::requestTopics(const QString &forum, const QString &search, const QString &description, Provider::SortMode mode, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("forum/topics/list"));
    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("forum"), forum);
    q.addQueryItem(QStringLiteral("search"), search);
    q.addQueryItem(QStringLiteral("description"), description);
    QString sortModeString;
    switch (mode) {
    case Newest:
        sortModeString = QLatin1String("new");
        break;
    case Alphabetical:
        sortModeString = QLatin1String("alpha");
        break;
    default:
        break;
    }
    if (!sortModeString.isEmpty()) {
        q.addQueryItem(QStringLiteral("sortmode"), sortModeString);
    }
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    return doRequestTopicList(url);
}

PostJob *Provider::postTopic(const QString &forumId, const QString &subject, const QString &content)
{
    if (!isValid()) {
        return nullptr;
    }

    StringMap postParameters;
    postParameters.insert(QLatin1String("subject"), subject);
    postParameters.insert(QLatin1String("content"), content);
    postParameters.insert(QLatin1String("forum"), forumId);
    return new PostJob(d->m_internals, createRequest(QLatin1String("forum/topic/add")), postParameters);
}

ItemJob<DownloadItem> *Provider::downloadLink(const QString &contentId, const QString &itemId)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("content/download/") + contentId + QLatin1Char('/') + itemId);
    ItemJob<DownloadItem> *job = new ItemJob<DownloadItem>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<KnowledgeBaseEntry> *Provider::requestKnowledgeBaseEntry(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("knowledgebase/data/") + id);
    ItemJob<KnowledgeBaseEntry> *job = new ItemJob<KnowledgeBaseEntry>(d->m_internals, createRequest(url));
    return job;
}

ListJob<KnowledgeBaseEntry> *Provider::searchKnowledgeBase(const Content &content, const QString &search, Provider::SortMode sortMode, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("knowledgebase/data"));
    QUrlQuery q(url);
    if (content.isValid()) {
        q.addQueryItem(QStringLiteral("content"), content.id());
    }

    q.addQueryItem(QStringLiteral("search"), search);
    QString sortModeString;
    switch (sortMode) {
    case Newest:
        sortModeString = QLatin1String("new");
        break;
    case Alphabetical:
        sortModeString = QLatin1String("alpha");
        break;
    case Rating:
        sortModeString = QLatin1String("high");
        break;
    // FIXME: knowledge base doesn't have downloads
    case Downloads:
        sortModeString = QLatin1String("new");
        break;
    }
    if (!sortModeString.isEmpty()) {
        q.addQueryItem(QStringLiteral("sortmode"), sortModeString);
    }

    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    ListJob<KnowledgeBaseEntry> *job = new ListJob<KnowledgeBaseEntry>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<Event> *Provider::requestEvent(const QString &id)
{
    if (!isValid()) {
        return nullptr;
    }

    ItemJob<Event> *job = new ItemJob<Event>(d->m_internals, createRequest(QLatin1String("event/data/") + id));
    return job;
}

ListJob<Event> *Provider::requestEvent(const QString &country, const QString &search, const QDate &startAt, Provider::SortMode mode, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QStringLiteral("event/data"));
    QUrlQuery q(url);

    if (!search.isEmpty()) {
        q.addQueryItem(QStringLiteral("search"), search);
    }

    QString sortModeString;
    switch (mode) {
    case Newest:
        sortModeString = QLatin1String("new");
        break;
    case Alphabetical:
        sortModeString = QLatin1String("alpha");
        break;
    default:
        break;
    }
    if (!sortModeString.isEmpty()) {
        q.addQueryItem(QStringLiteral("sortmode"), sortModeString);
    }

    if (!country.isEmpty()) {
        q.addQueryItem(QStringLiteral("country"), country);
    }

    q.addQueryItem(QStringLiteral("startat"), startAt.toString(Qt::ISODate));

    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    ListJob<Event> *job = new ListJob<Event>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Comment> *Provider::requestComments(const Comment::Type commentType, const QString &id, const QString &id2, int page, int pageSize)
{
    if (!isValid()) {
        return nullptr;
    }

    QString commentTypeString;
    commentTypeString = Comment::commentTypeToString(commentType);
    if (commentTypeString.isEmpty()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("comments/data/") + commentTypeString + QLatin1Char('/') + id + QLatin1Char('/') + id2);

    QUrlQuery q(url);
    q.addQueryItem(QStringLiteral("page"), QString::number(page));
    q.addQueryItem(QStringLiteral("pagesize"), QString::number(pageSize));
    url.setQuery(q);

    ListJob<Comment> *job = new ListJob<Comment>(d->m_internals, createRequest(url));
    return job;
}

ItemPostJob<Comment> *Provider::addNewComment(const Comment::Type commentType,
                                              const QString &id,
                                              const QString &id2,
                                              const QString &parentId,
                                              const QString &subject,
                                              const QString &message)
{
    if (!isValid()) {
        return nullptr;
    }

    QString commentTypeString;
    commentTypeString = Comment::commentTypeToString(commentType);
    if (commentTypeString.isEmpty()) {
        return nullptr;
    }

    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String("type"), commentTypeString);
    postParameters.insert(QLatin1String("content"), id);
    postParameters.insert(QLatin1String("content2"), id2);
    postParameters.insert(QLatin1String("parent"), parentId);
    postParameters.insert(QLatin1String("subject"), subject);
    postParameters.insert(QLatin1String("message"), message);

    return new ItemPostJob<Comment>(d->m_internals, createRequest(QLatin1String("comments/add")), postParameters);
}

PostJob *Provider::voteForComment(const QString &id, uint rating)
{
    if (!isValid() || (rating > 100)) {
        return nullptr;
    }

    QMap<QString, QString> postParameters;
    postParameters.insert(QLatin1String("vote"), QString::number(rating));

    QUrl url = createUrl(QLatin1String("comments/vote/") + id);
    return new PostJob(d->m_internals, createRequest(url), postParameters);
}

PostJob *Provider::setPrivateData(const QString &app, const QString &key, const QString &value)
{
    if (!isValid()) {
        return nullptr;
    }

    QUrl url = createUrl(QLatin1String("privatedata/setattribute/") + app + QLatin1Char('/') + key);
    PostFileData postRequest(url);

    postRequest.addArgument(QLatin1String("value"), value);

    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

ItemJob<PrivateData> *Provider::requestPrivateData(const QString &app, const QString &key)
{
    if (!isValid()) {
        return nullptr;
    }

    ItemJob<PrivateData> *job =
        new ItemJob<PrivateData>(d->m_internals, createRequest(QLatin1String("privatedata/getattribute/") + app + QLatin1Char('/') + key));
    return job;
}

QUrl Provider::createUrl(const QString &path)
{
    QUrl url(d->m_baseUrl.toString() + path);
    return url;
}

QNetworkRequest Provider::createRequest(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    QString agentHeader;
    if (QCoreApplication::instance()) {
        agentHeader = QString::fromLocal8Bit("%1/%2").arg(QCoreApplication::instance()->applicationName(), QCoreApplication::instance()->applicationVersion());
    } else {
        agentHeader = QString::fromLocal8Bit("Attica/%1").arg(QLatin1String(LIBATTICA_VERSION_STRING));
    }
    if (!d->m_additionalAgentInformation.isEmpty()) {
        agentHeader = QString::fromLocal8Bit("%1 (+%2)").arg(agentHeader, d->m_additionalAgentInformation);
    }
    request.setHeader(QNetworkRequest::UserAgentHeader, agentHeader);

    if (!d->m_credentialsUserName.isEmpty()) {
        request.setAttribute((QNetworkRequest::Attribute)BaseJob::UserAttribute, QVariant(d->m_credentialsUserName));
        request.setAttribute((QNetworkRequest::Attribute)BaseJob::PasswordAttribute, QVariant(d->m_credentialsPassword));
    }
    return request;
}

QNetworkRequest Provider::createRequest(const QString &path)
{
    return createRequest(createUrl(path));
}

ItemJob<Config> *Provider::doRequestConfig(const QUrl &url)
{
    return new ItemJob<Config>(d->m_internals, createRequest(url));
}

ItemJob<Person> *Provider::doRequestPerson(const QUrl &url)
{
    return new ItemJob<Person>(d->m_internals, createRequest(url));
}

ItemJob<AccountBalance> *Provider::doRequestAccountBalance(const QUrl &url)
{
    return new ItemJob<AccountBalance>(d->m_internals, createRequest(url));
}

ListJob<Person> *Provider::doRequestPersonList(const QUrl &url)
{
    return new ListJob<Person>(d->m_internals, createRequest(url));
}

ListJob<Achievement> *Provider::doRequestAchievementList(const QUrl &url)
{
    return new ListJob<Achievement>(d->m_internals, createRequest(url));
}

ListJob<Activity> *Provider::doRequestActivityList(const QUrl &url)
{
    return new ListJob<Activity>(d->m_internals, createRequest(url));
}

ListJob<Folder> *Provider::doRequestFolderList(const QUrl &url)
{
    return new ListJob<Folder>(d->m_internals, createRequest(url));
}

ListJob<Forum> *Provider::doRequestForumList(const QUrl &url)
{
    return new ListJob<Forum>(d->m_internals, createRequest(url));
}

ListJob<Topic> *Provider::doRequestTopicList(const QUrl &url)
{
    return new ListJob<Topic>(d->m_internals, createRequest(url));
}

ListJob<Message> *Provider::doRequestMessageList(const QUrl &url)
{
    return new ListJob<Message>(d->m_internals, createRequest(url));
}

QString Provider::achievementServiceVersion() const
{
    return d->m_achievementVersion;
}

QString Provider::activityServiceVersion() const
{
    return d->m_activityVersion;
}
QString Provider::commentServiceVersion() const
{
    return d->m_commentVersion;
}
QString Provider::contentServiceVersion() const
{
    return d->m_contentVersion;
}
QString Provider::fanServiceVersion() const
{
    return d->m_fanVersion;
}
QString Provider::forumServiceVersion() const
{
    return d->m_forumVersion;
}
QString Provider::friendServiceVersion() const
{
    return d->m_friendVersion;
}
QString Provider::knowledgebaseServiceVersion() const
{
    return d->m_knowledgebaseVersion;
}
QString Provider::messageServiceVersion() const
{
    return d->m_messageVersion;
}
QString Provider::personServiceVersion() const
{
    return d->m_personVersion;
}

bool Provider::hasAchievementService() const
{
    return !d->m_achievementVersion.isEmpty();
}

bool Provider::hasActivityService() const
{
    return !d->m_activityVersion.isEmpty();
}
bool Provider::hasCommentService() const
{
    return !d->m_commentVersion.isEmpty();
}
bool Provider::hasContentService() const
{
    return !d->m_contentVersion.isEmpty();
}
bool Provider::hasFanService() const
{
    return !d->m_fanVersion.isEmpty();
}
bool Provider::hasForumService() const
{
    return !d->m_forumVersion.isEmpty();
}
bool Provider::hasFriendService() const
{
    return !d->m_friendVersion.isEmpty();
}
bool Provider::hasKnowledgebaseService() const
{
    return !d->m_knowledgebaseVersion.isEmpty();
}
bool Provider::hasMessageService() const
{
    return !d->m_messageVersion.isEmpty();
}
bool Provider::hasPersonService() const
{
    return !d->m_personVersion.isEmpty();
}
