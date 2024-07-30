/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Model.h"

class ImageListFilter;

namespace Ui
{
class MainWindow;
}

class AverageLoadManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *) override;

public Q_SLOTS:
    void slotProgress(int step, int total);

private Q_SLOTS:
    void slotOpenFiles();
    void slotSelectOutputDirectory();
    void slotFileLoaderCapChanged();
    void slotImageLoaderCapChanged();
    void slotComputeThumbNailCapChanged();
    void slotSaveThumbNailCapChanged();
    void slotWorkerCapChanged();
    void slotEnableAverageLoadManager(bool);
    void slotRecommendedWorkerCountChanged(int);
    void slotQuit();

private:
    Ui::MainWindow *ui;
    QString m_outputDirectory;
    Model m_model;
    ImageListFilter *m_fileLoaderFilter;
    ImageListFilter *m_imageLoaderFilter;
    ImageListFilter *m_imageScalerFilter;
    ImageListFilter *m_imageWriterFilter;
    AverageLoadManager *m_averageLoadManager;

    static const QString Setting_OpenLocation;
    static const QString Setting_OutputLocation;
    static const QString Setting_WindowState;
    static const QString Setting_WindowGeometry;
};

#endif // MAINWINDOW_H
