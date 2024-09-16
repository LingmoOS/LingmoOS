// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "login_window.h"

#include <QDebug>
#include <QTimer>
#include <QUrl>
#include <QFile>
#include <QLocale>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusConnectionInterface>
#include <QtWebChannel/QWebChannel>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScriptCollection>
#include <QDBusPendingCallWatcher>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <DGuiApplicationHelper>
#include <DTitlebar>
#include <DWidgetUtil>
#include <DPlatformWindowHandle>
#include <QNetworkReply>
#include <string>
#include "sync_client.h"
#include "service/authentication_manager.h"
#include "utils/utils.h"
#include "login_page.h"
#include "ipc/deepinid_interface.h"
#include "ipc/const.h"

DGUI_USE_NAMESPACE

namespace ddc
{

extern QNetworkAccessManager manager;

class LoginWindowPrivate
{
public:
    explicit LoginWindowPrivate(LoginWindow *parent)
        : q_ptr(parent)
    {
        Q_Q(LoginWindow);
        qRegisterMetaType<AuthorizeRequest>("AuthorizeRequest");
        qRegisterMetaType<AuthorizeResponse>("AuthorizeResponse");

        DeepinIDInterface daemonIf(Const::SyncDaemonService, Const::SyncDaemonPath, QDBusConnection::sessionBus());
        QVariantMap userinfo = daemonIf.userInfo();
        this->hasLogin = !userinfo.value("UserID").toString().isEmpty();

        QString clientID = "163296859db7ff8d72010e715ac06bdf6a2a6f87";
        QString redirectURI = "https://sync.deepinid.deepin.com/oauth/callback";
        QStringList scopes = {"base", "user:read", "sync", "dstore"};
        url = utils::authCodeURL(clientID, scopes, redirectURI, "");
        QUrl::toPercentEncoding(url);
        QObject::connect(&authMgr, &AuthenticationManager::requestLogin, parent, [=](const AuthorizeRequest &authReq)
        {
            // if need login,clean cookie;
            this->page->runJavaScript(
                "document.cookie.split(\";\").forEach(function(c) { document.cookie = c.replace(/^ +/, \"\").replace(/=.*/, \"=;expires=\" + new Date().toUTCString() + \";path=/\"); });");

            // sync-daemon在logout时会讲session清空，这里可以不做处理，不然第三方授权登录时会退出系统unionid
//            this->client.cleanSession();

            url = utils::authCodeURL(
                authReq.clientID,
                authReq.scopes,
                authReq.callback,
                authReq.state);

            QUrl::toPercentEncoding(url);
            if(sizeReply->isRunning()) {
                sizeReply->abort();
            }

            q->load();
            Dtk::Widget::moveToCenter(q);
            q->show();
        });

        QObject::connect(&authMgr, &AuthenticationManager::authorizeFinished, parent, [=](const AuthorizeResponse &resp)
        {
            qDebug() << "window authorize finish:";
            auto clientCallback = megs.value(resp.req.clientID);
            qDebug() << resp.req.clientID << clientCallback;
            // 调用DBUS接口前判断接口是否有效
            if (nullptr == clientCallback) {
                qWarning() << "empty clientID" << resp.req.clientID;
                q_ptr->windowloadingEnd = true;
                return;
            }

            if(!clientCallback->isValid())
            {
                qDebug() << "sync daemon is quit, try to restart it";
                clientCallback = new QDBusInterface(clientCallback->service(),
                                                clientCallback->path(), clientCallback->interface());
            }

            qInfo() << "Call OnAuthorized:" << resp.code << resp.state;
            clientCallback->call("OnAuthorized", resp.code, resp.state);
            this->hasLogin = true;
            // 加载完后重新刷空白页面
            page->load(QUrl("about:blank"));
            parent->hide();
            q_ptr->windowloadingEnd = true;
        });

        QObject::connect(&client, &SyncClient::onLogin, parent, [=](
            const QString &sessionID,
            const QString &clientID,
            const QString &code,
            const QString &state)
        {
            qDebug() << "on login";
            this->hasLogin = true;
            if(activatorClientID != clientID){
                this->client.setSession();
            }

            this->authMgr.onLogin(sessionID, clientID, code, state);
        });

        QObject::connect(&client, &SyncClient::onCancel, parent, [=](
            const QString &clientID)
        {
            qDebug() << "onCancel";
            cancelAll(ErrCode::Err_Authorize);
            q_ptr->windowloadingEnd = true;
        });

        QObject::connect(&client, &SyncClient::JSIsReady, parent, [=]()
        {
            qDebug() << "JS is Ready";
            this->page->runJavaScript(
                QString("changeThemeType('%1')").arg(utils::getThemeName()));
            this->page->runJavaScript(
                QString("changeActiveColor('%1')").arg(utils::getActiveColor()));
        });
    }

