/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2012 Christoph Cullmann <cullmann@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kateproject.h"
#include "kateprojectplugin.h"
#include "kateprojectworker.h"

#include <KLocalizedString>

#include <ktexteditor/document.h>

#include <json_utils.h>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMimeData>
#include <QPlainTextDocumentLayout>
#include <QPointer>
#include <utility>

#include <KIO/CopyJob>
#include <KJobWidgets>

bool KateProjectModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    const auto index = this->index(row, column, parent);
    const auto type = (KateProjectItem::Type)index.data(KateProjectItem::TypeRole).toInt();
    const auto parentType = (KateProjectItem::Type)parent.data(KateProjectItem::TypeRole).toInt();
    QString pathToCopyTo;
    if (!index.isValid() && parent.isValid() && parentType == KateProjectItem::Directory) {
        pathToCopyTo = parent.data(Qt::UserRole).toString();
    } else if (index.isValid() && type == KateProjectItem::File) {
        if (index.parent().isValid()) {
            pathToCopyTo = index.parent().data(Qt::UserRole).toString();
        } else {
            pathToCopyTo = m_project->baseDir();
        }
    } else if (!index.isValid() && !parent.isValid()) {
        pathToCopyTo = m_project->baseDir();
    }

    const QDir d = pathToCopyTo;
    if (!d.exists()) {
        return false;
    }

    const auto urls = data->urls();
    const QString destDir = d.absolutePath();
    const QUrl dest = QUrl::fromLocalFile(destDir);
    QPointer<KIO::CopyJob> job = KIO::copy(urls, dest);
    KJobWidgets::setWindow(job, QApplication::activeWindow());
    connect(job, &KIO::Job::finished, this, [this, job, destDir] {
        if (!job || job->error() != 0 || !m_project)
            return;

        bool needsReload = false;
        QStandardItem *item = invisibleRootItem();
        if (destDir != m_project->baseDir()) {
            const auto indexes = match(this->index(0, 0), Qt::UserRole, destDir, 1, Qt::MatchStartsWith);
            if (indexes.empty()) {
                needsReload = true;
            } else {
                item = itemFromIndex(indexes.constFirst());
            }
        }

        const auto urls = job->srcUrls();
        if (!needsReload) {
            for (const auto &url : urls) {
                const QString newFile = destDir + QStringLiteral("/") + url.fileName();
                const QFileInfo fi(newFile);
                if (fi.exists() && fi.isFile()) {
                    KateProjectItem *i = new KateProjectItem(KateProjectItem::File, url.fileName());
                    item->appendRow(i);
                    m_project->addFile(newFile, i);
                } else {
                    // not a file? Just do a reload of the project on finish
                    needsReload = true;
                    break;
                }
            }
        }
        if (needsReload && m_project) {
            QMetaObject::invokeMethod(
                this,
                [this] {
                    m_project->reload(true);
                },
                Qt::QueuedConnection);
        }
    });
    job->start();

    return true;
}

bool KateProjectModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int, int, const QModelIndex &) const
{
    return data && data->hasUrls() && action == Qt::CopyAction;
}

KateProject::KateProject(QThreadPool &threadPool, KateProjectPlugin *plugin, const QString &fileName)
    : m_threadPool(threadPool)
    , m_plugin(plugin)
    , m_fileBacked(true)
    , m_fileName(QFileInfo(fileName).canonicalFilePath())
    , m_baseDir(QFileInfo(fileName).canonicalPath())
{
    // if canonicalFilePath already returned empty string, no need to try to load this
    if (m_fileName.isEmpty()) {
        return;
    }

    // ensure we get notified for project file changes
    connect(&m_plugin->fileWatcher(), &QFileSystemWatcher::fileChanged, this, &KateProject::slotFileChanged);
    m_plugin->fileWatcher().addPath(m_fileName);

    m_model.m_project = this;

    // try to load the project map from our file, will start worker thread, too
    reload();
}

KateProject::KateProject(QThreadPool &threadPool, KateProjectPlugin *plugin, const QVariantMap &globalProject, const QString &directory)
    : m_threadPool(threadPool)
    , m_plugin(plugin)
    , m_fileBacked(false)
    , m_fileName(QDir(QDir(directory).canonicalPath()).filePath(QStringLiteral(".kateproject")))
    , m_baseDir(QDir(directory).canonicalPath())
    , m_globalProject(globalProject)
{
    // try to load the project map, will start worker thread, too
    load(globalProject);
}

