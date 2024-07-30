/*

    SPDX-FileCopyrightText: 2017-2018 Red Hat Inc
    Contact: https://www.qt.io/licensing/

    This file is part of the plugins of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KFQF-Accepted-GPL OR LicenseRef-Qt-Commercial

*/

#include "qxdgdesktopportalfiledialog_p.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QEventLoop>
#include <QRegularExpression>

#include <QFile>
#include <QMetaType>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRandomGenerator>
#include <QWindow>

QT_BEGIN_NAMESPACE

QDBusArgument &operator<<(QDBusArgument &arg, const QXdgDesktopPortalFileDialog::FilterCondition &filterCondition)
{
    arg.beginStructure();
    arg << filterCondition.type << filterCondition.pattern;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, QXdgDesktopPortalFileDialog::FilterCondition &filterCondition)
{
    uint type;
    QString filterPattern;
    arg.beginStructure();
    arg >> type >> filterPattern;
    filterCondition.type = (QXdgDesktopPortalFileDialog::ConditionType)type;
    filterCondition.pattern = filterPattern;
    arg.endStructure();

    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const QXdgDesktopPortalFileDialog::Filter filter)
{
    arg.beginStructure();
    arg << filter.name << filter.filterConditions;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, QXdgDesktopPortalFileDialog::Filter &filter)
{
    QString name;
    QXdgDesktopPortalFileDialog::FilterConditionList filterConditions;
    arg.beginStructure();
    arg >> name >> filterConditions;
    filter.name = name;
    filter.filterConditions = filterConditions;
    arg.endStructure();

    return arg;
}

class QXdgDesktopPortalFileDialogPrivate
{
public:
    QXdgDesktopPortalFileDialogPrivate(QPlatformFileDialogHelper *nativeFileDialog)
        : nativeFileDialog(nativeFileDialog)
    {
    }

    WId winId = 0;
    bool modal = false;
    bool multipleFiles = false;
    bool selectDirectory = false;
    bool saveFile = false;
    QString acceptLabel;
    QUrl directory;
    QString title;
    QStringList nameFilters;
    QStringList mimeTypesFilters;
    QList<QUrl> selectedFiles;
    QPlatformFileDialogHelper *nativeFileDialog = nullptr;
};

QXdgDesktopPortalFileDialog::QXdgDesktopPortalFileDialog(QPlatformFileDialogHelper *nativeFileDialog)
    : QPlatformFileDialogHelper()
    , d_ptr(new QXdgDesktopPortalFileDialogPrivate(nativeFileDialog))
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        connect(d->nativeFileDialog, SIGNAL(accept()), this, SIGNAL(accept()));
        connect(d->nativeFileDialog, SIGNAL(reject()), this, SIGNAL(reject()));
    }
}

QXdgDesktopPortalFileDialog::~QXdgDesktopPortalFileDialog()
{
}

void QXdgDesktopPortalFileDialog::initializeDialog()
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog)
        d->nativeFileDialog->setOptions(options());

    if (options()->fileMode() == QFileDialogOptions::ExistingFiles)
        d->multipleFiles = true;

    if (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly) {
        d->selectDirectory = true;
    }

    if (options()->isLabelExplicitlySet(QFileDialogOptions::Accept))
        d->acceptLabel = options()->labelText(QFileDialogOptions::Accept);

    if (!options()->windowTitle().isEmpty())
        d->title = options()->windowTitle();

    if (options()->acceptMode() == QFileDialogOptions::AcceptSave)
        d->saveFile = true;

    if (!options()->nameFilters().isEmpty())
        d->nameFilters = options()->nameFilters();

    if (!options()->mimeTypeFilters().isEmpty())
        d->mimeTypesFilters = options()->mimeTypeFilters();

    setDirectory(options()->initialDirectory());
}

