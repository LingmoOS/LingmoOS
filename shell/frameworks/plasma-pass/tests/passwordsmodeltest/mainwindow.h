// SPDX-FileCopyrightText: 2018 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QPointer>

class QLabel;
class QPushButton;
class QProgressBar;
class QStackedWidget;

namespace PlasmaPass
{
class ProviderBase;
class PasswordFilterModel;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private Q_SLOTS:
    void onPasswordClicked(const QModelIndex &idx);
    void onSearchChanged(const QString &text);

private:
    void setProvider(PlasmaPass::ProviderBase *provider);

    QLabel *mTitle = nullptr;
    QLabel *mType = nullptr;
    QLabel *mPath = nullptr;
    QLabel *mPassword = nullptr;
    QLabel *mError = nullptr;
    QPushButton *mPassBtn = nullptr;
    QProgressBar *mPassProgress = nullptr;
    QModelIndex mCurrent;
    QPointer<PlasmaPass::ProviderBase> mProvider;
    QStackedWidget *mStack = nullptr;
    PlasmaPass::PasswordFilterModel *mFilterModel = nullptr;
};

#endif
