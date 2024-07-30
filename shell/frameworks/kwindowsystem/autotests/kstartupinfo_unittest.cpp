/*  This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2012, 2019 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2012 Kai Dombrowe <just89@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "netwm.h"
#include <QSignalSpy>
#include <QWidget>
#include <private/qtx11extras_p.h>

#include <kstartupinfo.h>
#include <qtest_widgets.h>

#include <xcb/xcb.h>

#include "cptr_p.h"

Q_DECLARE_METATYPE(KStartupInfoId)
Q_DECLARE_METATYPE(KStartupInfoData)

class KStartupInfo_UnitTest : public QObject
{
    Q_OBJECT
public:
    KStartupInfo_UnitTest()
        : m_listener(KStartupInfo::CleanOnCantDetect, this)
        , m_receivedCount(0)
    {
        qRegisterMetaType<KStartupInfoId>();
        qRegisterMetaType<KStartupInfoData>();
        connect(&m_listener, &KStartupInfo::gotNewStartup, this, &KStartupInfo_UnitTest::slotNewStartup);
    }

protected Q_SLOTS:
    void slotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data)
    {
        ++m_receivedCount;
        m_receivedId = id;
        m_receivedData = data;
        Q_EMIT ready();
    }
Q_SIGNALS:
    void ready();

private Q_SLOTS:
    void testStart();
    void dontCrashCleanup_data();
    void dontCrashCleanup();
    void checkCleanOnCantDetectTest();
    void checkStartupTest_data();
    void checkStartupTest();
    void createNewStartupIdTest();
    void createNewStartupIdForTimestampTest();

private:
    KStartupInfo m_listener;

    int m_receivedCount;
    KStartupInfoId m_receivedId;
    KStartupInfoData m_receivedData;
};

void KStartupInfo_UnitTest::testStart()
{
    KStartupInfoId id;
    id.initId(KStartupInfo::createNewStartupId());

    KStartupInfoData data;
    const QString appId = "/dir with space/kstartupinfo_unittest.desktop";
    data.setApplicationId(appId);
    const QString iconPath = "/dir with space/kstartupinfo_unittest.png";
    data.setIcon(iconPath);
    const QString description = "A description";
    data.setDescription(description);
    const QString name = "A name";
    data.setName(name);
    const int pid = 12345;
    data.addPid(pid);
    const QString bin = "dir with space/kstartupinfo_unittest";
    data.setBin(bin);

    QSignalSpy removedSpy(&m_listener, &KStartupInfo::gotRemoveStartup);
    QVERIFY(removedSpy.isValid());

    KStartupInfo::sendStartup(id, data);
    KStartupInfo::sendFinish(id, data);

    QSignalSpy spy(this, &KStartupInfo_UnitTest::ready);
    spy.wait(5000);

    QCOMPARE(m_receivedCount, 1);
    // qDebug() << m_receivedId.id(); // something like "$HOSTNAME;1342544979;490718;8602_TIME0"
    QCOMPARE(m_receivedData.name(), name);
    QCOMPARE(m_receivedData.description(), description);
    QCOMPARE(m_receivedData.applicationId(), appId);
    QCOMPARE(m_receivedData.icon(), iconPath);
    QCOMPARE(m_receivedData.bin(), bin);
    // qDebug() << m_receivedData.bin() << m_receivedData.name() << m_receivedData.description() << m_receivedData.icon() << m_receivedData.pids() <<
    // m_receivedData.hostname() << m_receivedData.applicationId();

    int waitTime = 0;
    while (waitTime < 5000 && removedSpy.count() < 1) {
        QTest::qWait(200);
        waitTime += 200;
    }
    QCOMPARE(removedSpy.count(), 1);
}

static void doSync()
{
    auto *c = QX11Info::connection();
    const auto cookie = xcb_get_input_focus(c);
    xcb_generic_error_t *error = nullptr;
    UniqueCPointer<xcb_get_input_focus_reply_t> sync(xcb_get_input_focus_reply(c, cookie, &error));
    if (error) {
        free(error);
    }
}

void KStartupInfo_UnitTest::dontCrashCleanup_data()
{
    QTest::addColumn<bool>("silent");
    QTest::addColumn<bool>("change");
    QTest::addColumn<int>("countRemoveStartup");

    QTest::newRow("normal") << false << false << 2;
    QTest::newRow("silent") << true << false << 0;
    QTest::newRow("uninited") << false << true << 0;
}

void KStartupInfo_UnitTest::dontCrashCleanup()
{
    qputenv("KSTARTUPINFO_TIMEOUT", QByteArrayLiteral("1"));

    KStartupInfoId id;
    KStartupInfoId id2;
    id.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_0"));
    id2.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_1"));

    KStartupInfoData data;
    data.setApplicationId(QStringLiteral("/dir with space/kstartupinfo_unittest.desktop"));
    data.setIcon(QStringLiteral("/dir with space/kstartupinfo_unittest.png"));
    data.setDescription(QStringLiteral("A description"));
    data.setName(QStringLiteral("A name"));
    data.addPid(12345);
    data.setBin(QStringLiteral("dir with space/kstartupinfo_unittest"));
    QFETCH(bool, silent);
    if (silent) {
        data.setSilent(KStartupInfoData::Yes);
    }

    QSignalSpy spy(&m_listener, &KStartupInfo::gotRemoveStartup);
    QFETCH(bool, change);
    if (change) {
        KStartupInfo::sendChange(id, data);
        KStartupInfo::sendChange(id2, data);
    } else {
        KStartupInfo::sendStartup(id, data);
        KStartupInfo::sendStartup(id2, data);
    }

    // let's do a roundtrip to the X server
    doSync();

    QFETCH(int, countRemoveStartup);
    int waitTime = 1900;
    QTest::qWait(1900);
    while (waitTime <= 5000) {
        QTest::qWait(200);
        waitTime += 200;
        if (spy.count() == countRemoveStartup) {
            break;
        }
    }
    QCOMPARE(spy.count(), countRemoveStartup);
}

void KStartupInfo_UnitTest::checkCleanOnCantDetectTest()
{
    KStartupInfoId id;
    KStartupInfoId id2;
    id.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_0"));
    id2.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_1"));

    KStartupInfoData data;
    data.setApplicationId(QStringLiteral("/dir with space/kstartupinfo_unittest.desktop"));
    data.setIcon(QStringLiteral("/dir with space/kstartupinfo_unittest.png"));
    data.setDescription(QStringLiteral("A description"));
    data.setName(QStringLiteral("A name"));
    data.setBin(QStringLiteral("dir with space/kstartupinfo_unittest"));
    data.setWMClass(QByteArrayLiteral("0"));

    xcb_connection_t *c = QX11Info::connection();
    xcb_window_t window = xcb_generate_id(c);
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(c,
                      XCB_COPY_FROM_PARENT,
                      window,
                      QX11Info::appRootWindow(),
                      0,
                      0,
                      100,
                      100,
                      0,
                      XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_EVENT_MASK,
                      values);

    KStartupInfo::sendStartup(id, data);
    KStartupInfo::sendStartup(id2, data);

    int previousCount = m_receivedCount;

    doSync();
    QTest::qWait(10);

    xcb_map_window(c, window);
    xcb_flush(c);
    QTest::qWait(10);

    xcb_unmap_window(c, window);
    xcb_flush(c);
    QTest::qWait(100);
    xcb_map_window(c, window);
    xcb_flush(c);

    QCOMPARE(m_receivedCount, previousCount + 2);
    QCOMPARE(m_receivedId, id2);
}

void KStartupInfo_UnitTest::checkStartupTest_data()
{
    QTest::addColumn<QByteArray>("wmClass");
    QTest::addColumn<int>("pid");

    QTest::newRow("wmClass") << QByteArrayLiteral("kstartupinfotest") << 0;
    QTest::newRow("pid") << QByteArray() << 12345;
}

void KStartupInfo_UnitTest::checkStartupTest()
{
    KStartupInfoId id;
    KStartupInfoId id2;
    id.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_0"));
    id2.initId(QByteArrayLiteral("somefancyidwhichisrandom_kstartupinfo_unittest_1"));

    KStartupInfoData data;
    data.setApplicationId(QStringLiteral("/dir with space/kstartupinfo_unittest.desktop"));
    data.setIcon(QStringLiteral("/dir with space/kstartupinfo_unittest.png"));
    data.setDescription(QStringLiteral("A description"));
    data.setName(QStringLiteral("A name"));
    data.setBin(QStringLiteral("dir with space/kstartupinfo_unittest"));
    QFETCH(int, pid);
    data.addPid(pid);
    data.setHostname(QByteArrayLiteral("localhost"));

    // important for this test: WMClass
    QFETCH(QByteArray, wmClass);
    data.setWMClass(wmClass);

    xcb_connection_t *c = QX11Info::connection();
    xcb_window_t window = xcb_generate_id(c);
    uint32_t values[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(c,
                      XCB_COPY_FROM_PARENT,
                      window,
                      QX11Info::appRootWindow(),
                      0,
                      0,
                      100,
                      100,
                      0,
                      XCB_COPY_FROM_PARENT,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_EVENT_MASK,
                      values);

    xcb_change_property(c,
                        XCB_PROP_MODE_REPLACE,
                        window,
                        XCB_ATOM_WM_CLASS,
                        XCB_ATOM_STRING,
                        8,
                        wmClass.length() * 2 + 1,
                        "kstartupinfotest\0kstartupinfotest");
    xcb_change_property(c, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_CLIENT_MACHINE, XCB_ATOM_STRING, 8, 9, "localhost");
    NETWinInfo winInfo(QX11Info::connection(), window, QX11Info::appRootWindow(), NET::Properties(), NET::Properties2());
    winInfo.setPid(pid);

    KStartupInfo info(KStartupInfo::DisableKWinModule | KStartupInfo::AnnounceSilenceChanges, this);
    KStartupInfo::sendStartup(id, data);
    KStartupInfo::sendStartup(id2, data);

    doSync();
    QTest::qWait(100);

    QCOMPARE(info.checkStartup(window), KStartupInfo::Match);
    QCOMPARE(info.checkStartup(window), KStartupInfo::Match);
}

void KStartupInfo_UnitTest::createNewStartupIdTest()
{
    const QByteArray &id = KStartupInfo::createNewStartupId();
    QVERIFY(!id.isEmpty());
    const int index = id.indexOf(QByteArrayLiteral("TIME"));
    QVERIFY(index != -1);
    const QByteArray time = id.mid(index + 4);
    QVERIFY(time.toULongLong() != 0u);
}

void KStartupInfo_UnitTest::createNewStartupIdForTimestampTest()
{
    const QByteArray &id = KStartupInfo::createNewStartupIdForTimestamp(5);
    QVERIFY(!id.isEmpty());
    const int index = id.indexOf(QByteArrayLiteral("TIME"));
    QVERIFY(index != -1);
    QCOMPARE(id.mid(index + 4).toULongLong(), 5u);
}

QTEST_MAIN(KStartupInfo_UnitTest)

#include "kstartupinfo_unittest.moc"
