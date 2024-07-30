/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef IMAGE_H
#define IMAGE_H

#include <QAtomicInt>
#include <QCoreApplication>
#include <QImage>
#include <QReadWriteLock>

#include "Progress.h"

class Model;

/** @brief Image loads an image from a path, and then calculates and saves a thumbnail for it. */
class Image
{
    Q_DECLARE_TR_FUNCTIONS(Image)

public:
    enum Steps {
        Step_NotStarted,
        Step_LoadFile,
        Step_LoadImage,
        Step_ComputeThumbNail,
        Step_SaveThumbNail,
        Step_NumberOfSteps = Step_SaveThumbNail,
        Step_Complete = Step_SaveThumbNail,
    };

    Image(const QString inputFileName = QString(), const QString outputFileName = QString(), Model *model = nullptr, int id = 0);
    Progress progress() const;
    QString description() const;
    QString details() const;
    QString details2() const;
    int processingOrder() const;

    const QString inputFileName() const;
    const QString outputFileName() const;
    QImage thumbNail() const;

    void loadFile();
    void loadImage();
    void computeThumbNail();
    void saveThumbNail();

    static const int ThumbHeight;
    static const int ThumbWidth;

private:
    void announceProgress(Steps step);
    void error(Steps step, const QString &message);

    QString m_inputFileName;
    QString m_outputFileName;
    QString m_description;
    QString m_details;
    QString m_details2;
    QAtomicInt m_progress;
    QAtomicInt m_failedStep;
    QAtomicInt m_processingOrder;

    QByteArray m_imageData;
    QImage m_image;
    QImage m_thumbnail;
    Model *m_model;
    int m_id;

    static QReadWriteLock Lock;
    static int ProcessingOrder;
};

Q_DECLARE_METATYPE(Image)
Q_DECLARE_METATYPE(const Image *)

#endif // IMAGE_H
