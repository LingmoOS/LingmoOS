/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2002-2004 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletd.h"
#include "kwalletd_debug.h"

#include "kbetterthankdialog.h"
#include "kwalletfreedesktopcollection.h"
#include "kwalletfreedesktopitem.h"
#include "kwalletfreedesktopprompt.h"
#include "kwalletfreedesktopservice.h"
#include "kwalletfreedesktopsession.h"
#include "kwalletportalsecrets.h"
#include "kwalletwizard.h"

#ifdef HAVE_GPGMEPP
#include "knewwalletdialog.h"
#endif

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNewPasswordDialog>
#include <KNotification>
#include <KPasswordDialog>
#include <KPluginFactory>
#include <KSharedConfig>
#include <kwalletentry.h>
#include <kwindowsystem.h>

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
#define HAVE_X11 1
#include <KX11Extras>
#else
#define HAVE_X11 0
#endif

#ifdef HAVE_GPGMEPP
#include <gpgme++/key.h>
#endif

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QTimer>

#include <assert.h>

#include "kwalletadaptor.h"

static void startManagerForKwalletd()
{
    if (!QStandardPaths::findExecutable(QStringLiteral("kstart")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("kstart"), {QStringLiteral("kwalletmanager5"), QStringLiteral("--"), QStringLiteral("--kwalletd")});
    } else if (!QStandardPaths::findExecutable(QStringLiteral("kstart5")).isEmpty()) {
        QProcess::startDetached(QStringLiteral("kstart"), {QStringLiteral("kwalletmanager5"), QStringLiteral("--"), QStringLiteral("--kwalletd")});
    } else {
        QProcess::startDetached(QStringLiteral("kwalletmanager5"), QStringList{QStringLiteral("--kwalletd")});
    }
}

class KWalletTransaction
{
public:
    explicit KWalletTransaction(QDBusConnection conn)
        : tId(nextTransactionId)
        , res(-1)
        , connection(conn)
    {
        nextTransactionId++;
        // make sure the id is never < 0 as that's used for the
        // error conditions.
        if (nextTransactionId < 0) {
            nextTransactionId = 0;
        }
    }

    static int getTransactionId()
    {
        return nextTransactionId;
    }

    ~KWalletTransaction()
    {
    }

    enum Type {
        Unknown,
        Open,
        ChangePassword,
        OpenFail,
        CloseCancelled,
    };
    Type tType = Unknown;
    QString appid;
    qlonglong wId;
    QString wallet;
    QString service;
    bool cancelled = false; // set true if the client dies before open
    bool modal;
    bool isPath;
    int tId; // transaction id
    int res;
    QDBusMessage message;
    QDBusConnection connection;

private:
    static int nextTransactionId;
};

int KWalletTransaction::nextTransactionId = 0;

KWalletD::KWalletD()
    : QObject(nullptr)
    , _failed(0)
    , _syncTime(5000)
    , _curtrans(nullptr)
    , _useGpg(false)
{
#ifdef HAVE_GPGMEPP
    _useGpg = true;
#endif

    srand(time(nullptr));
    _showingFailureNotify = false;
    _closeIdle = false;
    _idleTime = 0;
    connect(&_closeTimers, &KTimeout::timedOut, this, &KWalletD::timedOutClose);
    connect(&_syncTimers, &KTimeout::timedOut, this, &KWalletD::timedOutSync);

    KConfig kwalletrc(QStringLiteral("kwalletrc"));
    KConfigGroup cfgWallet(&kwalletrc, "Wallet");
    KConfigGroup cfgSecrets(&kwalletrc, "org.freedesktop.secrets");

    if (cfgWallet.readEntry<bool>("apiEnabled", true)) {
        (void)new KWalletAdaptor(this);

        // register services
        QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.kwalletd6"));
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/modules/kwalletd6"), this);
        // register also with the KF5 names for backward compatibility
        QDBusConnection::sessionBus().interface()->registerService(QStringLiteral("org.kde.kwalletd5"), QDBusConnectionInterface::QueueService);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/modules/kwalletd5"), this);

        new KWalletPortalSecrets(this);
    }

#ifdef Q_WS_X11
    screensaver = 0;
#endif

    reconfigure();
    //  KGlobal::dirs()->addResourceType("kwallet", 0, "share/apps/kwallet");
    _dw = new KDirWatch(this);
    _dw->setObjectName(QStringLiteral("KWallet Directory Watcher"));
    //  _dw->addDir(KGlobal::dirs()->saveLocation("kwallet"));
    _dw->addDir(KWallet::Backend::getSaveLocation());

    _dw->startScan(true);
    connect(_dw, &KDirWatch::dirty, this, &KWalletD::emitWalletListDirty);
    connect(_dw, &KDirWatch::deleted, this, &KWalletD::emitWalletListDirty);

    _serviceWatcher.setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
    connect(&_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &KWalletD::slotServiceOwnerChanged);

    if (cfgSecrets.readEntry<bool>("apiEnabled", true)) {
        _fdoService.reset(new KWalletFreedesktopService(this));
    }
}

KWalletD::~KWalletD()
{
#ifdef Q_WS_X11
    delete screensaver;
    screensaver = 0;
#endif
    closeAllWallets();
    qDeleteAll(_transactions);
}

QString KWalletD::encodeWalletName(const QString &name)
{
    return KWallet::Backend::encodeWalletName(name);
}

QString KWalletD::decodeWalletName(const QString &mangledName)
{
    return KWallet::Backend::decodeWalletName(mangledName);
}

#ifdef Q_WS_X11
void KWalletD::connectToScreenSaver()
{
    screensaver = new QDBusInterface("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    if (!screensaver->isValid()) {
        qCDebug(KWALLETD_LOG) << "Service org.freedesktop.ScreenSaver not found. Retrying in 10 seconds...";
        // keep attempting every 10 seconds
        QTimer::singleShot(10000, this, SLOT(connectToScreenSaver()));
    } else {
        connect(screensaver, SIGNAL(ActiveChanged(bool)), SLOT(screenSaverChanged(bool)));
        qCDebug(KWALLETD_LOG) << "connected to screen saver service.";
    }
}
#endif

int KWalletD::generateHandle()
{
    int rc;

    // ASSUMPTION: RAND_MAX is fairly large.
    do {
        rc = rand();
    } while (_wallets.contains(rc) || rc == 0);

    return rc;
}

QPair<int, KWallet::Backend *> KWalletD::findWallet(const QString &walletName) const
{
    Wallets::const_iterator it = _wallets.constBegin();
    const Wallets::const_iterator end = _wallets.constEnd();
    for (; it != end; ++it) {
        if (it.value()->walletName() == walletName) {
            return qMakePair(it.key(), it.value());
        }
    }
    return qMakePair(-1, static_cast<KWallet::Backend *>(nullptr));
}

bool KWalletD::_processing = false;

