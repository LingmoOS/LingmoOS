/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTWOFINGERSWIPE_TEST_H
#define KTWOFINGERSWIPE_TEST_H

#include <QMainWindow>
class QCheckBox;
class QLineEdit;
class QSpinBox;
class KTwoFingerSwipeRecognizer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;
public Q_SLOTS:
    void slotSwipeTimeChanged(int value);
    void slotSwipeDistanceChanged(int value);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;

private:
    void resetAll();
    QWidget *mWidget = nullptr;
    KTwoFingerSwipeRecognizer *mTwoFingerRec = nullptr;
    Qt::GestureType mKTwoFingerSwipeGesture;
    QCheckBox *mGStarted = nullptr;
    QCheckBox *mGUpdated = nullptr;
    QCheckBox *mGCanceled = nullptr;
    QCheckBox *mGFinished = nullptr;
    QLineEdit *mGPos = nullptr;
    QSpinBox *mGSwipeTime = nullptr;
    QSpinBox *mGSwipeDistance = nullptr;
};

#endif
