/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1999-2006 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kioclient.h"
#include "kio_version.h"
#include "urlinfo.h"

#include <KIO/CopyJob>
#include <KIO/DeleteJob>
#include <kio/listjob.h>
#include <kio/transferjob.h>
#ifndef KIOCORE_ONLY
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KIO/StatJob>
#include <KIO/UDSEntry>
#include <KPropertiesDialog>
#include <KService>
#endif
#include <KAboutData>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDebug>
#include <QFileDialog>
#include <QUrlQuery>
#include <iostream>

bool ClientApp::m_ok = true;
static bool s_interactive = false;
static KIO::JobFlags s_jobFlags = KIO::DefaultFlags;

static QUrl makeURL(const QString &urlArg)
{
    return QUrl::fromUserInput(urlArg, QDir::currentPath());
}

static QList<QUrl> makeUrls(const QStringList &urlArgs)
{
    QList<QUrl> ret;
    for (const QString &url : urlArgs) {
        ret += makeURL(url);
    }
    return ret;
}

#ifdef KIOCLIENT_AS_KIOCLIENT
static void usage()
{
    puts(i18n("\nSyntax:\n").toLocal8Bit().constData());
    puts(i18nc("The argument is the command \"kioclient openProperties\"",
               "  %1 'url'\n"
               "            # Opens a properties dialog of 'url'\n\n",
               "kioclient openProperties")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient exec\"",
               "  %1 'url' ['mimetype']\n"
               "            # Tries to open the document pointed to by 'url', in the application\n"
               "            # associated with it in KDE. You may omit 'mimetype'.\n"
               "            # In that case the mimetype is determined automatically.\n"
               "            # 'url' can be the URL of a document, a *.desktop file,\n"
               "            # or an executable.\n",
               "kioclient exec")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient move\"",
               "  %1 'src' 'dest'\n"
               "            # Moves the URL 'src' to 'dest'.\n"
               "            #   'src' may be a list of URLs.\n"
               "            #   'dest' may be \"trash:/\" to move the files to the trash.\n"
               "            #   The short version 'kioclient mv' is also available.\n\n",
               "kioclient move")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient download\"",
               "  %1 ['src']\n"
               "            # Copies the URL 'src' to a user-specified location.\n"
               "            #   'src' may be a list of URLs, if not present then\n"
               "            #   a URL will be requested.\n\n",
               "kioclient download")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient copy\"",
               "  %1 'src' 'dest'\n"
               "            # Copies the URL 'src' to 'dest'.\n"
               "            #   'src' may be a list of URLs.\n"
               "            #   The short version 'kioclient cp' is also available.\n\n",
               "kioclient copy")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient cat\"",
               "  %1 'url'\n"
               "            # Prints the contents of the file 'url' to the standard output\n\n",
               "kioclient cat")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient ls\"",
               "  %1 'url'\n"
               "            # Lists the contents of the directory 'url' to stdout\n\n",
               "kioclient ls")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient remove\"",
               "  %1 'url'\n"
               "            # Removes the URL\n"
               "            #   'url' may be a list of URLs.\n"
               "            #   The short version 'kioclient rm' is also available.\n\n",
               "kioclient remove")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient stat\"",
               "  %1 'url'\n"
               "            # Shows all of the available information for 'url'\n\n",
               "kioclient stat")
             .toLocal8Bit()
             .constData());

    puts(i18nc("The argument is the command \"kioclient appmenu\"",
               "  %1\n"
               "            # Opens a basic application launcher\n\n",
               "kioclient appmenu")
             .toLocal8Bit()
             .constData());

    puts(i18n("*** Examples:\n").toLocal8Bit().constData());
    puts(i18n("  kioclient exec file:/home/weis/data/test.html\n"
              "             // Opens the file with the default application associated\n"
              "             // with this MimeType\n\n")
             .toLocal8Bit()
             .constData());
    puts(i18n("  kioclient exec ftp://localhost/\n"
              "             // Opens URL with the default handler for ftp:// scheme\n\n")
             .toLocal8Bit()
             .constData());
    puts(i18n("  kioclient exec file:/root/Desktop/emacs.desktop\n"
              "             // Starts emacs\n\n")
             .toLocal8Bit()
             .constData());
    puts(i18n("  kioclient exec .\n"
              "             // Opens the current directory in the default\n"
              "             // file manager. Very convenient.\n\n")
             .toLocal8Bit()
             .constData());
}
#endif