void KWalletD::processTransactions()
{
    if (_processing) {
        return;
    }

    _processing = true;

    // Process remaining transactions
    while (!_transactions.isEmpty()) {
        _curtrans = _transactions.takeFirst();
        int res;

        assert(_curtrans->tType != KWalletTransaction::Unknown);

        switch (_curtrans->tType) {
        case KWalletTransaction::Open:
            res = doTransactionOpen(_curtrans->appid, _curtrans->wallet, _curtrans->isPath, _curtrans->wId, _curtrans->modal, _curtrans->service);

            // multiple requests from the same client
            // should not produce multiple password
            // dialogs on a failure
            if (res < 0) {
                QList<KWalletTransaction *>::iterator it;
                for (it = _transactions.begin(); it != _transactions.end(); ++it) {
                    KWalletTransaction *x = *it;
                    if (_curtrans->appid == x->appid && x->tType == KWalletTransaction::Open && x->wallet == _curtrans->wallet && x->wId == _curtrans->wId) {
                        x->tType = KWalletTransaction::OpenFail;
                    }
                }
            } else if (_curtrans->cancelled) {
                // the wallet opened successfully but the application
                // opening exited/crashed while the dialog was still shown.
                KWalletTransaction *_xact = new KWalletTransaction(_curtrans->connection);
                _xact->tType = KWalletTransaction::CloseCancelled;
                _xact->appid = _curtrans->appid;
                _xact->wallet = _curtrans->wallet;
                _xact->service = _curtrans->service;
                _transactions.append(_xact);
            }

            // emit the AsyncOpened signal as a reply
            _curtrans->res = res;
            Q_EMIT walletAsyncOpened(_curtrans->tId, res);
            break;

        case KWalletTransaction::OpenFail:
            // emit the AsyncOpened signal with an invalid handle
            _curtrans->res = -1;
            Q_EMIT walletAsyncOpened(_curtrans->tId, -1);
            break;

        case KWalletTransaction::ChangePassword:
            doTransactionChangePassword(_curtrans->appid, _curtrans->wallet, _curtrans->wId);
            break;

        case KWalletTransaction::CloseCancelled:
            doTransactionOpenCancelled(_curtrans->appid, _curtrans->wallet, _curtrans->service);
            break;

        case KWalletTransaction::Unknown:
            break;
        default:
            break;
        }

        // send delayed dbus message reply to the caller
        if (_curtrans->message.type() != QDBusMessage::InvalidMessage) {
            if (_curtrans->connection.isConnected()) {
                QDBusMessage reply = _curtrans->message.createReply();
                reply << _curtrans->res;
                _curtrans->connection.send(reply);
            }
        }

        delete _curtrans;
        _curtrans = nullptr;
    }

    _processing = false;
}

int KWalletD::openPath(const QString &path, qlonglong wId, const QString &appid)
{
    int tId = openPathAsync(path, wId, appid, false);
    if (tId < 0) {
        return tId;
    }

    // NOTE the real return value will be sent by the dbusmessage delayed
    // reply
    return 0;
    // wait for the open-transaction to be processed
    //  KWalletOpenLoop loop(this);
    //  return loop.waitForAsyncOpen(tId);
}

int KWalletD::open(const QString &wallet, qlonglong wId, const QString &appid)
{
    if (!_enabled) { // guard
        return -1;
    }

    KWalletTransaction *xact = new KWalletTransaction(connection());
    _transactions.append(xact);

    message().setDelayedReply(true);
    xact->message = message();

    xact->appid = appid;
    xact->wallet = wallet;
    xact->wId = wId;
    xact->modal = true; // mark dialogs as modal, the app has blocking wait
    xact->tType = KWalletTransaction::Open;
    xact->isPath = false;

    QTimer::singleShot(0, this, SLOT(processTransactions()));
    checkActiveDialog();
    // NOTE the real return value will be sent by the dbusmessage delayed
    // reply
    return 0;
}

int KWalletD::nextTransactionId() const
{
    return KWalletTransaction::getTransactionId();
}

int KWalletD::openAsync(const QString &wallet,
                        qlonglong wId,
                        const QString &appid,
                        bool handleSession,
                        const QDBusConnection &connection,
                        const QDBusMessage &message)
{
    if (!_enabled) { // guard
        return -1;
    }

    KWalletTransaction *xact = new KWalletTransaction(connection);
    _transactions.append(xact);

    xact->appid = appid;
    xact->wallet = wallet;
    xact->wId = wId;
    xact->modal = true; // mark dialogs as modal, the app has blocking wait
    xact->tType = KWalletTransaction::Open;
    xact->isPath = false;
    if (handleSession) {
        qCDebug(KWALLETD_LOG) << "openAsync for " << message.service();
        _serviceWatcher.setConnection(connection);
        _serviceWatcher.addWatchedService(message.service());
        xact->service = message.service();
    }
    QTimer::singleShot(0, this, SLOT(processTransactions()));
    checkActiveDialog();
    // opening is in progress. return the transaction number
    return xact->tId;
}

int KWalletD::openAsync(const QString &wallet, qlonglong wId, const QString &appid, bool handleSession)
{
    return openAsync(wallet, wId, appid, handleSession, connection(), message());
}

int KWalletD::openPathAsync(const QString &path, qlonglong wId, const QString &appid, bool handleSession)
{
    if (!_enabled) { // guard
        return -1;
    }

    KWalletTransaction *xact = new KWalletTransaction(connection());
    _transactions.append(xact);

    xact->appid = appid;
    xact->wallet = path;
    xact->wId = wId;
    xact->modal = true;
    xact->tType = KWalletTransaction::Open;
    xact->isPath = true;
    if (handleSession) {
        qCDebug(KWALLETD_LOG) << "openPathAsync " << message().service();
        _serviceWatcher.setConnection(connection());
        _serviceWatcher.addWatchedService(message().service());
        xact->service = message().service();
    }
    QTimer::singleShot(0, this, SLOT(processTransactions()));
    checkActiveDialog();
    // opening is in progress. return the transaction number
    return xact->tId;
}

// Sets up a dialog that will be shown by kwallet.
void KWalletD::setupDialog(QWidget *dialog, WId wId, const QString &appid, bool modal)
{
    if (wId != 0) {
        // correct, set dialog parent
        dialog->setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(dialog->windowHandle(), wId);
    } else {
        if (appid.isEmpty()) {
            qWarning() << "Using kwallet without parent window!";
        } else {
            qWarning() << "Application '" << appid << "' using kwallet without parent window!";
        }
        // allow dialog activation even if it interrupts, better than trying
        // hacks
        // with keeping the dialog on top or on all desktops
        // KF5 FIXME what should we use now instead of this:
        //         kapp->updateUserTimestamp();
    }

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        if (modal) {
            KX11Extras::setState(dialog->winId(), NET::Modal);
        } else {
            KX11Extras::clearState(dialog->winId(), NET::Modal);
        }
    }
#endif
    activeDialog = dialog;
}

// If there's a dialog already open and another application tries some
// operation that'd lead to
// opening a dialog, that application will be blocked by this dialog. A proper
// solution would
// be to set the second application's window also as a parent for the active
// dialog, so that
// KWin properly handles focus changes and so on, but there's currently no
// support for multiple
// dialog parents. In the absence of this support, we use all kinds of bad
// hacks to make
// sure the user doesn't overlook the active dialog.
void KWalletD::checkActiveDialog()
{
    if (!activeDialog) {
        return;
    }

    // KF5 FIXME what should we use now instead of this:
    //  kapp->updateUserTimestamp();

    activeDialog->show();

#if HAVE_X11
    if (KWindowSystem::isPlatformX11()) {
        WId window = activeDialog->winId();
        KX11Extras::setState(window, NET::KeepAbove);
        KX11Extras::setOnAllDesktops(window, true);
        KX11Extras::forceActiveWindow(window);
    }
#endif
}

