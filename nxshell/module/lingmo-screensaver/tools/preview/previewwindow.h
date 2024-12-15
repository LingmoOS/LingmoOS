// SPDX-FileCopyrightText: 2017 ~ 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWWINDOW_H
#define PREVIEWWINDOW_H

#include <QWidget>
#include <QProcess>

namespace Ui {
class PreviewWindow;
}

class PreviewWindow : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWindow(const QString &screenSaverPath, QWidget *parent = 0);
    ~PreviewWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::PreviewWindow *ui;
    QProcess process;
};

#endif // PREVIEWWINDOW_H
