/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QMainWindow>

#include <resultmodel.h>
#include <resultset.h>

#include <memory>

class QAbstractItemModelTester;
namespace Ui
{
class MainWindow;
}

namespace KActivities
{
class Consumer;
namespace Stats
{
class ResultModel;
}
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    Window();
    ~Window() override;

private Q_SLOTS:
    void updateResults();
    void updateRowCount();
    void selectPreset();

private:
    void setQuery(const KActivities::Stats::Query &query);

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<KActivities::Stats::ResultModel> model;
    std::unique_ptr<QAbstractItemModelTester> modelTest;
    std::unique_ptr<KActivities::Consumer> activities;

    QMap<QString, KActivities::Stats::Query> presets;
};