int KWalletD::doTransactionOpen(const QString &appid, const QString &wallet, bool isPath, qlonglong wId, bool modal, const QString &service)
{
    if (_firstUse && !isPath) {
        // if the user specifies a wallet name, the use it as the default
        // wallet name
        if (wallet != KWallet::Wallet::LocalWallet()) {
            KConfig kwalletrc(QStringLiteral("kwalletrc"));
            KConfigGroup cfg(&kwalletrc, "Wallet");
            cfg.writeEntry("Default Wallet", wallet);
        }
        if (wallets().contains(KWallet::Wallet::LocalWallet())) {
            KConfig kwalletrc(QStringLiteral("kwalletrc"));
            KConfigGroup cfg(&kwalletrc, "Wallet");
            _firstUse = false;
            cfg.writeEntry("First Use", false);
        }
        //         else {
        //             // First use wizard
        //             // TODO GPG adjust new smartcard options gathered by
        //             the wizard
        //             QPointer<KWalletWizard> wiz = new KWalletWizard(0);
        //             wiz->setWindowTitle(i18n("KDE Wallet Service"));
        //             setupDialog(wiz, (WId)wId, appid, modal);
        //             int rc = wiz->exec();
        //             if (rc == QDialog::Accepted && wiz) {
        //                 bool useWallet = wiz->field("useWallet").toBool();
        //                 KConfig kwalletrc("kwalletrc");
        //                 KConfigGroup cfg(&kwalletrc, "Wallet");
        //                 cfg.writeEntry("First Use", false);
        //                 cfg.writeEntry("Enabled", useWallet);
        //                 cfg.writeEntry("Close When Idle",
        //                 wiz->field("closeWhenIdle").toBool());
        //                 cfg.writeEntry("Use One Wallet",
        //                 !wiz->field("networkWallet").toBool());
        //                 cfg.sync();
        //                 reconfigure();
        //
        //                 if (!useWallet) {
        //                     delete wiz;
        //                     return -1;
        //                 }
        //
        //                 // Create the wallet
        //                 // TODO GPG select the correct wallet type upon
        //                 cretion (GPG or blowfish based)
        //                 KWallet::Backend *b = new
        //                 KWallet::Backend(KWallet::Wallet::LocalWallet());
        // #ifdef HAVE_GPGMEPP
        //                 if (wiz->field("useBlowfish").toBool()) {
        //                     b->setCipherType(KWallet::BACKEND_CIPHER_BLOWFISH);
        // #endif
        //                     QString pass = wiz->field("pass1").toString();
        //                     QByteArray p(pass.toUtf8(), pass.length());
        //                     b->open(p);
        //                     p.fill(0);
        // #ifdef HAVE_GPGMEPP
        //                 } else {
        //                     assert(wiz->field("useGpg").toBool());
        //                     b->setCipherType(KWallet::BACKEND_CIPHER_GPG);
        //                     b->open(wiz->gpgKey());
        //                 }
        // #endif
        //                 b->createFolder(KWallet::Wallet::PasswordFolder());
        //                 b->createFolder(KWallet::Wallet::FormDataFolder());
        //                 b->close(true);
        //                 delete b;
        //                 delete wiz;
        //             } else {
        //                 delete wiz;
        //                 return -1;
        //             }
        //         }
    }

    int rc = internalOpen(appid, wallet, isPath, WId(wId), modal, service);
    return rc;
}

int KWalletD::internalOpen(const QString &appid, const QString &wallet, bool isPath, WId w, bool modal, const QString &service)
{
    bool brandNew = false;

    QString thisApp;
    if (appid.isEmpty()) {
        thisApp = QStringLiteral("KDE System");
    } else {
        thisApp = appid;
    }

    if (implicitDeny(wallet, thisApp)) {
        return -1;
    }

    QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int rc = walletInfo.first;
    if (rc == -1) {
        if (_wallets.count() > 20) {
            qCDebug(KWALLETD_LOG) << "Too many wallets open.";
            return -1;
        }

        KWallet::Backend *b = new KWallet::Backend(wallet, isPath);
        QString password;
        bool emptyPass = false;
        if ((isPath && QFile::exists(wallet)) || (!isPath && KWallet::Backend::exists(wallet))) {
            // this open attempt will set wallet type from the file header,
            // even if password is needed
            int pwless = b->open(QByteArray(), w);
#ifdef HAVE_GPGMEPP
            assert(b->cipherType() != KWallet::BACKEND_CIPHER_UNKNOWN);
            if (b->cipherType() == KWallet::BACKEND_CIPHER_GPG) {
                // GPG based wallets do not prompt for password here. Instead,
                // GPG should already have popped pinentry utility for wallet
                // decryption
                if (!b->isOpen()) {
                    // for some reason, GPG operation failed
                    delete b;
                    return -1;
                }
                emptyPass = true;
            } else {
#endif
                if (0 != pwless || !b->isOpen()) {
                    if (pwless == 0) {
                        // release, start anew
                        delete b;
                        b = new KWallet::Backend(wallet, isPath);
                    }
                    KPasswordDialog *kpd = new KPasswordDialog();
                    if (appid.isEmpty()) {
                        kpd->setPrompt(i18n("<qt>KDE has requested to open the wallet '<b>%1</b>'. Please enter the password for this wallet below.</qt>",
                                            wallet.toHtmlEscaped()));
                    } else {
                        kpd->setPrompt(
                            i18n("<qt>The application '<b>%1</b>' has requested to open the wallet '<b>%2</b>'. Please enter the password for this wallet "
                                 "below.</qt>",
                                 appid.toHtmlEscaped(),
                                 wallet.toHtmlEscaped()));
                    }
                    brandNew = false;
                    // don't use KStdGuiItem::open() here which has trailing
                    // ellipsis!
                    // KF5 FIXME what should we use now instead of this:
                    //              kpd->setButtonGuiItem(KDialog::Ok,KGuiItem(
                    //              i18n( "&Open" ), "wallet-open"));
                    kpd->setWindowTitle(i18n("KDE Wallet Service"));
                    kpd->setIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));

#if HAVE_X11
                    if (KWindowSystem::isPlatformX11() && w != KX11Extras::activeWindow() && w != 0L) {
                        // If the dialog is modal to a minimized window it
                        // might not be visible
                        // (but still blocking the calling application).
                        // Notify the user about
                        // the request to open the wallet.
                        KNotification *notification =
                            new KNotification(QStringLiteral("needsPassword"), KNotification::Persistent | KNotification::CloseWhenWindowActivated);
                        notification->setWindow(kpd->windowHandle());
                        QString actionText;
                        if (appid.isEmpty()) {
                            notification->setText(i18n("An application has requested to open a wallet (%1).", wallet.toHtmlEscaped()));
                            actionText = i18nc("Text of a button for switching to the (unnamed) application requesting a password", "Switch there");
                        } else {
                            notification->setText(i18n("<b>%1</b> has requested to open a wallet (%2).", appid.toHtmlEscaped(), wallet.toHtmlEscaped()));
                            actionText =
                                i18nc("Text of a button for switching to the application requesting a password", "Switch to %1", appid.toHtmlEscaped());
                        }

                        KNotificationAction *action = notification->addAction(actionText);
                        connect(action, &KNotificationAction::activated, this, &KWalletD::activatePasswordDialog);
                        notification->sendEvent();
                    }
#endif

                    while (!b->isOpen()) {
                        setupDialog(kpd, w, appid, modal);
                        if (kpd->exec() == QDialog::Accepted) {
                            password = kpd->password();
                            int rc = b->open(password.toUtf8());
                            if (!b->isOpen()) {
                                const auto errorStr = KWallet::Backend::openRCToString(rc);
                                qCWarning(KWALLETD_LOG) << "Failed to open wallet" << wallet << errorStr;
                                kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br />(Error code %2: %3)</qt>",
                                                    wallet.toHtmlEscaped(),
                                                    rc,
                                                    errorStr));
                                kpd->setPassword(QLatin1String(""));
                            }
                        } else {
                            break;
                        }
                    }
                    delete kpd;
                } else {
                    emptyPass = true;
                }