    void getWindowSize()
    {
        Q_Q(LoginWindow);
        QString sizeurl = utils::windowSizeURL();
        sizeReply = manager.get(QNetworkRequest(QUrl(sizeurl)));
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QObject::connect(sizeReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error), q,
    #else
        QObject::connect(sizeReply, &QNetworkReply::errorOccurred, q,
    #endif
              [=](QNetworkReply::NetworkError code){
            qInfo() << "reply error:" << code << ", " << sizeReply->errorString();
        });
        QObject::connect(sizeReply, &QNetworkReply::finished, q, [=]{
            qInfo() << "size reply finish";
            QByteArray data = sizeReply->readAll();
            qInfo() << "read window size:" << data;
            QJsonParseError error;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
            if(jsonDoc.isNull()) {
                qInfo() << error.errorString();
            }
            else {
                QJsonObject jsonObj = jsonDoc.object();
                QJsonObject clientObj = jsonObj["client"].toObject();
                int width = clientObj["width"].toInt();
                int height = clientObj["height"].toInt() + q->titlebar()->height();
                qInfo() << "resize windows:" << width << height;
                q->resize(width, height);
                Dtk::Widget::moveToCenter(q);
            }
        });
    }

    void cancelAll(const int errCode)
    {
        Q_Q(LoginWindow);
        authMgr.cancel();

        if (client.userInfo().value("UserID").toLongLong() <= 0) {
            for (const auto &id: megs.keys()) {
                cancel(id);
                cancel1(id,errCode);
                megs.remove(id);
            }
        }

        if(ErrCode::Err_LoginPageLoad != errCode){
            //页面隐藏前界面刷为空白： 1. 再次打开时，避免show登录界面然后再刷新一遍的效果；2. 避免show出来时,显示之前界面中用户名密码记录，再刷新登录界面
            this->page->load(QUrl("about:blank"));
            q->hide();
        }
    }

    void cancel(const QString &clientID)
    {
        auto clientCallback = megs.value(clientID);
        qDebug() << "cancel:" << clientID << clientCallback;

        // Interface接口有效时才需要调用OnCancel函数
        if (nullptr == clientCallback || !clientCallback->isValid()) {
            qWarning() << "empty clientID" << clientID;
            return;
        }

        clientCallback->call(QDBus::Block, "OnCancel");
        qDebug() << "call" << clientCallback;

    }

    void cancel1(const QString &clientID,const int errCode)
    {
        auto clientCallback = megs.value(clientID);
        qDebug() << Q_FUNC_INFO << clientID << clientCallback;

        // Interface接口有效时才需要调用OnCancel函数
        if (nullptr == clientCallback || !clientCallback->isValid()) {
            qWarning() << "empty clientID" << clientID;
            return;
        }

        //当client主动关闭 （AuthTerm）的时候，不再回调应用的OnCancel，以免client关闭后又被重新拉起，复杂化处理逻辑
        if(ErrCode::Err_CloseClient != errCode){
            clientCallback->call(QDBus::Block, "OnCancelCode",QVariant(errCode));
            qDebug() << "call" << clientCallback;
        }
    }

    QString url;
    LoginPage *page;

