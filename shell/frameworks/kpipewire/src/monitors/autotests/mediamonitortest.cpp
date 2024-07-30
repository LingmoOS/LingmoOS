/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#define QT_FORCE_ASSERTS 1

#include <QProcess>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickView>
#include <QSignalSpy>
#include <QTest>

namespace
{
template<typename T>
[[nodiscard]] T evaluate(QObject *scope, const QString &expression)
{
    QQmlExpression expr(qmlContext(scope), scope, expression);
    QVariant result = expr.evaluate();
    Q_ASSERT_X(!expr.hasError(), "MediaMonitorTest", qUtf8Printable(expr.error().toString()));
    return result.value<T>();
}

template<>
void evaluate<void>(QObject *scope, const QString &expression)
{
    QQmlExpression expr(qmlContext(scope), scope, expression);
    expr.evaluate();
    Q_ASSERT_X(!expr.hasError(), "MediaMonitorTest", qUtf8Printable(expr.error().toString()));
}

bool initView(QQuickView *view, const QString &urlStr)
{
    view->setSource(QUrl(urlStr));

    QSignalSpy statusSpy(view, &QQuickView::statusChanged);
    if (view->status() == QQuickView::Loading) {
        statusSpy.wait();
    } else if (view->status() != QQuickView::Ready) {
        qCritical() << "Not loading" << view->errors();
        return false;
    }

    if (view->status() != QQuickView::Ready) {
        qCritical() << view->errors();
        return false;
    }

    return true;
}
}

class MediaMonitorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    /**
     * Loads MediaMonitor in QML, and tests it can monitor music streams
     */
    void test_MediaMonitor();
};

void MediaMonitorTest::test_MediaMonitor()
{
    QProcess pipewire;
    QProcess pipewire_pulse;
    QProcess wireplumber;
    QProcess pactl;
    QScopeGuard guard([&pipewire, &pipewire_pulse, &wireplumber, &pactl] {
        if (qEnvironmentVariableIsSet("KDECI_BUILD")) {
            pactl.setArguments({QStringLiteral("unload-module"), QStringLiteral("module-null-sink")});
            pactl.start();
            QVERIFY2(pactl.waitForFinished(), pactl.readAllStandardError().constData());
        }
        if (wireplumber.state() == QProcess::Running) {
            wireplumber.terminate();
        }
        if (pipewire_pulse.state() == QProcess::Running) {
            pipewire_pulse.terminate();
        }
        if (pipewire.state() == QProcess::Running) {
            pipewire.terminate();
        }
    });

    QProcess pidof;
    pidof.start(QStringLiteral("pidof"), {QStringLiteral("pipewire")});
    pidof.waitForFinished();
    if (QString::fromUtf8(pidof.readAllStandardOutput()).toInt() == 0) {
        pipewire.start(QStringLiteral("pipewire"));
        QVERIFY2(pipewire.waitForStarted(), pipewire.readAllStandardError().constData());
        wireplumber.start(QStringLiteral("wireplumber"));
#ifdef Q_OS_LINUX
        QVERIFY2(wireplumber.waitForStarted(), wireplumber.readAllStandardError().constData());
#endif
        QTest::qWait(1000);
        pipewire_pulse.start(QStringLiteral("pipewire-pulse"));
        QVERIFY2(pipewire_pulse.waitForStarted(), pipewire_pulse.readAllStandardError().constData());
    }

    // Dummy output
    if (qEnvironmentVariableIsSet("KDECI_BUILD")) {
        pactl.setProgram(QStringLiteral("pactl"));
        pactl.setArguments({QStringLiteral("load-module"),
                            QStringLiteral("module-null-sink"),
                            QStringLiteral("sink_name=DummyOutput"),
                            QStringLiteral("sink_properties=node.nick=DummyOutput")});
        pactl.start();
        QVERIFY2(pactl.waitForFinished(), pipewire.readAllStandardError().constData());
    }

    QQuickView view;
    QByteArray errorMessage;
    QVERIFY(initView(&view, QFINDTESTDATA(QStringLiteral("mediamonitortest.qml"))));

    QQuickItem *rootObject = view.rootObject();
    QSignalSpy countSpy(rootObject, SIGNAL(countChanged()));

    QProcess player;
    player.setProgram(QStringLiteral("pw-play"));
    player.setArguments({QStringLiteral("-v"), QFINDTESTDATA(QStringLiteral("alarm-clock-elapsed.oga"))});
    player.start();
    QVERIFY2(player.waitForStarted(), player.readAllStandardError().constData());

    if (evaluate<int>(rootObject, QStringLiteral("root.count")) == 0) {
        countSpy.wait();
    }

    QVERIFY(evaluate<bool>(rootObject, QStringLiteral("monitor.detectionAvailable")));
    QCOMPARE(evaluate<int>(rootObject, QStringLiteral("root.count")), 1);
    const int runningInt = evaluate<int>(rootObject, QStringLiteral("Monitor.NodeState.Running"));
    const int suspendedInt = evaluate<int>(rootObject, QStringLiteral("Monitor.NodeState.Suspended"));
    QTRY_COMPARE(evaluate<int>(rootObject, QStringLiteral("root.state")), wireplumber.state() == QProcess::Running ? runningInt : suspendedInt);

    // Changing role will trigger reconnecting
    QObject *monitor = evaluate<QObject *>(rootObject, QStringLiteral("monitor"));
    QVERIFY(monitor);
    QSignalSpy detectionAvailableChangedSpy(monitor, SIGNAL(detectionAvailableChanged()));
    QSignalSpy roleChangedSpy(monitor, SIGNAL(roleChanged()));
    evaluate<void>(rootObject, QStringLiteral("monitor.role = Monitor.MediaMonitor.Camera"));
    if (roleChangedSpy.empty()) {
        QVERIFY(roleChangedSpy.wait());
    }
    QVERIFY(evaluate<bool>(rootObject, QStringLiteral("monitor.detectionAvailable")));
    QCOMPARE(detectionAvailableChangedSpy.size(), 2); // True -> False -> True
    QCOMPARE(evaluate<int>(rootObject, QStringLiteral("root.count")), 0);

    // Change back to Music role
    roleChangedSpy.clear();
    detectionAvailableChangedSpy.clear();
    evaluate<void>(rootObject, QStringLiteral("monitor.role = Monitor.MediaMonitor.Music"));
    if (roleChangedSpy.empty()) {
        QVERIFY(roleChangedSpy.wait());
    }
    QVERIFY(evaluate<bool>(rootObject, QStringLiteral("monitor.detectionAvailable")));
    QCOMPARE(detectionAvailableChangedSpy.size(), 2); // True -> False -> True
    if (evaluate<int>(rootObject, QStringLiteral("root.count")) == 0) {
        QVERIFY(countSpy.wait());
    }
    QCOMPARE(evaluate<int>(rootObject, QStringLiteral("root.count")), 1);

    // Now close the player, and check if the count changes
    player.terminate();
    if (evaluate<int>(rootObject, QStringLiteral("root.count")) > 0) {
        QVERIFY(countSpy.wait());
    }
    QCOMPARE(evaluate<int>(rootObject, QStringLiteral("root.count")), 0);

    pipewire.terminate();
    QTRY_VERIFY(!evaluate<bool>(rootObject, QStringLiteral("monitor.detectionAvailable")));
}

QTEST_MAIN(MediaMonitorTest)

#include "mediamonitortest.moc"
