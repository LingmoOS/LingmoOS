#include "kwalletpath.h"

#include <QTest>
#include <QTextStream>

#include <KAboutData>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <kwallet.h>

static QTextStream _out(stdout, QIODevice::WriteOnly);

void KWalletPathTest::init()
{
    if (!qEnvironmentVariableIsSet("DISPLAY")) {
        QSKIP("$DISPLAY is not set. These tests cannot be done without a graphical system.");
    }
}

void KWalletPathTest::openWallet()
{
    _out << "About to ask for wallet /tmp/test.kwl sync\n";

    KWallet::Wallet *wallet = KWallet::Wallet::openWallet(QStringLiteral("/tmp/test.kwl"), 0, KWallet::Wallet::Path);
    QVERIFY(wallet != nullptr);

    _out << "Got path wallet: " << (wallet != nullptr) << '\n';

    if (wallet) {
        _out << "Closing wallet\n";
        delete wallet;
    }
    _out.flush();
}

QTEST_GUILESS_MAIN(KWalletPathTest)

#include "moc_kwalletpath.cpp"