#ifdef HAVE_GPGMEPP
            }
#endif
        } else {
            brandNew = true;
#ifdef HAVE_GPGMEPP
            // prompt the user for the new wallet format here
            KWallet::BackendCipherType newWalletType = KWallet::BACKEND_CIPHER_UNKNOWN;

            std::shared_ptr<KWallet::KNewWalletDialog> newWalletDlg(new KWallet::KNewWalletDialog(appid, wallet, QWidget::find(w)));
            GpgME::Key gpgKey;
            setupDialog(newWalletDlg.get(), (WId)w, appid, true);
            if (newWalletDlg->exec() == QDialog::Accepted) {
                newWalletType = newWalletDlg->isBlowfish() ? KWallet::BACKEND_CIPHER_BLOWFISH : KWallet::BACKEND_CIPHER_GPG;
                gpgKey = newWalletDlg->gpgKey();
            } else {
                // user cancelled the dialog box
                delete b;
                return -1;
            }

            if (newWalletType == KWallet::BACKEND_CIPHER_GPG) {
                b->setCipherType(newWalletType);
                b->open(gpgKey);
            } else if (newWalletType == KWallet::BACKEND_CIPHER_BLOWFISH) {
#endif // HAVE_GPGMEPP
                b->setCipherType(KWallet::BACKEND_CIPHER_BLOWFISH);
                KNewPasswordDialog *kpd = new KNewPasswordDialog();
                KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
                kpd->setBackgroundWarningColor(colorScheme.background(KColorScheme::NegativeBackground).color());
                if (wallet == KWallet::Wallet::LocalWallet() || wallet == KWallet::Wallet::NetworkWallet()) {
                    // Auto create these wallets.
                    if (appid.isEmpty()) {
                        kpd->setPrompt(
                            i18n("KDE has requested to open the wallet. This is used to store sensitive data in a "
                                 "secure fashion. Please enter a password to use with this wallet or click cancel to "
                                 "deny the application's request."));
                    } else {
                        kpd->setPrompt(
                            i18n("<qt>The application '<b>%1</b>' has requested to open the KDE wallet. This is "
                                 "used to store sensitive data in a secure fashion. Please enter a password to use "
                                 "with this wallet or click cancel to deny the application's request.</qt>",
                                 appid.toHtmlEscaped()));
                    }
                } else {
                    if (appid.length() == 0) {
                        kpd->setPrompt(
                            i18n("<qt>KDE has requested to create a new wallet named '<b>%1</b>'. Please choose a "
                                 "password for this wallet, or cancel to deny the application's request.</qt>",
                                 wallet.toHtmlEscaped()));
                    } else {
                        kpd->setPrompt(
                            i18n("<qt>The application '<b>%1</b>' has requested to create a new wallet named '<b>%2</b>'. "
                                 "Please choose a password for this wallet, or cancel to deny the application's request.</qt>",
                                 appid.toHtmlEscaped(),
                                 wallet.toHtmlEscaped()));
                    }
                }
                kpd->setWindowTitle(i18n("KDE Wallet Service"));
                // KF5 FIXME what should we use now instead of this:
                //              kpd->setButtonGuiItem(KDialog::Ok,KGuiItem(i18n("C&reate"),"document-new"));
                kpd->setIcon(QIcon::fromTheme(QStringLiteral("kwalletmanager")));
                while (!b->isOpen()) {
                    setupDialog(kpd, w, appid, modal);
                    if (kpd->exec() == QDialog::Accepted) {
                        password = kpd->password();
                        int rc = b->open(password.toUtf8());
                        if (!b->isOpen()) {
                            kpd->setPrompt(i18n("<qt>Error opening the wallet '<b>%1</b>'. Please try again.<br />(Error code %2: %3)</qt>",
                                                wallet.toHtmlEscaped(),
                                                rc,
                                                KWallet::Backend::openRCToString(rc)));
                        }
                    } else {
                        break;
                    }
                }
                delete kpd;
#ifdef HAVE_GPGMEPP
            }
#endif
        }

        if ((b->cipherType() == KWallet::BACKEND_CIPHER_BLOWFISH) && !emptyPass && (password.isNull() || !b->isOpen())) {
            delete b;
            return -1;
        }

        if (emptyPass && !isAuthorizedApp(appid, wallet, w)) {
            delete b;
            return -1;
        }

        _wallets.insert(rc = generateHandle(), b);
        _sessions.addSession(appid, service, rc);
        _syncTimers.addTimer(rc, _syncTime);

        if (brandNew) {
            createFolder(rc, KWallet::Wallet::PasswordFolder(), appid);
            createFolder(rc, KWallet::Wallet::FormDataFolder(), appid);
        }

        b->ref();
        if (_closeIdle) {
            _closeTimers.addTimer(rc, _idleTime);
        }
        if (brandNew) {
            Q_EMIT walletCreated(wallet);
        }
        Q_EMIT walletOpened(wallet);
        if (_wallets.count() == 1 && _launchManager) {
            startManagerForKwalletd();
        }
    } else {
        // prematurely add a reference so that the wallet does not close while
        // the
        // authorization dialog is being shown.
        walletInfo.second->ref();
        bool isAuthorized = _sessions.hasSession(appid, rc) || isAuthorizedApp(appid, wallet, w);
        // as the wallet might have been forcefully closed, find it again to
        // make sure it's
        // still available (isAuthorizedApp might show a dialog).
        walletInfo = findWallet(wallet);
        if (!isAuthorized) {
            if (walletInfo.first != -1) {
                walletInfo.second->deref();
                // check if the wallet should be closed now.
                internalClose(walletInfo.second, walletInfo.first, false);
            }
            return -1;
        } else {
            if (walletInfo.first != -1) {
                _sessions.addSession(appid, service, rc);
            } else {
                // wallet was forcefully closed.
                return -1;
            }
        }
    }

    return rc;
}

