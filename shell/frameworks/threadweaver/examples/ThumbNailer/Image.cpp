/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QLocale>
#include <QtDebug>

#include <ThreadWeaver/Exception>
#include <ThreadWeaver/ThreadWeaver>

#include "Image.h"
#include "Model.h"

// const int Image::ThumbHeight = 75;
// const int Image::ThumbWidth = 120;
const int Image::ThumbHeight = 60;
const int Image::ThumbWidth = 80;
QReadWriteLock Image::Lock;
int Image::ProcessingOrder;

Image::Image(const QString inputFileName, const QString outputFileName, Model *model, int id)
    : m_inputFileName(inputFileName)
    , m_outputFileName(outputFileName)
    , m_model(model)
    , m_id(id)
{
}

Progress Image::progress() const
{
    return qMakePair(m_progress.loadRelaxed(), Step_NumberOfSteps);
}

QString Image::description() const
{
    QReadLocker l(&Lock);
    return m_description;
}

QString Image::details() const
{
    QReadLocker l(&Lock);
    return m_details;
}

QString Image::details2() const
{
    QReadLocker l(&Lock);
    return m_details2;
}

int Image::processingOrder() const
{
    return m_processingOrder.loadAcquire();
}

const QString Image::inputFileName() const
{
    return m_inputFileName;
}

const QString Image::outputFileName() const
{
    return m_outputFileName;
}

QImage Image::thumbNail() const
{
    QReadLocker r(&Lock);
    return m_thumbnail;
}

void Image::loadFile()
{
    m_processingOrder.storeRelease(ProcessingOrder++);
    QFile file(m_inputFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        error(Step_LoadFile, tr("Unable to load input file!"));
    }
    m_imageData = file.readAll();
    QFileInfo fi(file);
    QLocale locale;
    QString details2 = tr("%1kB").arg(locale.toString(fi.size() / 1024));
    {
        QWriteLocker l(&Lock);
        m_description = fi.fileName();
        m_details2 = details2;
    }
    announceProgress(Step_LoadFile);
}

void Image::loadImage()
{
    m_processingOrder.storeRelease(ProcessingOrder++);
    QBuffer in(&m_imageData);
    in.open(QIODevice::ReadOnly);
    QImageReader reader(&in);
    m_image = reader.read();
    m_imageData.clear();
    if (m_image.isNull()) {
        QWriteLocker l(&Lock);
        m_details = tr("%1!").arg(reader.errorString());
        error(Step_LoadImage, m_details);
    }
    QString details = tr("%1x%2 pixels").arg(m_image.width()).arg(m_image.height());
    {
        QWriteLocker l(&Lock);
        m_details = details;
    }
    announceProgress(Step_LoadImage);
}

void Image::computeThumbNail()
{
    m_processingOrder.storeRelease(ProcessingOrder++);
    QImage thumb = m_image.scaled(ThumbWidth, ThumbHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    if (thumb.isNull()) {
        error(Step_ComputeThumbNail, tr("Unable to compute thumbnail!"));
    }
    { // thumb is implicitly shared, no copy:
        QWriteLocker l(&Lock);
        m_thumbnail = thumb;
    }

    m_image = QImage();
    announceProgress(Step_ComputeThumbNail);
}

void Image::saveThumbNail()
{
    QImage thumb;
    {
        QReadLocker r(&Lock);
        thumb = m_thumbnail;
    }
    if (!thumb.save(m_outputFileName)) {
        error(Step_SaveThumbNail, tr("Unable to save output file!"));
    }
    announceProgress(Step_SaveThumbNail);
}

void Image::announceProgress(Steps step)
{
    m_progress.storeRelease(step);
    if (m_model) {
        m_model->progressChanged();
        m_model->elementChanged(m_id);
    }
}

void Image::error(Image::Steps step, const QString &message)
{
    m_failedStep.storeRelaxed(step);
    announceProgress(Step_Complete);
    throw ThreadWeaver::JobFailed(message);
}
