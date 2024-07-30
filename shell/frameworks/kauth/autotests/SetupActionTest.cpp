/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "BackendsManager.h"
#include <QTest>
#include <kauth/actionreply.h>
#include <kauth/executejob.h>

class SetupActionTest : public QObject
{
    Q_OBJECT

public:
    SetupActionTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void init()
    {
    }

    void testBasicActionProperties();
    void testUserAuthorization();
    void testAuthorizationFail();

    void cleanup()
    {
    }
    void cleanupTestCase()
    {
    }

Q_SIGNALS:
    void changeCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
};

void SetupActionTest::initTestCase()
{
    connect(this,
            SIGNAL(changeCapabilities(KAuth::AuthBackend::Capabilities)),
            KAuth::BackendsManager::authBackend(),
            SLOT(setNewCapabilities(KAuth::AuthBackend::Capabilities)));
}

void SetupActionTest::testBasicActionProperties()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability);
    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("always.authorized"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.name(), QLatin1String("always.authorized"));
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::AuthorizedStatus);

    QVERIFY(action.arguments().isEmpty());
    QVariantMap args;
    args.insert(QLatin1String("akey"), QVariant::fromValue(42));
    action.setArguments(args);
    QCOMPARE(action.arguments(), args);

    Q_EMIT changeCapabilities(KAuth::AuthBackend::NoCapability);

    action = KAuth::Action(QLatin1String("i.do.not.exist"), detailsMap);

    QVERIFY(action.isValid());
    QCOMPARE(action.name(), QLatin1String("i.do.not.exist"));
    QCOMPARE(action.detailsV2(), detailsMap);
    QVERIFY(!action.hasHelper());
    QVERIFY(action.helperId().isEmpty());
    QCOMPARE(action.status(), KAuth::Action::InvalidStatus);
}

void SetupActionTest::testUserAuthorization()
{
    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("requires.auth"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::BackendError);

    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromClientCapability);

    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    job = action.execute();

    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void SetupActionTest::testAuthorizationFail()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromClientCapability);

    KAuth::Action::DetailsMap detailsMap{{KAuth::Action::AuthDetail::DetailOther, QLatin1String("details")}};
    KAuth::Action action(QLatin1String("doomed.to.fail"), detailsMap);
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::DeniedStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(!job->exec());

    QCOMPARE(job->error(), (int)KAuth::ActionReply::AuthorizationDeniedError);
    QVERIFY(job->data().isEmpty());
}

QTEST_MAIN(SetupActionTest)
#include "SetupActionTest.moc"
