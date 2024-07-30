/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2009, 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "templateloader.h"

#include "engine.h"
#include "exception.h"
#include "nulllocalizer_p.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

using namespace KTextTemplate;

AbstractTemplateLoader::~AbstractTemplateLoader() = default;

namespace KTextTemplate
{
class FileSystemTemplateLoaderPrivate
{
    FileSystemTemplateLoaderPrivate(FileSystemTemplateLoader *loader, QSharedPointer<AbstractLocalizer> localizer)
        : q_ptr(loader)
        , m_localizer(localizer ? localizer : QSharedPointer<AbstractLocalizer>(new NullLocalizer))
    {
    }
    Q_DECLARE_PUBLIC(FileSystemTemplateLoader)
    FileSystemTemplateLoader *const q_ptr;

    QString m_themeName;
    QStringList m_templateDirs;
    const QSharedPointer<AbstractLocalizer> m_localizer;
};
}

FileSystemTemplateLoader::FileSystemTemplateLoader(const QSharedPointer<AbstractLocalizer> localizer)
    : AbstractTemplateLoader()
    , d_ptr(new FileSystemTemplateLoaderPrivate(this, localizer))
{
}

FileSystemTemplateLoader::~FileSystemTemplateLoader()
{
    for (const QString &dir : templateDirs())
        d_ptr->m_localizer->unloadCatalog(dir + QLatin1Char('/') + themeName());
    delete d_ptr;
}

InMemoryTemplateLoader::InMemoryTemplateLoader()
    : AbstractTemplateLoader()
{
}

InMemoryTemplateLoader::~InMemoryTemplateLoader() = default;

void FileSystemTemplateLoader::setTheme(const QString &themeName)
{
    Q_D(FileSystemTemplateLoader);
    for (const QString &dir : templateDirs())
        d->m_localizer->unloadCatalog(dir + QLatin1Char('/') + d->m_themeName);
    d->m_themeName = themeName;
    for (const QString &dir : templateDirs())
        d->m_localizer->loadCatalog(dir + QLatin1Char('/') + themeName, themeName);
}

QString FileSystemTemplateLoader::themeName() const
{
    Q_D(const FileSystemTemplateLoader);
    return d->m_themeName;
}

void FileSystemTemplateLoader::setTemplateDirs(const QStringList &dirs)
{
    Q_D(FileSystemTemplateLoader);

    for (const QString &dir : templateDirs())
        d->m_localizer->unloadCatalog(dir + QLatin1Char('/') + d->m_themeName);
    d->m_templateDirs = dirs;
    for (const QString &dir : templateDirs())
        d->m_localizer->loadCatalog(dir + QLatin1Char('/') + d->m_themeName, d->m_themeName);
}

QStringList FileSystemTemplateLoader::templateDirs() const
{
    Q_D(const FileSystemTemplateLoader);
    return d->m_templateDirs;
}

bool FileSystemTemplateLoader::canLoadTemplate(const QString &name) const
{
    Q_D(const FileSystemTemplateLoader);
    auto i = 0;
    QFile file;

    while (!file.exists()) {
        if (i >= d->m_templateDirs.size())
            break;

        file.setFileName(d->m_templateDirs.at(i) + QLatin1Char('/') + d->m_themeName + QLatin1Char('/') + name);
        ++i;
    }

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    file.close();
    return true;
}

Template FileSystemTemplateLoader::loadByName(const QString &fileName, Engine const *engine) const
{
    Q_D(const FileSystemTemplateLoader);
    auto i = 0;
    QFile file;

    while (!file.exists()) {
        if (i >= d->m_templateDirs.size())
            break;

        file.setFileName(d->m_templateDirs.at(i) + QLatin1Char('/') + d->m_themeName + QLatin1Char('/') + fileName);
        const QFileInfo fi(file);

        if (file.exists() && !fi.canonicalFilePath().contains(QDir(d->m_templateDirs.at(i)).canonicalPath()))
            return {};
        ++i;
    }

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream fstream(&file);
    fstream.setEncoding(QStringConverter::Utf8);
    const auto fileContent = fstream.readAll();

    return engine->newTemplate(fileContent, fileName);
}

std::pair<QString, QString> FileSystemTemplateLoader::getMediaUri(const QString &fileName) const
{
    Q_D(const FileSystemTemplateLoader);
    auto i = 0;
    QFile file;
    while (!file.exists()) {
        if (i >= d->m_templateDirs.size())
            break;

        file.setFileName(d->m_templateDirs.at(i) + QLatin1Char('/') + d->m_themeName + QLatin1Char('/') + fileName);

        const QFileInfo fi(file);
        if (!fi.canonicalFilePath().contains(QDir(d->m_templateDirs.at(i)).canonicalPath())) {
            ++i;
            continue;
        }

        if (file.exists()) {
            auto path = fi.absoluteFilePath();
            path.chop(fileName.size());
            return std::make_pair(path, fileName);
        }
        ++i;
    }
    return {};
}

void InMemoryTemplateLoader::setTemplate(const QString &name, const QString &content)
{
    m_namedTemplates.insert(name, content);
}

bool InMemoryTemplateLoader::canLoadTemplate(const QString &name) const
{
    return m_namedTemplates.contains(name);
}

Template InMemoryTemplateLoader::loadByName(const QString &name, Engine const *engine) const
{
    const auto it = m_namedTemplates.constFind(name);
    if (it != m_namedTemplates.constEnd()) {
        return engine->newTemplate(it.value(), name);
    }
    throw KTextTemplate::Exception(TagSyntaxError, QStringLiteral("Couldn't load template %1. Template does not exist.").arg(name));
}

std::pair<QString, QString> InMemoryTemplateLoader::getMediaUri(const QString &fileName) const
{
    Q_UNUSED(fileName)
    // This loader doesn't make any media available yet.
    return {};
}
