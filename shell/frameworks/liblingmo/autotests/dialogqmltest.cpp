/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialogqmltest.h"

#include <lingmo.h>

#include <QQmlContext>
#include <QQmlEngine>

#include <QSignalSpy>

// this test checks that we don't set visible to true until after we set the window flags
void DialogQmlTest::loadAndShow()
{
    QQmlEngine engine;

    QByteArray dialogQml =
        "import QtQuick 2.0\n"
        "import org.kde.lingmo.core as LingmoCore\n"
        "\n"
        "LingmoCore.Dialog {\n"
        "    id: root\n"
        "\n"
        "    location: true && LingmoCore.Types.TopEdge\n"
        "    visible: true && true\n"
        "    type: true && LingmoCore.Dialog.Notification\n"
        "\n"
        "    mainItem: Rectangle {\n"
        "        width: 200\n"
        "        height: 200\n"
        "    }\n"
        "}\n";

    // we use true && Value to force it to be a complex binding, which won't be evaluated in
    // component.beginCreate
    // the bug still appears without this, but we need to delay it in this test
    // so we can connect to the visibleChanged signal

    QQmlComponent component(&engine);

    QSignalSpy spy(&component, SIGNAL(statusChanged(QQmlComponent::Status)));
    component.setData(dialogQml, QUrl(QStringLiteral("test://dialogTest")));
    spy.wait();

    LingmoQuick::Dialog *dialog = qobject_cast<LingmoQuick::Dialog *>(component.beginCreate(engine.rootContext()));
    qDebug() << component.errorString();
    Q_ASSERT(dialog);

    m_dialogShown = false;

    // this will be called during component.completeCreate
    auto c = connect(dialog, &QWindow::visibleChanged, [=, this]() {
        m_dialogShown = true;
        QCOMPARE(dialog->type(), LingmoQuick::Dialog::Notification);
        QCOMPARE(dialog->location(), Lingmo::Types::TopEdge);
    });

    component.completeCreate();
    QCOMPARE(m_dialogShown, true);

    // disconnect on visible changed before we delete the dialog
    disconnect(c);

    delete dialog;
}

QTEST_MAIN(DialogQmlTest)

#include "moc_dialogqmltest.cpp"
