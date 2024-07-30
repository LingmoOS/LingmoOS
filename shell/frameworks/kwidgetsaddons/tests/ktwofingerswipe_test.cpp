/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ktwofingerswipe_test.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QTouchEvent>
#include <QSpinBox>

#include <ktwofingerswipe.h>


MainWindow::MainWindow()
    : QMainWindow()
{
    mWidget = new QWidget(this);
    setCentralWidget(mWidget);

    QVBoxLayout *vLayout = new QVBoxLayout (mWidget);

    QLabel *msgLabel = new QLabel(QStringLiteral("Make a two finger swipe gesture in this window."), mWidget);
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

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    QLabel *timeLabel = new QLabel(QStringLiteral("maximum duration:"), mWidget);
    timeLabel->setSizePolicy(sizePolicy);
    mGSwipeTime = new QSpinBox(mWidget);
    mGSwipeTime->setMaximum(1000);

    QLabel *distanceLabel = new QLabel(QStringLiteral("minimum distance:"), mWidget);
    distanceLabel->setSizePolicy(sizePolicy);
    mGSwipeDistance = new QSpinBox(mWidget);
    mGSwipeDistance->setMaximum(1000);

    vLayout->addWidget(msgLabel);
    vLayout->addWidget(mGStarted);
    vLayout->addWidget(mGUpdated);
    vLayout->addWidget(mGCanceled);
    vLayout->addWidget(mGFinished);
    vLayout->addWidget(mGPos);
    vLayout->addWidget(timeLabel);
    vLayout->addWidget(mGSwipeTime);
    vLayout->addWidget(distanceLabel);
    vLayout->addWidget(mGSwipeDistance);

    mTwoFingerRec = new KTwoFingerSwipeRecognizer();
    mKTwoFingerSwipeGesture = QGestureRecognizer::registerRecognizer(mTwoFingerRec);

    mGSwipeTime->setValue(mTwoFingerRec->maxSwipeTime());
    mGSwipeDistance->setValue(mTwoFingerRec->minSswipeDistance());

    mWidget->grabGesture(mKTwoFingerSwipeGesture);
    mWidget->setAttribute(Qt::WA_AcceptTouchEvents);
    mWidget->installEventFilter(this);

    connect (mGSwipeTime, &QSpinBox::valueChanged, this, &MainWindow::slotSwipeTimeChanged);
    connect (mGSwipeDistance, &QSpinBox::valueChanged, this, &MainWindow::slotSwipeDistanceChanged);
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
        KTwoFingerSwipe *swipe = static_cast<KTwoFingerSwipe *>(gEvent->gesture(mKTwoFingerSwipeGesture));
        if (swipe) {
            // We need to map the local position, because the tap->pos() is the local position which on received the gesture.
            // In this case it is local to the Widgets in the layout.
            const QPoint lokalPos = mWidget->mapFromGlobal(swipe->scenePos().toPoint());
            QString posStr = QStringLiteral("tap Position = ");
            posStr += QString::number(lokalPos.x()) + QStringLiteral(",") + QString::number(lokalPos.y());

            const Qt::GestureState state = swipe->state();

            if (state == Qt::GestureFinished) {
                posStr += QStringLiteral("  angle = ");
                posStr += QString::number(swipe->swipeAngle());
                mGPos->setText(posStr);
                mGFinished->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureStarted) {
                mGPos->setText(posStr);
                mGStarted->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureUpdated) {
                mGUpdated->setCheckState(Qt::Checked);
            } else if (state == Qt::GestureCanceled) {
                mGCanceled->setCheckState(Qt::Checked);
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
    mTwoFingerRec->setMaxSwipeTime(value);
}

void MainWindow::slotSwipeDistanceChanged(int value)
{
    mTwoFingerRec->setSwipeDistance(value);
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("ktwofingerswipe_test"));
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setGeometry(500, 200, 500, 500);
    mainWindow.show();

    return app.exec();
}

#include "moc_ktwofingerswipe_test.cpp"
