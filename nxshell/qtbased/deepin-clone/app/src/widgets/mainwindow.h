// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "selectactionpage.h"

#include <DMainWindow>
#include <dpageindicator.h>
//#include <DVideoWidget>

#include <QStackedWidget>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QMediaPlayer;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class IconLabel;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    enum Status {
        SelectAction,
        SelectFile,
        WaitConfirm,
        ToLiveSystem,
        Working,
        End
    };
    Q_ENUM(Status)

    enum ButtonAction {
        None,
        Next,
        Cancel,
        Retry,
        ShowBackupFile,
        Quit,
        RestartSystem,
        RestartToLiveSystem
    };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void startWithFile(const QString &source, const QString &target);

    static bool showFileItem(const QString &file);

private slots:
    bool openUrl(const QUrl &url);

private:
    void init();
    void setStatus(Status status);
    void next();
    void setContent(QWidget *widget);
    QWidget *content() const;
    void onButtonClicked();
    void showErrorMessage(const QString &message);
    void playLoadingIndicator();
    void stopLoadingIndicator();

    bool isError() const;
    QString currentModeString() const;
    QString operateObjectString() const;

    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

    Status m_currentStatus = (Status)(End + 1);
    SelectActionPage::Action m_currentMode = SelectActionPage::Backup;
    SelectActionPage::Mode m_operateObject = SelectActionPage::Disk;
    ButtonAction m_buttonAction = None;

    QString m_sourceFile;
    QString m_targetFile;

    IconLabel *m_title;
    QLabel *m_subTitle;
    QStackedWidget *m_contentWidget;
    QCheckBox *m_fixBootCheckBox;
    QPushButton *m_cancelButton;
    QPushButton *m_bottomButton;
//    QMediaPlayer *m_player = 0;
//    DVideoWidget *m_loadingIndicator;
    DPageIndicator *m_pageIndicator;
    QString m_buttonTextBackup;

    friend QString parseSerialUrl(const QString &, MainWindow *);
};

#endif // MAINWINDOW_H
