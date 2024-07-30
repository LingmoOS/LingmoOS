/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "fdo_secrets_test.h"
#include "mockkwalletd.cpp"
// cannot be in mockkwalletd.cpp, as CMake's automoc does not look there
#include "moc_ktimeout.cpp"
#include "moc_kwalletd.cpp"
#include "static_mock.hpp"

void FdoSecretsTest::initTestCase()
{
    static QCA::Initializer init{};
}

void FdoSecretsTest::serviceStaticFunctions()
{
    auto labels = Testset<QString, FdoUniqueLabel>{
        {"label", {"label", -1}},
        {"label__0_", {"label", 0}},
        {"label___1_", {"label_", 1}},
        {"__0_label___200_", {"__0_label_", 200}},
    };

    runTestset(FdoUniqueLabel::fromName, labels);
    runRevTestset(
        [](const FdoUniqueLabel &l) {
            return l.toName();
        },
        labels);

    runTestset(KWalletFreedesktopService::wrapToCollectionPath,
               Testset<QString, QString>{
                   {"/org/freedesktop/secrets/collection/abcd", "/org/freedesktop/secrets/collection/abcd"},
                   {"/org/freedesktop/secrets/collection/abcd/2", "/org/freedesktop/secrets/collection/abcd"},
                   {"/org/freedesktop/secrets/collection/abcd/2/2/3/4", "/org/freedesktop/secrets/collection/abcd"},
               });

    QCOMPARE(KWalletFreedesktopService::nextPromptPath().path(), "/org/freedesktop/secrets/prompt/p0");
    QCOMPARE(KWalletFreedesktopService::nextPromptPath().path(), "/org/freedesktop/secrets/prompt/p1");
}

void FdoSecretsTest::collectionStaticFunctions()
{
    auto dirNameTestset = Testset<EntryLocation, FdoUniqueLabel>{
        {{FDO_SECRETS_DEFAULT_DIR, "entry1"}, {"entry1", -1}},
        {{FDO_SECRETS_DEFAULT_DIR, "entry__3_"}, {"entry", 3}},
        {{"Passwords", "password__"}, {"Passwords/password__", -1}},
        {{"Passwords__3_", "password__200_"}, {"Passwords__3_/password", 200}},
        {{"", "password"}, {"/password", -1}},
        {{"", "/"}, {"//", -1}},
        {{FDO_SECRETS_DEFAULT_DIR, "/"}, {"/", -1}},
        {{FDO_SECRETS_DEFAULT_DIR, "/__2_"}, {"/", 2}},
        {{"https:", "/foobar.org/"}, {"https://foobar.org/", -1}},
        {{"https:", "/foobar.org/__80_"}, {"https://foobar.org/", 80}},
    };

    runTestset(
        [](const EntryLocation &l) {
            return l.toUniqueLabel();
        },
        dirNameTestset);
    runRevTestset(
        [](const FdoUniqueLabel &l) {
            return l.toEntryLocation();
        },
        dirNameTestset);
}

void FdoSecretsTest::cleanup()
{
    SET_FUNCTION_RESULT(KWalletD::wallets, QStringList());
}

void FdoSecretsTest::precreatedWallets()
{
    const QStringList wallets = {"wallet1", "wallet2", "wallet2__0_", "wallet2__1_"};
    SET_FUNCTION_RESULT(KWalletD::wallets, wallets);
    SET_FUNCTION_RESULT_OVERLOADED(KWalletD::isOpen, true, bool(KWalletD::*)(int));

    std::unique_ptr<KWalletD> kwalletd{new KWalletD};
    std::unique_ptr<KWalletFreedesktopService> service{new KWalletFreedesktopService(kwalletd.get())};

    QCOMPARE(wallets.size(), service->collections().size());
    for (const auto &walletName : wallets) {
        auto collection = service->getCollectionByWalletName(walletName);
        QVERIFY(collection);
        QVERIFY(collection->label() == "wallet1" || collection->label() == "wallet2");
    }

    auto firstCollection = service->getCollectionByWalletName(wallets.front());
    auto &item1 = firstCollection->pushNewItem(FdoUniqueLabel{"item1", -1}, QDBusObjectPath(firstCollection->fdoObjectPath().path() + "/0"));
    QCOMPARE(item1.fdoObjectPath().path(), (firstCollection->fdoObjectPath().path() + "/0"));
    QCOMPARE(&item1, service->getItemByObjectPath(item1.fdoObjectPath()));
}

