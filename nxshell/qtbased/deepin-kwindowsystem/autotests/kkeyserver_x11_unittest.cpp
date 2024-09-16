/* This file is part of the KDE libraries
    Copyright (c) 2017 David Faure <faure@kde.org>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QTest>
#include "kkeyserver_x11.h"
#include <X11/keysym.h>
#include <xcb/xcb_keysyms.h>
#include <QX11Info>

class KKeyServerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase()
    {
        m_keySymbols = xcb_key_symbols_alloc(QX11Info::connection());

        // This makes me wonder why we have KKeyServer::modXShift :-)
        QCOMPARE(XCB_MOD_MASK_SHIFT, xcb_mod_mask_t(KKeyServer::modXShift()));
    }


    void cleanupTestCase()
    {
        if (m_keySymbols) {
            xcb_key_symbols_free(m_keySymbols);
        }
    }

    void keyQtToSymX_data()
    {
        QTest::addColumn<int>("keyQt");
        QTest::addColumn<uint>("modX");
        QTest::addColumn<uint>("additionalState"); // set to XCB_MOD_MASK_SHIFT if shift would indeed be pressed for this shortcut
        QTest::addColumn<int>("keySymX");

        const uint numLock = KKeyServer::modXNumLock();

        // Before adding any testcase below, check what `kcmshell5 keys` records, to make sure it matches
        QTest::newRow("a") << int(Qt::Key_A) << uint(0) << numLock << XK_A;
        QTest::newRow("CTRL_F1") << int(Qt::ControlModifier|Qt::Key_F1) << KKeyServer::modXCtrl() << numLock << XK_F1;
        QTest::newRow("CTRL_1") << int(Qt::ControlModifier|Qt::Key_1) << KKeyServer::modXCtrl() << numLock << XK_1;
        QTest::newRow("CTRL_keypad_1") << int(Qt::ControlModifier|Qt::KeypadModifier|Qt::Key_1) << KKeyServer::modXCtrl() << numLock << XK_KP_1;
        QTest::newRow("CTRL_keypad_slash") << int(Qt::ControlModifier|Qt::KeypadModifier|Qt::Key_Slash) << KKeyServer::modXCtrl() << numLock << XK_KP_Divide;
        QTest::newRow("CTRL_SHIFT_keypad_end") << int(Qt::ControlModifier|Qt::ShiftModifier|Qt::KeypadModifier|Qt::Key_End) << (KKeyServer::modXCtrl()|KKeyServer::modXShift()) << numLock << XK_KP_End;
        QTest::newRow("CTRL_keypad_end_no_numlock") << int(Qt::ControlModifier|Qt::KeypadModifier|Qt::Key_End) << (KKeyServer::modXCtrl()) << uint(0) << XK_KP_End;
        QTest::newRow("CTRL_ampersand") << int(Qt::ControlModifier|Qt::Key_Ampersand) << KKeyServer::modXCtrl() << uint(XCB_MOD_MASK_SHIFT|numLock) << XK_ampersand;
        QTest::newRow("ALT_SHIFT_right") << int(Qt::AltModifier|Qt::ShiftModifier|Qt::Key_Right) << (KKeyServer::modXAlt() | KKeyServer::modXShift()) << numLock << XK_Right;
        QTest::newRow("CTRL_SHIFT_right") << int(Qt::ControlModifier|Qt::ShiftModifier|Qt::Key_Right) << (KKeyServer::modXCtrl() | KKeyServer::modXShift()) << numLock << XK_Right;
        QTest::newRow("META_SHIFT_print") << int(Qt::MetaModifier|Qt::ShiftModifier|Qt::Key_Print) << (KKeyServer::modXMeta() | KKeyServer::modXShift()) << numLock << XK_Print;
        QTest::newRow("ALT_Tab") << int(Qt::AltModifier|Qt::Key_Tab) << (KKeyServer::modXAlt()) << numLock << XK_Tab;
        QTest::newRow("ALT_Shift_Tab") << int(Qt::AltModifier|Qt::ShiftModifier|Qt::Key_Tab) << (KKeyServer::modXAlt() | KKeyServer::modXShift()) << numLock << XK_Tab;
    }

    void keyQtToSymX()
    {
        QFETCH(int, keyQt);
        QFETCH(uint, modX);
        QFETCH(int, keySymX);
        int sym;
        QVERIFY(KKeyServer::keyQtToSymX(keyQt, &sym));
        QCOMPARE(QString::number(sym, 16), QString::number(keySymX, 16));
        uint mod;
        QVERIFY(KKeyServer::keyQtToModX(keyQt, &mod));
        QCOMPARE(mod, modX);
    }

    void symXToKeyQt_data()
    {
        keyQtToSymX_data();
    }

    void symXToKeyQt()
    {
        QFETCH(int, keyQt);
        QFETCH(uint, modX);
        QFETCH(int, keySymX);
        int keyCodeQt;

        //qDebug() << "modX=" << modX << "keySymX=0x" << QString::number(keySymX, 16) << "keyQt=0x" << QString::number(keyQt, 16);

        QVERIFY(KKeyServer::symXModXToKeyQt(keySymX, modX, &keyCodeQt));
        QCOMPARE(keyCodeQt, keyQt);
    }

    void decodeXcbEvent_data()
    {
        keyQtToSymX_data();
    }

    void decodeXcbEvent()
    {
        QFETCH(int, keyQt);
        QFETCH(uint, modX);
        QFETCH(uint, additionalState);
        QFETCH(int, keySymX);

        xcb_keycode_t *keyCodes = xcb_key_symbols_get_keycode(m_keySymbols, keySymX);
        QVERIFY(keyCodes);
        const xcb_keycode_t keyCodeX = keyCodes[0];
        QVERIFY(keyCodeX != XCB_NO_SYMBOL);
        free(keyCodes);

        xcb_key_press_event_t event{ XCB_KEY_PRESS, keyCodeX, 0, 0 /*time*/, 0 /*root*/, 0 /*event*/, 0 /*child*/, 0 /*root_x*/, 0 /*root_y*/, 0 /*event_x*/, 0 /*event_y*/,
            uint16_t(modX | additionalState), 0 /*same_screen*/, 0 /*pad0*/ };

        int decodedKeyQt;
        const bool ok = KKeyServer::xcbKeyPressEventToQt(&event, &decodedKeyQt);
        QVERIFY(ok);
        if (decodedKeyQt != keyQt) {
            qDebug() << "given modX=" << modX << "keySymX=0x" << QString::number(keySymX, 16) << "I expected keyQt=0x" << QString::number(keyQt, 16) << QKeySequence(keyQt).toString() << "got" << QString::number(decodedKeyQt, 16) << QKeySequence(decodedKeyQt).toString();
        }
        QCOMPARE(decodedKeyQt, keyQt);
    }

private:
    xcb_key_symbols_t *m_keySymbols;

};

QTEST_MAIN(KKeyServerTest)

#include "kkeyserver_x11_unittest.moc"