int main(int argc, char **argv)
{
#ifdef KIOCORE_ONLY
    QCoreApplication app(argc, argv);
#else
    QApplication app(argc, argv);
#endif

    KLocalizedString::setApplicationDomain(QByteArrayLiteral("kioclient"));

    QString appName = QStringLiteral("kioclient");
    QString programName = i18n("KIO Client");
    QString description = i18n("Command-line tool for network-transparent operations");
    QString version = QLatin1String(PROJECT_VERSION);
    KAboutData data(appName, programName, version, description, KAboutLicense::LGPL_V2);
    KAboutData::setApplicationData(data);

    QCommandLineParser parser;
    data.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringLiteral("interactive"), i18n("Use message boxes and other native notifications")));

    parser.addOption(QCommandLineOption(QStringLiteral("noninteractive"),
                                        i18n("Non-interactive use: no message boxes. If you don't want a "
                                             "graphical connection, use --platform offscreen")));

#if !defined(KIOCLIENT_AS_KDEOPEN)
    parser.addOption(QCommandLineOption(QStringLiteral("overwrite"), i18n("Overwrite destination if it exists (for copy and move)")));
#endif

#if defined(KIOCLIENT_AS_KDEOPEN)
    parser.addPositionalArgument(QStringLiteral("url"), i18n("file or URL"), i18n("urls..."));
#elif defined(KIOCLIENT_AS_KDECP)
    parser.addPositionalArgument(QStringLiteral("src"), i18n("Source URL or URLs"), i18n("urls..."));
    parser.addPositionalArgument(QStringLiteral("dest"), i18n("Destination URL"), i18n("url"));
#elif defined(KIOCLIENT_AS_KDEMV)
    parser.addPositionalArgument(QStringLiteral("src"), i18n("Source URL or URLs"), i18n("urls..."));
    parser.addPositionalArgument(QStringLiteral("dest"), i18n("Destination URL"), i18n("url"));
#elif defined(KIOCLIENT_AS_KIOCLIENT)
    parser.addOption(QCommandLineOption(QStringLiteral("commands"), i18n("Show available commands")));
    parser.addPositionalArgument(QStringLiteral("command"), i18n("Command (see --commands)"), i18n("command"));
    parser.addPositionalArgument(QStringLiteral("URLs"), i18n("Arguments for command"), i18n("urls..."));
#endif

    //   KCmdLineArgs::addTempFileOption();

    parser.process(app);
    data.processCommandLine(&parser);

#ifdef KIOCLIENT_AS_KIOCLIENT
    if (argc == 1 || parser.isSet(QStringLiteral("commands"))) {
        puts(parser.helpText().toLocal8Bit().constData());
        puts("\n\n");
        usage();
        return 0;
    }
#endif

    ClientApp client;
    return client.doIt(parser) ? 0 /*no error*/ : 1 /*error*/;
}

static void checkArgumentCount(int count, int min, int max)
{
    if (count < min) {
        fputs(i18nc("@info:shell", "%1: Syntax error, not enough arguments\n", qAppName()).toLocal8Bit().constData(), stderr);
        ::exit(1);
    }
    if (max && (count > max)) {
        fputs(i18nc("@info:shell", "%1: Syntax error, too many arguments\n", qAppName()).toLocal8Bit().constData(), stderr);
        ::exit(1);
    }
}

#ifndef KIOCORE_ONLY
bool ClientApp::kde_open(const QString &url, const QString &mimeType, bool allowExec)
{
    UrlInfo info(url);

    if (!info.atStart()) {
        QUrlQuery q;
        q.addQueryItem(QStringLiteral("line"), QString::number(info.line));
        q.addQueryItem(QStringLiteral("column"), QString::number(info.column));
        info.url.setQuery(q);
    }

    auto *job = new KIO::OpenUrlJob(info.url, mimeType);
    job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
    job->setRunExecutables(allowExec);
    job->setFollowRedirections(false);
    bool job_had_error = false;
    QObject::connect(job, &KJob::result, this, [&](KJob *job) {
        if (job->error()) {
            job_had_error = true;
            qWarning().noquote() << job->errorString();
        }
    });
    job->start();
    qApp->exec();
    return !job_had_error;
}
#endif

bool ClientApp::doCopy(const QStringList &urls)
{
    QList<QUrl> srcLst(makeUrls(urls));
    QUrl dest = srcLst.takeLast();
    KIO::Job *job = KIO::copy(srcLst, dest, s_jobFlags);
    if (!s_interactive) {
        job->setUiDelegate(nullptr);
        job->setUiDelegateExtension(nullptr);
    }
    connect(job, &KJob::result, this, &ClientApp::slotResult);
    qApp->exec();
    return m_ok;
}

