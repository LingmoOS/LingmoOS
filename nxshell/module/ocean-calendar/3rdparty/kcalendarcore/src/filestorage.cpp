/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FileStorage class.

  @brief
  This class provides a calendar storage as a local file.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "filestorage.h"
#include "exceptions.h"
#include "icalformat.h"
#include "memorycalendar.h"
#include "vcalformat.h"

#include <QDebug>

using namespace KCalendarCore;

/*
  Private class that helps to provide binary compatibility between releases.
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::FileStorage::Private
{
public:
    Private(const QString &fileName, CalFormat *format)
        : mFileName(fileName)
        , mSaveFormat(format)
    {
    }
    ~Private()
    {
        delete mSaveFormat;
    }

    QString mFileName;
    CalFormat *mSaveFormat = nullptr;
};
//@endcond

FileStorage::FileStorage(const Calendar::Ptr &cal, const QString &fileName, CalFormat *format)
    : CalStorage(cal)
    , d(new Private(fileName, format))
{
}

FileStorage::~FileStorage()
{
    delete d;
}

void FileStorage::setFileName(const QString &fileName)
{
    d->mFileName = fileName;
}

QString FileStorage::fileName() const
{
    return d->mFileName;
}

void FileStorage::setSaveFormat(CalFormat *format)
{
    delete d->mSaveFormat;
    d->mSaveFormat = format;
}

CalFormat *FileStorage::saveFormat() const
{
    return d->mSaveFormat;
}

bool FileStorage::open()
{
    return true;
}

bool FileStorage::load()
{
    if (d->mFileName.isEmpty()) {
        qWarning() << "Empty filename while trying to load";
        return false;
    }

    // Always try to load with iCalendar. It will detect, if it is actually a
    // vCalendar file.
    bool success;
    QString productId;
    // First try the supplied format. Otherwise fall through to iCalendar, then
    // to vCalendar
    success = saveFormat() && saveFormat()->load(calendar(), d->mFileName);
    if (success) {
        productId = saveFormat()->loadedProductId();
    } else {
        ICalFormat iCal;

        success = iCal.load(calendar(), d->mFileName);

        if (success) {
            productId = iCal.loadedProductId();
        } else {
            if (iCal.exception()) {
                if ((iCal.exception()->code() == Exception::ParseErrorIcal) || (iCal.exception()->code() == Exception::CalVersion1)) {
                    // Possible vCalendar or invalid iCalendar encountered
                    qDebug() << d->mFileName << " is an invalid iCalendar or possibly a vCalendar.";
                    qDebug() << "Try to load it as a vCalendar";
                    VCalFormat vCal;
                    success = vCal.load(calendar(), d->mFileName);
                    productId = vCal.loadedProductId();
                    if (!success) {
                        if (vCal.exception()) {
                            qWarning() << d->mFileName << " is not a valid vCalendar file."
                                       << " exception code " << vCal.exception()->code();
                        }
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                qWarning() << "There should be an exception set.";
                return false;
            }
        }
    }

    calendar()->setProductId(productId);
    calendar()->setModified(false);

    return true;
}

bool FileStorage::save()
{
    if (d->mFileName.isEmpty()) {
        return false;
    }

    CalFormat *format = d->mSaveFormat ? d->mSaveFormat : new ICalFormat;

    bool success = format->save(calendar(), d->mFileName);

    if (success) {
        calendar()->setModified(false);
    } else {
        if (!format->exception()) {
            qDebug() << "Error. There should be an exception set.";
        } else {
            qDebug() << int(format->exception()->code());
        }
    }

    if (!d->mSaveFormat) {
        delete format;
    }

    return success;
}

bool FileStorage::close()
{
    return true;
}
