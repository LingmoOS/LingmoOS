/*
    SPDX-FileCopyrightText: 2017, 2018 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dbusrunner_p.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingReply>
#include <QIcon>
#include <set>

#include "dbusutils_p.h"
#include "krunner_debug.h"

namespace KRunner
{
DBusRunner::DBusRunner(QObject *parent, const KPluginMetaData &data)
    : KRunner::AbstractRunner(parent, data)
    , m_path(data.value(QStringLiteral("X-Plasma-DBusRunner-Path"), QStringLiteral("/runner")))
    , m_hasUniqueResults(data.value(QStringLiteral("X-Plasma-Runner-Unique-Results"), false))
    , m_requestActionsOnce(data.value(QStringLiteral("X-Plasma-Request-Actions-Once"), false))
    , m_callLifecycleMethods(data.value(QStringLiteral("X-Plasma-API")) == QLatin1String("DBus2"))
    , m_ifaceName(QStringLiteral("org.kde.krunner1"))
{
    qDBusRegisterMetaType<RemoteMatch>();
    qDBusRegisterMetaType<RemoteMatches>();
    qDBusRegisterMetaType<KRunner::Action>();
    qDBusRegisterMetaType<KRunner::Actions>();
    qDBusRegisterMetaType<RemoteImage>();

    QString requestedServiceName = data.value(QStringLiteral("X-Plasma-DBusRunner-Service"));
    if (requestedServiceName.isEmpty() || m_path.isEmpty()) {
        qCWarning(KRUNNER) << "Invalid entry:" << data;
        return;
    }

    if (requestedServiceName.endsWith(QLatin1Char('*'))) {
        requestedServiceName.chop(1);
        // find existing matching names
        auto namesReply = QDBusConnection::sessionBus().interface()->registeredServiceNames();
        if (namesReply.isValid()) {
            const auto names = namesReply.value();
            for (const QString &serviceName : names) {
                if (serviceName.startsWith(requestedServiceName)) {
                    m_matchingServices << serviceName;
                }
            }
        }
        // and watch for changes
        connect(QDBusConnection::sessionBus().interface(),
                &QDBusConnectionInterface::serviceOwnerChanged,
                this,
                [this, requestedServiceName](const QString &serviceName, const QString &oldOwner, const QString &newOwner) {
                    if (!serviceName.startsWith(requestedServiceName)) {
                        return;
                    }
                    if (!oldOwner.isEmpty() && !newOwner.isEmpty()) {
                        // changed owner, but service still exists. Don't need to adjust anything
                        return;
                    }
                    if (!newOwner.isEmpty()) {
                        m_matchingServices.insert(serviceName);
                    }
                    if (!oldOwner.isEmpty()) {
                        m_matchingServices.remove(serviceName);
                    }
                });
    } else {
        // don't check when not wildcarded, as it could be used with DBus-activation
        m_matchingServices << requestedServiceName;
    }

    connect(this, &AbstractRunner::teardown, this, &DBusRunner::teardown);

    // Load the runner syntaxes
    const QStringList syntaxes = data.value(QStringLiteral("X-Plasma-Runner-Syntaxes"), QStringList());
    const QStringList syntaxDescriptions = data.value(QStringLiteral("X-Plasma-Runner-Syntax-Descriptions"), QStringList());
    const int descriptionCount = syntaxDescriptions.count();
    for (int i = 0; i < syntaxes.count(); ++i) {
        const QString &query = syntaxes.at(i);
        const QString description = i < descriptionCount ? syntaxDescriptions.at(i) : QString();
        addSyntax(query, description);
    }
}

void DBusRunner::reloadConfiguration()
{
    // If we have already loaded a config, but the runner is told to reload it's config
    if (m_callLifecycleMethods) {
        suspendMatching(true);
        requestConfig();
    }
}

void DBusRunner::teardown()
{
    if (m_matchWasCalled) {
        for (const QString &service : std::as_const(m_matchingServices)) {
            auto method = QDBusMessage::createMethodCall(service, m_path, m_ifaceName, QStringLiteral("Teardown"));
            QDBusConnection::sessionBus().asyncCall(method);
        }
    }
    m_actionsForSessionRequested = false;
    m_matchWasCalled = false;
}

void DBusRunner::requestActionsForService(const QString &service, const std::function<void()> &finishedCallback)
{
    if (m_actionsForSessionRequested) {
        finishedCallback();
        return; // only once per match session
    }
    if (m_requestActionsOnce) {
        if (m_requestedActionServices.contains(service)) {
            finishedCallback();
            return;
        } else {
            m_requestedActionServices << service;
        }
    }

    auto getActionsMethod = QDBusMessage::createMethodCall(service, m_path, m_ifaceName, QStringLiteral("Actions"));
    QDBusPendingReply<QList<KRunner::Action>> reply = QDBusConnection::sessionBus().asyncCall(getActionsMethod);
    connect(new QDBusPendingCallWatcher(reply), &QDBusPendingCallWatcher::finished, this, [this, service, reply, finishedCallback](auto watcher) {
        watcher->deleteLater();
        if (!reply.isValid()) {
            qCDebug(KRUNNER) << "Error requesting actions; calling" << service << " :" << reply.error().name() << reply.error().message();
        } else {
            m_actions[service] = reply.value();
        }
        finishedCallback();
    });
}

void DBusRunner::requestConfig()
{
    const QString service = *m_matchingServices.constBegin();
    auto getConfigMethod = QDBusMessage::createMethodCall(service, m_path, m_ifaceName, QStringLiteral("Config"));
    QDBusPendingReply<QVariantMap> reply = QDBusConnection::sessionBus().asyncCall(getConfigMethod);

    auto watcher = new QDBusPendingCallWatcher(reply);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, service]() {
        watcher->deleteLater();
        QDBusReply<QVariantMap> reply = *watcher;
        if (!reply.isValid()) {
            suspendMatching(false);
            qCWarning(KRUNNER) << "Error requesting config; calling" << service << " :" << reply.error().name() << reply.error().message();
            return;
        }
        const QVariantMap config = reply.value();
        for (auto it = config.cbegin(), end = config.cend(); it != end; ++it) {
            if (it.key() == QLatin1String("MatchRegex")) {
                QRegularExpression regex(it.value().toString());
                setMatchRegex(regex);
            } else if (it.key() == QLatin1String("MinLetterCount")) {
                setMinLetterCount(it.value().toInt());
            } else if (it.key() == QLatin1String("TriggerWords")) {
                setTriggerWords(it.value().toStringList());
            } else if (it.key() == QLatin1String("Actions")) {
                m_actions[service] = it.value().value<QList<KRunner::Action>>();
                m_requestedActionServices << service;
            }
        }
        suspendMatching(false);
    });
}

QList<QueryMatch> DBusRunner::convertMatches(const QString &service, const RemoteMatches &remoteMatches)
{
    QList<KRunner::QueryMatch> matches;
    for (const RemoteMatch &match : remoteMatches) {
        KRunner::QueryMatch m(this);

        m.setText(match.text);
        m.setIconName(match.iconName);
        m.setCategoryRelevance(match.categoryRelevance);
        m.setRelevance(match.relevance);

        // split is essential items are as native DBus types, optional extras are in the property map (which is obviously a lot slower to parse)
        m.setUrls(QUrl::fromStringList(match.properties.value(QStringLiteral("urls")).toStringList()));
        m.setMatchCategory(match.properties.value(QStringLiteral("category")).toString());
        m.setSubtext(match.properties.value(QStringLiteral("subtext")).toString());
        m.setData(QVariantList({service}));
        m.setId(match.id);
        m.setMultiLine(match.properties.value(QStringLiteral("multiline")).toBool());

        const auto actionsIt = match.properties.find(QStringLiteral("actions"));
        const KRunner::Actions actionList = m_actions.value(service);
        if (actionsIt == match.properties.cend()) {
            m.setActions(actionList);
        } else {
            KRunner::Actions requestedActions;
            const QStringList actionIds = actionsIt.value().toStringList();
            for (const auto &action : actionList) {
                if (actionIds.contains(action.id())) {
                    requestedActions << action;
                }
            }
            m.setActions(requestedActions);
        }

        const QVariant iconData = match.properties.value(QStringLiteral("icon-data"));
        if (iconData.isValid()) {
            const auto iconDataArgument = iconData.value<QDBusArgument>();
            if (iconDataArgument.currentType() == QDBusArgument::StructureType && iconDataArgument.currentSignature() == QLatin1String("(iiibiiay)")) {
                const RemoteImage remoteImage = qdbus_cast<RemoteImage>(iconDataArgument);
                if (QImage decodedImage = decodeImage(remoteImage); !decodedImage.isNull()) {
                    const QPixmap pix = QPixmap::fromImage(std::move(decodedImage));
                    QIcon icon(pix);
                    m.setIcon(icon);
                    // iconName normally takes precedence
                    m.setIconName(QString());
                }
            } else {
                qCWarning(KRUNNER) << "Invalid signature of icon-data property:" << iconDataArgument.currentSignature();
            }
        }
        matches.append(m);
    }
    return matches;
}
void DBusRunner::matchInternal(KRunner::RunnerContext context)
{
    const QString jobId = context.runnerJobId(this);
    if (m_matchingServices.isEmpty()) {
        Q_EMIT matchInternalFinished(jobId);
    }
    m_matchWasCalled = true;

    // we scope watchers to make sure the lambda that captures context by reference definitely gets disconnected when this function ends
    std::shared_ptr<std::set<QString>> pendingServices(new std::set<QString>);

    for (const QString &service : std::as_const(m_matchingServices)) {
        pendingServices->insert(service);

        const auto onActionsFinished = [=, this]() mutable {
            auto matchMethod = QDBusMessage::createMethodCall(service, m_path, m_ifaceName, QStringLiteral("Match"));
            matchMethod.setArguments(QList<QVariant>({context.query()}));
            QDBusPendingReply<RemoteMatches> reply = QDBusConnection::sessionBus().asyncCall(matchMethod);

            auto watcher = new QDBusPendingCallWatcher(reply);

            connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, service, context, reply, jobId, pendingServices, watcher]() mutable {
                watcher->deleteLater();
                pendingServices->erase(service);
                if (reply.isError()) {
                    qCWarning(KRUNNER) << "Error requesting matches; calling" << service << " :" << reply.error().name() << reply.error().message();
                } else {
                    context.addMatches(convertMatches(service, reply.value()));
                }
                // We are finished when all watchers finished
                if (pendingServices->size() == 0) {
                    Q_EMIT matchInternalFinished(jobId);
                }
            });
        };
        requestActionsForService(service, onActionsFinished);
    }
    m_actionsForSessionRequested = true;
}

void DBusRunner::run(const KRunner::RunnerContext & /*context*/, const KRunner::QueryMatch &match)
{
    QString actionId;
    QString matchId;
    if (m_hasUniqueResults) {
        matchId = match.id();
    } else {
        matchId = match.id().mid(id().length() + 1); // QueryMatch::setId mangles the match ID with runnerID + '_'. This unmangles it
    }
    const QString service = match.data().toList().constFirst().toString();

    if (match.selectedAction()) {
        actionId = match.selectedAction().id();
    }

    auto runMethod = QDBusMessage::createMethodCall(service, m_path, m_ifaceName, QStringLiteral("Run"));
    runMethod.setArguments(QList<QVariant>({matchId, actionId}));
    QDBusConnection::sessionBus().call(runMethod, QDBus::NoBlock);
}