    SyncClient client;
    AuthenticationManager authMgr;

    QNetworkReply *sizeReply;

    bool hasLogin = false;
    QMap<QString, QDBusInterface *> megs;

    LoginWindow *q_ptr;
    Q_DECLARE_PUBLIC(LoginWindow)

    //unsigned int authorizationState;
    const QString activatorClientID = "73560e1f5fcecea6af107d7aa638e3be8b8aa97f";
};

LoginWindow::LoginWindow(QWidget *parent)
    : Dtk::Widget::DMainWindow(parent)
    , dd_ptr(new LoginWindowPrivate(this))
    , m_loginView(new LoginView(this))
{
    Q_D(LoginWindow);

    QFile scriptFile(":/qtwebchannel/qwebchannel.js");
    scriptFile.open(QIODevice::ReadOnly);
    QString apiScript = QString::fromLatin1(scriptFile.readAll());
    scriptFile.close();
    QWebEngineScript script;
    script.setSourceCode(apiScript);
    script.setName("qwebchannel.js");
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setRunsOnSubFrames(false);
    QWebEngineProfile::defaultProfile()->scripts()->insert(script);

    this->titlebar()->setTitle("");
    this->titlebar()->setIcon(QIcon::fromTheme("deepin-id"));
    setWindowFlags(Qt::Window);

    auto flag = windowFlags();
    flag &= ~Qt::WindowMinMaxButtonsHint;
    flag |= Qt::WindowStaysOnTopHint;
    setWindowFlags(flag);
    this->titlebar()->setMenuVisible(false);
    this->titlebar()->setMenuDisabled(true);
    // TODO: workaround for old version dtk, remove as soon as possible.
    this->titlebar()->setDisableFlags(Qt::WindowSystemMenuHint);
    this->titlebar()->setBackgroundTransparent(true);

    auto machineID = d->client.machineID();
    auto *wuri = new WebUrlRequestInterceptor();
    wuri->setHeader({
                        {"X-Machine-ID", machineID.toLatin1()}
                    });
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QWebEngineProfile::defaultProfile()->setRequestInterceptor(wuri);
    #else
    QWebEngineProfile::defaultProfile()->setUrlRequestInterceptor(wuri);
    #endif
    auto *channel = new QWebChannel(this);
    channel->registerObject("client", &d->client);

    d->page = new LoginPage(this);
    d->page->setWebChannel(channel);

    connect(Dtk::Gui::DGuiApplicationHelper::instance(),&Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
        this, [=](Dtk::Gui::DGuiApplicationHelper::ColorType themeType) {
        Q_UNUSED(themeType)
        d->page->runJavaScript(
            QString("changeThemeType('%1')").arg(utils::getThemeName()));
    });

    QDBusConnection::sessionBus().connect(
        "org.deepin.dde.Appearance1",
        "/org/deepin/dde/Appearance1",
        "org.freedesktop.DBus.Properties",
        QLatin1String("PropertiesChanged"),
        this,
        SLOT(syncAppearanceProperties(QString,QMap<QString,QVariant>,QStringList)));

    connect(&d->client, &SyncClient::prepareClose, this, [&]()
    {
        this->close();
        this->windowloadingEnd = true;
    });

    connect(&d->client, &SyncClient::requestHide, this, [&]()
    {
        this->hide();
        this->windowloadingEnd = true;
    });

    QWidget *centerWidget = new QWidget;
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(QMargins(0,0,0,0));
    centerWidget->setLayout(mainlayout);
    setCentralWidget(centerWidget);

    m_loginView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainlayout->addWidget(m_loginView);
    //auto view = new LoginView(this);
    m_loginView->setPage(d->page);
    //this->setCentralWidget(view);
    m_loginView->setFocus();
    m_loginView->page()->setBackgroundColor(DGuiApplicationHelper::instance()->applicationPalette().window().color());

//    updateClient = new UpdateClient(this);
//    updateClient->moveToThread(QCoreApplication::instance()->thread());

    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::themeTypeChanged,
            this, [=](DGuiApplicationHelper::ColorType themeType) {
        switch (themeType) {
        case DGuiApplicationHelper::DarkType:
            m_loginView->page()->setBackgroundColor(DGuiApplicationHelper::instance()->applicationPalette().window().color());
            break;
        case DGuiApplicationHelper::UnknownType:
        case DGuiApplicationHelper::LightType:
        default:
            m_loginView->page()->setBackgroundColor(DGuiApplicationHelper::instance()->applicationPalette().window().color());
            break;
        }
    });

