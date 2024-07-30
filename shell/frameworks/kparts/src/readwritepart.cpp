/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "readwritepart.h"
#include "readwritepart_p.h"

#include "kparts_logging.h"

#define HAVE_KDIRNOTIFY __has_include(<KDirNotify>)
#if HAVE_KDIRNOTIFY
#include <KDirNotify>
#endif

#include <KIO/FileCopyJob>
#include <KJobWidgets>
#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QFileDialog>
#include <QTemporaryFile>

#include <qplatformdefs.h>

#ifdef Q_OS_WIN
#include <qt_windows.h> //CreateHardLink()
#endif

using namespace KParts;

ReadWritePart::ReadWritePart(QObject *parent, const KPluginMetaData &data)
    : ReadOnlyPart(*new ReadWritePartPrivate(this, data), parent)
{
}

ReadWritePart::~ReadWritePart()
{
    // parent destructor will delete temp file
    // we can't call our own closeUrl() here, because
    // "cancel" wouldn't cancel anything. We have to assume
    // the app called closeUrl() before destroying us.
}

void ReadWritePart::setReadWrite(bool readwrite)
{
    Q_D(ReadWritePart);

    // Perhaps we should check isModified here and issue a warning if true
    d->m_bReadWrite = readwrite;
}

void ReadWritePart::setModified(bool modified)
{
    Q_D(ReadWritePart);

    // qDebug() << "setModified(" << (modified ? "true" : "false") << ")";
    if (!d->m_bReadWrite && modified) {
        qCCritical(KPARTSLOG) << "Can't set a read-only document to 'modified' !";
        return;
    }
    d->m_bModified = modified;
}

void ReadWritePart::setModified()
{
    setModified(true);
}

bool ReadWritePart::queryClose()
{
    Q_D(ReadWritePart);

    if (!isReadWrite() || !isModified()) {
        return true;
    }

    QString docName = url().fileName();
    if (docName.isEmpty()) {
        docName = i18n("Untitled");
    }

    QWidget *parentWidget = widget();
    if (!parentWidget) {
        parentWidget = QApplication::activeWindow();
    }

    int res = KMessageBox::warningTwoActionsCancel(parentWidget,
                                                   i18n("The document \"%1\" has been modified.\n"
                                                        "Do you want to save your changes or discard them?",
                                                        docName),
                                                   i18n("Close Document"),
                                                   KStandardGuiItem::save(),
                                                   KStandardGuiItem::discard());

    bool abortClose = false;
    bool handled = false;

    switch (res) {
    case KMessageBox::PrimaryAction:
        Q_EMIT sigQueryClose(&handled, &abortClose);
        if (!handled) {
            if (d->m_url.isEmpty()) {
                QUrl url = QFileDialog::getSaveFileUrl(parentWidget);
                if (url.isEmpty()) {
                    return false;
                }

                saveAs(url);
            } else {
                save();
            }
        } else if (abortClose) {
            return false;
        }
        return waitSaveComplete();
    case KMessageBox::SecondaryAction:
        return true;
    default: // case KMessageBox::Cancel :
        return false;
    }
}

bool ReadWritePart::closeUrl()
{
    abortLoad(); // just in case
    if (isReadWrite() && isModified()) {
        if (!queryClose()) {
            return false;
        }
    }
    // Not modified => ok and delete temp file.
    return ReadOnlyPart::closeUrl();
}

bool ReadWritePart::closeUrl(bool promptToSave)
{
    return promptToSave ? closeUrl() : ReadOnlyPart::closeUrl();
}

bool ReadWritePart::save()
{
    Q_D(ReadWritePart);

    d->m_saveOk = false;
    if (d->m_file.isEmpty()) { // document was created empty
        d->prepareSaving();
    }
    if (saveFile()) {
        return saveToUrl();
    } else {
        Q_EMIT canceled(QString());
    }
    return false;
}

bool ReadWritePart::saveAs(const QUrl &url)
{
    Q_D(ReadWritePart);

    if (!url.isValid()) {
        qCCritical(KPARTSLOG) << "saveAs: Malformed URL" << url;
        return false;
    }
    d->m_duringSaveAs = true;
    d->m_originalURL = d->m_url;
    d->m_originalFilePath = d->m_file;
    d->m_url = url; // Store where to upload in saveToURL
    d->prepareSaving();
    bool result = save(); // Save local file and upload local file
    if (result) {
        if (d->m_originalURL != d->m_url) {
            Q_EMIT urlChanged(d->m_url);
        }

        Q_EMIT setWindowCaption(d->m_url.toDisplayString(QUrl::PreferLocalFile));
    } else {
        d->m_url = d->m_originalURL;
        d->m_file = d->m_originalFilePath;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
    }

    return result;
}