bool KWalletD::isAuthorizedApp(const QString &appid, const QString &wallet, WId w)
{
    if (!_openPrompt) {
        return true;
    }

    int response = 0;

    QString thisApp;
    if (appid.isEmpty()) {
        thisApp = QStringLiteral("KDE System");
    } else {
        thisApp = appid;
    }

    if (!implicitAllow(wallet, thisApp)) {
        KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Allow");
        if (!cfg.isEntryImmutable(wallet)) {
            KBetterThanKDialog *dialog = new KBetterThanKDialog;
            dialog->setWindowTitle(i18n("KDE Wallet Service"));
            if (appid.isEmpty()) {
                dialog->setLabel(i18n("<qt>KDE has requested access to the open wallet '<b>%1</b>'.</qt>", wallet.toHtmlEscaped()));
            } else {
                dialog->setLabel(i18n("<qt>The application '<b>%1</b>' has requested access to the open wallet '<b>%2</b>'.</qt>",
                                      appid.toHtmlEscaped(),
                                      wallet.toHtmlEscaped()));
            }
            setupDialog(dialog, w, appid, false);
            response = dialog->exec();
            delete dialog;
        }
    }

    if (response == 0 || response == 1) {
        if (response == 1) {
            KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Allow");
            QStringList apps = cfg.readEntry(wallet, QStringList());
            if (!apps.contains(thisApp)) {
                if (cfg.isEntryImmutable(wallet)) {
                    return false;
                }
                apps += thisApp;
                _implicitAllowMap[wallet] += thisApp;
                cfg.writeEntry(wallet, apps);
                cfg.sync();
            }
        }
    } else if (response == 3) {
        KConfigGroup cfg = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Deny");
        QStringList apps = cfg.readEntry(wallet, QStringList());
        if (!apps.contains(thisApp)) {
            apps += thisApp;
            _implicitDenyMap[wallet] += thisApp;
            cfg.writeEntry(wallet, apps);
            cfg.sync();
        }
        return false;
    } else {
        return false;
    }
    return true;
}

int KWalletD::deleteWallet(const QString &wallet)
{
    int result = -1;
    QString path = KWallet::Backend::getSaveLocation() + "/" + encodeWalletName(wallet) + ".kwl";
    QString pathSalt = KWallet::Backend::getSaveLocation() + "/" + encodeWalletName(wallet) + ".salt";

    if (QFile::exists(path)) {
        const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
        internalClose(walletInfo.second, walletInfo.first, true);
        QFile::remove(path);
        Q_EMIT walletDeleted(wallet);
        // also delete access control entries
        KConfigGroup cfgAllow = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Allow");
        cfgAllow.deleteEntry(wallet);

        KConfigGroup cfgDeny = KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Auto Deny");
        cfgDeny.deleteEntry(wallet);

        if (QFile::exists(pathSalt)) {
            QFile::remove(pathSalt);
        }

        result = 0;
    }

    return result;
}

void KWalletD::changePassword(const QString &wallet, qlonglong wId, const QString &appid)
{
    KWalletTransaction *xact = new KWalletTransaction(connection());

    message().setDelayedReply(true);
    xact->message = message();
    // TODO GPG this shouldn't be allowed on a GPG managed wallet; a warning
    // should be displayed about this

    xact->appid = appid;
    xact->wallet = wallet;
    xact->wId = wId;
    xact->modal = false;
    xact->tType = KWalletTransaction::ChangePassword;

    _transactions.append(xact);

    QTimer::singleShot(0, this, SLOT(processTransactions()));
    checkActiveDialog();
    checkActiveDialog();
}

void KWalletD::initiateSync(int handle)
{
    // add a timer and reset it right away
    _syncTimers.addTimer(handle, _syncTime);
    _syncTimers.resetTimer(handle, _syncTime);
}

void KWalletD::doTransactionChangePassword(const QString &appid, const QString &wallet, qlonglong wId)
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int handle = walletInfo.first;
    KWallet::Backend *w = walletInfo.second;

    bool reclose = false;
    if (!w) {
        handle = doTransactionOpen(appid, wallet, false, wId, false, QLatin1String(""));
        if (-1 == handle) {
            KMessageBox::errorWId((WId)wId,
                                  i18n("Unable to open wallet. The wallet must be opened in order to change the password."),
                                  i18n("KDE Wallet Service"));
            return;
        }

        w = _wallets.value(handle);
        reclose = true;
    }

    assert(w);

#ifdef HAVE_GPGMEPP
    if (w->cipherType() == KWallet::BACKEND_CIPHER_GPG) {
        QString keyID = w->gpgKey().shortKeyID();
        assert(!keyID.isNull());
        KMessageBox::errorWId((WId)wId,
                              i18n("<qt>The <b>%1</b> wallet is encrypted using GPG key <b>%2</b>. Please use <b>GPG</b> tools (such "
                                   "as <b>kleopatra</b>) to change the passphrase associated to that key.</qt>",
                                   wallet.toHtmlEscaped(),
                                   keyID));
    } else {
#endif
        QPointer<KNewPasswordDialog> kpd = new KNewPasswordDialog();
        kpd->setPrompt(i18n("<qt>Please choose a new password for the wallet '<b>%1</b>'.</qt>", wallet.toHtmlEscaped()));
        kpd->setWindowTitle(i18n("KDE Wallet Service"));
        kpd->setAllowEmptyPasswords(true);
        KColorScheme colorScheme(QPalette::Active, KColorScheme::View);
        kpd->setBackgroundWarningColor(colorScheme.background(KColorScheme::NegativeBackground).color());
        setupDialog(kpd, (WId)wId, appid, false);
        if (kpd->exec() == QDialog::Accepted && kpd) {
            QString p = kpd->password();
            if (!p.isNull()) {
                w->setPassword(p.toUtf8());
                int rc = w->close(true);
                if (rc < 0) {
                    KMessageBox::errorWId((WId)wId, i18n("Error re-encrypting the wallet. Password was not changed."), i18n("KDE Wallet Service"));
                    reclose = true;
                } else {
                    rc = w->open(p.toUtf8());
                    if (rc < 0) {
                        KMessageBox::errorWId((WId)wId, i18n("Error reopening the wallet. Data may be lost."), i18n("KDE Wallet Service"));
                        reclose = true;
                    }
                }
            }
        }

        delete kpd;
#ifdef HAVE_GPGMEPP
    }
#endif

    if (reclose) {
        internalClose(w, handle, true);
    }
}

int KWalletD::close(const QString &wallet, bool force)
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int handle = walletInfo.first;
    KWallet::Backend *w = walletInfo.second;

    return internalClose(w, handle, force);
}

int KWalletD::internalClose(KWallet::Backend *const w, const int handle, const bool force, const bool saveBeforeClose)
{
    if (w) {
        const QString &wallet = w->walletName();
        if ((w->refCount() == 0 && !_leaveOpen) || force) {
            // this is only a safety measure. sessions should be gone already.
            _sessions.removeAllSessions(handle);
            if (_closeIdle) {
                _closeTimers.removeTimer(handle);
            }
            _syncTimers.removeTimer(handle);
            _wallets.remove(handle);
            w->close(saveBeforeClose);
            doCloseSignals(handle, wallet);
            delete w;
            return 0;
        }
        return 1;
    }

    return -1;
}

