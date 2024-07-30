/*
    SPDX-FileCopyrightText: 2010-2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KIPPREQUEST_H
#define KIPPREQUEST_H

#include <KCupsConnection.h>
#include <kcupslib_export.h>

class KIppRequestPrivate;
class KCUPSLIB_EXPORT KIppRequest
{
    Q_DECLARE_PRIVATE(KIppRequest)
public:
    KIppRequest();
    KIppRequest(const KIppRequest &other);
    KIppRequest(ipp_op_t operation, const QString &resource, const QString &filename = QString());
    ~KIppRequest();

    ipp_op_t operation() const;
    QString resource() const;
    QString filename() const;

    ipp_t *sendIppRequest() const;

    void addString(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QString &value);
    void addStringList(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QStringList &value);
    void addInteger(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, int value);
    void addBoolean(ipp_tag_t group, const QString &name, bool value);
    void addVariantValues(const QVariantMap &values);
    void addPrinterUri(const QString &printerName, bool isClass = false);

    static QString assembleUrif(const QString &name, bool isClass);

    /**
     * Makes a copy of the KIppRequest object other.
     */
    KIppRequest &operator=(const KIppRequest &other);

private:
    KIppRequestPrivate *d_ptr;
};

Q_DECLARE_METATYPE(KIppRequest)

#endif // KIPPREQUEST_H
