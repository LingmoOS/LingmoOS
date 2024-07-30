/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef MIMEDATAWRAPPER_H
#define MIMEDATAWRAPPER_H

#include <QJsonArray>
#include <QObject>
#include <QString>

class QMimeData;
class QObject;
/**
 * Exposes a const QMimeData instance
 *
 * In contrast to DeclarativeMimeData, doesn't create a copy of the QMimeData instance
 */
class MimeDataWrapper : public QObject
{
    Q_OBJECT

    /**
     * A plain text (MIME type text/plain) representation of the data.
     */
    Q_PROPERTY(QString text READ text CONSTANT)

    /**
     * A string if the data stored in the object is HTML (MIME type text/html); otherwise returns an empty string.
     */
    Q_PROPERTY(QString html READ html CONSTANT)

    /**
     * Url contained in the mimedata
     */
    Q_PROPERTY(QUrl url READ url CONSTANT)

    /**
     * A list of URLs contained within the MIME data object.
     * URLs correspond to the MIME type text/uri-list.
     */
    Q_PROPERTY(QJsonArray urls READ urls CONSTANT)

    /**
     * A color if the data stored in the object represents a color (MIME type application/x-color); otherwise QVariant().
     */
    Q_PROPERTY(QVariant color READ color CONSTANT)

    /**
     * The graphical item on the scene that started the drag event. It may be null.
     */
    Q_PROPERTY(QVariant source READ source CONSTANT)

    /**
     * Mimetypes provided by the mime data instance
     *
     * @sa QMimeData::formats
     */
    Q_PROPERTY(QStringList formats READ formats CONSTANT)

    /**
     * @sa QMimeData::hasUrls
     */
    Q_PROPERTY(bool hasUrls READ hasUrls CONSTANT)

    /**
     * @returns the wrapped object
     */
    Q_PROPERTY(QMimeData *mimeData READ mimeData CONSTANT)

public:
    MimeDataWrapper(const QMimeData *data, QObject *parent);

    QString text() const;
    QString html() const;
    QUrl url() const;
    QJsonArray urls() const;
    bool hasUrls() const;
    QVariant color() const;
    QStringList formats() const;
    QVariant source() const;
    QMimeData *mimeData() const;

    Q_INVOKABLE QByteArray getDataAsByteArray(const QString &format);

private:
    const QMimeData *m_data;
};

#endif