int KWalletD::close(int handle, bool force, const QString &appid, const QDBusMessage &message)
{
    KWallet::Backend *w = _wallets.value(handle);

    if (w) {
        if (_sessions.hasSession(appid, handle)) {
            // remove one handle for the application
            bool removed = _sessions.removeSession(appid, message.service(), handle);
            // alternatively try sessionless
            if (removed || _sessions.removeSession(appid, QLatin1String(""), handle)) {
                w->deref();
            }
            return internalClose(w, handle, force);
        }
        return 1; // not closed, handle unknown
    }
    return -1; // not open to begin with, or other error
}

int KWalletD::close(int handle, bool force, const QString &appid)
{
    return close(handle, force, appid, message());
}

bool KWalletD::isOpen(const QString &wallet)
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    return walletInfo.second != nullptr;
}

bool KWalletD::isOpen(int handle)
{
    if (handle == 0) {
        return false;
    }

    KWallet::Backend *rc = _wallets.value(handle);

    if (rc == nullptr && ++_failed > 5) {
        _failed = 0;
        QTimer::singleShot(0, this, SLOT(notifyFailures()));
    } else if (rc != nullptr) {
        _failed = 0;
    }

    return rc != nullptr;
}

QStringList KWalletD::wallets() const
{
    QString path = KWallet::Backend::getSaveLocation();
    QDir dir(path, QStringLiteral("*.kwl"));
    QStringList rc;

    dir.setFilter(QDir::Files | QDir::Hidden);

    const auto list = dir.entryInfoList();
    for (const QFileInfo &fi : list) {
        QString fn = fi.fileName();
        if (fn.endsWith(QLatin1String(".kwl"))) {
            fn.truncate(fn.length() - 4);
        }
        rc += decodeWalletName(fn);
    }
    return rc;
}

void KWalletD::sync(int handle, const QString &appid)
{
    KWallet::Backend *b;

    // get the wallet and check if we have a password for it (safety measure)
    if ((b = getWallet(appid, handle))) {
        QString wallet = b->walletName();
        b->sync(0);
    }
}

void KWalletD::timedOutSync(int handle)
{
    _syncTimers.removeTimer(handle);
    if (_wallets.contains(handle) && _wallets[handle]) {
        _wallets[handle]->sync(0);
    } else {
        qDebug("wallet not found for sync!");
    }
}

void KWalletD::doTransactionOpenCancelled(const QString &appid, const QString &wallet, const QString &service)
{
    // there will only be one session left to remove - all others
    // have already been removed in slotServiceOwnerChanged and all
    // transactions for opening new sessions have been deleted.
    if (!_sessions.hasSession(appid)) {
        return;
    }

    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int handle = walletInfo.first;
    KWallet::Backend *b = walletInfo.second;
    if (handle != -1 && b) {
        b->deref();
        internalClose(b, handle, false);
    }

    // close the session in case the wallet hasn't been closed yet
    _sessions.removeSession(appid, service, handle);
}

QStringList KWalletD::folderList(int handle, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        return b->folderList();
    }

    return QStringList();
}

bool KWalletD::hasFolder(int handle, const QString &f, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        return b->hasFolder(f);
    }

    return false;
}

bool KWalletD::removeFolder(int handle, const QString &f, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        bool rc = b->removeFolder(f);
        initiateSync(handle);
        Q_EMIT folderListUpdated(b->walletName());
        return rc;
    }

    return false;
}

bool KWalletD::createFolder(int handle, const QString &f, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        bool rc = b->createFolder(f);
        initiateSync(handle);
        Q_EMIT folderListUpdated(b->walletName());
        return rc;
    }

    return false;
}

QByteArray KWalletD::readMap(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry *e = b->readEntry(key);
        if (e && e->type() == KWallet::Wallet::Map) {
            return e->map();
        }
    }

    return QByteArray();
}

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
QVariantMap KWalletD::readMapList(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        QVariantMap rc;
        const auto lst = b->readEntryList(key);
        for (KWallet::Entry *entry : lst) {
            if (entry->type() == KWallet::Wallet::Map) {
                rc.insert(entry->key(), entry->map());
            }
        }
        return rc;
    }

    return QVariantMap();
}
#endif

QVariantMap KWalletD::mapList(int handle, const QString &folder, const QString &appid)
{
    QVariantMap rc;

    KWallet::Backend *backend = getWallet(appid, handle);
    if (backend) {
        backend->setFolder(folder);
        const QList<KWallet::Entry *> lst = backend->entriesList();
        for (KWallet::Entry *entry : lst) {
            if (entry->type() == KWallet::Wallet::Map) {
                rc.insert(entry->key(), entry->map());
            }
        }
    }

    return rc;
}

QByteArray KWalletD::readEntry(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry *e = b->readEntry(key);
        if (e) {
            return e->value();
        }
    }

    return QByteArray();
}

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
QVariantMap KWalletD::readEntryList(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        QVariantMap rc;
        const auto lst = b->readEntryList(key);
        for (KWallet::Entry *entry : lst) {
            rc.insert(entry->key(), entry->value());
        }
        return rc;
    }

    return QVariantMap();
}
#endif

QVariantMap KWalletD::entriesList(int handle, const QString &folder, const QString &appid)
{
    QVariantMap rc;

    KWallet::Backend *backend = getWallet(appid, handle);
    if (backend) {
        backend->setFolder(folder);
        const QList<KWallet::Entry *> lst = backend->entriesList();
        for (KWallet::Entry *entry : lst) {
            rc.insert(entry->key(), entry->value());
        }
    }

    return rc;
}

QStringList KWalletD::entryList(int handle, const QString &folder, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        return b->entryList();
    }

    return QStringList();
}

QString KWalletD::readPassword(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry *e = b->readEntry(key);
        if (e && e->type() == KWallet::Wallet::Password) {
            return e->password();
        }
    }

    return QString();
}

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
QVariantMap KWalletD::readPasswordList(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        QVariantMap rc;
        const auto lst = b->readEntryList(key);
        for (KWallet::Entry *entry : lst) {
            if (entry->type() == KWallet::Wallet::Password) {
                rc.insert(entry->key(), entry->password());
            }
        }
        return rc;
    }

    return QVariantMap();
}
#endif

QVariantMap KWalletD::passwordList(int handle, const QString &folder, const QString &appid)
{
    QVariantMap rc;

    KWallet::Backend *backend = getWallet(appid, handle);
    if (backend) {
        backend->setFolder(folder);
        const QList<KWallet::Entry *> lst = backend->entriesList();
        for (KWallet::Entry *entry : lst) {
            if (entry->type() == KWallet::Wallet::Password) {
                rc.insert(entry->key(), entry->password());
            }
        }
    }

    return rc;
}

int KWalletD::writeMap(int handle, const QString &folder, const QString &key, const QByteArray &value, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry e;
        e.setKey(key);
        e.setValue(value);
        e.setType(KWallet::Wallet::Map);
        b->writeEntry(&e);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        emitEntryUpdated(b->walletName(), folder, key);
        return 0;
    }

    return -1;
}