    connect(m_loginView, &QWebEngineView::renderProcessTerminated, [=](QWebEnginePage::RenderProcessTerminationStatus status, int exitCode){
        qInfo() << "render process terminated:" << status << exitCode;
    }); //捕获renderProcessTerminated信号，重启render

    connect(d->page, &QWebEnginePage::loadStarted, this, [=]()
    {
        this->pageLoadOK = true;
        qDebug() << "load started ok";
    });
    connect(d->page, &QWebEnginePage::loadFinished, this, [=](bool ok)
    {
        Q_D(LoginWindow);
        qDebug() << "load finished:" << ok;

        if (!ok) {


            QString oauthURI = "https://login.deepin.org";

            if (!qEnvironmentVariableIsEmpty("DEEPINID_OAUTH_URI")) {
                oauthURI = qgetenv("DEEPINID_OAUTH_URI");
            }

            #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QNetworkConfigurationManager mgr;
            if(!mgr.isOnline()) {
            #else
            bool reachable = false;
            if (QNetworkInformation::loadDefaultBackend()) {
                qDebug() << "load network information default backend succeeded";
                QNetworkInformation *netInfo = QNetworkInformation::instance();
                reachable = netInfo->reachability() == QNetworkInformation::Reachability::Online;
            }
            if(!reachable) {
            #endif
                d->page->load(QUrl(
                                  QString("qrc:/web/network_error.html?%1").
                                  arg(utils::getDeviceType()))
                              );
            }else {
                QHostInfo::lookupHost(oauthURI,this,SLOT(onLookupHost(QHostInfo)));
            }
            this->pageLoadOK = false;
            d->cancelAll(ErrCode::Err_LoginPageLoad);
        }

        this->windowloadingEnd = true;
    });
    connect(d->page, &QWebEnginePage::loadProgress, this, [=](int progress)
    {
        qDebug() << "load progress:" << progress;
    });

    connect(this, &LoginWindow::loadError, this, &LoginWindow::onLoadError, Qt::QueuedConnection);

    login1_Manager_ifc_ = new DBusLogin1Manager(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        QDBusConnection::systemBus(), this);

    connect(login1_Manager_ifc_, &DBusLogin1Manager::PrepareForShutdown,
            this, &LoginWindow::onSystemDown);

    resize(380, 550 + this->titlebar()->height());
    QTimer::singleShot(100, this, SLOT(setFocus()));
    d->getWindowSize();
}

LoginWindow::~LoginWindow() = default;

void LoginWindow::setURL(const QString &url)
{
    Q_D(LoginWindow);
    d->url = url;
}

void LoginWindow::onLookupHost(QHostInfo host)
{
    Q_D(LoginWindow);

    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        //Remote server error
        d->page->load(QUrl("qrc:/web/service_error.html?" + utils::getDeviceType()));
    }else {
        //If the local network and remote server connection is normal ,but QWebEnginePage::loadFinished is not ok.
        d->page->load(QUrl("qrc:/web/unknow_error.html?" + utils::getDeviceType()));
    }

    this->windowloadingEnd = true;
}

void LoginWindow::syncAppearanceProperties(QString str, QMap<QString, QVariant> map, QStringList list)
{
    Q_D(LoginWindow);
    Q_UNUSED(str);
    Q_UNUSED(list);

    if(map.contains("QtActiveColor"))
        d->page->runJavaScript(
                    QString("changeActiveColor('%1')").arg(map.value("QtActiveColor").toString()));

    if(map.contains("StandardFont"))
        d->page->runJavaScript(
                    QString("changeStandardFont('%1')").arg(map.value("StandardFont").toString()));
}