KateProject::~KateProject()
{
    saveNotesDocument();

    // stop watching if we have some real project file
    if (m_fileBacked && !m_fileName.isEmpty()) {
        m_plugin->fileWatcher().removePath(m_fileName);
    }
}

bool KateProject::reload(bool force)
{
    const QVariantMap map = readProjectFile();
    if (!map.isEmpty()) {
        m_globalProject = map;
    }

    return load(m_globalProject, force);
}

void KateProject::renameFile(const QString &newName, const QString &oldName)
{
    auto it = m_file2Item->find(oldName);
    if (it == m_file2Item->end()) {
        qWarning() << "renameFile() File not found, new: " << newName << "old: " << oldName;
        return;
    }
    (*m_file2Item)[newName] = it.value();
    m_file2Item->erase(it);
}

void KateProject::removeFile(const QString &file)
{
    auto it = m_file2Item->find(file);
    if (it == m_file2Item->end()) {
        qWarning() << "removeFile() File not found: " << file;
        return;
    }
    m_file2Item->erase(it);
}

/**
 * Read a JSON document from file.
 *
 * In case of an error, the returned object verifies isNull() is true.
 */
QJsonDocument KateProject::readJSONFile(const QString &fileName) const
{
    /**
     * keep each project file last modification time to warn the user only once per malformed file.
     */
    static QMap<QString, QDateTime> lastModifiedTimes;

    if (fileName.isEmpty()) {
        return QJsonDocument();
    }

    QFile file(fileName);
    if (!file.exists() || !file.open(QFile::ReadOnly)) {
        return QJsonDocument();
    }

    /**
     * parse the whole file, bail out again on error!
     */
    const QByteArray jsonData = file.readAll();
    QJsonParseError parseError{};
    QJsonDocument document(QJsonDocument::fromJson(jsonData, &parseError));

    if (parseError.error != QJsonParseError::NoError) {
        QDateTime lastModified = QFileInfo(fileName).lastModified();
        if (lastModified > lastModifiedTimes.value(fileName, QDateTime())) {
            lastModifiedTimes[fileName] = lastModified;
            m_plugin->sendMessage(i18n("Malformed JSON file '%1': %2", fileName, parseError.errorString()), true);
        }
        return QJsonDocument();
    }

    return document;
}

QVariantMap KateProject::readProjectFile() const
{
    // not file back => will not work
    if (!m_fileBacked) {
        return QVariantMap();
    }

    // bail out on error
    QJsonDocument project(readJSONFile(m_fileName));
    if (project.isNull()) {
        return QVariantMap();
    }

    /**
     * convenience; align with auto-generated projects
     * generate 'name' and 'files' if not specified explicitly,
     * so those parts need not be given if only wishes to specify additional
     * project configuration (e.g. build, ctags)
     */
    if (project.isObject()) {
        auto dir = QFileInfo(m_fileName).dir();
        auto object = project.object();

        // if there are local settings (.kateproject.local), override values
        {
            const auto localSettings = readJSONFile(projectLocalFileName(QStringLiteral("local")));
            if (!localSettings.isNull() && localSettings.isObject()) {
                object = json::merge(object, localSettings.object());
            }
        }

        auto name = object[QStringLiteral("name")];
        if (name.isUndefined() || name.isNull()) {
            name = dir.dirName();
        }
        auto files = object[QStringLiteral("files")];
        if (files.isUndefined() || files.isNull()) {
            // support all we can, could try to detect,
            // but it will be sorted out upon loading anyway
            QJsonArray afiles;
            for (const auto &t : {QStringLiteral("git"), QStringLiteral("hg"), QStringLiteral("svn"), QStringLiteral("darcs")}) {
                afiles.push_back(QJsonObject{{t, true}});
            }
            files = afiles;
        }
        project.setObject(object);
    }

    return project.toVariant().toMap();
}