void FdoSecretsTest::aliases()
{
    std::unique_ptr<KWalletD> kwalletd{new KWalletD};
    std::unique_ptr<KWalletFreedesktopService> service{new KWalletFreedesktopService(kwalletd.get())};

    service->createCollectionAlias("alias", "walletName");
    service->createCollectionAlias("alias2", "walletName");
    service->createCollectionAlias("alias3", "walletName300");
    service->updateCollectionAlias("alias3", "walletName");
    QSet<QString> checkAliases = {"alias", "alias2", "alias3"};
    const QStringList aliases = service->readAliasesFor("walletName");
    for (const auto &alias : aliases)
        checkAliases.remove(alias);
    QVERIFY(checkAliases.isEmpty());

    service->removeAlias("alias");
    service->removeAlias("alias2");
    service->removeAlias("alias3");
    QVERIFY(service->readAliasesFor("walletName").isEmpty());
}

struct SetupSessionT {
    QDBusObjectPath sessionPath;
    QCA::SymmetricKey symmetricKey;
    QByteArray error;
};

#define SETUP_SESSION_VERIFY(cond)                                                                                                                             \
    do {                                                                                                                                                       \
        if (!(cond))                                                                                                                                           \
            return SetupSessionT{QDBusObjectPath(), QCA::SymmetricKey(), #cond};                                                                               \
    } while (false)

SetupSessionT setupSession(KWalletFreedesktopService *service)
{
    SetupSessionT result;
    QCA::KeyGenerator keygen;
    auto dlGroup = QCA::DLGroup(keygen.createDLGroup(QCA::IETF_1024));
    if (dlGroup.isNull()) {
        result.error = "createDLGroup failed, maybe libqca-ossl is missing";
        return result;
    }

    auto privateKey = QCA::PrivateKey(keygen.createDH(dlGroup));
    auto publicKey = QCA::PublicKey(privateKey);

    auto connection = QDBusConnection::sessionBus();
    auto message = QDBusMessage::createSignal("dummy", "dummy", "dummy");

    auto pubKeyBytes = publicKey.toDH().y().toArray().toByteArray();
    auto sessionPubKeyVariant = service->OpenSession("dh-ietf1024-sha256-aes128-cbc-pkcs7", QDBusVariant(pubKeyBytes), result.sessionPath);
    SETUP_SESSION_VERIFY(result.sessionPath.path() != "/");
    SETUP_SESSION_VERIFY(sessionPubKeyVariant.variant().canConvert<QByteArray>());

    auto servicePublicKeyBytes = sessionPubKeyVariant.variant().toByteArray();
    SETUP_SESSION_VERIFY(!servicePublicKeyBytes.isEmpty());

    auto servicePublicKey = QCA::DHPublicKey(dlGroup, QCA::BigInteger(QCA::SecureArray(servicePublicKeyBytes)));
    auto commonSecret = privateKey.deriveKey(servicePublicKey);
    result.symmetricKey = QCA::HKDF().makeKey(commonSecret, {}, {}, FDO_SECRETS_CIPHER_KEY_SIZE);

    return result;
}

void FdoSecretsTest::items()
{
    const QStringList wallets = {"wallet1"};
    const QStringList folders = {FDO_SECRETS_DEFAULT_DIR};
    const QStringList entries = {"item1", "item2", "item3"};
    SET_FUNCTION_RESULT(KWalletD::wallets, wallets);
    SET_FUNCTION_RESULT(KWalletD::folderList, folders);
    SET_FUNCTION_RESULT(KWalletD::entryList, entries);

    SET_FUNCTION_IMPL(KWalletD::entryType, [](int, const QString &, const QString &key, const QString &) -> int {
        if (key == "item1")
            return KWallet::Wallet::Password;
        else if (key == "item2")
            return KWallet::Wallet::Map;
        else if (key == "item3")
            return KWallet::Wallet::Stream;

        QTEST_ASSERT(false);
        return KWallet::Wallet::Unknown;
    });

    QString _secretHolder1 = "It's a password";
    QByteArray _secretHolder2;
    QByteArray _secretHolder3;

    {
        QByteArray a = "It's a";
        QString b = "stream";

        QDataStream ds{&_secretHolder2, QIODevice::WriteOnly};
        ds << a << b;
    }

    {
        StrStrMap map;
        map["it's a"] = "map";

        QDataStream ds{&_secretHolder3, QIODevice::WriteOnly};
        ds << map;
    }

    SET_FUNCTION_IMPL(KWalletD::readPassword, [&](int, const QString &, const QString &key, const QString &) -> QString {
        QTEST_ASSERT(key == "item1");
        return _secretHolder1;
    });

    SET_FUNCTION_IMPL(KWalletD::readEntry, [&](int, const QString &, const QString &key, const QString &) -> QByteArray {
        QTEST_ASSERT(key == "item3" || key == "item2");
        if (key == "item2")
            return _secretHolder2;
        else
            return _secretHolder3;
    });

    SET_FUNCTION_IMPL(KWalletD::writePassword, [&](int, const QString &, const QString &key, const QString &value, const QString &) -> int {
        QTEST_ASSERT(key == "item1");
        _secretHolder1 = value;
        return 0;
    });

    using writeEntryT = int (KWalletD::*)(int, const QString &, const QString &, const QByteArray &, int, const QString &);
    SET_FUNCTION_IMPL_OVERLOADED(KWalletD::writeEntry,
                                 writeEntryT,
                                 [&](int, const QString &, const QString &key, const QByteArray &value, int, const QString &) -> int {
                                     QTEST_ASSERT(key == "item3" || key == "item2");
                                     if (key == "item2")
                                         _secretHolder2 = value;
                                     else
                                         _secretHolder3 = value;
                                     return 0;
                                 });

    std::unique_ptr<KWalletD> kwalletd{new KWalletD};
    std::unique_ptr<KWalletFreedesktopService> service{new KWalletFreedesktopService(kwalletd.get())};

    auto collection = service->getCollectionByWalletName("wallet1");
    QVERIFY(collection);

    /* Write some attributes */
    {
        collection->itemAttributes().newItem({FDO_SECRETS_DEFAULT_DIR, "item1"});
        collection->itemAttributes().newItem({FDO_SECRETS_DEFAULT_DIR, "item2"});
        collection->itemAttributes().newItem({FDO_SECRETS_DEFAULT_DIR, "item3"});
        collection->itemAttributes().setParam({FDO_SECRETS_DEFAULT_DIR, "item3"}, FDO_KEY_CREATED, 100200300ULL);
        collection->itemAttributes().setParam({FDO_SECRETS_DEFAULT_DIR, "item3"}, FDO_KEY_MODIFIED, 100200301ULL);
        auto attribs = collection->itemAttributes().getAttributes({FDO_SECRETS_DEFAULT_DIR, "item3"});
        attribs["Attrib1"] = "value1";
        attribs["Attrib2"] = "value2";
        collection->itemAttributes().setAttributes({FDO_SECRETS_DEFAULT_DIR, "item3"}, attribs);
    }

    /* Create collection */
    using OpenAsyncT = int (KWalletD::*)(const QString &, qlonglong, const QString &, bool, const QDBusConnection &, const QDBusMessage &);
    bool openAsyncCalled = false;
    SET_FUNCTION_IMPL_OVERLOADED(KWalletD::openAsync,
                                 OpenAsyncT,
                                 [&](const QString &, qlonglong, const QString &, bool, const QDBusConnection &, const QDBusMessage &) -> int {
                                     openAsyncCalled = true;
                                     return 0;
                                 });

    QDBusObjectPath promptPath;
    service->Unlock({collection->fdoObjectPath()}, promptPath);
    auto prompt = service->getPromptByObjectPath(promptPath);
    QVERIFY(prompt);
    prompt->Prompt("wndid");
    Q_EMIT kwalletd->walletAsyncOpened(0, 0);
    SET_FUNCTION_RESULT_OVERLOADED(KWalletD::isOpen, true, bool(KWalletD::*)(int));
    QVERIFY(!collection->locked());

    auto item1 = collection->findItemByEntryLocation({FDO_SECRETS_DEFAULT_DIR, "item1"});
    auto item2 = collection->findItemByEntryLocation({FDO_SECRETS_DEFAULT_DIR, "item2"});
    auto item3 = collection->findItemByEntryLocation({FDO_SECRETS_DEFAULT_DIR, "item3"});
    QVERIFY(item1 && item2 && item3);

    auto message = QDBusMessage::createSignal("dummy", "dummy", "dummy");
    auto [sessionPath, symmetricKey, errorStr] = setupSession(service.get());
    QVERIFY2(errorStr.isEmpty(), errorStr.constData());

    /* Check secrets */
    auto secret1 = item1->GetSecret(sessionPath);
    service->desecret(message, secret1);
    QCOMPARE(secret1.value.toByteArray(), "It's a password");
    // QCOMPARE(secret1.mimeType, "text/plain");

    auto secret2 = item2->GetSecret(sessionPath);
    service->desecret(message, secret2);
    QByteArray secretBytes = secret2.value.toByteArray();
    QDataStream ds{secretBytes};
    QByteArray a;
    QString b;
    ds >> a >> b;

    QCOMPARE(secret2.mimeType, "application/octet-stream");
    QCOMPARE(a, "It's a");
    QCOMPARE(b, "stream");

    auto secret3 = item3->GetSecret(sessionPath);
    service->desecret(message, secret3);
    auto bytes3 = secret3.value.toByteArray();
    QDataStream ds2(bytes3);
    StrStrMap map3;
    ds2 >> map3;

    QVERIFY(map3.find("it's a") != map3.end() && map3["it's a"] == "map");
    QCOMPARE(item3->created(), 100200300);
    QCOMPARE(item3->modified(), 100200301);
    QCOMPARE(item3->attributes()["Attrib1"], "value1");
    QCOMPARE(item3->attributes()["Attrib2"], "value2");

    /* Set new secrets */
    secret1.value = QByteArray("It's a new password");
    secret1.mimeType = "text/plain";
    service->ensecret(message, secret1);
    item1->SetSecret(secret1);
    secret1 = item1->GetSecret(sessionPath);
    service->desecret(message, secret1);
    QCOMPARE(secret1.value.toByteArray(), "It's a new password");
    QCOMPARE(secret1.mimeType, "text/plain");

    secret2.value = QByteArray("It's a new secret");
    secret2.mimeType = "application/octet-stream";
    service->ensecret(message, secret2);
    item2->SetSecret(secret2);
    auto attribs = item2->attributes();
    attribs["newAttrib"] = ")))";
    item2->setAttributes(attribs);

    secret2 = item2->GetSecret(sessionPath);
    service->desecret(message, secret2);
    QCOMPARE(secret2.value.toByteArray(), "It's a new secret");
    QCOMPARE(item2->attributes()["newAttrib"], ")))");

    /* Search items */
    attribs.clear();
    attribs["Attrib1"] = "value1";
    QList<QDBusObjectPath> lockedItems;
    auto unlockedItems = service->SearchItems(attribs, lockedItems);
    QCOMPARE(unlockedItems.size(), 1);
    QCOMPARE(unlockedItems.front(), item3->fdoObjectPath());
}

void FdoSecretsTest::createLockUnlockCollection()
{
    std::unique_ptr<KWalletD> kwalletd{new KWalletD};
    std::unique_ptr<KWalletFreedesktopService> service{new KWalletFreedesktopService(kwalletd.get())};

    /* Create collection */
    using OpenAsyncT = int (KWalletD::*)(const QString &, qlonglong, const QString &, bool, const QDBusConnection &, const QDBusMessage &);
    bool openAsyncCalled = false;
    SET_FUNCTION_IMPL_OVERLOADED(KWalletD::openAsync,
                                 OpenAsyncT,
                                 [&](const QString &, qlonglong, const QString &, bool, const QDBusConnection &, const QDBusMessage &) -> int {
                                     openAsyncCalled = true;
                                     return 0;
                                 });

    QVariantMap props;
    props["org.freedesktop.Secret.Collection.Label"] = QString("walletName");
    QDBusObjectPath promptPath;
    service->CreateCollection(props, "", promptPath);
    auto prompt = service->getPromptByObjectPath(promptPath);
    QVERIFY(prompt);
    prompt->Prompt("wndid");
    QVERIFY(openAsyncCalled);
    Q_EMIT kwalletd->walletAsyncOpened(0, 0);

    auto createdCollection = service->getCollectionByWalletName("walletName");
    QVERIFY(createdCollection);
    QCOMPARE(createdCollection->label(), "walletName");

    /* Check aliases */
    service->createCollectionAlias("alias", "walletName");
    service->createCollectionAlias("alias2", "walletName");
    service->createCollectionAlias("alias3", "walletName");

    QCOMPARE(service->resolveIfAlias(QStringLiteral(FDO_ALIAS_PATH) + "alias"), createdCollection->fdoObjectPath().path());
    QCOMPARE(service->resolveIfAlias(QStringLiteral(FDO_ALIAS_PATH) + "alias2"), createdCollection->fdoObjectPath().path());
    QCOMPARE(service->resolveIfAlias(QStringLiteral(FDO_ALIAS_PATH) + "alias3"), createdCollection->fdoObjectPath().path());
    QCOMPARE(service->ReadAlias("alias"), createdCollection->fdoObjectPath());

    service->removeAlias("alias");
    service->removeAlias("alias2");
    service->removeAlias("alias3");

    /* Lock/Unlock */
    auto lockedObjects = service->Lock({createdCollection->fdoObjectPath()}, promptPath);
    QCOMPARE(lockedObjects.size(), 1);
    QCOMPARE(lockedObjects.front(), createdCollection->fdoObjectPath());
    SET_FUNCTION_RESULT_OVERLOADED(KWalletD::isOpen, false, bool(KWalletD::*)(int));
    QVERIFY(createdCollection->locked());

    service->Unlock({createdCollection->fdoObjectPath()}, promptPath);
    prompt = service->getPromptByObjectPath(promptPath);
    QVERIFY(prompt);
    openAsyncCalled = false;
    prompt->Prompt("wndid");
    QVERIFY(openAsyncCalled);
    Q_EMIT kwalletd->walletAsyncOpened(0, 0);
    SET_FUNCTION_RESULT_OVERLOADED(KWalletD::isOpen, true, bool(KWalletD::*)(int));
    QVERIFY(!createdCollection->locked());
}

void FdoSecretsTest::session()
{
    std::unique_ptr<KWalletD> kwalletd{new KWalletD};
    std::unique_ptr<KWalletFreedesktopService> service{new KWalletFreedesktopService(kwalletd.get())};

    auto message = QDBusMessage::createSignal("dummy", "dummy", "dummy");
    auto [sessionPath, symmetricKey, errorStr] = setupSession(service.get());

    /* Generate secret */
    auto secret = FreedesktopSecret(sessionPath, QByteArray("It's a secret"), "text/plain");
    QVERIFY(service->ensecret(message, secret));

    /* Try to decrypt by hand with symmetricKey */
    auto cipher = QCA::Cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::PKCS7, QCA::Decode, symmetricKey, secret.parameters);
    QCA::SecureArray result;
    result.append(cipher.update(QCA::MemoryRegion(secret.value.toByteArray())));
    result.append(cipher.final());

    QCOMPARE(QString::fromUtf8(result.toByteArray()), "It's a secret");

    /* Try to decrypt by session */
    QVERIFY(service->desecret(message, secret));
    QCOMPARE(secret.value.toByteArray(), QByteArray("It's a secret"));
}

