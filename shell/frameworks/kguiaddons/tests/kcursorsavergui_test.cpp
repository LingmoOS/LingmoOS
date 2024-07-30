/*
    SPDX-FileCopyrightText: 2020 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QApplication>
#include <QPushButton>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>

#include "kcursorsaver.h"

class KCursorSaverTestWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KCursorSaverTestWidget(QWidget *parent = nullptr);
};

KCursorSaverTestWidget::KCursorSaverTestWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QPushButton *busyButton = new QPushButton(QStringLiteral("busy"), this);
    layout->addWidget(busyButton);
    connect(busyButton, &QPushButton::clicked, this, []() {
        KCursorSaver saver(Qt::WaitCursor);
        QThread::sleep(3);
    });
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KCursorSaverTestWidget mainWidget;
    mainWidget.show();

    return app.exec();
}

#include "kcursorsavergui_test.moc"
