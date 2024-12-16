// Copyright (C) 2023 Dingyuan Zhang <lxz@mkacg.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "treeland.h"

#include "DisplayManager.h"
#include "DisplayManagerSession.h"
#include "MessageHandler.h"
#include "Messages.h"
#include "SignalHandler.h"
#include "SocketWriter.h"
#include "compositor1adaptor.h"
#include "foreigntoplevelmanagerv1.h"
#include "helper.h"
#include "outputmanagement.h"
#include "personalizationmanager.h"
#include "shortcutmanager.h"
#include "wallpapercolor.h"
#include "windowmanagement.h"

#include <WCursor>
#include <WSeat>
#include <WSurface>
#include <wordexp.h>
#include <woutputrenderwindow.h>
#include <wrenderhelper.h>
#include <wsocket.h>

#include <qwbackend.h>
#include <qwcompositor.h>
#include <qwdisplay.h>
#include <qwlogging.h>
#include <qwoutput.h>
#include <qwxwayland.h>

#include <DLog>

#include <QCommandLineParser>
#include <QDebug>
#include <QEventLoop>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickView>
#include <QTimer>
#include <QtLogging>

#include <pwd.h>
#include <sys/socket.h>

extern "C" {
#define class _class
#include <wlr/xwayland/server.h>
#include <wlr/xwayland/xwayland.h>
#undef class
}

Q_LOGGING_CATEGORY(debug, "treeland.kernel.debug", QtDebugMsg);

using namespace DDM;
DCORE_USE_NAMESPACE;

QString unescape(const QString &str) noexcept
{
    QString unescapedStr;
    for (qsizetype i = 0; i < str.size(); ++i) {
        auto c = str.at(i);
        if (c != '\\') {
            unescapedStr.append(c);
            continue;
        }

        switch (str.at(i + 1).toLatin1()) {
        default:
            unescapedStr.append(c);
            break;
        case 'n':
            unescapedStr.append('\n');
            ++i;
            break;
        case 't':
            unescapedStr.append('\t');
            ++i;
            break;
        case 'r':
            unescapedStr.append('\r');
            ++i;
            break;
        case '\\':
            unescapedStr.append('\\');
            ++i;
            break;
        case ';':
            unescapedStr.append(';');
            ++i;
            break;
        case 's': {
            unescapedStr.append(R"(\ )");
            ++i;
        } break;
        }
    }

    return unescapedStr;
}

std::optional<QStringList> unescapeExecArgs(const QString &str) noexcept
{
    auto unescapedStr = unescape(str);
    if (unescapedStr.isEmpty()) {
        qWarning() << "unescape Exec failed.";
        return std::nullopt;
    }

    auto deleter = [](wordexp_t *word) {
        wordfree(word);
        delete word;
    };
    std::unique_ptr<wordexp_t, decltype(deleter)> words{ new (std::nothrow)
                                                             wordexp_t{ 0, nullptr, 0 },
                                                         deleter };

    if (auto ret = wordexp(unescapedStr.toLocal8Bit(), words.get(), WRDE_SHOWERR); ret != 0) {
        QString errMessage;
        switch (ret) {
        case WRDE_BADCHAR:
            errMessage = "BADCHAR";
            break;
        case WRDE_BADVAL:
            errMessage = "BADVAL";
            break;
        case WRDE_CMDSUB:
            errMessage = "CMDSUB";
            break;
        case WRDE_NOSPACE:
            errMessage = "NOSPACE";
            break;
        case WRDE_SYNTAX:
            errMessage = "SYNTAX";
            break;
        default:
            errMessage = "unknown";
        }
        qWarning() << "wordexp error: " << errMessage;
        return std::nullopt;
    }

    QStringList execList;
    for (std::size_t i = 0; i < words->we_wordc; ++i) {
        execList.emplace_back(words->we_wordv[i]);
    }

    return execList;
}