void FdoSecretsTest::attributes()
{
    KWalletFreedesktopAttributes attribs{"test"};

    attribs.newItem({"dir", "name"});

    attribs.setParam({"dir", "name"}, "param1", 0xff00ff00ff00ff00);
    attribs.setParam({"dir", "name"}, "param2", "string_param");

    QCOMPARE(attribs.getULongLongParam({"dir", "name"}, "param1", 0), 0xff00ff00ff00ff00);
    QCOMPARE(attribs.getStringParam({"dir", "name"}, "param2", ""), "string_param");

    attribs.renameLabel({"dir", "name"}, {"newdir", "newname"});

    QCOMPARE(attribs.getULongLongParam({"newdir", "newname"}, "param1", 0), 0xff00ff00ff00ff00);
    QCOMPARE(attribs.getStringParam({"newdir", "newname"}, "param2", ""), "string_param");
    QCOMPARE(attribs.getULongLongParam({"dir", "name"}, "param1", 0xdef017), 0xdef017);
    QCOMPARE(attribs.getStringParam({"dir", "name"}, "param2", "default"), "default");

    attribs.setParam({"newdir", "newname"}, "param1", 100200300ULL);
    attribs.setParam({"newdir", "newname"}, "param2", "another_string_param");

    QCOMPARE(attribs.getULongLongParam({"newdir", "newname"}, "param1", 0), 100200300ULL);
    QCOMPARE(attribs.getStringParam({"newdir", "newname"}, "param2", ""), "another_string_param");

    QVERIFY(attribs.getAttributes({"newdir", "newname"}).empty());

    StrStrMap attribMap;
    attribMap["key1"] = "value1";
    attribMap["key2"] = "value2";

    attribs.setAttributes({"newdir", "newname"}, attribMap);
    QCOMPARE(attribs.getAttributes({"newdir", "newname"}), attribMap);

    attribs.setAttributes({"dir", "name"}, attribMap);
    /* Item not exists - expects empty attributes map */
    QVERIFY(attribs.getAttributes({"dir", "name"}).empty());

    attribs.setParam({"dir1", "name1"}, "param1", "some_param");
    QCOMPARE(attribs.getStringParam({"dir1", "name1"}, "param1", "default"), "default");
}

