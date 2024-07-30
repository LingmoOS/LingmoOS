#include "kwalletasync.h"

#include <QApplication>
#include <QTest>
#include <QTextStream>
#include <QTimer>

#include <KAboutData>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <kwallet.h>

#include "kwallettest.h"

static QTextStream _out(stdout, QIODevice::WriteOnly);

void KWalletAsyncTest::init()
{
    if (!qEnvironmentVariableIsSet("DISPLAY")) {
        QSKIP("$DISPLAY is not set. These tests cannot be done without a graphical system.");
    }
}

void KWalletAsyncTest::openWallet()
{
    _out << "About to ask for wallet async\n";

    // we have no wallet: ask for one.
    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), 0, KWallet::Wallet::Asynchronous);
    QVERIFY(wallet != nullptr);

    WalletReceiver r;
    QVERIFY(connect(wallet, &KWallet::Wallet::walletOpened, &r, &WalletReceiver::walletOpened));

    _out << "About to start 30 second event loop\n";

    QTimer::singleShot(30000, qApp, SLOT(quit()));
    int ret = qApp->exec();

    if (ret == 0) {
        _out << "Timed out!\n";
    } else {
        _out << "Success!\n";
    }
    _out.flush();

    QVERIFY2(ret == 1, "Timeout when waiting for wallet open");
}

void WalletReceiver::walletOpened(bool got)
{
    _out << "Got async wallet: " << got << '\n';
    _out.flush();
    qApp->exit(1);
}

QTEST_GUILESS_MAIN(KWalletAsyncTest)

#include "moc_kwalletasync.cpp"