void ClientApp::slotEntries(KIO::Job *, const KIO::UDSEntryList &list)
{
    for (const auto &entry : list) {
        // For each file...
        std::cout << qPrintable(entry.stringValue(KIO::UDSEntry::UDS_NAME)) << '\n';
    }

    std::cout << std::endl;
}

bool ClientApp::doList(const QStringList &urls)
{
    const QUrl dir = makeURL(urls.at(0));

    KIO::ListJob *job = KIO::listDir(dir, KIO::HideProgressInfo);
    if (!s_interactive) {
        job->setUiDelegate(nullptr);
        job->setUiDelegateExtension(nullptr);
    }

    connect(job, &KIO::ListJob::entries, this, &ClientApp::slotEntries);
    connect(job, &KJob::result, this, &ClientApp::slotResult);

    qApp->exec();
    return m_ok;
}

bool ClientApp::doMove(const QStringList &urls)
{
    QList<QUrl> srcLst(makeUrls(urls));
    const QUrl dest = srcLst.takeLast();

    KIO::Job *job = KIO::move(srcLst, dest, s_jobFlags);
    if (!s_interactive) {
        job->setUiDelegate(nullptr);
        job->setUiDelegateExtension(nullptr);
    }

    connect(job, &KJob::result, this, &ClientApp::slotResult);

    qApp->exec();
    return m_ok;
}

bool ClientApp::doRemove(const QStringList &urls)
{
    KIO::Job *job = KIO::del(makeUrls(urls), s_jobFlags);
    if (!s_interactive) {
        job->setUiDelegate(nullptr);
        job->setUiDelegateExtension(nullptr);
    }

    connect(job, &KJob::result, this, &ClientApp::slotResult);

    qApp->exec();
    return m_ok;
}

bool ClientApp::doStat(const QStringList &urls)
{
    KIO::Job *job = KIO::stat(makeURL(urls.first()),
                              KIO::StatJob::SourceSide,
                              (KIO::StatBasic | KIO::StatUser | KIO::StatTime | KIO::StatInode | KIO::StatMimeType | KIO::StatAcl),
                              s_jobFlags);
    if (!s_interactive) {
        job->setUiDelegate(nullptr);
        job->setUiDelegateExtension(nullptr);
    }

    connect(job, &KJob::result, this, &ClientApp::slotStatResult);

    qApp->exec();
    return m_ok;
}

