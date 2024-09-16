// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <QApplication>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QThread>

#include <DDesktopServices>

#include <unistd.h>
#include <stdio.h>

class Thread : public QThread
{
public:
    explicit Thread(QObject *parent = 0) : QThread(parent) {}

    void run()
    {
        while (true) {
            char ch = getchar();

            if (ch == 'a' && getchar() == '\n') {
                qRegisterMetaType<Qt::ApplicationState>("Qt::ApplicationState");
                QMetaObject::invokeMethod(qApp, "applicationStateChanged", Qt::QueuedConnection,
                                          Q_ARG(Qt::ApplicationState, Qt::ApplicationActive));
            }
        }
    }
};

int main(int argc, char *argv[])
{
    if (!qEnvironmentVariableIsEmpty(DEEPIN_CLONE_UID)) {
        const quint32 &uid = qgetenv(DEEPIN_CLONE_UID).toUInt();

        setuid(uid);
    }

    if (qEnvironmentVariableIsSet(DEEPIN_CLONE_OPEN_DIALOG)) {
        QApplication *app = new QApplication(argc, argv);
        QFileDialog dialog(0, QString(), QDir::homePath());

        dialog.setWindowFlags(dialog.windowFlags() | Qt::WindowStaysOnTopHint);
        dialog.setMimeTypeFilters(QStringList() << "application-x-deepinclone-dim");
        dialog.setNameFilters(QStringList() << QString::fromUtf8(qgetenv(DEEPIN_CLONE_NAME_FILTER)) + "(*.dim)");
        dialog.setDefaultSuffix("dim");
        dialog.setWindowTitle(qgetenv(DEEPIN_CLONE_TITLE));
        dialog.setModal(true);

        if (qgetenv(DEEPIN_CLONE_OPEN_DIALOG) == DEEPIN_CLONE_GET_FILE) {
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setAcceptMode(QFileDialog::AcceptSave);
            dialog.selectFile(QString::fromUtf8(qgetenv(DEEPIN_CLONE_FILE_NAME)));
        } else {
            dialog.setFileMode(QFileDialog::ExistingFile);
            dialog.setAcceptMode(QFileDialog::AcceptOpen);
        }

        qputenv(DEEPIN_CLONE_FILE_NAME, "");

        Thread *thread = new Thread(app);

        thread->start();

        if (dialog.exec() == QFileDialog::Accepted) {
            printf("%s", dialog.selectedFiles().first().toUtf8().constData());
            fflush(stdout);
        }

        thread->terminate();
        thread->wait();
        thread->quit();
    } else if (qEnvironmentVariableIsSet(DEEPIN_CLONE_OPEN_URL)) {
        QGuiApplication *app = new QGuiApplication(argc, argv);
        Q_UNUSED(app)
        QDesktopServices::openUrl(QUrl(QString::fromUtf8(qgetenv(DEEPIN_CLONE_OPEN_URL))));
    } else if (qEnvironmentVariableIsSet(DEEPIN_CLONE_SHOW_FILE)) {
        DTK_WIDGET_NAMESPACE::DDesktopServices::showFileItem(QString::fromUtf8(qgetenv(DEEPIN_CLONE_SHOW_FILE)));
    } else {
        return -1;
    }

    return 0;
}
