/* -*- C++ -*-
    This file is part of ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2014 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODEL_H
#define MODEL_H

#include <QAbstractListModel>
#include <QFileInfoList>
#include <QList>

#include <ThreadWeaver/ResourceRestrictionPolicy>

#include "Image.h"
#include "Progress.h"

class Model : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        Role_SortRole = Qt::UserRole,
        Role_ImageRole,
        Role_StepRole,
    };

    explicit Model(QObject *parent = nullptr);

    int fileLoaderCap() const;
    void setFileLoaderCap(int cap);

    int imageLoaderCap() const;
    void setImageLoaderCap(int cap);

    int computeThumbNailCap() const;
    void setComputeThumbNailCap(int cap);

    int saveThumbNailCap() const;
    void setSaveThumbNailCap(int cap);

    void clear();
    void prepareConversions(const QFileInfoList &filenames, const QString &outputDirectory);
    bool computeThumbNailsBlockingInLoop();
    bool computeThumbNailsBlockingConcurrent();

    void queueUpConversion(const QStringList &files, const QString &outputDirectory);
    Progress progress() const;
    void progressChanged();
    void elementChanged(int id);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void completed();
    void progressStepChanged(int, int);
    void signalElementChanged(int);

private Q_SLOTS:
    void slotElementChanged(int id);

private:
    QList<Image> m_images;
    ThreadWeaver::ResourceRestrictionPolicy m_fileLoaderRestriction;
    ThreadWeaver::ResourceRestrictionPolicy m_imageLoaderRestriction;
    ThreadWeaver::ResourceRestrictionPolicy m_imageScalerRestriction;
    ThreadWeaver::ResourceRestrictionPolicy m_fileWriterRestriction;
};

#endif // MODEL_H
