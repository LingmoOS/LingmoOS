/*
    SPDX-FileCopyrightText: 2010-2018 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KIppRequest.h"
#include "KIppRequest_p.h"

#include "kcupslib_log.h"

KIppRequest::KIppRequest()
    : d_ptr(new KIppRequestPrivate)
{
}

KIppRequest::KIppRequest(const KIppRequest &other)
    : d_ptr(new KIppRequestPrivate)
{
    *this = other;
}

KIppRequest::KIppRequest(ipp_op_t operation, const QString &resource, const QString &filename)
    : d_ptr(new KIppRequestPrivate)
{
    Q_D(KIppRequest);

    d->operation = operation;
    d->resource = resource;
    d->filename = filename;

    // send our user name on the request too
    addString(IPP_TAG_OPERATION, IPP_TAG_NAME, QLatin1String(KCUPS_REQUESTING_USER_NAME), QString::fromUtf8(cupsUser()));
}

KIppRequest::~KIppRequest()
{
    delete d_ptr;
}

ipp_op_t KIppRequest::operation() const
{
    Q_D(const KIppRequest);
    return d->operation;
}

QString KIppRequest::resource() const
{
    Q_D(const KIppRequest);
    return d->resource;
}

QString KIppRequest::filename() const
{
    Q_D(const KIppRequest);
    return d->filename;
}

ipp_t *KIppRequest::sendIppRequest() const
{
    Q_D(const KIppRequest);

    ipp_t *request = ippNewRequest(d->operation);

    d->addRawRequestsToIpp(request);

    if (d->filename.isNull()) {
        return cupsDoRequest(CUPS_HTTP_DEFAULT, request, qUtf8Printable(d->resource));
    } else {
        return cupsDoFileRequest(CUPS_HTTP_DEFAULT, request, qUtf8Printable(d->resource), qUtf8Printable(d->filename));
    }
}

void KIppRequest::addString(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QString &value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name, value);
}

void KIppRequest::addStringList(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QStringList &value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name, value);
}

void KIppRequest::addInteger(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, int value)
{
    Q_D(KIppRequest);

    d->addRequest(group, valueTag, name, value);
}

void KIppRequest::addBoolean(ipp_tag_t group, const QString &name, bool value)
{
    Q_D(KIppRequest);

    d->addRequest(group, IPP_TAG_ZERO, name, value);
}

void KIppRequest::addVariantValues(const QVariantMap &values)
{
    auto i = values.constBegin();
    while (i != values.constEnd()) {
        const QString &key = i.key();
        const QVariant &value = i.value();
        switch (value.type()) {
        case QVariant::Bool:
            // Still in use at add-printer/PageAddPrinter.cpp
            if (key == QLatin1String(KCUPS_PRINTER_IS_ACCEPTING_JOBS)) {
                addBoolean(IPP_TAG_PRINTER, key, value.toBool());
            } else {
                addBoolean(IPP_TAG_OPERATION, key, value.toBool());
            }
            break;
        case QVariant::Int:
            // Still in use at add-printer/PageAddPrinter.cpp
            if (key == QLatin1String(KCUPS_PRINTER_STATE)) {
                addInteger(IPP_TAG_PRINTER, IPP_TAG_ENUM, key, value.toInt());
            } else {
                addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, key, value.toInt());
            }
            break;
        case QVariant::String:
            // Still in use at add-printer/*
            if (key == QLatin1String(KCUPS_DEVICE_URI)) {
                // device uri has a different TAG
                addString(IPP_TAG_PRINTER, IPP_TAG_URI, key, value.toString());
            } else if (key == QLatin1String(KCUPS_PRINTER_OP_POLICY) || key == QLatin1String(KCUPS_PRINTER_ERROR_POLICY) || key == QLatin1String("ppd-name")) {
                // printer-op-policy has a different TAG
                addString(IPP_TAG_PRINTER, IPP_TAG_NAME, key, value.toString());
            } else if (key == QLatin1String(KCUPS_JOB_NAME)) {
                addString(IPP_TAG_OPERATION, IPP_TAG_NAME, key, value.toString());
            } else if (key == QLatin1String(KCUPS_WHICH_JOBS)) {
                addString(IPP_TAG_OPERATION, IPP_TAG_KEYWORD, key, value.toString());
            } else {
                addString(IPP_TAG_PRINTER, IPP_TAG_TEXT, key, value.toString());
            }
            break;
        case QVariant::StringList:
            if (key == QLatin1String(KCUPS_MEMBER_URIS)) {
                addStringList(IPP_TAG_PRINTER, IPP_TAG_URI, key, value.toStringList());
            } else {
                addStringList(IPP_TAG_PRINTER, IPP_TAG_NAME, key, value.toStringList());
            }
            break;
        case QVariant::UInt:
            addInteger(IPP_TAG_OPERATION, IPP_TAG_ENUM, key, value.toInt());
            break;
        default:
            qCWarning(LIBKCUPS) << "type NOT recognized! This will be ignored:" << key << "values" << i.value();
        }
        ++i;
    }
}

void KIppRequest::addPrinterUri(const QString &printerName, bool isClass)
{
    QString uri = assembleUrif(printerName, isClass);
    addString(IPP_TAG_OPERATION, IPP_TAG_URI, QLatin1String(KCUPS_PRINTER_URI), uri);
}

QString KIppRequest::assembleUrif(const QString &name, bool isClass)
{
    char uri[HTTP_MAX_URI]; // printer URI

    QString destination;
    if (isClass) {
        destination = QLatin1String("/classes/") + name;
    } else {
        destination = QLatin1String("/printers/") + name;
    }

    httpAssembleURI(HTTP_URI_CODING_ALL, uri, sizeof(uri), "ipp", cupsUser(), "localhost", ippPort(), destination.toUtf8().constData());
    return QString::fromLatin1(uri);
}

KIppRequest &KIppRequest::operator=(const KIppRequest &other)
{
    Q_D(KIppRequest);
    if (this == &other)
        return *this;

    *d = *other.d_ptr;

    return *this;
}

void KIppRequestPrivate::addRequest(ipp_tag_t group, ipp_tag_t valueTag, const QString &name, const QVariant &value)
{
    KCupsRawRequest request;
    request.group = group;
    request.valueTag = valueTag;
    request.name = name;
    request.value = value;

    rawRequests << request;
}

void KIppRequestPrivate::addRawRequestsToIpp(ipp_t *ipp) const
{
    // sort the values as CUPS requires it
    std::sort(rawRequests.begin(), rawRequests.end(), [](const KCupsRawRequest &a, const KCupsRawRequest &b) {
        return a.group < b.group;
    });

    const QList<KCupsRawRequest> &requests = rawRequests;
    for (const KCupsRawRequest &request : requests) {
        switch (request.value.type()) {
        case QVariant::Bool:
            ippAddBoolean(ipp, request.group, request.name.toUtf8().constData(), request.value.toBool());
            break;
        case QVariant::Int:
        case QVariant::UInt:
            ippAddInteger(ipp, request.group, request.valueTag, request.name.toUtf8().constData(), request.value.toInt());
            break;
        case QVariant::String:
            ippAddString(ipp, request.group, request.valueTag, request.name.toUtf8().constData(), "utf-8", request.value.toString().toUtf8().constData());
            break;
        case QVariant::StringList: {
            QStringList list = request.value.toStringList();
            QList<QByteArray> valuesQByteArrayList;
            const char **values = qStringListToCharPtrPtr(list, valuesQByteArrayList);

            ippAddStrings(ipp, request.group, request.valueTag, request.name.toUtf8().constData(), list.size(), "utf-8", values);

            // ippAddStrings deep copies everything so we can throw away the values.
            // the QBAList and content is auto discarded when going out of scope.
            delete[] values;
            break;
        }
        default:
            qCWarning(LIBKCUPS) << "type NOT recognized! This will be ignored:" << request.name << "values" << request.value;
        }
    }
}
