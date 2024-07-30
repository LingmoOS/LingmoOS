/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_JXR_P_H
#define KIMG_JXR_P_H

#include <QImageIOPlugin>
#include <QSharedDataPointer>

class JXRHandlerPrivate;

class JXRHandler : public QImageIOHandler
{
public:
    JXRHandler();

    bool canRead() const override;
    bool read(QImage *outImage) override;
    bool write(const QImage &image) override;

    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    mutable QSharedDataPointer<JXRHandlerPrivate> d;

    qint32 m_quality;
};

class JXRPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "jxr.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_JXR_P_H
