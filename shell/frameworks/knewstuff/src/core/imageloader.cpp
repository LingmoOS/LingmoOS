/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2006, 2007 Josef Spillner <spillner@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "imageloader_p.h"

using namespace KNSCore;

ImageLoader::ImageLoader(const Entry &entry, Entry::PreviewType type, QObject *parent)
    : QObject(parent)
    , m_entry(entry)
    , m_previewType(type)
{
}

void ImageLoader::start()
{
    QUrl url(m_entry.previewUrl(m_previewType));
    if (!url.isEmpty()) {
        m_job = HTTPJob::get(url, NoReload, JobFlag::HideProgressInfo, this);
        connect(m_job, &KJob::result, this, &ImageLoader::slotDownload);
        connect(m_job, &HTTPJob::data, this, &ImageLoader::slotData);
    } else {
        Q_EMIT signalError(m_entry, m_previewType, QStringLiteral("Empty url"));
        deleteLater();
    }
}

KJob *ImageLoader::job()
{
    return m_job;
}

void ImageLoader::slotData(KJob * /*job*/, const QByteArray &buf)
{
    m_buffer.append(buf);
}

void ImageLoader::slotDownload(KJob *job)
{
    if (job->error()) {
        m_buffer.clear();
        Q_EMIT signalError(m_entry, m_previewType, job->errorText());
        deleteLater();
        return;
    }
    QImage image;
    image.loadFromData(std::move(m_buffer));

    if (m_previewType == Entry::PreviewSmall1 || m_previewType == Entry::PreviewSmall2 || m_previewType == Entry::PreviewSmall3) {
        if (image.width() > PreviewWidth || image.height() > PreviewHeight) {
            // if the preview is really big, first scale fast to a smaller size, then smooth to desired size
            if (image.width() > 4 * PreviewWidth || image.height() > 4 * PreviewHeight) {
                image = image.scaled(2 * PreviewWidth, 2 * PreviewHeight, Qt::KeepAspectRatio, Qt::FastTransformation);
            }
            image = image.scaled(PreviewWidth, PreviewHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else if (image.width() <= PreviewWidth / 2 && image.height() <= PreviewHeight / 2) {
            // upscale tiny previews to double size
            image = image.scaled(2 * image.width(), 2 * image.height());
        }
    }

    m_entry.setPreviewImage(image, m_previewType);
    Q_EMIT signalPreviewLoaded(m_entry, m_previewType);
    deleteLater();
}

#include "moc_imageloader_p.cpp"
