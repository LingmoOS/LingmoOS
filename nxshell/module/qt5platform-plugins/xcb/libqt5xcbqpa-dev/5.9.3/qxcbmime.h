// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBMIME_H
#define QXCBMIME_H

#include <private/qdnd_p.h>

#include <QtGui/QClipboard>

#include "qxcbintegration.h"
#include "qxcbconnection.h"

QT_BEGIN_NAMESPACE

#if !(defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

class QXcbMime : public QInternalMimeData {
    Q_OBJECT
public:
    QXcbMime();
    ~QXcbMime();

    static QVector<xcb_atom_t> mimeAtomsForFormat(QXcbConnection *connection, const QString &format);
    static QString mimeAtomToString(QXcbConnection *connection, xcb_atom_t a);
    static bool mimeDataForAtom(QXcbConnection *connection, xcb_atom_t a, QMimeData *mimeData, QByteArray *data,
                                xcb_atom_t *atomFormat, int *dataFormat);
    static QVariant mimeConvertToFormat(QXcbConnection *connection, xcb_atom_t a, const QByteArray &data, const QString &format,
                                        QVariant::Type requestedType, const QByteArray &encoding);
    static xcb_atom_t mimeAtomForFormat(QXcbConnection *connection, const QString &format, QVariant::Type requestedType,
                                        const QVector<xcb_atom_t> &atoms, QByteArray *requestedEncoding);
};

#endif // !(defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))


QT_END_NAMESPACE

#endif // QXCBMIME_H