namespace TreeLand {
TreeLand::TreeLand(TreeLandAppContext context)
    : QObject()
    , m_context(context)
{
    if (!context.socket.isEmpty()) {
        qInstallMessageHandler(DDM::GreeterMessageHandler);

        new DDM::SignalHandler;
    }

    m_socket = new QLocalSocket(this);

    connect(m_socket, &QLocalSocket::connected, this, &TreeLand::connected);
    connect(m_socket, &QLocalSocket::disconnected, this, &TreeLand::disconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &TreeLand::readyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &TreeLand::error);

    setup();
    Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");

    if (!context.socket.isEmpty()) {
        Q_ASSERT(helper);
        auto connectToServer = [this, context] {
            m_socket->connectToServer(context.socket);
        };

        connect(helper, &Helper::socketFileChanged, this, connectToServer);

        if (!helper->waylandSocket().isEmpty()) {
            connectToServer();
        }
    } else {
        struct passwd *pw = getpwuid(getuid());
        helper->setCurrentUser(pw->pw_name);
    }

    if (!context.run.isEmpty()) {
        qInfo() << "run cmd:" << context.run;
        auto exec = [runCmd = context.run, helper] {
            if (auto cmdline = unescapeExecArgs(runCmd); cmdline) {
                auto cmdArgs = cmdline.value();

                auto envs = QProcessEnvironment::systemEnvironment();
                envs.insert("WAYLAND_DISPLAY", helper->waylandSocket());
                envs.insert("DISPLAY", helper->xwaylandSocket());

                QProcess process;
                process.setProgram(cmdArgs.constFirst());
                process.setArguments(cmdArgs.mid(1));
                process.setProcessEnvironment(envs);
                process.startDetached();
            }
        };
        auto con =
            connect(helper, &Helper::socketFileChanged, this, exec, Qt::SingleShotConnection);

        if (!helper->waylandSocket().isEmpty()) {
            QObject::disconnect(con);
            exec();
        }
    }
}

class DtkInterceptor : public QObject, public QQmlAbstractUrlInterceptor
{
public:
    DtkInterceptor(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    QUrl intercept(const QUrl &path, DataType type)
    {
        if (type != DataType::QmlFile)
            return path;
        if (path.path().endsWith("overridable/InWindowBlur.qml")) {
            qDebug() << "Override dtk's InWindowBlur";
            return QStringLiteral("qrc:/treeland/override/dtk/InWindowBlur.qml");
        }

        return path;
    }
};

void TreeLand::setup()
{
    WRenderHelper::setupRendererBackend();
    m_engine = new QQmlApplicationEngine(this);
    m_engine->addUrlInterceptor(new DtkInterceptor(this));
    m_engine->rootContext()->setContextProperty("TreeLand", this);

    m_server = new WServer(this);
    m_server->start();

    qmlRegisterSingletonInstance<Helper>("TreeLand.Utils", 1, 0, "Helper", new Helper(m_server));

    qmlRegisterSingletonInstance<ForeignToplevelV1>("TreeLand.Protocols",
                                                    1,
                                                    0,
                                                    "ForeignToplevelV1",
                                                    m_server->attach<ForeignToplevelV1>());

    qmlRegisterSingletonInstance<PrimaryOutputV1>("TreeLand.Protocols",
                                                  1,
                                                  0,
                                                  "PrimaryOutputV1",
                                                  m_server->attach<PrimaryOutputV1>());

    qmlRegisterSingletonInstance<PersonalizationV1>("TreeLand.Protocols",
                                                    1,
                                                    0,
                                                    "PersonalizationV1",
                                                    m_server->attach<PersonalizationV1>());

    qmlRegisterSingletonInstance<WallpaperColorV1>("TreeLand.Protocols",
                                                   1,
                                                   0,
                                                   "WallpaperColorV1",
                                                   m_server->attach<WallpaperColorV1>());

    qmlRegisterSingletonInstance<WindowManagementV1>("TreeLand.Protocols",
                                                     1,
                                                     0,
                                                     "WindowManagementV1",
                                                     m_server->attach<WindowManagementV1>());

    m_engine->loadFromModule("TreeLand", "Main");

    auto window = m_engine->rootObjects().first()->findChild<WOutputRenderWindow *>();
    Q_ASSERT(window);

    Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");
    Q_ASSERT(helper);

    helper->initProtocols(window);

    m_server->attach<ShortcutV1>(helper);

    PersonalizationV1 *personalization =
        m_engine->singletonInstance<PersonalizationV1 *>("TreeLand.Protocols", "PersonalizationV1");
    Q_ASSERT(personalization);

    connect(
        helper,
        &Helper::currentUserChanged,
        personalization,
        [personalization](const QString &user) {
            struct passwd *pwd = getpwnam(user.toUtf8());
            personalization->setUserId(pwd->pw_uid);
        },
        Qt::QueuedConnection);
}

bool TreeLand::testMode() const
{
    return m_context.socket.isEmpty();
}

void TreeLand::connected()
{
    // log connection
    qDebug() << "Connected to the daemon.";

    Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");
    Q_ASSERT(helper);

    connect(helper, &Helper::backToNormal, this, [this] {
        DDM::SocketWriter(m_socket) << quint32(DDM::GreeterMessages::BackToNormal);
    });
    connect(helper, &Helper::reboot, this, [this] {
        DDM::SocketWriter(m_socket) << quint32(DDM::GreeterMessages::Reboot);
    });

    // send connected message
    DDM::SocketWriter(m_socket) << quint32(DDM::GreeterMessages::Connect);
}

void TreeLand::retranslate() noexcept
{
    m_engine->retranslate();
}

void TreeLand::disconnected()
{
    // log disconnection
    qDebug() << "Disconnected from the daemon.";

    Q_EMIT socketDisconnected();

    qDebug() << "Display Manager is closed socket connect, quiting treeland.";
    qApp->exit();
}

void TreeLand::error()
{
    qCritical() << "Socket error: " << m_socket->errorString();
}

void TreeLand::readyRead()
{
    // input stream
    QDataStream input(m_socket);

    while (input.device()->bytesAvailable()) {
        // read message
        quint32 message;
        input >> message;

        switch (DDM::DaemonMessages(message)) {
        case DDM::DaemonMessages::Capabilities: {
            // log message
            qDebug() << "Message received from daemon: Capabilities";
        } break;
        case DDM::DaemonMessages::UserActivateMessage: {
            QString user;
            input >> user;

            for (auto key : m_userWaylandSocket.keys()) {
                m_userWaylandSocket[key]->setEnabled(key == user);
            }

            struct passwd *pwd = getpwnam(user.toUtf8());
            Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");
            helper->setCurrentUser(pwd->pw_name);
        } break;
        case DDM::DaemonMessages::SwitchToGreeter: {
            Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");

            Q_ASSERT(helper);
            Q_EMIT helper->greeterVisibleChanged();
        } break;
        default:
            break;
        }
    }
}

bool TreeLand::ActivateWayland(QDBusUnixFileDescriptor _fd)
{
    if (!_fd.isValid()) {
        return false;
    }

    auto fd = std::make_shared<QDBusUnixFileDescriptor>();
    fd->swap(_fd);

    auto socket = std::make_shared<WSocket>(true);
    socket->create(fd->fileDescriptor(), false);

    m_server->addSocket(socket.get());

    auto uid = connection().interface()->serviceUid(message().service());
    struct passwd *pw;
    pw = getpwuid(uid);
    QString user{ pw->pw_name };
    m_userWaylandSocket[user] = socket;
    m_userDisplayFds[user] = fd;

    DisplayManager manager("org.freedesktop.DisplayManager",
                           "/org/freedesktop/DisplayManager",
                           QDBusConnection::systemBus());

    const auto sessionPath = manager.lastSession();
    if (!sessionPath.path().isEmpty()) {
        DisplaySession session(manager.service(), sessionPath.path(), QDBusConnection::systemBus());
        socket->setEnabled(session.userName() == user);
    }

    connect(connection().interface(),
            &QDBusConnectionInterface::serviceUnregistered,
            socket.get(),
            [this, user] {
                m_userWaylandSocket.remove(user);
                m_userDisplayFds.remove(user);
            });

    return true;
}

QString TreeLand::XWaylandName()
{
    auto uid = connection().interface()->serviceUid(message().service());
    struct passwd *pw;
    pw = getpwuid(uid);
    QString user{ pw->pw_name };

    Helper *helper = m_engine->singletonInstance<Helper *>("TreeLand.Utils", "Helper");
    Q_ASSERT(helper);

    const QString &display = helper->xwaylandSocket();
    qCDebug(debug) << QString("user %1 got xwayland display %2.").arg(user).arg(display);

    return display;
}

} // namespace TreeLand

int main(int argc, char *argv[])
{
    WRenderHelper::setupRendererBackend();

    QWLog::init();
    WServer::initializeQPA();
    // QQuickStyle::setStyle("Material");

    QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QGuiApplication::setQuitOnLastWindowClosed(false);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("lingmo");
    app.setApplicationName("treeland");

    DLogManager::registerConsoleAppender();
    DLogManager::registerJournalAppender();
    DLogManager::registerFileAppender();

    QCommandLineOption socket({ "s", "socket" }, "set ddm socket", "socket");
    QCommandLineOption run({ "r", "run" }, "run a process", "run");

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({ socket, run });

    parser.process(app);

    TreeLand::TreeLand treeland({ parser.value(socket), parser.value(run) });

    new Compositor1Adaptor(&treeland);

    // init dbus after QML engine loaded.
    QDBusConnection::systemBus().registerService("org.lingmo.Compositor1");
    QDBusConnection::systemBus().registerObject("/org/lingmo/Compositor1", &treeland);

    QDBusConnection::sessionBus().registerService("org.lingmo.Compositor1");
    QDBusConnection::sessionBus().registerObject("/org/lingmo/Compositor1", &treeland);

    return app.exec();
}