void QXdgDesktopPortalFileDialog::openPortal()
{
    Q_D(const QXdgDesktopPortalFileDialog);

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"),
                                                          QStringLiteral("/org/freedesktop/portal/desktop"),
                                                          QStringLiteral("org.freedesktop.portal.FileChooser"),
                                                          d->saveFile ? QStringLiteral("SaveFile") : QStringLiteral("OpenFile"));
    QString parentWindowId = QStringLiteral("x11:") + QString::number(d->winId, 16);

    QVariantMap options;
    if (!d->acceptLabel.isEmpty())
        options.insert(QStringLiteral("accept_label"), d->acceptLabel);

    options.insert(QStringLiteral("modal"), d->modal);
    options.insert(QStringLiteral("multiple"), d->multipleFiles);
    options.insert(QStringLiteral("directory"), d->selectDirectory);

    if (d->saveFile) {
        if (!d->directory.isEmpty())
            options.insert(QStringLiteral("current_folder"), QFile::encodeName(d->directory.toLocalFile()).append('\0'));

        if (!d->selectedFiles.isEmpty())
            options.insert(QStringLiteral("current_file"), QFile::encodeName(d->selectedFiles.first().toLocalFile()).append('\0'));
    }

    // Insert filters
    qDBusRegisterMetaType<FilterCondition>();
    qDBusRegisterMetaType<FilterConditionList>();
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<FilterList>();

    FilterList filterList;

    if (!d->mimeTypesFilters.isEmpty()) {
        for (const QString &mimeTypefilter : d->mimeTypesFilters) {
            QMimeDatabase mimeDatabase;
            QMimeType mimeType = mimeDatabase.mimeTypeForName(mimeTypefilter);

            // Creates e.g. (1, "image/png")
            FilterCondition filterCondition;
            filterCondition.type = MimeType;
            filterCondition.pattern = mimeTypefilter;

            // Creates e.g. [((1, "image/png"))]
            FilterConditionList filterConditions;
            filterConditions << filterCondition;

            // Creates e.g. [("Images", [((1, "image/png"))])]
            Filter filter;
            filter.name = mimeType.comment();
            filter.filterConditions = filterConditions;

            filterList << filter;
        }
    } else if (!d->nameFilters.isEmpty()) {
        for (const QString &filter : d->nameFilters) {
            // Do parsing:
            // Supported format is ("Images (*.png *.jpg)")
            QRegularExpression regexp(QString::fromLatin1(QPlatformFileDialogHelper::filterRegExp));
            QRegularExpressionMatch match = regexp.match(filter);
            if (match.hasMatch()) {
                QString userVisibleName = match.captured(1);
                QStringList filterStrings = match.captured(2).split(QLatin1Char(' '), Qt::SkipEmptyParts);

                FilterConditionList filterConditions;
                for (const QString &filterString : std::as_const(filterStrings)) {
                    FilterCondition filterCondition;
                    filterCondition.type = GlobalPattern;
                    filterCondition.pattern = filterString;
                    filterConditions << filterCondition;
                }

                Filter filter;
                filter.name = userVisibleName;
                filter.filterConditions = filterConditions;

                filterList << filter;
            }
        }
    }

    if (!filterList.isEmpty())
        options.insert(QStringLiteral("filters"), QVariant::fromValue(filterList));

    options.insert(QStringLiteral("handle_token"), QStringLiteral("qt%1").arg(QRandomGenerator::global()->generate()));

    // TODO choices a(ssa(ss)s)
    // List of serialized combo boxes to add to the file chooser.

    message << parentWindowId << d->title << options;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (reply.isError()) {
            Q_EMIT reject();
        } else {
            QDBusConnection::sessionBus().connect({},
                                                  reply.value().path(),
                                                  QStringLiteral("org.freedesktop.portal.Request"),
                                                  QStringLiteral("Response"),
                                                  this,
                                                  SLOT(gotResponse(uint, QVariantMap)));
        }
    });
}

bool QXdgDesktopPortalFileDialog::defaultNameFilterDisables() const
{
    return false;
}

void QXdgDesktopPortalFileDialog::setDirectory(const QUrl &directory)
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->setDirectory(directory);
    }

    d->directory = directory;
}

QUrl QXdgDesktopPortalFileDialog::directory() const
{
    Q_D(const QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->directory();

    return d->directory;
}

void QXdgDesktopPortalFileDialog::selectFile(const QUrl &filename)
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->selectFile(filename);
    }

    d->selectedFiles << filename;
}

QList<QUrl> QXdgDesktopPortalFileDialog::selectedFiles() const
{
    Q_D(const QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->selectedFiles();

    return d->selectedFiles;
}

void QXdgDesktopPortalFileDialog::setFilter()
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->setFilter();
    }
}

void QXdgDesktopPortalFileDialog::selectNameFilter(const QString &filter)
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog) {
        d->nativeFileDialog->setOptions(options());
        d->nativeFileDialog->selectNameFilter(filter);
    }
}

QString QXdgDesktopPortalFileDialog::selectedNameFilter() const
{
    // TODO
    return QString();
}

void QXdgDesktopPortalFileDialog::exec()
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly)) {
        d->nativeFileDialog->exec();
        return;
    }

    // HACK we have to avoid returning until we emit that the dialog was accepted or rejected
    QEventLoop loop;
    QObject::connect(this, &QXdgDesktopPortalFileDialog::accept, &loop, &QEventLoop::quit);
    QObject::connect(this, &QXdgDesktopPortalFileDialog::reject, &loop, &QEventLoop::quit);
    loop.exec();
}

void QXdgDesktopPortalFileDialog::hide()
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (d->nativeFileDialog)
        d->nativeFileDialog->hide();
}

bool QXdgDesktopPortalFileDialog::show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent)
{
    Q_D(QXdgDesktopPortalFileDialog);

    initializeDialog();

    d->modal = windowModality != Qt::NonModal;
    d->winId = parent ? parent->winId() : 0;

    if (d->nativeFileDialog && (options()->fileMode() == QFileDialogOptions::Directory || options()->fileMode() == QFileDialogOptions::DirectoryOnly))
        return d->nativeFileDialog->show(windowFlags, windowModality, parent);

    openPortal();

    return true;
}

void QXdgDesktopPortalFileDialog::gotResponse(uint response, const QVariantMap &results)
{
    Q_D(QXdgDesktopPortalFileDialog);

    if (!response) {
        if (results.contains(QStringLiteral("uris"))) {
            const QStringList uris = results.value(QStringLiteral("uris")).toStringList();
            d->selectedFiles.clear();
            d->selectedFiles.reserve(uris.size());
            for (const QString &uri : uris) {
                // uris are expected to have proper "file:" scheme set
                d->selectedFiles.append(QUrl(uri));
            }
        }
        Q_EMIT accept();
    } else {
        Q_EMIT reject();
    }
}

QT_END_NAMESPACE

#include "moc_qxdgdesktopportalfiledialog_p.cpp"
