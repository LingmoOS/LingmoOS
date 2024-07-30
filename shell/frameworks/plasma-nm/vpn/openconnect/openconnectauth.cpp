/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openconnectauth.h"
#include "openconnectauthworkerthread.h"
#include "openconnectwebauthdialog.h"
#include "ui_openconnectauth.h"

#include "passwordfield.h"
#include "plasma_nm_openconnect.h"

#include <QAtomicPointer>
#include <QByteArray>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDomDocument>
#include <QEventLoop>
#include <QFile>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QMutex>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QWaitCondition>
#include <QWebEngineCookieStore>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <QWebEngineWebAuthUxRequest>

#include <KLocalizedString>

#include "nm-openconnect-service.h"

#include <cstdarg>

extern "C" {
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
}

#if !OPENCONNECT_CHECK_VER(2, 1)
#define __openconnect_set_token_mode(...) -EOPNOTSUPP
#elif !OPENCONNECT_CHECK_VER(2, 2)
#define __openconnect_set_token_mode(vpninfo, mode, secret) openconnect_set_stoken_mode(vpninfo, 1, secret)
#else
#define __openconnect_set_token_mode openconnect_set_token_mode
#endif

#if OPENCONNECT_CHECK_VER(3, 4)
static int updateToken(void *, const char *);
#endif

// name/address: IP/domain name of the host (OpenConnect accepts both, so no difference here)
// group: user group on the server
using VPNHost = struct {
    QString name;
    QString group;
    QString address;
};

using Token = struct {
    oc_token_mode_t tokenMode;
    QByteArray tokenSecret;
};

class OpenconnectAuthWidgetPrivate
{
public:
    Ui_OpenconnectAuth ui;
    NetworkManager::VpnSetting::Ptr setting;
    struct openconnect_info *vpninfo;
    NMStringMap secrets;
    NMStringMap tmpSecrets;
    QMutex mutex;
    QWaitCondition workerWaiting;
    OpenconnectAuthWorkerThread *worker;
    QList<VPNHost> hosts;
    bool userQuit;
    bool formGroupChanged;
    int cancelPipes[2];
    QList<QPair<QString, int>> serverLog;
    int passwordFormIndex;
    QByteArray tokenMode;
    Token token;
    QAtomicPointer<QSemaphore> waitForWebEngineFinish;
    OpenconnectWebAuthDialog *authDialog;

    enum LogLevels { Error = 0, Info, Debug, Trace };
};

OpenconnectAuthWidget::OpenconnectAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
    : SettingWidget(setting, hints, parent)
    , d_ptr(new OpenconnectAuthWidgetPrivate)
{
    Q_D(OpenconnectAuthWidget);
    d->setting = setting;
    d->ui.setupUi(this);
    d->userQuit = false;
    d->formGroupChanged = false;

    if (pipe2(d->cancelPipes, O_NONBLOCK | O_CLOEXEC)) {
        // Should never happen. Just don't do real cancellation if it does
        d->cancelPipes[0] = -1;
        d->cancelPipes[1] = -1;
    }

    connect(d->ui.cmbLogLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OpenconnectAuthWidget::logLevelChanged);
    connect(d->ui.viewServerLog, &QCheckBox::toggled, this, &OpenconnectAuthWidget::viewServerLogToggled);
    connect(d->ui.btnConnect, &QPushButton::clicked, this, &OpenconnectAuthWidget::connectHost);

    d->ui.cmbLogLevel->setCurrentIndex(OpenconnectAuthWidgetPrivate::Debug);
    d->ui.btnConnect->setIcon(QIcon::fromTheme("network-connect"));
    d->ui.viewServerLog->setChecked(false);

    d->worker = new OpenconnectAuthWorkerThread(&d->mutex, &d->workerWaiting, &d->userQuit, &d->formGroupChanged, d->cancelPipes[0]);

    // gets the pointer to struct openconnect_info (defined in openconnect.h), which contains data that OpenConnect needs,
    // and which needs to be populated with settings we get from NM, like host, certificate or private key
    d->vpninfo = d->worker->getOpenconnectInfo();

    connect(d->worker,
            QOverload<const QString &, const QString &, const QString &, bool *>::of(&OpenconnectAuthWorkerThread::validatePeerCert),
            this,
            &OpenconnectAuthWidget::validatePeerCert);
    connect(d->worker, &OpenconnectAuthWorkerThread::openWebEngine, this, &OpenconnectAuthWidget::openWebEngine);
    connect(d->worker, &OpenconnectAuthWorkerThread::processAuthForm, this, &OpenconnectAuthWidget::processAuthForm);
    connect(d->worker, &OpenconnectAuthWorkerThread::updateLog, this, &OpenconnectAuthWidget::updateLog);
    connect(d->worker, QOverload<const QString &>::of(&OpenconnectAuthWorkerThread::writeNewConfig), this, &OpenconnectAuthWidget::writeNewConfig);
    connect(d->worker, &OpenconnectAuthWorkerThread::cookieObtained, this, &OpenconnectAuthWidget::workerFinished);
    connect(d->worker, &OpenconnectAuthWorkerThread::initTokens, this, &OpenconnectAuthWidget::initTokens);

    readConfig();
    readSecrets();

#if OPENCONNECT_CHECK_VER(3, 4)
    openconnect_set_token_callbacks(d->vpninfo, &d->secrets, NULL, &updateToken);
#endif

    // This might be set by readSecrets() so don't connect it until now
    connect(d->ui.cmbHosts, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OpenconnectAuthWidget::connectHost);

    d->authDialog = nullptr;

    KAcceleratorManager::manage(this);
}

