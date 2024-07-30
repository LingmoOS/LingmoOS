/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktwofingertap_test.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QTouchEvent>
#include <QSpinBox>

#include <ktwofingertap.h>


MainWindow::MainWindow()
    : QMainWindow()
{
    mWidget = new QWidget(this);
    setCentralWidget(mWidget);

    QVBoxLayout *vLayout = new QVBoxLayout (mWidget);

    QLabel *msgLabel = new QLabel(QStringLiteral("Make a two finger tap gesture in this window."), mWidget);
    msgLabel->setAlignment(Qt::AlignHCenter);

    mGStarted = new QCheckBox(QStringLiteral("gesture started"), mWidget);
    mGStarted->setEnabled(false);

    mGUpdated = new QCheckBox(QStringLiteral("gesture updated"), mWidget);
    mGUpdated->setEnabled(false);

    mGCanceled = new QCheckBox(QStringLiteral("gesture canceled"), mWidget);
    mGCanceled->setEnabled(false);

    mGFinished = new QCheckBox(QStringLiteral("gesture finished"), mWidget);
    mGFinished->setEnabled(false);

    mGPos = new QLineEdit(mWidget);
    mGPos->setReadOnly(true);

    QLabel *radiusLabel = new QLabel(QStringLiteral("current tapRadius:"), mWidget);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    radiusLabel->setSizePolicy(sizePolicy);
    mGTapRadius = new QSpinBox(mWidget);
    mGTapRadius->setMaximum(1000);

    vLayout->addWidget(msgLabel);
    vLayout->addWidget(mGStarted);
    vLayout->addWidget(mGUpdated);
    vLayout->addWidget(mGCanceled);
    vLayout->addWidget(mGFinished);
    vLayout->addWidget(mGPos);
    vLayout->addWidget(radiusLabel);
    vLayout->addWidget(mGTapRadius);

    mTwoFingerRec = new KTwoFingerTapRecognizer();
    kTwoFingerTapGesture = QGestureRecognizer::registerRecognizer(mTwoFingerRec);

    mGTapRadius->setValue(mTwoFingerRec->tapRadius());
    mWidget->grabGesture(kTwoFingerTapGesture);
    mWidget->setAttribute(Qt::WA_AcceptTouchEvents);
    mWidget->installEventFilter(this);

    connect (mGTapRadius, &QSpinBox::valueChanged, this, &MainWindow::slotSwipeTimeChanged);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject *watched, QEvent *e)
{
    if (e->type() == QEvent::TouchBegin) {
        resetAll();
    }

    if (e->type() == QEvent::Gesture) {
        QGestureEvent *gEvent = static_cast<QGestureEvent *>(e);
        KTwoFingerTap *tap = static_cast<KTwoFingerTap *>(gEvent->gesture(kTwoFingerTapGesture));

        if (tap) {
            const Qt::GestureState state = tap->state();

            if (state == Qt::GestureFinished) {
                mGFinished->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureStarted) {
                // We need to map the local position, because the tap->pos() is the local position which on received the gesture.
                // In this case it is local to the Widgets in the layout.
                const QPoint lokalPos = mWidget->mapFromGlobal(tap->scenePos().toPoint());
                QString posStr = QStringLiteral("tap Position = ");
                posStr += QString::number(lokalPos.x()) + QStringLiteral(",") + QString::number(lokalPos.y());
                mGPos->setText(posStr);
                mGStarted->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureUpdated) {
                mGUpdated->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureCanceled) {
                mGCanceled->setCheckState(Qt::Checked);
            } else if (mGCanceled->isChecked() && mGFinished->isChecked()) {
                qWarning()<<"ERROR: The gesture state is canceled and finished at the same time";
            }
        }
        e->accept();
        return true;
    }
    return QObject::eventFilter(watched, e);
}

void MainWindow::resetAll()
{
    mGStarted->setCheckState(Qt::Unchecked);
    mGUpdated->setCheckState(Qt::Unchecked);
    mGCanceled->setCheckState(Qt::Unchecked);
    mGFinished->setCheckState(Qt::Unchecked);

    mGPos->setText(QString());
}

void MainWindow::slotSwipeTimeChanged(int value)
{
    mTwoFingerRec->setTapRadius(value);
}


int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("ktwofingertap_test"));
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setGeometry(500, 200, 500, 500);
    mainWindow.show();

    return app.exec();
}

#include "moc_ktwofingertap_test.cpp"