bool KateProject::load(const QVariantMap &globalProject, bool force)
{
    /**
     * no name, bad => bail out
     */
    if (globalProject[QStringLiteral("name")].toString().isEmpty()) {
        return false;
    }

    /**
     * support out-of-source project files
     * ensure we handle relative paths properly => relative to the potential invented .kateproject file name
     */
    const auto baseDir = globalProject[QStringLiteral("directory")].toString();
    if (!baseDir.isEmpty()) {
        m_baseDir = QFileInfo(QFileInfo(m_fileName).dir(), baseDir).canonicalFilePath();
    }

    /**
     * anything changed?
     * else be done without forced reload!
     */
    if (!force && (m_projectMap == globalProject)) {
        return true;
    }

    /**
     * setup global attributes in this object
     */
    m_projectMap = globalProject;

    // emit that we changed stuff
    Q_EMIT projectMapChanged();

    // trigger loading of project in background thread
    QString indexDir;
    if (m_plugin->getIndexEnabled()) {
        indexDir = m_plugin->getIndexDirectory().toLocalFile();
        // if empty, use regular tempdir
        if (indexDir.isEmpty()) {
            indexDir = QDir::tempPath();
        }
    }

    // let's run the stuff in our own thread pool
    // do manual queued connect, as only run() is done in extra thread, object stays in this one
    auto w = new KateProjectWorker(m_baseDir, indexDir, m_projectMap, force);
    connect(w, &KateProjectWorker::loadDone, this, &KateProject::loadProjectDone, Qt::QueuedConnection);
    connect(w, &KateProjectWorker::loadIndexDone, this, &KateProject::loadIndexDone, Qt::QueuedConnection);
    m_threadPool.start(w);

    // we are done here
    return true;
}

void KateProject::loadProjectDone(const KateProjectSharedQStandardItem &topLevel, KateProjectSharedQHashStringItem file2Item)
{
    m_model.clear();
    m_model.invisibleRootItem()->appendColumn(topLevel->takeColumn(0));

    m_file2Item = std::move(file2Item);

    /**
     * readd the documents that are open atm
     */
    m_untrackedDocumentsRoot = nullptr;
    for (auto i = m_documents.constBegin(); i != m_documents.constEnd(); i++) {
        registerDocument(i.key());
    }

    Q_EMIT modelChanged();
}

void KateProject::loadIndexDone(KateProjectSharedProjectIndex projectIndex)
{
    /**
     * move to our project
     */
    m_projectIndex = std::move(projectIndex);

    /**
     * notify external world that data is available
     */
    Q_EMIT indexChanged();
}

QString KateProject::projectLocalFileName(const QString &suffix) const
{
    /**
     * nothing on empty file names for project
     * should not happen
     */
    if (m_baseDir.isEmpty() || suffix.isEmpty()) {
        return QString();
    }

    /**
     * compute full file name
     */
    return QDir(m_baseDir).filePath(QStringLiteral(".kateproject.") + suffix);
}

QTextDocument *KateProject::notesDocument()
{
    /**
     * already there?
     */
    if (m_notesDocument) {
        return m_notesDocument;
    }

    /**
     * else create it
     */
    m_notesDocument = new QTextDocument(this);
    m_notesDocument->setDocumentLayout(new QPlainTextDocumentLayout(m_notesDocument));

    /**
     * get file name
     */
    const QString notesFileName = projectLocalFileName(QStringLiteral("notes"));
    if (notesFileName.isEmpty()) {
        return m_notesDocument;
    }

    /**
     * and load text if possible
     */
    QFile inFile(notesFileName);
    if (inFile.open(QIODevice::ReadOnly)) {
        QTextStream inStream(&inFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        inStream.setCodec("UTF-8");
#endif
        m_notesDocument->setPlainText(inStream.readAll());
    }

    /**
     * and be done
     */
    return m_notesDocument;
}

void KateProject::saveNotesDocument()
{
    /**
     * no notes document, nothing to do
     */
    if (!m_notesDocument) {
        return;
    }

    /**
     * get content & filename
     */
    const QString content = m_notesDocument->toPlainText();
    const QString notesFileName = projectLocalFileName(QStringLiteral("notes"));
    if (notesFileName.isEmpty()) {
        return;
    }

    /**
     * no content => unlink file, if there
     */
    if (content.isEmpty()) {
        if (QFile::exists(notesFileName)) {
            QFile::remove(notesFileName);
        }
        return;
    }

    /**
     * else: save content to file
     */
    QFile outFile(projectLocalFileName(QStringLiteral("notes")));
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream outStream(&outFile);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        outStream.setCodec("UTF-8");
#endif
        outStream << content;
    }
}

