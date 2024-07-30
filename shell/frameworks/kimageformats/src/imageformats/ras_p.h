/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Dominik Seichter <domseichter@web.de>
    SPDX-FileCopyrightText: 2010 Troy Unrau <troy@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_RAS_P_H
#define KIMG_RAS_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class RASHandlerPrivate;
class RASHandler : public QImageIOHandler
{
public:
    RASHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<RASHandlerPrivate> d;
};

class RASPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ras.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_RAS_P_H