OpenconnectAuthWidget::~OpenconnectAuthWidget()
{
    Q_D(OpenconnectAuthWidget);
    QSemaphore *webEngineCancel = d->waitForWebEngineFinish.fetchAndStoreRelaxed(nullptr);
    if (webEngineCancel) {
        webEngineCancel->release();
    }
    d->userQuit = true;
    if (write(d->cancelPipes[1], "x", 1)) {
        // not a lot we can do
    }
    d->workerWaiting.wakeAll();
    d->worker->wait();
    ::close(d->cancelPipes[0]);
    ::close(d->cancelPipes[1]);
    deleteAllFromLayout(d->ui.loginBoxLayout);
    delete d->worker;
    delete d;
}

void OpenconnectAuthWidget::readConfig()
{
    Q_D(OpenconnectAuthWidget);

    const NMStringMap dataMap = d->setting->data();

    if (!dataMap[NM_OPENCONNECT_KEY_GATEWAY].isEmpty()) {
        const QString gw = dataMap[NM_OPENCONNECT_KEY_GATEWAY];
        VPNHost host;
        const int index = gw.indexOf(QLatin1Char('/'));
        if (index > -1) {
            host.name = host.address = gw.left(index);
            host.group = gw.right(gw.length() - index - 1);
        } else {
            host.name = host.address = gw;
        }
        d->hosts.append(host);
    }
    if (!dataMap[NM_OPENCONNECT_KEY_CACERT].isEmpty()) {
        const QByteArray crt = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_CACERT]);
        openconnect_set_cafile(d->vpninfo, OC3DUP(crt.data()));
    }
    if (dataMap[NM_OPENCONNECT_KEY_CSD_ENABLE] == "yes") {
        char *wrapper;
        wrapper = nullptr;
        if (!dataMap[NM_OPENCONNECT_KEY_CSD_WRAPPER].isEmpty()) {
            const QByteArray wrapperScript = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_CSD_WRAPPER]);
            wrapper = strdup(wrapperScript.data());
        }
        openconnect_setup_csd(d->vpninfo, getuid(), 1, wrapper);
    }
    if (!dataMap[NM_OPENCONNECT_KEY_PROXY].isEmpty()) {
        const QByteArray proxy = dataMap[NM_OPENCONNECT_KEY_PROXY].toUtf8();
        openconnect_set_http_proxy(d->vpninfo, OC3DUP(proxy.data()));
    }
#if OPENCONNECT_CHECK_VER(5, 8)
    if (!dataMap[NM_OPENCONNECT_KEY_USERAGENT].isEmpty()) {
        const QByteArray useragent = dataMap[NM_OPENCONNECT_KEY_USERAGENT].toUtf8();
        openconnect_set_useragent(d->vpninfo, OC3DUP(useragent.data()));
    }

    if (!dataMap[NM_OPENCONNECT_KEY_MCACERT].isEmpty()) {
        const QByteArray mca_crt = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_MCACERT]);
        const QByteArray mca_key = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_MCAKEY]);
        openconnect_set_mca_cert(d->vpninfo, OC3DUP(mca_crt.data()), OC3DUP(mca_key.isEmpty() ? nullptr : mca_key.data()));

        if (!mca_crt.isEmpty() && dataMap[NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID] == "yes") {
            openconnect_passphrase_from_fsid(d->vpninfo);
        }
    }
#endif
#if OPENCONNECT_CHECK_VER(5, 5)
    if (!dataMap[NM_OPENCONNECT_KEY_VERSION_STRING].isEmpty()) {
        const QByteArray versionstring = dataMap[NM_OPENCONNECT_KEY_VERSION_STRING].toUtf8();
        openconnect_set_version_string(d->vpninfo, OC3DUP(versionstring.data()));
    }
