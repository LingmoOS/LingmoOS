/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMainWindow>

#include <activitiesmodel.h>
#include <consumer.h>

namespace Ui
{
class MainWindow;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();
    ~Window() override;

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::MainWindow *const ui;
    KActivities::Consumer *const activities;
    KActivities::ActivitiesModel *const modelRunningActivities;
    KActivities::ActivitiesModel *const modelStoppedActivities;
};