int KWalletD::writeEntry(int handle, const QString &folder, const QString &key, const QByteArray &value, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry e;
        e.setKey(key);
        e.setValue(value);
        e.setType(KWallet::Wallet::Stream);
        b->writeEntry(&e);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        emitEntryUpdated(b->walletName(), folder, key);
        return 0;
    }

    return -1;
}

int KWalletD::writeEntry(int handle, const QString &folder, const QString &key, const QByteArray &value, int entryType, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry e;
        e.setKey(key);
        e.setValue(value);
        e.setType(KWallet::Wallet::EntryType(entryType));
        b->writeEntry(&e);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        return 0;
    }

    return -1;
}

int KWalletD::writePassword(int handle, const QString &folder, const QString &key, const QString &value, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        KWallet::Entry e;
        e.setKey(key);
        e.setValue(value);
        e.setType(KWallet::Wallet::Password);
        b->writeEntry(&e);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        emitEntryUpdated(b->walletName(), folder, key);
        return 0;
    }

    return -1;
}

int KWalletD::entryType(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        if (!b->hasFolder(folder)) {
            return KWallet::Wallet::Unknown;
        }
        b->setFolder(folder);
        if (b->hasEntry(key)) {
            return b->readEntry(key)->type();
        }
    }

    return KWallet::Wallet::Unknown;
}

bool KWalletD::hasEntry(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        if (!b->hasFolder(folder)) {
            return false;
        }
        b->setFolder(folder);
        return b->hasEntry(key);
    }

    return false;
}

int KWalletD::removeEntry(int handle, const QString &folder, const QString &key, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        if (!b->hasFolder(folder)) {
            return 0;
        }
        b->setFolder(folder);
        bool rc = b->removeEntry(key);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        emitEntryDeleted(b->walletName(), folder, key);
        return rc ? 0 : -3;
    }

    return -1;
}

void KWalletD::slotServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(name);
    qCDebug(KWALLETD_LOG) << "slotServiceOwnerChanged " << name << ", " << oldOwner << ", " << newOwner;

    if (!newOwner.isEmpty()) {
        return; // no application exit, don't care.
    }

    // as we don't have the application id we have to cycle
    // all sessions. As an application can basically open wallets
    // with several appids, we can't stop if we found one.
    QString service(oldOwner);
    const QList<KWalletAppHandlePair> sessremove(_sessions.findSessions(service));
    KWallet::Backend *b = nullptr;

    // check all sessions for wallets to close
    for (const KWalletAppHandlePair &s : sessremove) {
        b = getWallet(s.first, s.second);
        if (b) {
            b->deref();
            internalClose(b, s.second, false);
        }
    }

    // remove all the sessions in case they aren't gone yet
    for (const KWalletAppHandlePair &s : sessremove) {
        _sessions.removeSession(s.first, service, s.second);
    }

    // cancel all open-transactions still running for the service
    QList<KWalletTransaction *>::iterator tit;
    for (tit = _transactions.begin(); tit != _transactions.end(); ++tit) {
        if ((*tit)->tType == KWalletTransaction::Open && (*tit)->service == oldOwner) {
            delete (*tit);
            *tit = nullptr;
        }
    }
    _transactions.removeAll(nullptr);

    // if there's currently an open-transaction being handled,
    // mark it as cancelled.
    if (_curtrans && _curtrans->tType == KWalletTransaction::Open && _curtrans->service == oldOwner) {
        qCDebug(KWALLETD_LOG) << "Cancelling current transaction!";
        _curtrans->cancelled = true;
    }
    _serviceWatcher.removeWatchedService(oldOwner);
}

KWallet::Backend *KWalletD::getWallet(const QString &appid, int handle)
{
    if (handle == 0) {
        return nullptr;
    }

    KWallet::Backend *w = _wallets.value(handle);

    if (w) { // the handle is valid
        if (_sessions.hasSession(appid, handle)) {
            // the app owns this handle
            _failed = 0;
            if (_closeIdle) {
                _closeTimers.resetTimer(handle, _idleTime);
            }
            return w;
        }
    }

    if (++_failed > 5) {
        _failed = 0;
        QTimer::singleShot(0, this, SLOT(notifyFailures()));
    }

    return nullptr;
}

void KWalletD::notifyFailures()
{
    if (!_showingFailureNotify) {
        _showingFailureNotify = true;
        KMessageBox::information(nullptr,
                                 i18n("There have been repeated failed attempts to gain access to a wallet. An application may be misbehaving."),
                                 i18n("KDE Wallet Service"));
        _showingFailureNotify = false;
    }
}

void KWalletD::doCloseSignals(int handle, const QString &wallet)
{
    Q_EMIT walletClosed(handle);
    Q_EMIT walletClosedId(handle);

    Q_EMIT walletClosed(wallet);
    if (_wallets.isEmpty()) {
        Q_EMIT allWalletsClosed();
    }
}

int KWalletD::renameEntry(int handle, const QString &folder, const QString &oldName, const QString &newName, const QString &appid)
{
    KWallet::Backend *b;

    if ((b = getWallet(appid, handle))) {
        b->setFolder(folder);
        int rc = b->renameEntry(oldName, newName);
        initiateSync(handle);
        emitFolderUpdated(b->walletName(), folder);
        emitEntryRenamed(b->walletName(), folder, oldName, newName);
        return rc;
    }

    return -1;
}

int KWalletD::renameWallet(const QString &oldName, const QString &newName)
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(oldName);
    return walletInfo.second->renameWallet(newName);
}

QStringList KWalletD::users(const QString &wallet) const
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    return _sessions.getApplications(walletInfo.first);
}

bool KWalletD::disconnectApplication(const QString &wallet, const QString &application)
{
    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int handle = walletInfo.first;
    KWallet::Backend *backend = walletInfo.second;

    if (handle != -1 && _sessions.hasSession(application, handle)) {
        int removed = _sessions.removeAllSessions(application, handle);

        for (int i = 0; i < removed; ++i) {
            backend->deref();
        }
        internalClose(backend, handle, false);

        Q_EMIT applicationDisconnected(wallet, application);
        return true;
    }

    return false;
}

void KWalletD::emitFolderUpdated(const QString &wallet, const QString &folder)
{
    Q_EMIT folderUpdated(wallet, folder);
}

void KWalletD::emitEntryUpdated(const QString &wallet, const QString &folder, const QString &key)
{
    Q_EMIT entryUpdated(wallet, folder, key);
}

void KWalletD::emitEntryRenamed(const QString &wallet, const QString &folder, const QString &oldName, const QString &newName)
{
    Q_EMIT entryRenamed(wallet, folder, oldName, newName);
}

void KWalletD::emitEntryDeleted(const QString &wallet, const QString &folder, const QString &key)
{
    Q_EMIT entryDeleted(wallet, folder, key);
}

void KWalletD::emitWalletListDirty()
{
    const QStringList walletsInDisk = wallets();
    const auto lst = _wallets.values();
    for (auto i : lst) {
        if (!walletsInDisk.contains(i->walletName())) {
            internalClose(i, _wallets.key(i), true, false);
        }
    }
    Q_EMIT walletListDirty();
}

