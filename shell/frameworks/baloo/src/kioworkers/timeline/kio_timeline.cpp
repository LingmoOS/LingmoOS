/*
    SPDX-FileCopyrightText: 2009-2010 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2018-2020 Stefan Brüns <bruns@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kio_timeline.h"
#include "timelinetools.h"
#include "query.h"
#include "resultiterator.h"
#include "../common/udstools.h"
#include <sys/stat.h>

#include <QCoreApplication>

#include <KUser>
#include <KFormat>

#include <KLocalizedString>

using namespace Baloo;

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.worker.timeline" FILE "timeline.json")
};

namespace
{
KIO::UDSEntry createFolderUDSEntry(const QString& name)
{
    KIO::UDSEntry uds;
    uds.reserve(5);
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, name);
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
#ifdef Q_OS_WIN
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, _S_IREAD );
#else
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR);
#endif
    uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    return uds;
}

KIO::UDSEntry createDateFolderUDSEntry(const QString& name, const QString& displayName, const QDate& date)
{
    KIO::UDSEntry uds;
    uds.reserve(8);
    QDateTime dt(date, QTime(0, 0, 0));
    uds.fastInsert(KIO::UDSEntry::UDS_NAME, name);
    uds.fastInsert(KIO::UDSEntry::UDS_DISPLAY_NAME, displayName);
    uds.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR);
    uds.fastInsert(KIO::UDSEntry::UDS_MIME_TYPE, QStringLiteral("inode/directory"));
    uds.fastInsert(KIO::UDSEntry::UDS_MODIFICATION_TIME, dt.toSecsSinceEpoch());
    uds.fastInsert(KIO::UDSEntry::UDS_CREATION_TIME, dt.toSecsSinceEpoch());
#ifdef Q_OS_WIN
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, _S_IREAD );
#else
    uds.fastInsert(KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IXUSR);
#endif
    uds.fastInsert(KIO::UDSEntry::UDS_USER, KUser().loginName());
    return uds;
}

KIO::UDSEntry createMonthUDSEntry(int month, int year)
{
    QString dateString = QDate(year, month, 1).toString(
                i18nc("Month and year used in a tree above the actual days. "
                      "Have a look at https://doc.qt.io/qt-5/qdate.html#toString "
                      "to see which variables you can use and ask kde-i18n-doc@kde.org if you have "
                      "problems understanding how to translate this",
                      "MMMM yyyy"));
    return createDateFolderUDSEntry(QDate(year, month, 1).toString(QStringLiteral("yyyy-MM")),
                                dateString,
                                QDate(year, month, 1));
}

KIO::UDSEntry createDayUDSEntry(const QDate& date)
{
    KIO::UDSEntry uds = createDateFolderUDSEntry(date.toString(QStringLiteral("yyyy-MM-dd")),
                        KFormat().formatRelativeDate(date, QLocale::LongFormat),
                        date);

    return uds;
}

}

TimelineProtocol::TimelineProtocol(const QByteArray& poolSocket, const QByteArray& appSocket)
    : KIO::WorkerBase("timeline", poolSocket, appSocket)
{
}

TimelineProtocol::~TimelineProtocol()
{
}

KIO::WorkerResult TimelineProtocol::listDir(const QUrl& url)
{
    QUrl canonicalUrl = canonicalizeTimelineUrl(url);
    if (url != canonicalUrl) {
        redirection(canonicalUrl);
        return KIO::WorkerResult::pass();
    }

    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder:
        listEntry(createFolderUDSEntry(QStringLiteral(".")));
        listEntry(createDateFolderUDSEntry(QStringLiteral("today"), i18n("Today"), QDate::currentDate()));
        listEntry(createDateFolderUDSEntry(QStringLiteral("calendar"), i18n("Calendar"), QDate::currentDate()));
        break;

    case CalendarFolder:
        listEntry(createFolderUDSEntry(QStringLiteral(".")));
        listThisYearsMonths();
        // TODO: add entry for previous years
        break;

    case MonthFolder:
        listEntry(createFolderUDSEntry(QStringLiteral(".")));
        listDays(m_date.month(), m_date.year());
        break;

    case DayFolder: {
        listEntry(createFolderUDSEntry(QStringLiteral(".")));
        Query query;
        query.setDateFilter(m_date.year(), m_date.month(), m_date.day());
        query.setSortingOption(Query::SortNone);

        UdsFactory udsf;

        ResultIterator it = query.exec();
        while (it.next()) {
            KIO::UDSEntry uds = udsf.createUdsEntry(it.filePath());
            if (uds.count()) {
                listEntry(uds);
            }
        }
        break;
    }

    case NoFolder:
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toString());
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TimelineProtocol::mimetype(const QUrl& url)
{
    QUrl canonicalUrl = canonicalizeTimelineUrl(url);
    if (url != canonicalUrl) {
        redirection(canonicalUrl);
        return KIO::WorkerResult::pass();
    }

    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder:
    case CalendarFolder:
    case MonthFolder:
    case DayFolder:
        mimetype(QUrl(QLatin1String("inode/directory")));
        break;

    case NoFolder:
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toString());
    }

    return KIO::WorkerResult::pass();
}

KIO::WorkerResult TimelineProtocol::stat(const QUrl& url)
{
    QUrl canonicalUrl = canonicalizeTimelineUrl(url);
    if (url != canonicalUrl) {
        redirection(canonicalUrl);
        return KIO::WorkerResult::pass();
    }

    switch (parseTimelineUrl(url, &m_date, &m_filename)) {
    case RootFolder: {
        statEntry(createFolderUDSEntry(QStringLiteral("/")));
        break;
    }

    case CalendarFolder:
        statEntry(createDateFolderUDSEntry(QStringLiteral("calendar"), i18n("Calendar"), QDate::currentDate()));
        break;

    case MonthFolder:
        statEntry(createMonthUDSEntry(m_date.month(), m_date.year()));
        break;

    case DayFolder:
        if (m_filename.isEmpty()) {
            statEntry(createDayUDSEntry(m_date));
        }
        break;

    case NoFolder:
        return KIO::WorkerResult::fail(KIO::ERR_DOES_NOT_EXIST, url.toString());
    }

    return KIO::WorkerResult::pass();
}

void TimelineProtocol::listDays(int month, int year)
{
    const int days = QDate(year, month, 1).daysInMonth();
    for (int day = 1; day <= days; ++day) {
        QDate date(year, month, day);

        if (date <= QDate::currentDate() && filesInDate(date)) {
            listEntry(createDayUDSEntry(date));
        }
    }
}

bool TimelineProtocol::filesInDate(const QDate& date)
{
    Query query;
    query.setLimit(1);
    query.setDateFilter(date.year(), date.month(), date.day());
    query.setSortingOption(Query::SortNone);

    ResultIterator it = query.exec();
    return it.next();
}

void TimelineProtocol::listThisYearsMonths()
{
    Query query;
    query.setLimit(1);
    query.setSortingOption(Query::SortNone);

    int year = QDate::currentDate().year();
    int currentMonth = QDate::currentDate().month();
    for (int month = 1; month <= currentMonth; ++month) {
        query.setDateFilter(year, month);
        ResultIterator it = query.exec();
        if (it.next()) {
            listEntry(createMonthUDSEntry(month, year));
        }
    }
}

extern "C"
{
    Q_DECL_EXPORT int kdemain(int argc, char** argv)
    {
        QCoreApplication app(argc, argv);
        app.setApplicationName(QStringLiteral("kio_timeline"));
        Baloo::TimelineProtocol worker(argv[2], argv[3]);
        worker.dispatchLoop();
        return 0;
    }
}

#include "kio_timeline.moc"