void KateProject::slotModifiedChanged(KTextEditor::Document *document)
{
    KateProjectItem *item = itemForFile(m_documents.value(document));

    if (!item) {
        return;
    }

    item->slotModifiedChanged(document);
}

void KateProject::slotModifiedOnDisk(KTextEditor::Document *document, bool isModified, KTextEditor::ModificationInterface::ModifiedOnDiskReason reason)
{
    KateProjectItem *item = itemForFile(m_documents.value(document));

    if (!item) {
        return;
    }

    item->slotModifiedOnDisk(document, isModified, reason);
}

void KateProject::registerDocument(KTextEditor::Document *document)
{
    // remember the document, if not already there
    if (!m_documents.contains(document)) {
        m_documents[document] = document->url().toLocalFile();
    }

    // try to get item for the document
    KateProjectItem *item = itemForFile(document->url().toLocalFile());

    // if we got one, we are done, else create a dummy!
    // clang-format off
    if (item) {
        disconnect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
        disconnect(document,
                   SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                   this,
                   SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
        item->slotModifiedChanged(document);

        /*FIXME    item->slotModifiedOnDisk(document,document->isModified(),qobject_cast<KTextEditor::ModificationInterface*>(document)->modifiedOnDisk());
         * FIXME*/

        connect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
        connect(document,
                SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
                this,
                SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));

        return;
    }
    // clang-format on

    registerUntrackedDocument(document);
}

void KateProject::registerUntrackedDocument(KTextEditor::Document *document)
{
    // perhaps create the parent item
    if (!m_untrackedDocumentsRoot) {
        m_untrackedDocumentsRoot = new KateProjectItem(KateProjectItem::Directory, i18n("<untracked>"));
        m_model.insertRow(0, m_untrackedDocumentsRoot);
    }

    // create document item
    QFileInfo fileInfo(document->url().toLocalFile());
    KateProjectItem *fileItem = new KateProjectItem(KateProjectItem::File, fileInfo.fileName());
    fileItem->slotModifiedChanged(document);
    connect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);
    // clang-format off
    connect(document,
            SIGNAL(modifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)),
            this,
            SLOT(slotModifiedOnDisk(KTextEditor::Document*,bool,KTextEditor::ModificationInterface::ModifiedOnDiskReason)));
    // clang-format on

    bool inserted = false;
    for (int i = 0; i < m_untrackedDocumentsRoot->rowCount(); ++i) {
        if (m_untrackedDocumentsRoot->child(i)->data(Qt::UserRole).toString() > document->url().toLocalFile()) {
            m_untrackedDocumentsRoot->insertRow(i, fileItem);
            inserted = true;
            break;
        }
    }
    if (!inserted) {
        m_untrackedDocumentsRoot->appendRow(fileItem);
    }

    fileItem->setData(document->url().toLocalFile(), Qt::UserRole);
    fileItem->setData(QVariant(true), Qt::UserRole + 3);

    if (!m_file2Item) {
        m_file2Item = KateProjectSharedQHashStringItem(new QHash<QString, KateProjectItem *>());
    }
    (*m_file2Item)[document->url().toLocalFile()] = fileItem;
}

void KateProject::unregisterDocument(KTextEditor::Document *document)
{
    if (!m_documents.contains(document)) {
        return;
    }

    disconnect(document, &KTextEditor::Document::modifiedChanged, this, &KateProject::slotModifiedChanged);

    const QString &file = m_documents.value(document);

    if (m_untrackedDocumentsRoot) {
        KateProjectItem *item = static_cast<KateProjectItem *>(itemForFile(file));
        if (item && item->data(Qt::UserRole + 3).toBool()) {
            unregisterUntrackedItem(item);
            m_file2Item->remove(file);
        }
    }

    m_documents.remove(document);
}

void KateProject::unregisterUntrackedItem(const KateProjectItem *item)
{
    for (int i = 0; i < m_untrackedDocumentsRoot->rowCount(); ++i) {
        if (m_untrackedDocumentsRoot->child(i) == item) {
            m_untrackedDocumentsRoot->removeRow(i);
            break;
        }
    }

    if (m_untrackedDocumentsRoot->rowCount() < 1) {
        m_model.removeRow(0);
        m_untrackedDocumentsRoot = nullptr;
    }
}

void KateProject::slotFileChanged(const QString &file)
{
    if (file == m_fileName) {
        reload();
    }
}