void KWalletD::reconfigure()
{
    KConfig cfg(QStringLiteral("kwalletrc"));
    KConfigGroup walletGroup(&cfg, "Wallet");
    _firstUse = walletGroup.readEntry("First Use", true);
    _enabled = walletGroup.readEntry("Enabled", true);
    _launchManager = walletGroup.readEntry("Launch Manager", false);
    _leaveOpen = walletGroup.readEntry("Leave Open", true);
    bool idleSave = _closeIdle;
    _closeIdle = walletGroup.readEntry("Close When Idle", false);
    _openPrompt = walletGroup.readEntry("Prompt on Open", false);
    int timeSave = _idleTime;
    // in minutes!
    _idleTime = walletGroup.readEntry("Idle Timeout", 10) * 60 * 1000;
#ifdef Q_WS_X11
    if (walletGroup.readEntry("Close on Screensaver", false)) {
        // BUG 254273 : if kwalletd starts before the screen saver, then the
        // connection fails and kwalletd never receives it's notifications
        // To fix this, we use a timer and perform periodic connection
        // attempts until connection succeeds
        QTimer::singleShot(0, this, SLOT(connectToScreenSaver()));
    } else {
        if (screensaver && screensaver->isValid()) {
            screensaver->disconnect(SIGNAL(ActiveChanged(bool)), this, SLOT(screenSaverChanged(bool)));
            delete screensaver;
            screensaver = 0;
        }
    }
#endif
    // Handle idle changes
    if (_closeIdle) {
        if (_idleTime != timeSave) { // Timer length changed
            Wallets::const_iterator it = _wallets.constBegin();
            const Wallets::const_iterator end = _wallets.constEnd();
            for (; it != end; ++it) {
                _closeTimers.resetTimer(it.key(), _idleTime);
            }
        }

        if (!idleSave) { // add timers for all the wallets
            Wallets::const_iterator it = _wallets.constBegin();
            const Wallets::const_iterator end = _wallets.constEnd();
            for (; it != end; ++it) {
                _closeTimers.addTimer(it.key(), _idleTime);
            }
        }
    } else {
        _closeTimers.clear();
    }

    // Update the implicit allow stuff
    _implicitAllowMap.clear();
    const KConfigGroup autoAllowGroup(&cfg, "Auto Allow");
    QStringList entries = autoAllowGroup.entryMap().keys();
    for (QStringList::const_iterator i = entries.constBegin(); i != entries.constEnd(); ++i) {
        _implicitAllowMap[*i] = autoAllowGroup.readEntry(*i, QStringList());
    }

    // Update the implicit allow stuff
    _implicitDenyMap.clear();
    const KConfigGroup autoDenyGroup(&cfg, "Auto Deny");
    entries = autoDenyGroup.entryMap().keys();
    for (QStringList::const_iterator i = entries.constBegin(); i != entries.constEnd(); ++i) {
        _implicitDenyMap[*i] = autoDenyGroup.readEntry(*i, QStringList());
    }

    // Update if wallet was enabled/disabled
    if (!_enabled) { // close all wallets
        while (!_wallets.isEmpty()) {
            Wallets::const_iterator it = _wallets.constBegin();
            internalClose(it.value(), it.key(), true);
        }
        QApplication::exit(0);
    }
}

bool KWalletD::isEnabled() const
{
    return _enabled;
}

bool KWalletD::folderDoesNotExist(const QString &wallet, const QString &folder)
{
    if (!wallets().contains(wallet)) {
        return true;
    }

    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    if (walletInfo.second) {
        return walletInfo.second->folderDoesNotExist(folder);
    }

    KWallet::Backend *b = new KWallet::Backend(wallet);
    b->open(QByteArray());
    bool rc = b->folderDoesNotExist(folder);
    delete b;
    return rc;
}

bool KWalletD::keyDoesNotExist(const QString &wallet, const QString &folder, const QString &key)
{
    if (!wallets().contains(wallet)) {
        return true;
    }

    const QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    if (walletInfo.second) {
        return walletInfo.second->entryDoesNotExist(folder, key);
    }

    KWallet::Backend *b = new KWallet::Backend(wallet);
    b->open(QByteArray());
    bool rc = b->entryDoesNotExist(folder, key);
    delete b;
    return rc;
}

bool KWalletD::implicitAllow(const QString &wallet, const QString &app)
{
    return _implicitAllowMap[wallet].contains(app);
}

bool KWalletD::implicitDeny(const QString &wallet, const QString &app)
{
    return _implicitDenyMap[wallet].contains(app);
}

void KWalletD::timedOutClose(int id)
{
    KWallet::Backend *w = _wallets.value(id);
    if (w) {
        internalClose(w, id, true);
    }
}

void KWalletD::closeAllWallets()
{
    Wallets walletsCopy = _wallets;

    Wallets::const_iterator it = walletsCopy.constBegin();
    const Wallets::const_iterator end = walletsCopy.constEnd();
    for (; it != end; ++it) {
        internalClose(it.value(), it.key(), true);
    }

    walletsCopy.clear();

    // All of this should be basically noop.  Let's just be safe.
    _wallets.clear();
}

QString KWalletD::networkWallet()
{
    return KWallet::Wallet::NetworkWallet();
}

QString KWalletD::localWallet()
{
    return KWallet::Wallet::LocalWallet();
}

void KWalletD::screenSaverChanged(bool s)
{
    if (s) {
        closeAllWallets();
    }
}

void KWalletD::activatePasswordDialog()
{
    checkActiveDialog();
}

int KWalletD::pamOpen(const QString &wallet, const QByteArray &passwordHash, int sessionTimeout)
{
    if (_processing) {
        return -1;
    }

    bool brandNew = false;

    // check if the wallet is already open
    QPair<int, KWallet::Backend *> walletInfo = findWallet(wallet);
    int rc = walletInfo.first;
    if (rc != -1) {
        return rc; // Wallet already opened, return handle
    }
    if (_wallets.count() > 20) {
        return -1;
    }

    KWallet::Backend *b = nullptr;
    // If the wallet we want to open does not exists. create it and set pam
    // hash
    if (!wallets().contains(wallet)) {
        b = new KWallet::Backend(wallet);
        b->setCipherType(KWallet::BACKEND_CIPHER_BLOWFISH);
        brandNew = true;
    } else {
        b = new KWallet::Backend(wallet);
    }

    int openrc = b->openPreHashed(passwordHash);
    if (openrc != 0 || !b->isOpen()) {
        delete b;
        return openrc;
    }

    // opening the wallet was successful
    int handle = generateHandle();
    _wallets.insert(handle, b);
    _syncTimers.addTimer(handle, _syncTime);

    // don't reference the wallet or add a session so it
    // can be reclosed easily.

    if (sessionTimeout > 0) {
        _closeTimers.addTimer(handle, sessionTimeout);
    } else if (_closeIdle) {
        _closeTimers.addTimer(handle, _idleTime);
    }
    if (brandNew) {
        Q_EMIT walletCreated(wallet);
    }
    Q_EMIT walletOpened(wallet);

    if (_wallets.count() == 1 && _launchManager) {
        startManagerForKwalletd();
    }

    return handle;
}

// vim: tw=220:ts=4

#include "moc_kwalletd.cpp"
