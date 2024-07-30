/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#include "kkeysequencerecordertest.h"

#include "kkeysequencerecorder.h"

#include <QSignalSpy>
#include <QTest>
#include <QWindow>

QTEST_MAIN(KKeySequenceRecorderTest)

void KKeySequenceRecorderTest::initTestCase()
{
    m_window = new QWindow;
}

void KKeySequenceRecorderTest::cleanupTestCase()
{
    delete m_window;
}

void KKeySequenceRecorderTest::testValidWindow()
{
    KKeySequenceRecorder recorder(nullptr);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);

    recorder.startRecording();
    QCOMPARE(recordingSpy.count(), 0);
    QVERIFY(!recorder.isRecording());

    recorder.setWindow(m_window);
    recorder.startRecording();
    QCOMPARE(recordingSpy.count(), 2);
    QVERIFY(recorder.isRecording());
}

void KKeySequenceRecorderTest::testRecording()
{
    KKeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 1);
    QCOMPARE(sequenceSpy.count(), 1);

    QTest::keyClick(m_window, Qt::Key_A, Qt::ControlModifier);
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 1);
    QCOMPARE(sequenceSpy.count(), 4); // two key events + modifier release
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));

    QTest::qWait(800);
    QCOMPARE(sequenceSpy.count(), 4);
    QCOMPARE(recordingSpy.count(), 2);
    QVERIFY(!recorder.isRecording());
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
}

void KKeySequenceRecorderTest::testModifiers()
{
    KKeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);

    recorder.startRecording();
    QCOMPARE(sequenceSpy.count(), 1);
    QCOMPARE(recordingSpy.count(), 1);
    QVERIFY(recorder.isRecording());

    QTest::keyPress(m_window, Qt::Key_Control);
    QCOMPARE(sequenceSpy.count(), 2);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::ControlModifier));

    // Releasing a different key because when releasing Key_Meta, QTest sends a KeyRelease event
    // with MetaModifier even though modifieres should be empty
    QTest::keyRelease(m_window, Qt::Key_A, Qt::NoModifier);
    QCOMPARE(sequenceSpy.count(), 3);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence());
    QCOMPARE(recordingSpy.count(), 1);
    QVERIFY(recorder.isRecording());

    QTRY_VERIFY_WITH_TIMEOUT(recorder.isRecording(), 800);

    QTest::keyPress(m_window, Qt::Key_Control);
    QCOMPARE(sequenceSpy.count(), 4);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::ControlModifier));

    QTest::keyPress(m_window, Qt::Key_Alt, Qt::ControlModifier);
    QCOMPARE(sequenceSpy.count(), 6); // QTest sends two key events, one for each modifier
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::AltModifier | Qt::ControlModifier));

    QCOMPARE(resultSpy.count(), 0); // modifierless not allowed
}

void KKeySequenceRecorderTest::testModifierless()
{
    KKeySequenceRecorder recorder(m_window);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);

    recorder.setModifierlessAllowed(false);
    QTest::keyPress(m_window, Qt::Key_A);
    QTRY_VERIFY_WITH_TIMEOUT(recorder.isRecording(), 800);
    QCOMPARE(sequenceSpy.count(), 1);
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence());

    recorder.setModifierlessAllowed(true);
    QTest::keyClick(m_window, Qt::Key_A);
    QTRY_VERIFY_WITH_TIMEOUT(!recorder.isRecording(), 800);
    QCOMPARE(sequenceSpy.count(), 2);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A));
}

void KKeySequenceRecorderTest::testModifierOnly()
{
    KKeySequenceRecorder recorder(m_window);
    recorder.setModifierOnlyAllowed(true);
    recorder.setModifierlessAllowed(true);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);

    QTest::keyClick(m_window, Qt::Key_Shift);
    recordingSpy.wait();
    QVERIFY(!recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 3);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_Shift));
}

void KKeySequenceRecorderTest::testModifierOnlyMultiple()
{
    KKeySequenceRecorder recorder(m_window);
    recorder.setModifierOnlyAllowed(true);
    recorder.setModifierlessAllowed(true);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());

    QTest::keyPress(m_window, Qt::Key_Shift);
    Qt::KeyboardModifiers modifiers = Qt::ShiftModifier;
    QTest::keyPress(m_window, Qt::Key_Control, modifiers);
    modifiers |= Qt::ControlModifier;
    modifiers |= Qt::AltModifier;
    QTest::keyPress(m_window, Qt::Key_Alt, modifiers);
    QTest::keyPress(m_window, Qt::Key_Meta, modifiers);
    modifiers |= Qt::MetaModifier;
    QTest::qWait(200);
    QTest::keyRelease(m_window, Qt::Key_Control, modifiers);
    modifiers &= ~Qt::ControlModifier;
    QTest::qWait(10);
    QTest::keyRelease(m_window, Qt::Key_Alt, modifiers);
    modifiers &= ~Qt::AltModifier;
    modifiers &= ~Qt::ShiftModifier;
    QTest::qWait(10);
    QTest::keyRelease(m_window, Qt::Key_Shift, modifiers);
    QTest::qWait(10);
    QTest::keyRelease(m_window, Qt::Key_Meta, modifiers);
    recordingSpy.wait();
    QVERIFY(!recorder.isRecording());
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(Qt::MetaModifier | Qt::ControlModifier | Qt::AltModifier | Qt::Key_Shift));
}

