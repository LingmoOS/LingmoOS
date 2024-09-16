// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "previewwindow.h"
#include "ui_previewwindow.h"

#include <QStringListModel>
#include <QDir>
#include <QProcess>
#include <QScreen>
#include <QWindow>
#include <QDebug>

PreviewWindow::PreviewWindow(const QString &screenSaverPath, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PreviewWindow)
{
    ui->setupUi(this);
    ui->preview->setAttribute(Qt::WA_NativeWindow);

    QStringListModel *model = new QStringListModel(ui->listView);

    model->setStringList(QDir(screenSaverPath).entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Executable));
    ui->listView->setModel(model);

    connect(ui->listView, &QListView::clicked, this, [this, screenSaverPath] (const QModelIndex &index) {
        if (process.state() != QProcess::NotRunning) {
            process.terminate();
            process.waitForFinished();
        }

        process.start(screenSaverPath + "/" + index.data().toString(), {"-install", "-window-id", QString::number(ui->preview->winId())}, QIODevice::ReadOnly);
    });
}

PreviewWindow::~PreviewWindow()
{
    process.terminate();
    process.waitForFinished();

    delete ui;
}

void PreviewWindow::on_pushButton_clicked()
{
    if (process.state() == QProcess::NotRunning)
        return;

    const QPixmap &pix = ui->preview->window()->windowHandle()->screen()->grabWindow(ui->preview->winId());
    QDir image_save_dir(QDir::home().absoluteFilePath("deepin-screensaver"));

    image_save_dir.mkpath("preview");

    const QString &name = ui->listView->currentIndex().data().toString();

    QFile::copy(process.program(), image_save_dir.absoluteFilePath(name));
    pix.save(image_save_dir.absoluteFilePath(QString("preview/%1@3x.png").arg(name)));
    pix.scaledToHeight(200, Qt::SmoothTransformation).save(image_save_dir.absoluteFilePath(QString("preview/%1@2x.png").arg(name)));
    pix.scaledToHeight(100, Qt::SmoothTransformation).save(image_save_dir.absoluteFilePath(QString("preview/%1.png").arg(name)));
}
