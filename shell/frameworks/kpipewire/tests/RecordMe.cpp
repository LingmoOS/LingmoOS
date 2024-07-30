/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "RecordMe.h"
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include "xdp_dbus_screencast_interface.h"

Q_DECLARE_METATYPE(Stream)

QDebug operator<<(QDebug debug, const Stream& plug)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Stream(id: " << plug.id << ", opts: " << plug.opts << ')';
    return debug;
}


const QDBusArgument &operator<<(const QDBusArgument &argument, const Stream &/*stream*/)
{
    argument.beginStructure();
//     argument << stream.id << stream.opts;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Stream &stream)
{
    argument.beginStructure();
    argument >> stream.id >> stream.opts;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QList<Stream> &stream)
{
    argument.beginArray();
    while ( !argument.atEnd() ) {
        Stream element;
        argument >> element;
        stream.append( element );
    }
    argument.endArray();
    return argument;
}

RecordMe::RecordMe(QObject* parent)
    : QObject(parent)
    , iface(new OrgFreedesktopPortalScreenCastInterface(
        QLatin1String("org.freedesktop.portal.Desktop"), QLatin1String("/org/freedesktop/portal/desktop"), QDBusConnection::sessionBus(), this))
    , m_durationTimer(new QTimer(this))
    , m_handleToken(QStringLiteral("RecordMe%1").arg(QRandomGenerator::global()->generate()))
    , m_engine(new QQmlApplicationEngine(this))
{
    m_engine->rootContext()->setContextProperty(QStringLiteral("app"), this);
    m_engine->load(QUrl(QStringLiteral("qrc:/main.qml")));

    // create session
    const auto sessionParameters = QVariantMap {
        { QLatin1String("session_handle_token"), m_handleToken },
        { QLatin1String("handle_token"), m_handleToken }
    };
    auto sessionReply = iface->CreateSession(sessionParameters);
    sessionReply.waitForFinished();
    if (!sessionReply.isValid()) {
        qWarning("Couldn't initialize the remote control session");
        exit(1);
        return;
    }

    const bool ret = QDBusConnection::sessionBus().connect(QString(),
                                                           sessionReply.value().path(),
                                                           QLatin1String("org.freedesktop.portal.Request"),
                                                           QLatin1String("Response"),
                                                           this,
                                                           SLOT(response(uint, QVariantMap)));
    if (!ret) {
        qWarning() << "failed to create session";
        exit(2);
        return;
    }

    qDBusRegisterMetaType<Stream>();
    qDBusRegisterMetaType<QList<Stream>>();

    m_durationTimer->setSingleShot(true);
}

RecordMe::~RecordMe() = default;

void RecordMe::init(const QDBusObjectPath& path)
{
    m_path = path;
    {
        uint32_t cursor_mode;
        if (iface->availableCursorModes() & Metadata) {
            cursor_mode = Metadata;
        } else {
            cursor_mode = Hidden;
        }
        QVariantMap sourcesParameters = {{QLatin1String("handle_token"), m_handleToken},
                                         {QLatin1String("types"), iface->availableSourceTypes()},
                                         {QLatin1String("multiple"), false}, // for now?
                                         {QLatin1String("cursor_mode"), uint(cursor_mode)},
                                         {QLatin1String("persist_mode"), uint(m_persistMode)}};

        if (!m_restoreToken.isEmpty()) {
            sourcesParameters[QLatin1String("restore_token")] = m_restoreToken;
        }

        auto reply = iface->SelectSources(m_path, sourcesParameters);
        reply.waitForFinished();

        if (reply.isError()) {
            qWarning() << "Could not select sources" << reply.error();
            exit(1);
            return;
        }
        qDebug() << "select sources done" << reply.value().path();
    }
}

void RecordMe::response(uint code, const QVariantMap& results)
{
    if (code > 0) {
        qWarning() << "error!!!" << code << results;
        exit(666);
        return;
    }

    if (results.contains(QLatin1String("restore_token"))) {
        qDebug() << "Restore token:" << results[QLatin1String("restore_token")].toString();
    }

    const auto streamsIt = results.constFind(QStringLiteral("streams"));
    if (streamsIt != results.constEnd()) {
        QList<Stream> streams;
        streamsIt->value<QDBusArgument>() >> streams;

        handleStreams(streams);
        return;
    }

    const auto handleIt = results.constFind(QStringLiteral("session_handle"));
    if (handleIt != results.constEnd()) {
        init(QDBusObjectPath(handleIt->toString()));
        return;
    }

    qDebug() << "params" << results << code;
    if (results.isEmpty()) {
        start();
        return;
    }
}

void RecordMe::start()
{
    const QVariantMap startParameters = {
        { QLatin1String("handle_token"), m_handleToken }
    };

    auto reply = iface->Start(m_path, QStringLiteral("org.freedesktop.RecordMe"), startParameters);
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "Could not start stream" << reply.error();
        exit(1);
        return;
    }
    qDebug() << "started!" << reply.value().path();
}

void RecordMe::handleStreams(const QList<Stream> &streams)
{
    const QVariantMap startParameters = {
        { QLatin1String("handle_token"), m_handleToken }
    };

    auto reply = iface->OpenPipeWireRemote(m_path, startParameters);
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "Could not start stream" << reply.error();
        exit(1);
        return;
    }

    const int fd = reply.value().takeFileDescriptor();

    const auto roots = m_engine->rootObjects();
    for (const auto &stream : streams) {
        for (auto root : roots) {
            auto mo = root->metaObject();
            qDebug() << "feeding..." << stream.id << fd;
            mo->invokeMethod(root,
                             "addStream",
                             Q_ARG(QVariant, QVariant::fromValue<quint32>(stream.id)),
                             Q_ARG(QVariant, m_handleToken),
                             Q_ARG(QVariant, fd),
                             Q_ARG(QVariant, true));
        }
    }
}

void RecordMe::setPersistMode(PersistMode persistMode)
{
    m_persistMode = persistMode;
}

void RecordMe::setRestoreToken(const QString &restoreToken)
{
    m_restoreToken = restoreToken;
}

void RecordMe::setDuration(int duration)
{
    m_durationTimer->setInterval(duration);
}

#include "moc_RecordMe.cpp"
