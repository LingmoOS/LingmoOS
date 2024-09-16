// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dvirtualimagefileengine.h"
#include "dvirtualimagefileio.h"

#include <QDebug>

QAbstractFileEngine *DVirtualImageFileEngineHandler::create(const QString &fileName) const
{
    if (!fileName.startsWith("dim://"))
        return 0;

    QFileInfo file_info(fileName.mid(6));

    DVirtualImageFileIO *info = new DVirtualImageFileIO(file_info.suffix() == "dim"
                                                        ? file_info.absoluteFilePath()
                                                        : file_info.absolutePath());

    if (!info->isValid()) {
        delete info;

        return 0;
    }

    return new DVirtualImageFileEngine(info, (fileName.endsWith("/") || file_info.suffix() == "dim") ? "/" : file_info.fileName());
}

static DVirtualImageFileEngineHandler _global_dim_helper;

DVirtualImageFileEngine::DVirtualImageFileEngine(DVirtualImageFileIO *info, const QString &fileName)
    : m_info(info)
    , m_name(fileName)
{

}

DVirtualImageFileEngine::~DVirtualImageFileEngine()
{
    close();
    delete m_info;
}

bool DVirtualImageFileEngine::open(QIODevice::OpenMode openMode)
{
    return m_info->open(m_name, openMode);
}

bool DVirtualImageFileEngine::close()
{
    return m_info->close();
}

bool DVirtualImageFileEngine::flush()
{
    return m_info->flush();
}

qint64 DVirtualImageFileEngine::size() const
{
    return m_info->size(m_name);
}

qint64 DVirtualImageFileEngine::pos() const
{
    return m_info->pos();
}

bool DVirtualImageFileEngine::seek(qint64 pos)
{
    return m_info->seek(pos);
}

bool DVirtualImageFileEngine::isSequential() const
{
    return m_info->isSequential();
}

bool DVirtualImageFileEngine::setSize(qint64 size)
{
    return m_info->setSize(m_name, size);
}

QAbstractFileEngine::FileFlags DVirtualImageFileEngine::fileFlags(QAbstractFileEngine::FileFlags type) const
{
    FileFlags flags;

    if (type & TypesMask) {
        if (m_name == "/")
            flags |= DirectoryType;
        else
            flags |= FileType;
    }

    if ((type & FlagsMask)) {
        if (m_name == "/") {
            if (m_info->isValid())
                flags |= ExistsFlag;
        } else if (m_info->existes(m_name)) {
            flags |= ExistsFlag;
        }
    }

    if ((type & PermsMask) && (m_name == "/" ? m_info->isValid() : m_info->existes(m_name))) {
        flags |= (FileFlags)(int)m_info->permissions();

        if (m_name == "/" || !m_info->isWritable(m_name)) {
            flags &= ~(WriteGroupPerm | WriteOtherPerm | WriteOwnerPerm | WriteUserPerm);
        }
    }

    return flags;
}

QString DVirtualImageFileEngine::fileName(QAbstractFileEngine::FileName file) const
{
    Q_UNUSED(file)

    QFileInfo info(m_info->fileName() + QDir::separator() + (m_name == "/" ? QString() : m_name));

    switch (file) {
    case AbsoluteName:
    case CanonicalName:
    case DefaultName:
        return "dim://" + info.absoluteFilePath();
    case AbsolutePathName:
        return "dim://" + (m_name == "/" ? info.absoluteFilePath() : info.absolutePath());
    case BaseName:
        return m_name;
    case BundleName:
        return info.absoluteFilePath();
    default:
        break;
    }

    return QString();
}

void DVirtualImageFileEngine::setFileName(const QString &file)
{
    m_info->rename(m_name, file);
}

DVirtualImageFileEngine::Iterator *DVirtualImageFileEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    if (m_name != "/" || !filters.testFlag(QDir::Files))
        return 0;

    m_fileList.clear();

    auto filter = [&] (const QString &name) {
        for (const QString &rule : filterNames) {
            QRegExp rg(rule, Qt::CaseSensitive, QRegExp::Wildcard);

            if (rg.exactMatch(name))
                return true;
        }

        return false;
    };

    for (const QString &file : m_info->fileList()) {
        if (filterNames.isEmpty() || filter(file))
            m_fileList.append(file);
    }

    if (m_fileList.isEmpty())
        return 0;

    DVirtualImageFileIterator *iterator = new DVirtualImageFileIterator(filters, filterNames);

    iterator->list = m_fileList;

    return iterator;
}

DVirtualImageFileEngine::Iterator *DVirtualImageFileEngine::endEntryList()
{
    return 0;
}

qint64 DVirtualImageFileEngine::read(char *data, qint64 maxlen)
{
    if (!m_info->isOpen(m_name))
        return -1;

    return m_info->read(data, maxlen);
}

qint64 DVirtualImageFileEngine::write(const char *data, qint64 len)
{
    if (!m_info->isOpen(m_name))
        return -1;

    return m_info->write(data, len);
}

bool DVirtualImageFileEngine::extension(QAbstractFileEngine::Extension extension, const QAbstractFileEngine::ExtensionOption *option, QAbstractFileEngine::ExtensionReturn *output)
{
    Q_UNUSED(option)
    Q_UNUSED(output)

    return extension == AtEndExtension && (m_info->pos() < 0 || m_info->pos() == m_info->size(m_name));
}

bool DVirtualImageFileEngine::supportsExtension(QAbstractFileEngine::Extension extension) const
{
    return extension == AtEndExtension;
}

DVirtualImageFileIterator::DVirtualImageFileIterator(QDir::Filters filters, const QStringList &nameFilters)
    : QAbstractFileEngineIterator(filters, nameFilters)
{

}

QString DVirtualImageFileIterator::next()
{
    return list.at(++index);
}

bool DVirtualImageFileIterator::hasNext() const
{
    return index < list.count() - 1;
}

QString DVirtualImageFileIterator::currentFileName() const
{
    return list.at(index);
}
