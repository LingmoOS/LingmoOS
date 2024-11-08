/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filepropertiesdialog.h"
#include "../desktopiconprovider.h"
#include "../helper/fm.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QFileInfo>
#include <QDir>

#include <KIO/CopyJob>

inline QString concatPaths(const QString &path1, const QString &path2)
{
    Q_ASSERT(!path2.startsWith(QLatin1Char('/')));

    if (path1.isEmpty()) {
        return path2;
    } else if (!path1.endsWith(QLatin1Char('/'))) {
        return path1 + QLatin1Char('/') + path2;
    } else {
        return path1 + path2;
    }
}

FilePropertiesDialog::FilePropertiesDialog(const KFileItem &item, QQuickView *parent)
    : QQuickView(parent)
{
    m_items.append(item);
    init();
}

FilePropertiesDialog::FilePropertiesDialog(const KFileItemList &items, QQuickView *parent)
    : QQuickView(parent)
    , m_items(items)
{
    init();
}

FilePropertiesDialog::FilePropertiesDialog(const QUrl &url, QQuickView *parent)
    : QQuickView(parent)
{
    m_items.append(KFileItem(url));

    init();
}

FilePropertiesDialog::~FilePropertiesDialog()
{
    if (m_sizeJob) {
        m_sizeJob->stop();
        m_sizeJob->deleteLater();
        m_sizeJob = nullptr;
    }
}

void FilePropertiesDialog::updateSize(int width, int height)
{
    resize(QSize(width, height));
    setMinimumSize(QSize(width, height));
    setMaximumSize(QSize(width, height));
}

void FilePropertiesDialog::accept(const QString &text)
{
    KFileItemList list = m_items;

    if (list.size() == 1) {
        KFileItem item = list.first();

        QString n = text;
        while (!n.isEmpty() && n[n.length() - 1].isSpace())
            n.chop(1);

        if (n.isEmpty())
            return;

        QString newFileName = KIO::encodeFileName(n);

        if (fileName() != newFileName) {
            QUrl newUrl;

            if (!location().isEmpty() && !Fm::isFixedFolder(m_items.first().url())) {
                newUrl = location();
                newUrl.setPath(concatPaths(newUrl.path(), newFileName));
                newUrl.setScheme(item.url().scheme());

                auto job = KIO::move(item.url(), newUrl, KIO::HideProgressInfo);
                job->start();
            }
        }
    }

    this->destroy();
    this->deleteLater();
}

void FilePropertiesDialog::reject()
{
    if (m_sizeJob) {
        m_sizeJob->stop();
        m_sizeJob->deleteLater();
        m_sizeJob = nullptr;
    }

    this->destroy();
    this->deleteLater();
}

bool FilePropertiesDialog::multiple() const
{
    return m_multiple;
}

bool FilePropertiesDialog::isWritable() const
{
    return m_isWritable;
}

QString FilePropertiesDialog::location() const
{
    return m_location;
}

QString FilePropertiesDialog::fileName() const
{
    return m_fileName;
}

QString FilePropertiesDialog::iconName() const
{
    return m_iconName;
}

QString FilePropertiesDialog::mimeType() const
{
    return m_mimeType;
}

QString FilePropertiesDialog::fileSize() const
{
    return m_size;
}

QString FilePropertiesDialog::creationTime() const
{
    return m_creationTime;
}

QString FilePropertiesDialog::modifiedTime() const
{
    return m_modifiedTime;
}

QString FilePropertiesDialog::accessedTime() const
{
    return m_accessedTime;
}

bool FilePropertiesDialog::event(QEvent *e)
{
    if (e->type() == QEvent::Close) {
        this->deleteLater();
    }

    return QQuickView::event(e);
}

void FilePropertiesDialog::init()
{
    engine()->rootContext()->setContextProperty("main", this);
    // engine()->addImageProvider(QStringLiteral("icontheme"), new DesktopIconProvider());

    setFlag(Qt::Dialog);
    setTitle(tr("Properties"));
    setResizeMode(QQuickView::SizeViewToRootObject);
    setSource(QUrl("qrc:/qml/Dialogs/PropertiesDialog.qml"));

    m_multiple = m_items.count() > 1;

    QList<QUrl> list;
    for (KFileItem item : m_items) {
        list.append(item.url());
    }

    m_sizeJob = std::shared_ptr<CFileSizeJob>(new CFileSizeJob);
    m_sizeJob->start(list);

    connect(m_sizeJob.get(), &CFileSizeJob::sizeChanged, this, &FilePropertiesDialog::updateTotalSize);
    connect(m_sizeJob.get(), &CFileSizeJob::result, this, &FilePropertiesDialog::updateTotalSize);

    if (!m_multiple) {
        KFileItem item = m_items.first();
        QFileInfo info(item.url().toLocalFile());

        QString path;
        m_fileName = m_items.first().name();

        if (item.isDir())
            m_iconName = "folder";
        else
            m_iconName = m_items.first().iconName();

        m_mimeType = m_items.first().mimetype();
        m_size = KIO::convertSize(m_items.first().size());
        m_location = info.dir().path();

        m_creationTime = info.birthTime().toString(Qt::SystemLocaleLongDate);
        m_modifiedTime = info.lastModified().toString(Qt::SystemLocaleLongDate);
        m_accessedTime = info.lastRead().toString(Qt::SystemLocaleLongDate);

//        m_creationTime = item.time(KFileItem::CreationTime).toString();
//        m_modifiedTime = item.time(KFileItem::ModificationTime).toString();
//        m_accessedTime = item.time(KFileItem::AccessTime).toString();

        m_isWritable = m_items.first().isWritable();

        emit fileNameChanged();
        emit iconNameChanged();
        emit mimeTypeChanged();
        emit fileSizeChanged();
        emit locationChanged();
        emit creationTimeChanged();
        emit modifiedTimeChanged();
        emit accessedTimeChanged();
    } else {
        m_isWritable = false;
        m_fileName = tr("%1 files").arg(m_items.count());
        m_location = QFileInfo(m_items.first().localPath()).dir().path();
        m_iconName = "unknown";

        emit fileNameChanged();
        emit locationChanged();
        emit iconNameChanged();
    }

    emit isWritableChanged();
}

void FilePropertiesDialog::updateTotalSize()
{
    if (!m_sizeJob)
        return;

    m_size = KIO::convertSize(m_sizeJob->totalSize());
    emit fileSizeChanged();
}
