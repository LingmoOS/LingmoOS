/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Steffen Hartleib <steffenhartleib@t-online.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KTWOFINGERTAP_TEST_H
#define KTWOFINGERTAP_TEST_H

#include <QMainWindow>
class QCheckBox;
class QLineEdit;
class QSpinBox;
class KTwoFingerTapRecognizer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override;
public Q_SLOTS:
    void slotSwipeTimeChanged(int value);

protected:
    bool eventFilter(QObject *watched, QEvent *e) override;

private:
    void resetAll();
    QWidget *mWidget = nullptr;
    KTwoFingerTapRecognizer *mTwoFingerRec = nullptr;
    Qt::GestureType kTwoFingerTapGesture;
    QCheckBox *mGStarted = nullptr;
    QCheckBox *mGUpdated = nullptr;
    QCheckBox *mGCanceled = nullptr;
    QCheckBox *mGFinished = nullptr;
    QLineEdit *mGPos = nullptr;
    QSpinBox *mGTapRadius = nullptr;
};

#endif