#endif
    if (!dataMap[NM_OPENCONNECT_KEY_USERCERT].isEmpty()) {
        const QByteArray crt = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_USERCERT]);
        const QByteArray key = QFile::encodeName(dataMap[NM_OPENCONNECT_KEY_PRIVKEY]);
        openconnect_set_client_cert(d->vpninfo, OC3DUP(crt.data()), OC3DUP(key.isEmpty() ? nullptr : key.data()));

        if (!crt.isEmpty() && dataMap[NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID] == "yes") {
            openconnect_passphrase_from_fsid(d->vpninfo);
        }
    }
    if (!dataMap[NM_OPENCONNECT_KEY_PROTOCOL].isEmpty()) {
        const QString protocol = dataMap[NM_OPENCONNECT_KEY_PROTOCOL];
        openconnect_set_protocol(d->vpninfo, OC3DUP(protocol == "juniper" ? "nc" : protocol.toUtf8().data()));
    }
    if (!dataMap[NM_OPENCONNECT_KEY_REPORTED_OS].isEmpty()) {
        const QString reportedOs = dataMap[NM_OPENCONNECT_KEY_REPORTED_OS];
        openconnect_set_reported_os(d->vpninfo, reportedOs.toUtf8().data());
    }

    d->tokenMode = dataMap[NM_OPENCONNECT_KEY_TOKEN_MODE].toUtf8();
}

void OpenconnectAuthWidget::readSecrets()
{
    Q_D(OpenconnectAuthWidget);

    d->secrets = d->setting->secrets();

    if (!d->secrets["xmlconfig"].isEmpty()) {
        const QByteArray config = QByteArray::fromBase64(d->secrets["xmlconfig"].toLatin1());

        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(config.data(), config.size());
        const char *sha1_text = hash.result().toHex();
        openconnect_set_xmlsha1(d->vpninfo, (char *)sha1_text, strlen(sha1_text) + 1);

        QDomDocument xmlconfig;
        xmlconfig.setContent(config);
        const QDomNode anyConnectProfile = xmlconfig.elementsByTagName(QLatin1String("AnyConnectProfile")).at(0);
        bool matchedGw = false;
        const QDomNode serverList = anyConnectProfile.firstChildElement(QLatin1String("ServerList"));
        for (QDomElement entry = serverList.firstChildElement(QLatin1String("HostEntry")); !entry.isNull();
             entry = entry.nextSiblingElement(QLatin1String("HostEntry"))) {
            VPNHost host;
            host.name = entry.firstChildElement(QLatin1String("HostName")).text();
            host.group = entry.firstChildElement(QLatin1String("UserGroup")).text();
            host.address = entry.firstChildElement(QLatin1String("HostAddress")).text();
            // We added the originally configured host in readConfig(). But if
            // it matches one of the ones in the XML config (as presumably it
            // should), remove the original and use the one with the pretty name.
            if (!matchedGw && host.address == d->hosts.at(0).address) {
                d->hosts.removeFirst();
                matchedGw = true;
            }
            d->hosts.append(host);
        }
    }

    for (int i = 0; i < d->hosts.size(); i++) {
        d->ui.cmbHosts->addItem(d->hosts.at(i).name, i);
        if (d->secrets["lasthost"] == d->hosts.at(i).name || d->secrets["lasthost"] == d->hosts.at(i).address) {
            d->ui.cmbHosts->setCurrentIndex(i);
        }
    }

    if (d->secrets["autoconnect"] == "yes") {
        d->ui.chkAutoconnect->setChecked(true);
        QTimer::singleShot(0, this, &OpenconnectAuthWidget::connectHost);
    }

    if (d->secrets["save_passwords"] == "yes") {
        d->ui.chkStorePasswords->setChecked(true);
    }

    d->token.tokenMode = OC_TOKEN_MODE_NONE;
    d->token.tokenSecret = nullptr;

    if (!d->tokenMode.isEmpty()) {
        int ret = 0;
        QByteArray tokenSecret = d->secrets[NM_OPENCONNECT_KEY_TOKEN_SECRET].toUtf8();

        if (d->tokenMode == QStringLiteral("manual") && !tokenSecret.isEmpty()) {
            ret = __openconnect_set_token_mode(d->vpninfo, OC_TOKEN_MODE_STOKEN, tokenSecret);
        } else if (d->tokenMode == QStringLiteral("stokenrc")) {
            ret = __openconnect_set_token_mode(d->vpninfo, OC_TOKEN_MODE_STOKEN, NULL);
        } else if (d->tokenMode == QStringLiteral("totp") && !tokenSecret.isEmpty()) {
            ret = __openconnect_set_token_mode(d->vpninfo, OC_TOKEN_MODE_TOTP, tokenSecret);
        }
#if OPENCONNECT_CHECK_VER(3, 4)
        else if (d->tokenMode == QStringLiteral("hotp") && !tokenSecret.isEmpty()) {
            ret = __openconnect_set_token_mode(d->vpninfo, OC_TOKEN_MODE_HOTP, tokenSecret);
        }
#endif
#if OPENCONNECT_CHECK_VER(5, 0)
        else if (d->tokenMode == "yubioath") {
            /* This needs to be done from a thread because it can call back to
                ask for the PIN */
            d->token.tokenMode = OC_TOKEN_MODE_YUBIOATH;
            if (!tokenSecret.isEmpty()) {
                d->token.tokenSecret = tokenSecret;
            }
        }
#endif
        if (ret) {
            addFormInfo(QLatin1String("dialog-error"), i18n("Failed to initialize software token: %1", ret));
        }
    }
}

