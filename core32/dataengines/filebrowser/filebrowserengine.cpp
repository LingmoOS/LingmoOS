/*
    SPDX-FileCopyrightText: 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filebrowserengine.h"

#include <Plasma/DataContainer>

#include <KDirWatch>
#include <QDebug>
#include <QDir>

#define InvalidIfEmpty(A) ((A.isEmpty()) ? (QVariant()) : (QVariant(A)))
#define forMatchingSources                                                                                                                                     \
    for (DataEngine::SourceDict::iterator it = sources.begin(); it != sources.end(); ++it)                                                                     \
        if (dir == QDir(it.key()))

FileBrowserEngine::FileBrowserEngine(QObject *parent, const QVariantList &args)
    : Plasma::DataEngine(parent, args)
    , m_dirWatch(new KDirWatch(this))
{
    Q_UNUSED(args)

    connect(m_dirWatch, &KDirWatch::created, this, &FileBrowserEngine::dirCreated);
    connect(m_dirWatch, &KDirWatch::deleted, this, &FileBrowserEngine::dirDeleted);
    connect(m_dirWatch, &KDirWatch::dirty, this, &FileBrowserEngine::dirDirty);
}

FileBrowserEngine::~FileBrowserEngine()
{
    delete m_dirWatch;
}

void FileBrowserEngine::init()
{
    qDebug() << "init() called";
}

bool FileBrowserEngine::sourceRequestEvent(const QString &path)
{
    qDebug() << "source requested() called: " << path;
    m_dirWatch->addDir(path);
    setData(path, QStringLiteral("type"), QVariant("unknown"));
    updateData(path, INIT);
    return true;
}

void FileBrowserEngine::dirDirty(const QString &path)
{
    updateData(path, DIRTY);
}

void FileBrowserEngine::dirCreated(const QString &path)
{
    updateData(path, CREATED);
}

void FileBrowserEngine::dirDeleted(const QString &path)
{
    updateData(path, DELETED);
}

void FileBrowserEngine::updateData(const QString &path, EventType event)
{
    Q_UNUSED(event)

    ObjectType type = NOTHING;
    if (QDir(path).exists()) {
        type = DIRECTORY;
    } else if (QFile::exists(path)) {
        type = FILE;
    }

    DataEngine::SourceDict sources = containerDict();

    QDir dir(path);
    clearData(path);

    if (type == DIRECTORY) {
        qDebug() << "directory info processing: " << path;
        if (dir.isReadable()) {
            const QStringList visibleFiles = dir.entryList(QDir::Files, QDir::Name);
            const QStringList allFiles = dir.entryList(QDir::Files | QDir::Hidden, QDir::Name);

            const QStringList visibleDirectories = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
            const QStringList allDirectories = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden, QDir::Name);

            forMatchingSources
            {
                qDebug() << "MATCH";
                it.value()->setData(QStringLiteral("item.type"), QVariant("directory"));

                QVariant vdTmp;
                if (!visibleDirectories.isEmpty())
                    vdTmp = QVariant(visibleDirectories);
                it.value()->setData(QStringLiteral("directories.visible"), vdTmp);

                QVariant adTmp;
                if (!allDirectories.empty())
                    adTmp = QVariant(allDirectories);
                it.value()->setData(QStringLiteral("directories.all"), adTmp);

                QVariant vfTmp;
                if (!visibleFiles.empty())
                    vfTmp = QVariant(visibleFiles);
                it.value()->setData(QStringLiteral("files.visible"), vfTmp);

                QVariant afTmp;
                if (!allFiles.empty())
                    afTmp = QVariant(allFiles);
                it.value()->setData(QStringLiteral("files.all"), afTmp);
            }
        }
    } else if (type == FILE) {
        qDebug() << "file info processing: " << path;
        forMatchingSources
        {
            it.value()->setData(QStringLiteral("item.type"), QVariant("file"));
        }
    } else {
        forMatchingSources
        {
            it.value()->setData(QStringLiteral("item.type"), QVariant("imaginary"));
        }
    };
}

void FileBrowserEngine::clearData(const QString &path)
{
    QDir dir(path);
    const DataEngine::SourceDict sources = containerDict();
    for (DataEngine::SourceDict::const_iterator it = sources.begin(); it != sources.end(); ++it) {
        if (dir == QDir(it.key())) {
            qDebug() << "matched: " << path << " " << it.key();
            it.value()->removeAllData();

        } else {
            qDebug() << "didn't match: " << path << " " << it.key();
        }
    }
}

K_PLUGIN_CLASS_WITH_JSON(FileBrowserEngine, "lingmo-dataengine-filebrowser.json")

#include "filebrowserengine.moc"