void KKeySequenceRecorderTest::testModifierOnlyMultipleInterrupt()
{
    KKeySequenceRecorder recorder(m_window);
    recorder.setModifierOnlyAllowed(true);
    recorder.setModifierlessAllowed(true);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());

    QTest::keyPress(m_window, Qt::Key_Control);
    QTest::keyPress(m_window, Qt::Key_Meta, Qt::ControlModifier);
    QTest::keyClick(m_window, Qt::Key_A, Qt::ControlModifier);

    recordingSpy.wait();
    QVERIFY(!recorder.isRecording());
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(Qt::ControlModifier | Qt::Key_A));
}

void KKeySequenceRecorderTest::testModifierOnlyDisabled()
{
    KKeySequenceRecorder recorder(m_window);
    recorder.setModifierOnlyAllowed(false);
    recorder.setModifierlessAllowed(true);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);

    QTest::keyClick(m_window, Qt::Key_Shift);
    recordingSpy.wait();
    QVERIFY(recorder.isRecording());
    recorder.cancelRecording();
    QCOMPARE(sequenceSpy.count(), 3);
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence());
}

void KKeySequenceRecorderTest::testMultiKeyAllowed()
{
    KKeySequenceRecorder recorder(m_window);
    QSignalSpy recordingSpy(&recorder, &KKeySequenceRecorder::recordingChanged);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);

    recorder.startRecording();

    recorder.setMultiKeyShortcutsAllowed(true);
    int keys[4] = {0};
    for (int i = 0; i < 4; ++i) {
        QVERIFY(recorder.isRecording());
        QCOMPARE(recordingSpy.count(), 1);
        QCOMPARE(resultSpy.count(), 0);
        keys[i] = (Qt::Key_A | Qt::ControlModifier).toCombined();
        QKeySequence result(keys[0], keys[1], keys[2], keys[3]);
        QTest::keyPress(m_window, Qt::Key_A, Qt::ControlModifier);
        QTest::keyRelease(m_window, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(recorder.currentKeySequence(), QKeySequence(keys[0], keys[1], keys[2], keys[3]));
    }
    QVERIFY(!recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 2);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeFirst().at(0).value<QKeySequence>(), QKeySequence(keys[0], keys[1], keys[2], keys[3]));

    recorder.setMultiKeyShortcutsAllowed(false);
    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 3);
    QTest::keyPress(m_window, Qt::Key_A, Qt::ControlModifier);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
    QVERIFY(!recorder.isRecording());
    QCOMPARE(recordingSpy.count(), 4);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(resultSpy.takeAt(0).at(0).value<QKeySequence>(), QKeySequence(Qt::Key_A | Qt::ControlModifier));
}

void KKeySequenceRecorderTest::testKeyNonLetterNoModifier()
{
    KKeySequenceRecorder recorder(m_window);
    QSignalSpy resultSpy(&recorder, &KKeySequenceRecorder::gotKeySequence);
    QSignalSpy sequenceSpy(&recorder, &KKeySequenceRecorder::currentKeySequenceChanged);

    recorder.startRecording();
    QVERIFY(recorder.isRecording());
    QCOMPARE(sequenceSpy.count(), 1);

    recorder.setModifierlessAllowed(false);
    QTest::keyPress(m_window, Qt::Key_Insert);
    QTRY_VERIFY_WITH_TIMEOUT(recorder.isRecording(), 800);
    QCOMPARE(sequenceSpy.count(), 2);
    QCOMPARE(resultSpy.count(), 0);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_Insert));

    recorder.setModifierlessAllowed(true);
    QTest::keyClick(m_window, Qt::Key_Insert);
    QTRY_VERIFY_WITH_TIMEOUT(!recorder.isRecording(), 800);
    QCOMPARE(sequenceSpy.count(), 3);
    QCOMPARE(resultSpy.count(), 1);
    QCOMPARE(recorder.currentKeySequence(), QKeySequence(Qt::Key_Insert, Qt::Key_Insert));
}

#include "moc_kkeysequencerecordertest.cpp"