void OpenconnectAuthWidget::acceptDialog()
{
    // Find top-level widget as this should be the QDialog itself
    QWidget *widget = parentWidget();
    while (widget->parentWidget() != nullptr) {
        widget = widget->parentWidget();
    }

    auto dialog = qobject_cast<QDialog *>(widget);
    if (dialog) {
        dialog->accept();
    }
}

// This starts the worker thread, which connects to the selected AnyConnect host
// and retrieves the login form
void OpenconnectAuthWidget::connectHost()
{
    Q_D(OpenconnectAuthWidget);

    d->userQuit = true;
    if (write(d->cancelPipes[1], "x", 1)) {
        // not a lot we can do
    }
    d->workerWaiting.wakeAll();
    d->worker->wait();
    d->userQuit = false;

    /* Suck out the cancel byte(s) */
    char buf;
    while (read(d->cancelPipes[0], &buf, 1) == 1) {
        ;
    }
    deleteAllFromLayout(d->ui.loginBoxLayout);
    int i = d->ui.cmbHosts->currentIndex();
    if (i == -1) {
        return;
    }
    i = d->ui.cmbHosts->itemData(i).toInt();
    const VPNHost &host = d->hosts.at(i);
    if (openconnect_parse_url(d->vpninfo, host.address.toLatin1().data())) {
        qCWarning(PLASMA_NM_OPENCONNECT_LOG) << "Failed to parse server URL" << host.address;
        openconnect_set_hostname(d->vpninfo, OC3DUP(host.address.toLatin1().data()));
    }
    if (!openconnect_get_urlpath(d->vpninfo) && !host.group.isEmpty()) {
        openconnect_set_urlpath(d->vpninfo, OC3DUP(host.group.toLatin1().data()));
    }
    d->secrets["lasthost"] = host.name;
    addFormInfo(QLatin1String("dialog-information"), i18n("Contacting host, please wait…"));
    d->worker->start();
}

void OpenconnectAuthWidget::initTokens()
{
    Q_D(OpenconnectAuthWidget);

    if (d->token.tokenMode != OC_TOKEN_MODE_NONE) {
        __openconnect_set_token_mode(d->vpninfo, d->token.tokenMode, d->token.tokenSecret);
    }
}

QVariantMap OpenconnectAuthWidget::setting() const
{
    Q_D(const OpenconnectAuthWidget);

    NMStringMap secrets;
    QVariantMap secretData;

    secrets.insert(d->secrets);
    QString host(openconnect_get_hostname(d->vpninfo));
    const QString port = QString::number(openconnect_get_port(d->vpninfo));
    QString gateway = host + ':' + port;
    const char *urlpath = openconnect_get_urlpath(d->vpninfo);
    if (urlpath) {
        gateway += '/';
        gateway += urlpath;
    }
    secrets.insert(QLatin1String(NM_OPENCONNECT_KEY_GATEWAY), gateway);

    secrets.insert(QLatin1String(NM_OPENCONNECT_KEY_COOKIE), QLatin1String(openconnect_get_cookie(d->vpninfo)));
    openconnect_clear_cookie(d->vpninfo);

#if OPENCONNECT_CHECK_VER(5, 0)
    const char *fingerprint = openconnect_get_peer_cert_hash(d->vpninfo);
#else
    OPENCONNECT_X509 *cert = openconnect_get_peer_cert(d->vpninfo);
    char fingerprint[41];
    openconnect_get_cert_sha1(d->vpninfo, cert, fingerprint);
#endif
    secrets.insert(QLatin1String(NM_OPENCONNECT_KEY_GWCERT), QLatin1String(fingerprint));
    secrets.insert(QLatin1String("autoconnect"), d->ui.chkAutoconnect->isChecked() ? "yes" : "no");
    secrets.insert(QLatin1String("save_passwords"), d->ui.chkStorePasswords->isChecked() ? "yes" : "no");

    NMStringMap::iterator i = secrets.begin();
    while (i != secrets.end()) {
        if (i.value().isEmpty()) {
            i = secrets.erase(i);
        } else {
            ++i;
        }
    }

    secretData.insert("secrets", QVariant::fromValue<NMStringMap>(secrets));

    // These secrets are not officially part of the secrets which would be returned back to NetworkManager. We just
    // need to somehow get them to our secret agent which will handle them separately and store them.
    if (!d->tmpSecrets.isEmpty()) {
        secretData.insert("tmp-secrets", QVariant::fromValue<NMStringMap>(d->tmpSecrets));
    }
    return secretData;
}

#if OPENCONNECT_CHECK_VER(3, 4)
static int updateToken(void *cbdata, const char *tok)
{
    auto secrets = static_cast<NMStringMap *>(cbdata);
    secrets->insert(QLatin1String(NM_OPENCONNECT_KEY_TOKEN_SECRET), QLatin1String(tok));
    return 0;
}
#endif

void OpenconnectAuthWidget::writeNewConfig(const QString &buf)
{
    Q_D(OpenconnectAuthWidget);
    d->secrets["xmlconfig"] = buf;
}

