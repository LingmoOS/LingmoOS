/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVariant>

#include <ThreadWeaver/ThreadWeaver>

#include "AverageLoadManager.h"
#include "ImageListFilter.h"
#include "ItemDelegate.h"
#include "MainWindow.h"

#include "ui_MainWindow.h"

const QString MainWindow::Setting_OpenLocation = QLatin1String("OpenFilesLocation");
const QString MainWindow::Setting_OutputLocation = QLatin1String("OutputLocation");
const QString MainWindow::Setting_WindowState = QLatin1String("WindowState");
const QString MainWindow::Setting_WindowGeometry = QLatin1String("WindowGeometry");

using namespace ThreadWeaver;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_fileLoaderFilter(new ImageListFilter(Image::Step_LoadFile, this))
    , m_imageLoaderFilter(new ImageListFilter(Image::Step_LoadImage, this))
    , m_imageScalerFilter(new ImageListFilter(Image::Step_ComputeThumbNail, this))
    , m_imageWriterFilter(new ImageListFilter(Image::Step_SaveThumbNail, this))
    , m_averageLoadManager(new AverageLoadManager(this))
{
    ui->setupUi(this);
    // The file loader list:
    m_fileLoaderFilter->setSourceModel(&m_model);
    ui->fileLoaderList->setModel(m_fileLoaderFilter);
    ui->fileLoaderList->setItemDelegate(new ItemDelegate(this));
    ui->fileLoaderCap->setValue(m_model.fileLoaderCap());
    // The image loader list:
    m_imageLoaderFilter->setSourceModel(&m_model);
    ui->imageLoaderList->setModel(m_imageLoaderFilter);
    ui->imageLoaderList->setItemDelegate(new ItemDelegate(this));
    ui->imageLoaderCap->setValue(m_model.imageLoaderCap());

    // The image scaler list:
    m_imageScalerFilter->setSourceModel(&m_model);
    ui->imageScalerList->setModel(m_imageScalerFilter);
    ui->imageScalerList->setItemDelegate(new ItemDelegate(this));
    ui->imageScalerCap->setValue(m_model.computeThumbNailCap());

    // The image writer list:
    m_imageWriterFilter->setSourceModel(&m_model);
    ui->imageWriterList->setModel(m_imageWriterFilter);
    ui->imageWriterList->setItemDelegate(new ItemDelegate(this));
    ui->fileWriterCap->setValue(m_model.saveThumbNailCap());

    ui->workers->setValue(Queue::instance()->maximumNumberOfThreads());

    // Configure average load manager:
    ui->loadManager->setEnabled(m_averageLoadManager->available());
    connect(ui->loadManager, SIGNAL(toggled(bool)), SLOT(slotEnableAverageLoadManager(bool)));

    connect(ui->actionOpen_Files, SIGNAL(triggered()), SLOT(slotOpenFiles()));
    connect(ui->outputDirectory, SIGNAL(clicked()), SLOT(slotSelectOutputDirectory()));
    connect(ui->actionQuit, SIGNAL(triggered()), SLOT(slotQuit()));
    connect(&m_model, SIGNAL(progress(int, int)), SLOT(slotProgress(int, int)));
    connect(ui->fileLoaderCap, SIGNAL(valueChanged(int)), SLOT(slotFileLoaderCapChanged()));
    connect(ui->imageLoaderCap, SIGNAL(valueChanged(int)), SLOT(slotImageLoaderCapChanged()));
    connect(ui->imageScalerCap, SIGNAL(valueChanged(int)), SLOT(slotComputeThumbNailCapChanged()));
    connect(ui->fileWriterCap, SIGNAL(valueChanged(int)), SLOT(slotSaveThumbNailCapChanged()));

    connect(ui->workers, SIGNAL(valueChanged(int)), SLOT(slotWorkerCapChanged()));

    QSettings settings;
    m_outputDirectory = settings.value(Setting_OutputLocation).toString();
    if (!m_outputDirectory.isEmpty()) {
        ui->outputDirectory->setText(m_outputDirectory);
    }
    restoreGeometry(settings.value(Setting_WindowGeometry).toByteArray());
    restoreState(settings.value(Setting_WindowState).toByteArray());

    connect(m_averageLoadManager, SIGNAL(recommendedWorkerCount(int)), SLOT(slotRecommendedWorkerCountChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    QMainWindow::closeEvent(event);
    slotQuit();
}

void MainWindow::slotProgress(int step, int total)
{
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(total);
    ui->progressBar->setValue(step);
    ui->actionOpen_Files->setEnabled(step >= total);
}

void MainWindow::slotOpenFiles()
{
    QSettings settings;
    const QString previousLocation = settings.value(Setting_OpenLocation, QDir::homePath()).toString();
    auto const files = QFileDialog::getOpenFileNames(this, tr("Select images to convert"), previousLocation);
    if (files.isEmpty()) {
        return;
    }
    if (m_outputDirectory.isNull()) {
        slotSelectOutputDirectory();
    }
    m_model.clear();
    const QFileInfo fi(files.at(0));
    settings.setValue(Setting_OpenLocation, fi.absolutePath());
    slotProgress(0, files.count());
    m_model.queueUpConversion(files, m_outputDirectory);
}

void MainWindow::slotSelectOutputDirectory()
{
    QSettings settings;
    const QString previousLocation = settings.value(Setting_OutputLocation, QDir::homePath()).toString();
    auto directory = QFileDialog::getExistingDirectory(this, tr("Select output directory..."));
    if (directory.isNull()) {
        return;
    }
    m_outputDirectory = directory;
    settings.setValue(Setting_OutputLocation, directory);
    ui->outputDirectory->setText(directory);
}

void MainWindow::slotFileLoaderCapChanged()
{
    const int value = ui->fileLoaderCap->value();
    Q_ASSERT(value > 0); // limits set in UI file
    m_model.setFileLoaderCap(value);
}

void MainWindow::slotImageLoaderCapChanged()
{
    const int value = ui->imageLoaderCap->value();
    Q_ASSERT(value > 0); // limits set in UI file
    m_model.setImageLoaderCap(value);
}

void MainWindow::slotComputeThumbNailCapChanged()
{
    const int value = ui->imageScalerCap->value();
    Q_ASSERT(value > 0); // limits set in UI file
    m_model.setComputeThumbNailCap(value);
}

void MainWindow::slotSaveThumbNailCapChanged()
{
    const int value = ui->fileWriterCap->value();
    Q_ASSERT(value > 0); // limits set in UI file
    m_model.setSaveThumbNailCap(value);
}

void MainWindow::slotWorkerCapChanged()
{
    const int value = ui->workers->value();
    Q_ASSERT(value >= 0); // limits set in UI file
    Queue::instance()->setMaximumNumberOfThreads(value);
}

void MainWindow::slotEnableAverageLoadManager(bool enabled)
{
    m_averageLoadManager->activate(enabled);
}

void MainWindow::slotRecommendedWorkerCountChanged(int value)
{
    auto const minMax = m_averageLoadManager->workersRange();
    ui->workersSlider->setRange(minMax.first, minMax.second);
    ui->workersSlider->setValue(value);
    ui->loadManager->setText(tr("%1 workers").arg(value));
    ui->workersMin->setText(QString::number(minMax.first));
    ui->workersMax->setText(QString::number(minMax.second));
    Queue::instance()->setMaximumNumberOfThreads(value);
}

void MainWindow::slotQuit()
{
    QSettings settings;
    settings.setValue(Setting_WindowGeometry, saveGeometry());
    settings.setValue(Setting_WindowState, saveState());
    ThreadWeaver::Queue::instance()->dequeue();
    ThreadWeaver::Queue::instance()->finish();
    QApplication::instance()->quit();
}

#include "moc_MainWindow.cpp"
