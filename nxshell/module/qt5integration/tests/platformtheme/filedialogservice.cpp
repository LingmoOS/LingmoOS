/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "filedialogservice.h"
#include <QUrl>

FileDialogService::FileDialogService(const QString &path)
    : m_objPath{path}
    , m_online{false}
    , m_accepted{false}
    , m_rejected{false}
    , m_visible{false}
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    m_online = conn.registerObject(m_objPath, this, QDBusConnection::ExportAllContents);
    connect(this, &QObject::destroyed, this, &FileDialogService::destroyed);
}

FileDialogService::~FileDialogService()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    conn.unregisterObject(m_objPath);
    m_online = false;
}

void FileDialogService::accept()
{
    m_accepted = true;
    Q_EMIT this->accepted();
}

void FileDialogService::activateWindow()
{
}

void FileDialogService::addCustomWidget(int type, const QString & data)
{
    Q_UNUSED(type)
    Q_UNUSED(data)
}

void FileDialogService::addDisableUrlScheme(const QString & type)
{
    Q_UNUSED(type)
}

QMap<QString,QVariant> FileDialogService::allCustomWidgetsValue(int type)
{
    Q_UNUSED(type)
    return QMap<QString,QVariant>();
}

void FileDialogService::beginAddCustomWidget()
{
}

void FileDialogService::endAddCustomWidget()
{
}

QDBusVariant FileDialogService::getCustomWidgetValue(int type, const QString & text)
{
    Q_UNUSED(type)
    Q_UNUSED(text)
    return QDBusVariant();
}

void FileDialogService::hide()
{
    m_visible = false;
}

QString FileDialogService::labelText(int label)
{
    Q_UNUSED(label)
    return QString();
}

void FileDialogService::makeHeartbeat()
{
}

void FileDialogService::open()
{
}

int FileDialogService::options()
{
    return 0;
}

void FileDialogService::reject()
{
    m_rejected = true;
    Q_EMIT this->rejected();
}

void FileDialogService::selectFile(const QString & filename)
{
    Q_UNUSED(filename)
}

void FileDialogService::selectNameFilter(const QString & filter)
{
    m_nameFilters.append(filter);
}

void FileDialogService::selectNameFilterByIndex(int index)
{
    Q_UNUSED(index)
}

void FileDialogService::selectUrl(const QString & url)
{
    m_selectedFiles.append(url);
}

QStringList FileDialogService::selectedFiles()
{
    QStringList files;
    for (auto fileUrl : m_selectedFiles) {
        files.append(fileUrl.toString());
    }
    return files;
}

QString FileDialogService::selectedNameFilter()
{
    return m_nameFilters.join(';');
}

int FileDialogService::selectedNameFilterIndex()
{
    return 0;
}

QStringList FileDialogService::selectedUrls()
{
    QStringList files;
    for (auto fileUrl : m_selectedFiles) {
        files.append(fileUrl.toString());
    }
    return files;
}

void FileDialogService::setAllowMixedSelection(bool on)
{
    Q_UNUSED(on)
}

void FileDialogService::setCurrentInputName(const QString & name)
{
    Q_UNUSED(name)
}

void FileDialogService::setFileMode(int fileMode)
{
    Q_UNUSED(fileMode)
}

void FileDialogService::setLabelText(int label, const QString & text)
{
    Q_UNUSED(label)
    Q_UNUSED(text)
}

void FileDialogService::setOption(int option, bool on)
{
    Q_UNUSED(option)
    Q_UNUSED(on)
}

void FileDialogService::setOptions(int options)
{
    Q_UNUSED(options)
}

void FileDialogService::setWindowTitle(const QString & title)
{
    Q_UNUSED(title)
}

void FileDialogService::show()
{
    m_visible = true;
}

bool FileDialogService::testOption(int option)
{
    Q_UNUSED(option)
    return false;
}

qulonglong FileDialogService::winId()
{
    return qulonglong();
}