void OpenconnectAuthWidget::updateLog(const QString &message, const int &level)
{
    Q_D(OpenconnectAuthWidget);

    QPair<QString, int> pair;
    pair.first = message;
    if (pair.first.endsWith(QLatin1String("\n"))) {
        pair.first.chop(1);
    }
    switch (level) {
    case PRG_ERR:
        pair.second = OpenconnectAuthWidgetPrivate::Error;
        break;
    case PRG_INFO:
        pair.second = OpenconnectAuthWidgetPrivate::Info;
        break;
    case PRG_DEBUG:
        pair.second = OpenconnectAuthWidgetPrivate::Debug;
        break;
    case PRG_TRACE:
        pair.second = OpenconnectAuthWidgetPrivate::Trace;
        break;
    }
    if (pair.second <= d->ui.cmbLogLevel->currentIndex()) {
        d->ui.serverLog->append(pair.first);
    }

    d->serverLog.append(pair);
    if (d->serverLog.size() > 100) {
        d->serverLog.removeFirst();
    }
}

void OpenconnectAuthWidget::logLevelChanged(int newLevel)
{
    Q_D(OpenconnectAuthWidget);
    d->ui.serverLog->clear();
    QList<QPair<QString, int>>::const_iterator i;

    for (i = d->serverLog.constBegin(); i != d->serverLog.constEnd(); ++i) {
        QPair<QString, int> pair = *i;
        if (pair.second <= newLevel) {
            d->ui.serverLog->append(pair.first);
        }
    }
}

void OpenconnectAuthWidget::handleWebEngineCookie(const QNetworkCookie &cookie)
{
    Q_D(OpenconnectAuthWidget);
    const char *cookiesArr[3] = {nullptr};

    cookiesArr[0] = cookie.name().constData();
    cookiesArr[1] = cookie.value().constData();

#if OPENCONNECT_CHECK_VER(5, 7)
    struct oc_webview_result res;
    res.headers = nullptr;
    res.cookies = cookiesArr;
    // Hack due to lack of NULL pointer check in AnyConnect sso_detect_done
    // logic in libopenconnect.
    res.uri = "";
    if (!openconnect_webview_load_changed(d->vpninfo, &res)) {
        QSemaphore *waitForWebEngineFinish = d->waitForWebEngineFinish.fetchAndStoreRelaxed(nullptr);
        if (waitForWebEngineFinish) {
            waitForWebEngineFinish->release();
        }
    }
#endif
}

void OpenconnectAuthWidget::handleWebEngineLoad(const QWebEngineLoadingInfo &loadingInfo)
{
    Q_D(OpenconnectAuthWidget);
    const char *cookiesArr[1] = {nullptr};
    QList<const char *> responseHeaderList;

    switch (loadingInfo.status()) {
    case QWebEngineLoadingInfo::LoadSucceededStatus:
    case QWebEngineLoadingInfo::LoadFailedStatus:
        break;
    default:
        return;
    }

    const QMultiMap<QByteArray, QByteArray> responseHeaders = loadingInfo.responseHeaders();
    QMultiMapIterator<QByteArray, QByteArray> headerIter(responseHeaders);
    responseHeaderList.reserve((responseHeaders.size() * 2) + 1);
    while (headerIter.hasNext()) {
        headerIter.next();
        responseHeaderList.push_back(headerIter.key().constData());
        responseHeaderList.push_back(headerIter.value().constData());
    }
    responseHeaderList.push_back(nullptr);

#if OPENCONNECT_CHECK_VER(5, 7)
    struct oc_webview_result res;
    res.headers = responseHeaderList.data();
    // Hack due to lack of NULL pointer check in AnyConnect sso_detect_done
    // logic in libopenconnect.
    res.cookies = cookiesArr;
    res.uri = "";
    if (!openconnect_webview_load_changed(d->vpninfo, &res)) {
        QSemaphore *waitForWebEngineFinish = d->waitForWebEngineFinish.fetchAndStoreRelaxed(nullptr);
        if (waitForWebEngineFinish) {
            waitForWebEngineFinish->release();
        }
    }
#endif
}

void OpenconnectAuthWidget::handleWebEngineUrl(const QUrl &url)
{
    Q_D(OpenconnectAuthWidget);
    // Hack due to lack of NULL pointer check in AnyConnect sso_detect_done
    // logic in libopenconnect.
    const char *cookiesArr[1] = {nullptr};
    QByteArray urlByteArray = url.toString().toLocal8Bit();

#if OPENCONNECT_CHECK_VER(5, 7)
    struct oc_webview_result res;
    res.headers = nullptr;
    res.cookies = cookiesArr;
    res.uri = urlByteArray.constData();
    if (!openconnect_webview_load_changed(d->vpninfo, &res)) {
        QSemaphore *waitForWebEngineFinish = d->waitForWebEngineFinish.fetchAndStoreRelaxed(nullptr);
        if (waitForWebEngineFinish) {
            waitForWebEngineFinish->release();
        }
    }
#endif
}

