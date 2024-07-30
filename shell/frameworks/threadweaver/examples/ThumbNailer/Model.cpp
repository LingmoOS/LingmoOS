/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <algorithm> //for transform
#include <numeric> //for accumulate

#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QStringList>
#include <QtDebug>

#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/Exception>
#include <ThreadWeaver/ThreadWeaver>

#include "ComputeThumbNailJob.h"
#include "FileLoaderJob.h"
#include "ImageLoaderJob.h"
#include "Model.h"
#include "PriorityDecorator.h"

using namespace std;
using namespace ThreadWeaver;

Model::Model(QObject *parent)
    : QAbstractListModel(parent)
    , m_fileLoaderRestriction(4)
    , m_imageLoaderRestriction(4)
    , m_imageScalerRestriction(4)
    , m_fileWriterRestriction(4)
{
    ThreadWeaver::setDebugLevel(true, 0);
    connect(this, SIGNAL(signalElementChanged(int)), this, SLOT(slotElementChanged(int)));
}

int Model::fileLoaderCap() const
{
    return m_fileLoaderRestriction.cap();
}

void Model::setFileLoaderCap(int cap)
{
    m_fileLoaderRestriction.setCap(cap);
    Queue::instance()->reschedule();
}

int Model::imageLoaderCap() const
{
    return m_imageLoaderRestriction.cap();
}

void Model::setImageLoaderCap(int cap)
{
    m_imageLoaderRestriction.setCap(cap);
    Queue::instance()->reschedule();
}

int Model::computeThumbNailCap() const
{
    return m_imageScalerRestriction.cap();
}

void Model::setComputeThumbNailCap(int cap)
{
    m_imageScalerRestriction.setCap(cap);
}

int Model::saveThumbNailCap() const
{
    return m_fileWriterRestriction.cap();
}

void Model::setSaveThumbNailCap(int cap)
{
    m_imageScalerRestriction.setCap(cap);
}

void Model::clear()
{
    beginResetModel();
    m_images.clear();
    endResetModel();
}

void Model::prepareConversions(const QFileInfoList &filenames, const QString &outputDirectory)
{
    beginResetModel();
    Q_ASSERT(m_images.isEmpty());
    m_images.reserve(filenames.size());
    int counter = 0;
    auto initializeImage = [this, outputDirectory, &counter](const QFileInfo &file) {
        auto const out = QFileInfo(outputDirectory, file.fileName()).absoluteFilePath();
        return Image(file.absoluteFilePath(), out, this, counter++);
    };
    for (const auto &filename : filenames) {
        m_images << initializeImage(filename);
    }
    endResetModel();
}

bool Model::computeThumbNailsBlockingInLoop()
{
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        Image &image = *it;
        try {
            image.loadFile();
            image.loadImage();
            image.computeThumbNail();
            image.saveThumbNail();

        } catch (const ThreadWeaver::Exception &ex) {
            qDebug() << ex.message();
            return false;
        }
    }
    return true;
}

bool Model::computeThumbNailsBlockingConcurrent()
{
    auto queue = stream();
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        Image &image = *it;
        auto sequence = new Sequence();
        *sequence << make_job([&image]() {
            image.loadFile();
        });
        *sequence << make_job([&image]() {
            image.loadImage();
        });
        *sequence << make_job([&image]() {
            image.computeThumbNail();
        });
        *sequence << make_job([&image]() {
            image.saveThumbNail();
        });
        queue << sequence;
    }
    queue.flush();
    Queue::instance()->finish();
    // figure out result:
    for (const Image &image : std::as_const(m_images)) {
        if (image.progress().first != Image::Step_NumberOfSteps) {
            return false;
        }
    }
    return true;
}

void Model::queueUpConversion(const QStringList &files, const QString &outputDirectory)
{
    QFileInfoList fileInfos;
    transform(files.begin(), files.end(), back_inserter(fileInfos), [](const QString &file) {
        return QFileInfo(file);
    });
    prepareConversions(fileInfos, outputDirectory);
    // FIXME duplicated code
    auto queue = stream();
    for (auto it = m_images.begin(); it != m_images.end(); ++it) {
        Image &image = *it;
        auto saveThumbNail = [&image]() {
            image.saveThumbNail();
        };
        auto saveThumbNailJob = new Lambda<decltype(saveThumbNail)>(saveThumbNail);
        {
            QMutexLocker l(saveThumbNailJob->mutex());
            saveThumbNailJob->assignQueuePolicy(&m_fileWriterRestriction);
        }

        auto sequence = new Sequence();
        /* clang-format off */
        *sequence << new FileLoaderJob(&image, &m_fileLoaderRestriction)
                  << new ImageLoaderJob(&image, &m_imageLoaderRestriction)
                  << new ComputeThumbNailJob(&image, &m_imageScalerRestriction)
                  << new PriorityDecorator(Image::Step_SaveThumbNail, saveThumbNailJob);
        /* clang-format on */
        queue << sequence;
    }
}

Progress Model::progress() const
{
    auto sumItUp = [](const Progress &sum, const Image &image) {
        auto const values = image.progress();
        return qMakePair(sum.first + values.first, sum.second + values.second);
    };
    auto const soFar = accumulate(m_images.begin(), m_images.end(), Progress(), sumItUp);
    return soFar;
}

void Model::progressChanged()
{
    auto const p = progress();
    Q_EMIT progressStepChanged(p.first, p.second);
}

void Model::elementChanged(int id)
{
    signalElementChanged(id);
}

int Model::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_images.size();
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }
    const Image &image = m_images.at(index.row());
    if (role == Qt::DisplayRole) {
        return image.description();
    } else if (role == Role_SortRole) {
        return -image.processingOrder();
    } else if (role == Role_ImageRole) {
        return QVariant::fromValue(&image);
    } else if (role == Role_StepRole) {
        return QVariant::fromValue(image.progress().first);
    }
    return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

void Model::slotElementChanged(int id)
{
    if (id >= 0 && id < m_images.count()) {
        auto const i = index(id, 0);
        Q_EMIT dataChanged(i, i);
    }
}

#include "moc_Model.cpp"