void LoginWindow::onSystemDown(bool isReady)
{
    if(isReady){
        qWarning() << "The operating system prepare for shutdown !";
        qApp->quit();
    }
}

void LoginWindow::load()
{
    Q_D(LoginWindow);
    qDebug() << d->url;
    //d->page->load(QUrl(d->url));
    m_loginView->setUrl(d->url);
}

void LoginWindow::Authorize(const QString &clientID,
                            const QStringList &scopes,
                            const QString &callback,
                            const QString &state)
{
    Q_D(LoginWindow);

    // 界面已显示，请求的验证还未完成
    if (isVisible()) {
        // 如果窗口最小化了，则还原显示
        if (!isActiveWindow()) {
            activateWindow();
        }
        raise();
        return;
    }

    if(this->windowloadingEnd == true){
        this->windowloadingEnd = false;
    }else {
        if (clientID_last == clientID) {
            qDebug() << " MY_Warning: Same twice Authorize";
            return;
        }
    }

    // 打开客户端直接监测更新
//    this->updateClient->checkForUpdate();
    qDebug() << "requestAuthorize" << clientID << scopes << callback << state;
    d->authMgr.requestAuthorize(AuthorizeRequest{
                                    clientID, scopes, callback, state
                                });
    clientID_last = clientID;
//    if (!this->updateClient->isInstartSuccess()) {
//        qDebug() << " INSTART FAIL TRY AGAIN ";
//    }
//    if (!d->hasLogin)
//        this->show();
}

void LoginWindow::AuthTerm(const QString &clientID)
{
    Q_D(LoginWindow);

    if(!d->megs.contains(clientID) || (clientID == ""))
        return;

    std::string curClientID = d->megs.firstKey().toStdString();
    std::string reqClientID = clientID.toStdString();

    if(curClientID == reqClientID){
        d->cancelAll(ErrCode::Err_CloseClient);
    }else{
        d->megs.remove(clientID);
        d->authMgr.delAuthReq(clientID);
    }
}

void LoginWindow::LoadPage(const QString &pageUrl)
{
    setURL(pageUrl);
    load();

    DTK_NAMESPACE::Widget::moveToCenter(this);
    if(windowState() == Qt::WindowMinimized) {
        showNormal();
    }
    else {
        show();
    }
    // 如果窗口最小化了，则还原显示
    if (!isActiveWindow()) {
        activateWindow();
    }

    raise();
}

void LoginWindow::onLoadError()
{
    Q_D(LoginWindow);
    qDebug() << "load error page";
    d->page->load(QUrl("qrc:/web/unknow_error.html?" + utils::getDeviceType()));
}

void LoginWindow::Register(const QString &clientID,
                           const QString &service, const QString &path,
                           const QString &interface)
{
    Q_D(LoginWindow);
    // TODO: memory leak
    qDebug() << "register" << clientID << service << path << interface;
    auto dbusIfc = new QDBusInterface(service, path, interface);

    // Interface接口有效时才缓存列表
    if (!d->megs.contains(clientID) && dbusIfc->isValid()) {
        d->megs.insert(clientID, dbusIfc);
    }
}

void LoginWindow::closeEvent(QCloseEvent *event)
{
    Q_D(LoginWindow);
    m_loginView->close();
    if(this->pageLoadOK)
        d->cancelAll(ErrCode::Err_CloseLoginWindow);
    else
        d->cancelAll(ErrCode::Err_CloseClient);

    QWidget::closeEvent(event);
}

bool LoginWindow::event(QEvent *event)
{
    // FIX #8853 notitlebar not work
    // QWebEngineView in Qt6 will recreate window handle...
    if (QEvent::WinIdChange == event->type())
        Dtk::Widget::DPlatformWindowHandle handle(this);

    return Dtk::Widget::DMainWindow::event(event);
}
}