bool ClientApp::doIt(const QCommandLineParser &parser)
{
    const int argc = parser.positionalArguments().count();
    checkArgumentCount(argc, 1, 0);

    if (parser.isSet(QStringLiteral("interactive"))) {
        s_interactive = true;
    } else {
        // "noninteractive" is currently the default mode, so we don't check.
        // The argument still needs to exist for compatibility
        s_interactive = false;
        s_jobFlags = KIO::HideProgressInfo;
    }
#if !defined(KIOCLIENT_AS_KDEOPEN)
    if (parser.isSet(QStringLiteral("overwrite"))) {
        s_jobFlags |= KIO::Overwrite;
    }
#endif

#ifdef KIOCLIENT_AS_KDEOPEN
    return kde_open(parser.positionalArguments().at(0), QString(), false);
#elif defined(KIOCLIENT_AS_KDECP)
    checkArgumentCount(argc, 2, 0);
    return doCopy(parser.positionalArguments());
#elif defined(KIOCLIENT_AS_KDEMV)
    checkArgumentCount(argc, 2, 0);
    return doMove(parser.positionalArguments());
#else
    // Normal kioclient mode
    const QString command = parser.positionalArguments().at(0);
#ifndef KIOCORE_ONLY
    if (command == QLatin1String("openProperties")) {
        checkArgumentCount(argc, 2, 2); // openProperties <url>
        const QUrl url = makeURL(parser.positionalArguments().constLast());

        KPropertiesDialog *dlg = new KPropertiesDialog(url, nullptr /*no parent*/);
        QObject::connect(dlg, &QObject::destroyed, qApp, &QCoreApplication::quit);
        QObject::connect(dlg, &KPropertiesDialog::canceled, this, &ClientApp::slotDialogCanceled);
        dlg->show();

        qApp->exec();
        return m_ok;
    } else
#endif
        if (command == QLatin1String("cat")) {
        checkArgumentCount(argc, 2, 2); // cat <url>
        const QUrl url = makeURL(parser.positionalArguments().constLast());

        KIO::TransferJob *job = KIO::get(url, KIO::NoReload, s_jobFlags);
        if (!s_interactive) {
            job->setUiDelegate(nullptr);
            job->setUiDelegateExtension(nullptr);
        }
        connect(job, &KIO::TransferJob::data, this, &ClientApp::slotPrintData);
        connect(job, &KJob::result, this, &ClientApp::slotResult);

        qApp->exec();
        return m_ok;
    }
#ifndef KIOCORE_ONLY
    else if (command == QLatin1String("exec")) {
        checkArgumentCount(argc, 2, 3);
        return kde_open(parser.positionalArguments().at(1), argc == 3 ? parser.positionalArguments().constLast() : QString(), true);
    } else if (command == QLatin1String("appmenu")) {
        auto *job = new KIO::ApplicationLauncherJob();
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        connect(job, &KJob::result, this, &ClientApp::slotResult);
        job->start();

        qApp->exec();
        return m_ok;
    }
#endif
    else if (command == QLatin1String("download")) {
        checkArgumentCount(argc, 0, 0);
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        const QList<QUrl> srcLst = makeUrls(args);

        if (srcLst.isEmpty()) {
            return m_ok;
        }

        const QUrl dsturl = QFileDialog::getSaveFileUrl(nullptr, i18n("Destination where to download the files"), srcLst.at(0));

        if (dsturl.isEmpty()) { // canceled
            return m_ok; // AK - really okay?
        }

        KIO::Job *job = KIO::copy(srcLst, dsturl, s_jobFlags);
        if (!s_interactive) {
            job->setUiDelegate(nullptr);
            job->setUiDelegateExtension(nullptr);
        }

        connect(job, &KJob::result, this, &ClientApp::slotResult);

        qApp->exec();
        return m_ok;
    } else if (command == QLatin1String("copy") || command == QLatin1String("cp")) {
        checkArgumentCount(argc, 3, 0); // cp <src> <dest>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doCopy(args);
    } else if (command == QLatin1String("move") || command == QLatin1String("mv")) {
        checkArgumentCount(argc, 3, 0); // mv <src> <dest>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doMove(args);
    } else if (command == QLatin1String("list") || command == QLatin1String("ls")) {
        checkArgumentCount(argc, 2, 2); // ls <url>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doList(args);
    } else if (command == QLatin1String("remove") || command == QLatin1String("rm")) {
        checkArgumentCount(argc, 2, 0); // rm <url>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doRemove(args);
    } else if (command == QLatin1String("stat")) {
        checkArgumentCount(argc, 2, 2); // stat <url>
        QStringList args = parser.positionalArguments();
        args.removeFirst();
        return doStat(args);
    } else {
        fputs(i18nc("@info:shell", "%1: Syntax error, unknown command '%2'\n", qAppName(), command).toLocal8Bit().data(), stderr);
        return false;
    }
    Q_UNREACHABLE();
#endif
}

void ClientApp::slotResult(KJob *job)
{
    if (job->error()) {
#ifndef KIOCORE_ONLY
        if (s_interactive) {
            static_cast<KIO::Job *>(job)->uiDelegate()->showErrorMessage();
        } else
#endif
        {
            fputs(qPrintable(i18nc("@info:shell", "%1: %2\n", qAppName(), job->errorString())), stderr);
        }
    }
    m_ok = !job->error();
    if (qApp->topLevelWindows().isEmpty()) {
        qApp->quit();
    } else {
        qApp->setQuitOnLastWindowClosed(true);
    }
}

void ClientApp::slotDialogCanceled()
{
    m_ok = false;
    qApp->quit();
}

void ClientApp::slotPrintData(KIO::Job *, const QByteArray &data)
{
    if (!data.isEmpty()) {
        std::cout.write(data.constData(), data.size());
    }
}