// Set m_file correctly for m_url
void ReadWritePartPrivate::prepareSaving()
{
    // Local file
    if (m_url.isLocalFile()) {
        if (m_bTemp) { // get rid of a possible temp file first
            // (happens if previous url was remote)
            QFile::remove(m_file);
            m_bTemp = false;
        }
        m_file = m_url.toLocalFile();
    } else {
        // Remote file
        // We haven't saved yet, or we did but locally - provide a temp file
        if (m_file.isEmpty() || !m_bTemp) {
            QTemporaryFile tempFile;
            tempFile.setAutoRemove(false);
            tempFile.open();
            m_file = tempFile.fileName();
            m_bTemp = true;
        }
        // otherwise, we already had a temp file
    }
}

static inline bool makeHardLink(const QString &src, const QString &dest)
{
#ifndef Q_OS_WIN
    return ::link(QFile::encodeName(src).constData(), QFile::encodeName(dest).constData()) == 0;
#else
    return CreateHardLinkW((LPCWSTR)dest.utf16(), (LPCWSTR)src.utf16(), nullptr) != 0;
#endif
}

bool ReadWritePart::saveToUrl()
{
    Q_D(ReadWritePart);

    if (d->m_url.isLocalFile()) {
        setModified(false);
        Q_EMIT completed();
        // if m_url is a local file there won't be a temp file -> nothing to remove
        Q_ASSERT(!d->m_bTemp);
        d->m_saveOk = true;
        d->m_duringSaveAs = false;
        d->m_originalURL = QUrl();
        d->m_originalFilePath.clear();
        return true; // Nothing to do
    } else {
        if (d->m_uploadJob) {
            QFile::remove(d->m_uploadJob->srcUrl().toLocalFile());
            d->m_uploadJob->kill();
            d->m_uploadJob = nullptr;
        }
        QTemporaryFile *tempFile = new QTemporaryFile();
        tempFile->open();
        QString uploadFile = tempFile->fileName();
        delete tempFile;
        QUrl uploadUrl = QUrl::fromLocalFile(uploadFile);
        // Create hardlink
        if (!makeHardLink(d->m_file, uploadFile)) {
            // Uh oh, some error happened.
            return false;
        }
        d->m_uploadJob = KIO::file_move(uploadUrl, d->m_url, -1, KIO::Overwrite);
        KJobWidgets::setWindow(d->m_uploadJob, widget());

        connect(d->m_uploadJob, &KJob::result, this, [d](KJob *job) {
            d->slotUploadFinished(job);
        });

        return true;
    }
}

void ReadWritePartPrivate::slotUploadFinished(KJob *)
{
    Q_Q(ReadWritePart);

    if (m_uploadJob->error()) {
        QFile::remove(m_uploadJob->srcUrl().toLocalFile());
        QString error = m_uploadJob->errorString();
        m_uploadJob = nullptr;
        if (m_duringSaveAs) {
            q->setUrl(m_originalURL);
            m_file = m_originalFilePath;
        }
        Q_EMIT q->canceled(error);
    } else {
#if HAVE_KDIRNOTIFY
        ::org::kde::KDirNotify::emitFilesAdded(m_url.adjusted(QUrl::RemoveFilename));
#endif

        m_uploadJob = nullptr;
        q->setModified(false);
        Q_EMIT q->completed();
        m_saveOk = true;
    }
    m_duringSaveAs = false;
    m_originalURL = QUrl();
    m_originalFilePath.clear();
    if (m_waitForSave) {
        m_eventLoop.quit();
    }
}

bool ReadWritePart::isReadWrite() const
{
    Q_D(const ReadWritePart);

    return d->m_bReadWrite;
}

bool ReadWritePart::isModified() const
{
    Q_D(const ReadWritePart);

    return d->m_bModified;
}

bool ReadWritePart::waitSaveComplete()
{
    Q_D(ReadWritePart);

    if (!d->m_uploadJob) {
        return d->m_saveOk;
    }

    d->m_waitForSave = true;

    d->m_eventLoop.exec(QEventLoop::ExcludeUserInputEvents);

    d->m_waitForSave = false;

    return d->m_saveOk;
}

#include "moc_readwritepart.cpp"
