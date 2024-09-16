// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TESTTASKBARWINDOW_H
#define TESTTASKBARWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QUuid>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QColor>
#include <QCheckBox>
#include <QSlider>
#include <QColorDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dtaskbarcontrol.h"

DGUI_USE_NAMESPACE

class TestTaskbarWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TestTaskbarWindow(QWidget *parent = nullptr);
    ~TestTaskbarWindow();

protected:
    void closeEvent(QCloseEvent *event);

Q_SIGNALS:
    void closeWindow();

private:
    DTaskbarControl *m_pTaskbarControl;
    QCheckBox *m_pProgressBox;
    QCheckBox *m_pCounterBox;
    QLineEdit *m_pNumEdit;
    QSlider *m_pProgress;
    QCheckBox *m_pUrgencyBox;
};

#endif // TESTTASKBARWINDOW_H