static void showStatField(const KIO::UDSEntry &entry, uint field, const char *name)
{
    if (!entry.contains(field))
        return;
    std::cout << qPrintable(QString::fromLocal8Bit(name).leftJustified(20, ' ')) << "  ";

    if (field == KIO::UDSEntry::UDS_ACCESS) {
        std::cout << qPrintable(QString("0%1").arg(entry.numberValue(field), 3, 8, QLatin1Char('0')));
    } else if (field == KIO::UDSEntry::UDS_FILE_TYPE) {
        std::cout << qPrintable(QString("0%1").arg((entry.numberValue(field) & S_IFMT), 6, 8, QLatin1Char('0')));
    } else if (field & KIO::UDSEntry::UDS_STRING) {
        std::cout << qPrintable(entry.stringValue(field));
    } else if ((field & KIO::UDSEntry::UDS_TIME) == KIO::UDSEntry::UDS_TIME) {
        // The previous comparison is necessary because the value
        // of UDS_TIME is 0x04000000|UDS_NUMBER which is 0x06000000.
        // So simply testing with (field & KIO::UDSEntry::UDS_TIME)
        // would be true for both UDS_TIME and UDS_NUMBER fields.
        // The same would happen if UDS_NUMBER were tested first.
        const QDateTime dt = QDateTime::fromSecsSinceEpoch(entry.numberValue(field));
        if (dt.isValid())
            std::cout << qPrintable(dt.toString(Qt::TextDate));
    } else if (field & KIO::UDSEntry::UDS_NUMBER) {
        std::cout << entry.numberValue(field);
    }
    std::cout << std::endl;
}

void ClientApp::slotStatResult(KJob *job)
{
    if (!job->error()) {
        KIO::StatJob *statJob = qobject_cast<KIO::StatJob *>(job);
        Q_ASSERT(statJob != nullptr);
        const KIO::UDSEntry &result = statJob->statResult();

        showStatField(result, KIO::UDSEntry::UDS_NAME, "NAME");
        showStatField(result, KIO::UDSEntry::UDS_DISPLAY_NAME, "DISPLAY_NAME");
        showStatField(result, KIO::UDSEntry::UDS_COMMENT, "COMMENT");
        showStatField(result, KIO::UDSEntry::UDS_SIZE, "SIZE");
        // This is not requested for the StatJob, so should never be seen
        showStatField(result, KIO::UDSEntry::UDS_RECURSIVE_SIZE, "RECURSIVE_SIZE");

        showStatField(result, KIO::UDSEntry::UDS_FILE_TYPE, "FILE_TYPE");
        showStatField(result, KIO::UDSEntry::UDS_USER, "USER");
        showStatField(result, KIO::UDSEntry::UDS_GROUP, "GROUP");
        showStatField(result, KIO::UDSEntry::UDS_HIDDEN, "HIDDEN");
        showStatField(result, KIO::UDSEntry::UDS_DEVICE_ID, "DEVICE_ID");
        showStatField(result, KIO::UDSEntry::UDS_INODE, "INODE");

        showStatField(result, KIO::UDSEntry::UDS_LINK_DEST, "LINK_DEST");
        showStatField(result, KIO::UDSEntry::UDS_URL, "URL");
        showStatField(result, KIO::UDSEntry::UDS_LOCAL_PATH, "LOCAL_PATH");
        showStatField(result, KIO::UDSEntry::UDS_TARGET_URL, "TARGET_URL");

        showStatField(result, KIO::UDSEntry::UDS_MIME_TYPE, "MIME_TYPE");
        showStatField(result, KIO::UDSEntry::UDS_GUESSED_MIME_TYPE, "GUESSED_MIME_TYPE");

        showStatField(result, KIO::UDSEntry::UDS_ICON_NAME, "ICON_NAME");
        showStatField(result, KIO::UDSEntry::UDS_ICON_OVERLAY_NAMES, "ICON_OVERLAY_NAMES");

        showStatField(result, KIO::UDSEntry::UDS_ACCESS, "ACCESS");
        showStatField(result, KIO::UDSEntry::UDS_EXTENDED_ACL, "EXTENDED_ACL");
        showStatField(result, KIO::UDSEntry::UDS_ACL_STRING, "ACL_STRING");
        showStatField(result, KIO::UDSEntry::UDS_DEFAULT_ACL_STRING, "DEFAULT_ACL_STRING");

        showStatField(result, KIO::UDSEntry::UDS_MODIFICATION_TIME, "MODIFICATION_TIME");
        showStatField(result, KIO::UDSEntry::UDS_ACCESS_TIME, "ACCESS_TIME");
        showStatField(result, KIO::UDSEntry::UDS_CREATION_TIME, "CREATION_TIME");

        showStatField(result, KIO::UDSEntry::UDS_XML_PROPERTIES, "XML_PROPERTIES");
        showStatField(result, KIO::UDSEntry::UDS_DISPLAY_TYPE, "DISPLAY_TYPE");
    }

    slotResult(job);
}

ClientApp::ClientApp()
    : QObject()
{
}

#include "moc_kioclient.cpp"
