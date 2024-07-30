/*
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Diana-Victoria Tiriplica <diana.tiriplica@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config.h"

#include "katebuffer.h"
#include "kateconfig.h"
#include "katedocument.h"
#include "katepartdebug.h"
#include "kateswapdiffcreator.h"
#include "kateswapfile.h"
#include "katetextbuffer.h"
#include "kateundomanager.h"
#include "ktexteditor/message.h"
#include <ktexteditor/view.h>

#include <KLocalizedString>
#include <KStandardGuiItem>

#include <QApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

// swap file version header
const static char swapFileVersionString[] = "Kate Swap File 2.0";

// tokens for swap files
const static qint8 EA_StartEditing = 'S';
const static qint8 EA_FinishEditing = 'E';
const static qint8 EA_WrapLine = 'W';
const static qint8 EA_UnwrapLine = 'U';
const static qint8 EA_InsertText = 'I';
const static qint8 EA_RemoveText = 'R';

namespace Kate
{
QTimer *SwapFile::s_timer = nullptr;

SwapFile::SwapFile(KTextEditor::DocumentPrivate *document)
    : QObject(document)
    , m_document(document)
    , m_trackingEnabled(false)
    , m_recovered(false)
    , m_needSync(false)
{
    // fixed version of serialisation
    m_stream.setVersion(QDataStream::Qt_4_6);

    // connect the timer
    connect(syncTimer(), &QTimer::timeout, this, &Kate::SwapFile::writeFileToDisk, Qt::DirectConnection);

    // connecting the signals
    connect(&m_document->buffer(), &KateBuffer::saved, this, &Kate::SwapFile::fileSaved);
    connect(&m_document->buffer(), &KateBuffer::loaded, this, &Kate::SwapFile::fileLoaded);
    connect(m_document, &KTextEditor::Document::configChanged, this, &SwapFile::configChanged);

    // tracking on!
    setTrackingEnabled(true);
}

SwapFile::~SwapFile()
{
    // only remove swap file after data recovery (bug #304576)
    if (!shouldRecover()) {
        removeSwapFile();
    }
}

void SwapFile::configChanged()
{
}

void SwapFile::setTrackingEnabled(bool enable)
{
    if (m_trackingEnabled == enable) {
        return;
    }

    m_trackingEnabled = enable;

    if (m_trackingEnabled) {
        connect(m_document, &KTextEditor::Document::editingStarted, this, &Kate::SwapFile::startEditing);
        connect(m_document, &KTextEditor::Document::editingFinished, this, &Kate::SwapFile::finishEditing);
        connect(m_document, &KTextEditor::DocumentPrivate::modifiedChanged, this, &SwapFile::modifiedChanged);

        connect(m_document, &KTextEditor::Document::lineWrapped, this, &Kate::SwapFile::wrapLine);
        connect(m_document, &KTextEditor::Document::lineUnwrapped, this, &Kate::SwapFile::unwrapLine);
        connect(m_document, &KTextEditor::Document::textInserted, this, &Kate::SwapFile::insertText);
        connect(m_document, &KTextEditor::Document::textRemoved, this, &Kate::SwapFile::removeText);
    } else {
        disconnect(m_document, &KTextEditor::Document::editingStarted, this, &Kate::SwapFile::startEditing);
        disconnect(m_document, &KTextEditor::Document::editingFinished, this, &Kate::SwapFile::finishEditing);
        disconnect(m_document, &KTextEditor::DocumentPrivate::modifiedChanged, this, &SwapFile::modifiedChanged);

        disconnect(m_document, &KTextEditor::Document::lineWrapped, this, &Kate::SwapFile::wrapLine);
        disconnect(m_document, &KTextEditor::Document::lineUnwrapped, this, &Kate::SwapFile::unwrapLine);
        disconnect(m_document, &KTextEditor::Document::textInserted, this, &Kate::SwapFile::insertText);
        disconnect(m_document, &KTextEditor::Document::textRemoved, this, &Kate::SwapFile::removeText);
    }
}

void SwapFile::fileClosed()
{
    // remove old swap file, file is now closed
    if (!shouldRecover()) {
        removeSwapFile();
    } else {
        m_document->setReadWrite(true);
    }

    // purge filename
    updateFileName();
}

KTextEditor::DocumentPrivate *SwapFile::document()
{
    return m_document;
}

bool SwapFile::isValidSwapFile(QDataStream &stream, bool checkDigest) const
{
    // read and check header
    QByteArray header;
    stream >> header;

    if (header != swapFileVersionString) {
        qCWarning(LOG_KTE) << "Can't open swap file, wrong version";
        return false;
    }

    // read checksum
    QByteArray checksum;
    stream >> checksum;
    // qCDebug(LOG_KTE) << "DIGEST:" << checksum << m_document->checksum();
    if (checkDigest && checksum != m_document->checksum()) {
        qCWarning(LOG_KTE) << "Can't recover from swap file, checksum of document has changed";
        return false;
    }

    return true;
}

void SwapFile::fileLoaded(const QString &)
{
    // look for swap file
    if (!updateFileName()) {
        return;
    }

    if (!m_swapfile.exists()) {
        // qCDebug(LOG_KTE) << "No swap file";
        return;
    }

    if (!QFileInfo(m_swapfile).isReadable()) {
        qCWarning(LOG_KTE) << "Can't open swap file (missing permissions)";
        return;
    }

    // sanity check
    QFile peekFile(fileName());
    if (peekFile.open(QIODevice::ReadOnly)) {
        QDataStream stream(&peekFile);
        if (!isValidSwapFile(stream, true)) {
            removeSwapFile();
            return;
        }
        peekFile.close();
    } else {
        qCWarning(LOG_KTE) << "Can't open swap file:" << fileName();
        return;
    }

    // show swap file message
    m_document->setReadWrite(false);
    showSwapFileMessage();
}

void SwapFile::modifiedChanged()
{
    if (!m_document->isModified() && !shouldRecover()) {
        m_needSync = false;
        // the file is not modified and we are not in recover mode
        removeSwapFile();
    }
}

void SwapFile::recover()
{
    m_document->setReadWrite(true);

    // if isOpen() returns true, the swap file likely changed already (appended data)
    // Example: The document was falsely marked as writable and the user changed
    // text even though the recover bar was visible. In this case, a replay of
    // the swap file across wrong document content would happen -> certainly wrong
    if (m_swapfile.isOpen()) {
        qCWarning(LOG_KTE) << "Attempt to recover an already modified document. Aborting";
        removeSwapFile();
        return;
    }

    // if the file doesn't exist, abort (user might have deleted it, or use two editor instances)
    if (!m_swapfile.open(QIODevice::ReadOnly)) {
        qCWarning(LOG_KTE) << "Can't open swap file";
        return;
    }

    // remember that the file has recovered
    m_recovered = true;

    // open data stream
    m_stream.setDevice(&m_swapfile);

    // replay the swap file
    bool success = recover(m_stream);

    // close swap file
    m_stream.setDevice(nullptr);
    m_swapfile.close();

    if (!success) {
        removeSwapFile();
    }

    // recover can also be called through the KTE::RecoveryInterface.
    // Make sure, the message is hidden in this case as well.
    if (m_swapMessage) {
        m_swapMessage->deleteLater();
    }
}

bool SwapFile::recover(QDataStream &stream, bool checkDigest)
{
    if (!isValidSwapFile(stream, checkDigest)) {
        return false;
    }

    // disconnect current signals
    setTrackingEnabled(false);

    // needed to set undo/redo cursors in a sane way
    bool firstEditInGroup = false;
    KTextEditor::Cursor undoCursor = KTextEditor::Cursor::invalid();
    KTextEditor::Cursor redoCursor = KTextEditor::Cursor::invalid();

    // replay swapfile
    bool editRunning = false;
    bool brokenSwapFile = false;
    while (!stream.atEnd()) {
        if (brokenSwapFile) {
            break;
        }

        qint8 type;
        stream >> type;
        switch (type) {
        case EA_StartEditing: {
            m_document->editStart();
            editRunning = true;
            firstEditInGroup = true;
            undoCursor = KTextEditor::Cursor::invalid();
            redoCursor = KTextEditor::Cursor::invalid();
            break;
        }
        case EA_FinishEditing: {
            m_document->editEnd();

            // empty editStart() / editEnd() groups exist: only set cursor if required
            if (!firstEditInGroup) {
                // set undo/redo cursor of last KateUndoGroup of the undo manager
                m_document->undoManager()->setUndoRedoCursorsOfLastGroup(undoCursor, redoCursor);
                m_document->undoManager()->undoSafePoint();
            }
            firstEditInGroup = false;
            editRunning = false;
            break;
        }
        case EA_WrapLine: {
            if (!editRunning) {
                brokenSwapFile = true;
                break;
            }

            int line = 0;
            int column = 0;
            stream >> line >> column;

            // emulate buffer unwrapLine with document
            m_document->editWrapLine(line, column, true);

            // track undo/redo cursor
            if (firstEditInGroup) {
                firstEditInGroup = false;
                undoCursor = KTextEditor::Cursor(line, column);
            }
            redoCursor = KTextEditor::Cursor(line + 1, 0);

            break;
        }
        case EA_UnwrapLine: {
            if (!editRunning) {
                brokenSwapFile = true;
                break;
            }

            int line = 0;
            stream >> line;

            // assert valid line
            Q_ASSERT(line > 0);

            const int undoColumn = m_document->lineLength(line - 1);

            // emulate buffer unwrapLine with document
            m_document->editUnWrapLine(line - 1, true, 0);

            // track undo/redo cursor
            if (firstEditInGroup) {
                firstEditInGroup = false;
                undoCursor = KTextEditor::Cursor(line, 0);
            }
            redoCursor = KTextEditor::Cursor(line - 1, undoColumn);

            break;
        }
        case EA_InsertText: {
            if (!editRunning) {
                brokenSwapFile = true;
                break;
            }

            int line;
            int column;
            QByteArray text;
            stream >> line >> column >> text;
            m_document->insertText(KTextEditor::Cursor(line, column), QString::fromUtf8(text.data(), text.size()));

            // track undo/redo cursor
            if (firstEditInGroup) {
                firstEditInGroup = false;
                undoCursor = KTextEditor::Cursor(line, column);
            }
            redoCursor = KTextEditor::Cursor(line, column + text.size());

            break;
        }
        case EA_RemoveText: {
            if (!editRunning) {
                brokenSwapFile = true;
                break;
            }

            int line;
            int startColumn;
            int endColumn;
            stream >> line >> startColumn >> endColumn;
            m_document->removeText(KTextEditor::Range(KTextEditor::Cursor(line, startColumn), KTextEditor::Cursor(line, endColumn)));

            // track undo/redo cursor
            if (firstEditInGroup) {
                firstEditInGroup = false;
                undoCursor = KTextEditor::Cursor(line, endColumn);
            }
            redoCursor = KTextEditor::Cursor(line, startColumn);

            break;
        }
        default: {
            qCWarning(LOG_KTE) << "Unknown type:" << type;
        }
        }
    }

    // balanced editStart and editEnd?
    if (editRunning) {
        brokenSwapFile = true;
        m_document->editEnd();
    }

    // warn the user if the swap file is not complete
    if (brokenSwapFile) {
        qCWarning(LOG_KTE) << "Some data might be lost";
    } else {
        // set sane final cursor, if possible
        KTextEditor::View *view = m_document->activeView();
        redoCursor = m_document->undoManager()->lastRedoCursor();
        if (view && redoCursor.isValid()) {
            view->setCursorPosition(redoCursor);
        }
    }

    // reconnect the signals
    setTrackingEnabled(true);

    return true;
}

void SwapFile::fileSaved(const QString &)
{
    m_needSync = false;

    // remove old swap file (e.g. if a file A was "saved as" B)
    removeSwapFile();

    // set the name for the new swap file
    updateFileName();
}

void SwapFile::startEditing()
{
    // no swap file, no work
    if (m_swapfile.fileName().isEmpty()) {
        return;
    }

    // if swap file doesn't exists, open it in WriteOnly mode
    // if it does, append the data to the existing swap file,
    // in case you recover and start editing again
    if (!m_swapfile.exists()) {
        // create path if not there
        if (KateDocumentConfig::global()->swapFileMode() == KateDocumentConfig::SwapFilePresetDirectory
            && !QDir(KateDocumentConfig::global()->swapDirectory()).exists()) {
            QDir().mkpath(KateDocumentConfig::global()->swapDirectory());
        }

        m_swapfile.open(QIODevice::WriteOnly);
        m_swapfile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        m_stream.setDevice(&m_swapfile);

        // write file header
        m_stream << QByteArray(swapFileVersionString);

        // write checksum
        m_stream << m_document->checksum();
    } else if (m_stream.device() == nullptr) {
        m_swapfile.open(QIODevice::Append);
        m_swapfile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        m_stream.setDevice(&m_swapfile);
    }

    // format: qint8
    m_stream << EA_StartEditing;
}

void SwapFile::finishEditing()
{
    // skip if not open
    if (!m_swapfile.isOpen()) {
        return;
    }

    // write the file to the disk every 15 seconds (default)
    // skip this if we disabled that
    if (m_document->config()->swapSyncInterval() != 0 && !syncTimer()->isActive()) {
        // important: we store the interval as seconds, start wants milliseconds!
        syncTimer()->start(m_document->config()->swapSyncInterval() * 1000);
    }

    // format: qint8
    m_stream << EA_FinishEditing;
    m_swapfile.flush();
}

void SwapFile::wrapLine(KTextEditor::Document *, const KTextEditor::Cursor position)
{
    // skip if not open
    if (!m_swapfile.isOpen()) {
        return;
    }

    // format: qint8, int, int
    m_stream << EA_WrapLine << position.line() << position.column();

    m_needSync = true;
}

void SwapFile::unwrapLine(KTextEditor::Document *, int line)
{
    // skip if not open
    if (!m_swapfile.isOpen()) {
        return;
    }

    // format: qint8, int
    m_stream << EA_UnwrapLine << line;

    m_needSync = true;
}

void SwapFile::insertText(KTextEditor::Document *, const KTextEditor::Cursor position, const QString &text)
{
    // skip if not open
    if (!m_swapfile.isOpen()) {
        return;
    }

    // format: qint8, int, int, bytearray
    m_stream << EA_InsertText << position.line() << position.column() << text.toUtf8();

    m_needSync = true;
}

void SwapFile::removeText(KTextEditor::Document *, KTextEditor::Range range, const QString &)
{
    // skip if not open
    if (!m_swapfile.isOpen()) {
        return;
    }

    // format: qint8, int, int, int
    Q_ASSERT(range.start().line() == range.end().line());
    m_stream << EA_RemoveText << range.start().line() << range.start().column() << range.end().column();

    m_needSync = true;
}

bool SwapFile::shouldRecover() const
{
    // should not recover if the file has already recovered in another view
    if (m_recovered) {
        return false;
    }

    return !m_swapfile.fileName().isEmpty() && m_swapfile.exists() && m_stream.device() == nullptr;
}

void SwapFile::discard()
{
    m_document->setReadWrite(true);
    removeSwapFile();

    // discard can also be called through the KTE::RecoveryInterface.
    // Make sure, the message is hidden in this case as well.
    if (m_swapMessage) {
        m_swapMessage->deleteLater();
    }
}

void SwapFile::removeSwapFile()
{
    if (!m_swapfile.fileName().isEmpty() && m_swapfile.exists()) {
        m_stream.setDevice(nullptr);
        m_swapfile.close();
        m_swapfile.remove();
    }
}

bool SwapFile::updateFileName()
{
    // first clear filename
    m_swapfile.setFileName(QString());

    // get the new path
    QString path = fileName();
    if (path.isNull()) {
        return false;
    }

    m_swapfile.setFileName(path);
    return true;
}

QString SwapFile::fileName()
{
    const QUrl &url = m_document->url();
    if (url.isEmpty() || !url.isLocalFile()) {
        return QString();
    }

    const QString fullLocalPath(url.toLocalFile());
    QString path;
    if (KateDocumentConfig::global()->swapFileMode() == KateDocumentConfig::SwapFilePresetDirectory) {
        path = KateDocumentConfig::global()->swapDirectory();
        path.append(QLatin1Char('/'));

        // append the sha1 sum of the full path + filename, to avoid "too long" paths created
        path.append(QString::fromLatin1(QCryptographicHash::hash(fullLocalPath.toUtf8(), QCryptographicHash::Sha1).toHex()));
        path.append(QLatin1Char('-'));
        path.append(QFileInfo(fullLocalPath).fileName());

        path.append(QLatin1String(".kate-swp"));
    } else {
        path = fullLocalPath;
        int poz = path.lastIndexOf(QLatin1Char('/'));
        path.insert(poz + 1, QLatin1Char('.'));
        path.append(QLatin1String(".kate-swp"));
    }

    return path;
}

QTimer *SwapFile::syncTimer()
{
    if (s_timer == nullptr) {
        s_timer = new QTimer(QApplication::instance());
        s_timer->setSingleShot(true);
    }

    return s_timer;
}

void SwapFile::writeFileToDisk()
{
    if (m_needSync) {
        m_needSync = false;

#ifndef Q_OS_WIN
        // ensure that the file is written to disk
#if HAVE_FDATASYNC
        fdatasync(m_swapfile.handle());
#else
        fsync(m_swapfile.handle());
#endif
#endif
    }
}

void SwapFile::showSwapFileMessage()
{
    m_swapMessage = new KTextEditor::Message(i18n("The file was not closed properly."), KTextEditor::Message::Warning);
    m_swapMessage->setWordWrap(true);

    QAction *diffAction = new QAction(QIcon::fromTheme(QStringLiteral("split")), i18n("View Changes"), nullptr);
    QAction *recoverAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-redo")), i18n("Recover Data"), nullptr);
    QAction *discardAction = new QAction(KStandardGuiItem::discard().icon(), i18n("Discard"), nullptr);

    m_swapMessage->addAction(diffAction, false);
    m_swapMessage->addAction(recoverAction);
    m_swapMessage->addAction(discardAction);

    connect(diffAction, &QAction::triggered, this, &SwapFile::showDiff);
    connect(recoverAction, &QAction::triggered, this, qOverload<>(&Kate::SwapFile::recover), Qt::QueuedConnection);
    connect(discardAction, &QAction::triggered, this, &SwapFile::discard, Qt::QueuedConnection);

    m_document->postMessage(m_swapMessage);
}

void SwapFile::showDiff()
{
    // the diff creator deletes itself through deleteLater() when it's done
    SwapDiffCreator *diffCreator = new SwapDiffCreator(this);
    diffCreator->viewDiff();
}

}