QImage DBusRunner::decodeImage(const RemoteImage &remoteImage)
{
    auto copyLineRGB32 = [](QRgb *dst, const char *src, int width) {
        const char *end = src + width * 3;
        for (; src != end; ++dst, src += 3) {
            *dst = qRgb(src[0], src[1], src[2]);
        }
    };

    auto copyLineARGB32 = [](QRgb *dst, const char *src, int width) {
        const char *end = src + width * 4;
        for (; src != end; ++dst, src += 4) {
            *dst = qRgba(src[0], src[1], src[2], src[3]);
        }
    };

    if (remoteImage.width <= 0 || remoteImage.width >= 2048 || remoteImage.height <= 0 || remoteImage.height >= 2048 || remoteImage.rowStride <= 0) {
        qCWarning(KRUNNER) << "Invalid image metadata (width:" << remoteImage.width << "height:" << remoteImage.height << "rowStride:" << remoteImage.rowStride
                           << ")";
        return QImage();
    }

    QImage::Format format = QImage::Format_Invalid;
    void (*copyFn)(QRgb *, const char *, int) = nullptr;
    if (remoteImage.bitsPerSample == 8) {
        if (remoteImage.channels == 4) {
            format = QImage::Format_ARGB32;
            copyFn = copyLineARGB32;
        } else if (remoteImage.channels == 3) {
            format = QImage::Format_RGB32;
            copyFn = copyLineRGB32;
        }
    }
    if (format == QImage::Format_Invalid) {
        qCWarning(KRUNNER) << "Unsupported image format (hasAlpha:" << remoteImage.hasAlpha << "bitsPerSample:" << remoteImage.bitsPerSample
                           << "channels:" << remoteImage.channels << ")";
        return QImage();
    }

    QImage image(remoteImage.width, remoteImage.height, format);
    const QByteArray pixels = remoteImage.data;
    const char *ptr = pixels.data();
    const char *end = ptr + pixels.length();
    for (int y = 0; y < remoteImage.height; ++y, ptr += remoteImage.rowStride) {
        if (Q_UNLIKELY(ptr + remoteImage.channels * remoteImage.width > end)) {
            qCWarning(KRUNNER) << "Image data is incomplete. y:" << y << "height:" << remoteImage.height;
            break;
        }
        copyFn(reinterpret_cast<QRgb *>(image.scanLine(y)), ptr, remoteImage.width);
    }

    return image;
}
}
#include "moc_dbusrunner_p.cpp"
