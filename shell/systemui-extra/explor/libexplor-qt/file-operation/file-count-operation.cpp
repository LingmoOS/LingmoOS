/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file-count-operation.h"

#include "file-node-reporter.h"
#include "file-node.h"

using namespace Peony;

FileCountOperation::FileCountOperation(const QStringList &uris, bool countRoot, QObject *parent)
    : FileOperation (parent)
{
    m_count_root = countRoot;
    m_reporter = new FileNodeReporter(this);
    //connect(m_reporter, &FileNodeReporter::nodeFound, this, &FileOperation::operationPreparedOne);
    connect(m_reporter, &FileNodeReporter::nodeFound, [=](const QString &uri, quint64 size) {
        m_file_count++;
        if (uri.contains("/.")) {
            m_hidden_file_count++;
        }
        m_total_size += size;
        Q_EMIT this->operationPreparedOne(uri, size);
    });
    m_uris = uris;
}

FileCountOperation::~FileCountOperation()
{

}

void FileCountOperation::cancel()
{
    FileOperation::cancel();
    m_reporter->cancel();
}

void FileCountOperation::run()
{
    Q_EMIT operationStarted();
    if (m_uris.isEmpty())
        Q_EMIT operationFinished();

    QList<FileNode *> nodes;
    for (auto uri : m_uris) {
        m_reporter->setUsedInCount(true);
        auto node = new FileNode(FileUtils::urlEncode(uri), nullptr, m_reporter);
        node->findChildrenRecursively();
        nodes<<node;
    }
    if (!this->isCancelled()) {
        if (!m_count_root) {
            for (auto node : nodes) {
                m_file_count--;
                if (node->baseName().startsWith(".")) {
                    m_hidden_file_count--;
                }
                //m_total_size -= node->size();
            }
        }
        Q_EMIT countDone(m_file_count, m_hidden_file_count, m_total_size);
    }
    qDebug()<<m_file_count<<m_hidden_file_count<<m_total_size;
    Q_EMIT operationPrepared();
    Q_EMIT operationFinished();
    for (auto node : nodes) {
        delete node;
    }
}