void FdoSecretsTest::walletNameEncodeDecode()
{
#define ENCODE_DECODE_CHECK(DECODED, ENCODED)                                                                                                                  \
    do {                                                                                                                                                       \
        auto encodedResult = KWallet::Backend::encodeWalletName(DECODED);                                                                                      \
        auto decodedResult = KWallet::Backend::decodeWalletName(ENCODED);                                                                                      \
        QCOMPARE(encodedResult, ENCODED);                                                                                                                      \
        QCOMPARE(decodedResult, DECODED);                                                                                                                      \
    } while (false)

    ENCODE_DECODE_CHECK("/", ";2F");
    QString allowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789^&'@{}[],$=!-#()%.+_\r\n\t\f\v ";
    ENCODE_DECODE_CHECK(allowedChars, allowedChars);
    ENCODE_DECODE_CHECK("a/b/c\\", "a;2Fb;2Fc;5C");
    ENCODE_DECODE_CHECK("/\\/", ";2F;5C;2F");
    ENCODE_DECODE_CHECK(";;;", ";3B;3B;3B");
    ENCODE_DECODE_CHECK(";3B", ";3B3B");

#undef ENCODE_DECODE_CHECK
}

QTEST_GUILESS_MAIN(FdoSecretsTest)

#include "moc_fdo_secrets_test.cpp"