void OpenconnectAuthWidget::handleWebAuthUxRequested(QWebEngineWebAuthUxRequest *request)
{
    Q_D(OpenconnectAuthWidget);

    if (d->authDialog)
        d->authDialog->close();

    d->authDialog = new OpenconnectWebAuthDialog(request, this);
    d->authDialog->setAttribute(Qt::WA_DeleteOnClose);
    d->authDialog->setModal(false);
    d->authDialog->setWindowFlags(d->authDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(request, &QWebEngineWebAuthUxRequest::stateChanged, this, &OpenconnectAuthWidget::handleWebAuthUxStateChanged);

    d->authDialog->show();
}

void OpenconnectAuthWidget::handleWebAuthUxStateChanged(QWebEngineWebAuthUxRequest::WebAuthUxState state)
{
    Q_D(OpenconnectAuthWidget);

    if (QWebEngineWebAuthUxRequest::WebAuthUxState::Completed == state || QWebEngineWebAuthUxRequest::WebAuthUxState::Cancelled == state) {
        if (d->authDialog)
            d->authDialog->close();
    } else {
        d->authDialog->updateDisplay();
    }
}

void OpenconnectAuthWidget::openWebEngine(const char *loginUri, QSemaphore *waitForWebEngineFinish)
{
    Q_D(OpenconnectAuthWidget);
    d->waitForWebEngineFinish.storeRelease(waitForWebEngineFinish);
    auto webEngineView = new QWebEngineView(this);
    QWebEnginePage *page = webEngineView->page();
    QWebEngineCookieStore *cookieStore = page->profile()->cookieStore();

    connect(webEngineView, &QWebEngineView::urlChanged, this, &OpenconnectAuthWidget::handleWebEngineUrl);
    connect(page, &QWebEnginePage::loadingChanged, this, &OpenconnectAuthWidget::handleWebEngineLoad);
    connect(page, &QWebEnginePage::webAuthUxRequested, this, &OpenconnectAuthWidget::handleWebAuthUxRequested);
    connect(cookieStore, &QWebEngineCookieStore::cookieAdded, this, &OpenconnectAuthWidget::handleWebEngineCookie);
    cookieStore->loadAllCookies();

    webEngineView->load(QUrl(loginUri, QUrl::TolerantMode));
    // QWebEngineView sizeHint fails to size window correctly based on contents
    // when QLayout::setSizeConstraint(QLayout::SetFixedSize) is set. Using same
    // size as webkitgtk is set to in GNOME/NetworkManager-openconnect.
    webEngineView->setFixedSize(640, 480);

    d->ui.loginBoxLayout->addWidget(webEngineView);
}

void OpenconnectAuthWidget::addFormInfo(const QString &iconName, const QString &message)
{
    Q_D(OpenconnectAuthWidget);

    auto layout = new QHBoxLayout();
    auto icon = new QLabel(this);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(icon->sizePolicy().hasHeightForWidth());
    icon->setSizePolicy(sizePolicy);
    icon->setMinimumSize(QSize(16, 16));
    icon->setMaximumSize(QSize(16, 16));
    layout->addWidget(icon);

    auto text = new QLabel(this);
    text->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
    text->setWordWrap(true);
    layout->addWidget(text);

    const int iconSize = icon->style()->pixelMetric(QStyle::PixelMetric::PM_SmallIconSize);
    icon->setPixmap(QIcon::fromTheme(iconName).pixmap(iconSize));
    text->setText(message);

    d->ui.loginBoxLayout->addLayout(layout);
}

void OpenconnectAuthWidget::processAuthForm(struct oc_auth_form *form)
{
    Q_D(OpenconnectAuthWidget);

    deleteAllFromLayout(d->ui.loginBoxLayout);

    struct oc_form_opt *opt;
    auto layout = new QFormLayout();
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bool focusSet = false;
    for (opt = form->opts; opt; opt = opt->next) {
        if (opt->type == OC_FORM_OPT_HIDDEN || IGNORE_OPT(opt)) {
            continue;
        }
        auto text = new QLabel(this);
        text->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
        text->setText(QString(opt->label));
        QWidget *widget = nullptr;
        const QString key = QString("form:%1:%2").arg(QLatin1String(form->auth_id)).arg(QLatin1String(opt->name));
        const QString value = d->secrets.value(key);
        if (opt->type == OC_FORM_OPT_PASSWORD || opt->type == OC_FORM_OPT_TEXT) {
            auto le = new PasswordField(this);
            le->setText(value);
            if (opt->type == OC_FORM_OPT_PASSWORD) {
                le->setPasswordModeEnabled(true);
            }
            if (!focusSet && le->text().isEmpty()) {
                le->setFocus(Qt::OtherFocusReason);
                focusSet = true;
            }
            widget = qobject_cast<QWidget *>(le);
        } else if (opt->type == OC_FORM_OPT_SELECT) {
            auto cmb = new QComboBox(this);
            auto sopt = reinterpret_cast<oc_form_opt_select *>(opt);
#if !OPENCONNECT_CHECK_VER(8, 0)
            const QString protocol = d->setting->data()[NM_OPENCONNECT_KEY_PROTOCOL];
#endif
            for (int i = 0; i < sopt->nr_choices; i++) {
                cmb->addItem(QString::fromUtf8(FORMCHOICE(sopt, i)->label), QString::fromUtf8(FORMCHOICE(sopt, i)->name));
                if (value == QString::fromUtf8(FORMCHOICE(sopt, i)->name)) {
                    cmb->setCurrentIndex(i);
#if !OPENCONNECT_CHECK_VER(8, 0)
                    if (protocol != QLatin1String("nc") && sopt == AUTHGROUP_OPT(form) && i != AUTHGROUP_SELECTION(form)) {
#else
                    if (sopt == AUTHGROUP_OPT(form) && i != AUTHGROUP_SELECTION(form)) {
#endif
                        QTimer::singleShot(0, this, &OpenconnectAuthWidget::formGroupChanged);
                    }
                }
            }
#if !OPENCONNECT_CHECK_VER(8, 0)
            if (protocol != QLatin1String("nc") && sopt == AUTHGROUP_OPT(form)) {
#else
            if (sopt == AUTHGROUP_OPT(form)) {
#endif
                connect(cmb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OpenconnectAuthWidget::formGroupChanged);
            }
            widget = qobject_cast<QWidget *>(cmb);
        }
        if (widget) {
            widget->setProperty("openconnect_opt", (quintptr)opt);
            widget->setSizePolicy(policy);
            layout->addRow(text, widget);
        }
    }
    if (!layout->rowCount()) {
        delete layout;
        d->workerWaiting.wakeAll();
        return;
    }

    if (form->banner) {
        addFormInfo(QLatin1String("dialog-information"), form->banner);
    }
    if (form->message) {
        addFormInfo(QLatin1String("dialog-information"), form->message);
    }
    if (form->error) {
        addFormInfo(QLatin1String("dialog-error"), form->error);
    }

    d->ui.loginBoxLayout->addLayout(layout);
    d->passwordFormIndex = d->ui.loginBoxLayout->count() - 1;

    auto box = new QDialogButtonBox(this);
    QPushButton *btn = box->addButton(QDialogButtonBox::Ok);
    btn->setText(i18nc("Verb, to proceed with login", "Login"));
    btn->setDefault(true);
    d->ui.loginBoxLayout->addWidget(box);
    box->setProperty("openconnect_form", (quintptr)form);

    connect(box, &QDialogButtonBox::accepted, this, &OpenconnectAuthWidget::formLoginClicked);
}

void OpenconnectAuthWidget::validatePeerCert(const QString &fingerprint, const QString &peerCert, const QString &reason, bool *accepted)
{
    Q_D(OpenconnectAuthWidget);

    const QString host = QLatin1String(openconnect_get_hostname(d->vpninfo));
    const QString port = QString::number(openconnect_get_port(d->vpninfo));
    const QString key = QString("certificate:%1:%2").arg(host, port);
    const QString value = d->secrets.value(key);

#if !OPENCONNECT_CHECK_VER(5, 0)
#define openconnect_check_peer_cert_hash(v, d) strcmp(d, fingerprint.toUtf8().data())
#endif

    if (openconnect_check_peer_cert_hash(d->vpninfo, value.toUtf8().data())) {
        QPointer<QDialog> dialog = new QDialog(this);
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog.data()->setWindowModality(Qt::WindowModal);

        auto widget = new QWidget(dialog.data());
        QVBoxLayout *verticalLayout;
        QHBoxLayout *horizontalLayout;
        QLabel *icon;
        QLabel *infoText;
        QTextBrowser *certificate;

        verticalLayout = new QVBoxLayout(widget);
        horizontalLayout = new QHBoxLayout(widget);
        icon = new QLabel(widget);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(icon->sizePolicy().hasHeightForWidth());
        icon->setSizePolicy(sizePolicy);
        icon->setMinimumSize(QSize(48, 48));
        icon->setMaximumSize(QSize(48, 48));

        horizontalLayout->addWidget(icon);

        infoText = new QLabel(widget);
        infoText->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

        horizontalLayout->addWidget(infoText);

        verticalLayout->addLayout(horizontalLayout);

        certificate = new QTextBrowser(widget);
        certificate->setTextInteractionFlags(Qt::TextSelectableByMouse);
        certificate->setOpenLinks(false);

        verticalLayout->addWidget(certificate);

        const int iconSize = icon->style()->pixelMetric(QStyle::PixelMetric::PM_LargeIconSize);
        icon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(iconSize));
        infoText->setText(
            i18n("Check failed for certificate from VPN server \"%1\".\n"
                 "Reason: %2\nAccept it anyway?",
                 openconnect_get_hostname(d->vpninfo),
                 reason));
        infoText->setWordWrap(true);
        certificate->setText(peerCert);

        dialog->setLayout(new QVBoxLayout);
        auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
        connect(buttons, &QDialogButtonBox::accepted, dialog.data(), &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, dialog.data(), &QDialog::reject);
        dialog->layout()->addWidget(widget);
        dialog->layout()->addWidget(buttons);

        const NMStringMap dataMap = d->setting->data();
        buttons->button(QDialogButtonBox::Ok)->setEnabled(dataMap[NM_OPENCONNECT_KEY_PREVENT_INVALID_CERT] != "yes");

        if (dialog.data()->exec() == QDialog::Accepted) {
            *accepted = true;
        } else {
            *accepted = false;
        }
    } else {
        *accepted = true;
    }
    if (*accepted) {
        d->secrets.insert(key, QString(fingerprint));
    }
    d->mutex.lock();
    d->workerWaiting.wakeAll();
    d->mutex.unlock();
}

void OpenconnectAuthWidget::formGroupChanged()
{
    Q_D(OpenconnectAuthWidget);

    d->formGroupChanged = true;
    formLoginClicked();
}

// Writes the user input from the form into the oc_auth_form structs we got from
// libopenconnect, and wakes the worker thread up to try to log in and obtain a
// cookie with this data
void OpenconnectAuthWidget::formLoginClicked()
{
    Q_D(OpenconnectAuthWidget);

    const int lastIndex = d->ui.loginBoxLayout->count() - 1;
    QLayout *layout = d->ui.loginBoxLayout->itemAt(d->passwordFormIndex)->layout();
    struct oc_auth_form *form = (struct oc_auth_form *)d->ui.loginBoxLayout->itemAt(lastIndex)->widget()->property("openconnect_form").value<quintptr>();

    for (int i = 0; i < layout->count(); i++) {
        QLayoutItem *item = layout->itemAt(i);
        QWidget *widget = item->widget();
        if (widget && widget->property("openconnect_opt").isValid()) {
            struct oc_form_opt *opt = (struct oc_form_opt *)widget->property("openconnect_opt").value<quintptr>();
            const QString key = QString("form:%1:%2").arg(QLatin1String(form->auth_id)).arg(QLatin1String(opt->name));
            if (opt->type == OC_FORM_OPT_PASSWORD || opt->type == OC_FORM_OPT_TEXT) {
                auto le = qobject_cast<PasswordField *>(widget);
                QByteArray text = le->text().toUtf8();
                openconnect_set_option_value(opt, text.data());
                if (opt->type == OC_FORM_OPT_TEXT) {
                    d->secrets.insert(key, le->text());
                } else {
                    d->tmpSecrets.insert(key, le->text());
                }
            } else if (opt->type == OC_FORM_OPT_SELECT) {
                auto cbo = qobject_cast<QComboBox *>(widget);
                QByteArray text = cbo->itemData(cbo->currentIndex()).toString().toLatin1();
                openconnect_set_option_value(opt, text.data());
                d->secrets.insert(key, cbo->itemData(cbo->currentIndex()).toString());
            }
        }
    }

    deleteAllFromLayout(d->ui.loginBoxLayout);
    d->workerWaiting.wakeAll();
}

void OpenconnectAuthWidget::workerFinished(const int &ret)
{
    Q_D(OpenconnectAuthWidget);

    if (ret < 0) {
        QString message;
        QList<QPair<QString, int>>::const_iterator i;
        for (i = d->serverLog.constEnd() - 1; i >= d->serverLog.constBegin(); --i) {
            QPair<QString, int> pair = *i;
            if (pair.second <= OpenconnectAuthWidgetPrivate::Error) {
                message = pair.first;
                break;
            }
        }
        if (message.isEmpty()) {
            message = i18n("Connection attempt was unsuccessful.");
        }
        deleteAllFromLayout(d->ui.loginBoxLayout);
        addFormInfo(QLatin1String("dialog-error"), message);
    } else {
        deleteAllFromLayout(d->ui.loginBoxLayout);
        acceptDialog();
    }
}

void OpenconnectAuthWidget::deleteAllFromLayout(QLayout *layout)
{
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QLayout *itemLayout = item->layout()) {
            deleteAllFromLayout(itemLayout);
            itemLayout->deleteLater();
        } else {
            item->widget()->deleteLater();
        }
        delete item;
    }
    layout->invalidate();
}

void OpenconnectAuthWidget::viewServerLogToggled(bool toggled)
{
    Q_D(OpenconnectAuthWidget);
    d->ui.lblLogLevel->setVisible(toggled);
    d->ui.cmbLogLevel->setVisible(toggled);
    if (toggled) {
        delete d->ui.verticalLayout->takeAt(5);
        QSizePolicy policy = d->ui.serverLogBox->sizePolicy();
        policy.setVerticalPolicy(QSizePolicy::Expanding);
        d->ui.serverLogBox->setSizePolicy(policy);
        d->ui.serverLog->setVisible(true);
    } else {
        auto verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        d->ui.verticalLayout->addItem(verticalSpacer);
        d->ui.serverLog->setVisible(false);
        QSizePolicy policy = d->ui.serverLogBox->sizePolicy();
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        d->ui.serverLogBox->setSizePolicy(policy);
    }
}

#include "moc_openconnectauth.cpp"
